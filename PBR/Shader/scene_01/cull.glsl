#version 460 core
// Takahiro Harada et al. 2015, Forward+: 将延迟光照提升到新的水平
// 参考链接:
// https://takahiroharada.files.wordpress.com/2015/04/forward_plus.pdf
// https://www.3dgep.com/forward-plus/
// https://github.com/bcrusco/Forward-Plus-Renderer
// https://wickedengine.net/2018/01/10/optimizing-tile-based-light-culling/

layout(std140, binding = 0) uniform Camera {
    vec4 position;     // 相机位置
    vec4 direction;    // 相机方向
    mat4 view;         // 视图矩阵
    mat4 projection;   // 投影矩阵
} camera;

#include "../core/renderer_input.glsl"

////////////////////////////////////////////////////////////////////////////////

#ifdef compute_shader

#define TILE_SIZE 16  // 瓦片大小
#define MAX_LIGHT 1024  // 最大光源数

layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;

layout(std430, binding = 0) readonly buffer Color    { vec4  pl_color[];    };  // 光源颜色
layout(std430, binding = 1) readonly buffer Position { vec4  pl_position[]; };  // 光源位置
layout(std430, binding = 2) readonly buffer Range    { float pl_range[];    };  // 光源范围
layout(std430, binding = 3) writeonly buffer Index   { int   pl_index[];    };  // 可见光源索引

layout(location = 0) uniform uint n_lights;             // 光源数量
layout(location = 1) uniform mat4 inverse_projection;   // 投影矩阵的逆矩阵
layout(binding = 0) uniform sampler2D depth_texture;    // 深度纹理

// 当前瓦片内的共享局部存储（本地工作组内）
shared uint min_depth;             // 最小深度
shared uint max_depth;             // 最大深度
shared uint n_visible_lights;      // 可见光源数
shared vec4 tile_corner[4];        // 瓦片的四个角点
shared vec4 tile_frustum[4];       // 瓦片的视锥体截面
shared int local_indices[MAX_LIGHT];  // 当前瓦片内可见光源的索引数组

// 将屏幕空间中的深度线性化到视图空间中的 [near, far] 范围内
float LinearizeDepth(float depth) {
    float near = rdr_in.near_clip;   // 近裁剪面
    float far = rdr_in.far_clip;     // 远裁剪面
    float ndc_depth = depth * 2.0 - 1.0;  // 转换回NDC空间
    return (2.0 * near * far) / (far + near - ndc_depth * (far - near));
}

// 将一个在NDC空间中的顶点转换回视图/相机空间
vec4 NDC2View(const vec4 v) {
    vec4 u = inverse_projection * v;  // 使用逆投影矩阵将顶点从NDC空间转换为视图空间
    u /= u.w;  // 执行透视除法
    return u;
}

void main() {
    // 步骤 0: 使用第一个线程初始化共享的局部数据
    if (gl_LocalInvocationIndex == 0) {
        min_depth = 0xFFFFFFFF;  // 无符号整数的最大值
        max_depth = 0;
        n_visible_lights = 0;
    }

    barrier();  // 同步所有本地线程组中的共享数据

    // 步骤 1: 找到当前瓦片（当前工作组）的最小/最大深度值
    vec2 uv = vec2(gl_GlobalInvocationID.xy) / rdr_in.resolution;
    float depth = LinearizeDepth(texture(depth_texture, uv).r);

    // 原子操作比较本地线程中的深度值并更新瓦片中的最小/最大深度
    uint depth_uint = floatBitsToUint(depth);  // 转换为uint以便使用原子操作
    atomicMin(min_depth, depth_uint);
    atomicMax(max_depth, depth_uint);

    // 特殊情况处理：如果像素超出屏幕范围，我们可以将最小深度设置为最大值以剔除它
    // 这将使视锥体的远处无限远，确保与所有其他对象的交集都将失败
    if (uv.x > 1.0 || uv.y > 1.0) {
        min_depth = 0x00000FFF;  // 当后续转换为浮点数时，0xFFFFFFFF将导致溢出
    }

    barrier();  // 同步本地线程

    // 将最小/最大深度值转换回浮点数
    float min_depth_float = uintBitsToFloat(min_depth);
    float max_depth_float = uintBitsToFloat(max_depth);

    /* 步骤 2: 使用第一个线程计算该瓦片的视锥体平面

       视锥体有1个近裁剪面、1个远裁剪面和4个侧面
       为了测试光源和平面的交集，我们只需要计算4个侧面的平面
       数学上，每个平面由一个法向量和到原点的距离唯一定义
       对于侧面平面，法向量就是通过两个底部顶点v1和v2计算得到的，因为相机位于视图空间的原点(0,0,0)
       所以v1和v2也是平面的边界向量，因此法向量就是cross(v1, v2)的结果

       注意，这是最简单的用于测试球体与视锥体交集的解决方案，在某些边界情况下，光源可能无法正确剔除，
       因为我们假设了无限大的视锥体平面。一个更优化的解决方案是使用AABB包围盒进行交集测试：
       https://wickedengine.net/2018/01/10/optimizing-tile-based-light-culling/
    */

    if (gl_LocalInvocationIndex == 0) {
        // 计算屏幕空间中瓦片的四个角点
        vec2 lower = vec2(gl_WorkGroupID.xy) / vec2(gl_NumWorkGroups.xy);
        vec2 upper = vec2(gl_WorkGroupID.xy + uvec2(1)) / vec2(gl_NumWorkGroups.xy);

        /* 转换回NDC空间并按以下顺序分配
           3------2
           | tile |
           0------1
        */
        lower = lower * 2.0 - 1.0;
        upper = upper * 2.0 - 1.0;

        tile_corner[0] = vec4(lower, 1.0, 1.0);
        tile_corner[1] = vec4(upper.x, lower.y, 1.0, 1.0);
        tile_corner[2] = vec4(upper, 1.0, 1.0);
        tile_corner[3] = vec4(lower.x, upper.y, 1.0, 1.0);

        // 转换回相机/视图空间
        tile_corner[0] = NDC2View(tile_corner[0]);
        tile_corner[1] = NDC2View(tile_corner[1]);
        tile_corner[2] = NDC2View(tile_corner[2]);
        tile_corner[3] = NDC2View(tile_corner[3]);

        // 构建视锥体的侧面平面，plane.xyz = 表面法线，plane.w = 到原点的距离 = 0
        tile_frustum[0] = vec4(normalize(cross(tile_corner[0].xyz, tile_corner[1].xyz)), 0.0);  // 下面
        tile_frustum[1] = vec4(normalize(cross(tile_corner[1].xyz, tile_corner[2].xyz)), 0.0);  // 右侧
        tile_frustum[2] = vec4(normalize(cross(tile_corner[2].xyz, tile_corner[3].xyz)), 0.0);  // 上面
        tile_frustum[3] = vec4(normalize(cross(tile_corner[3].xyz, tile_corner[0].xyz)), 0.0);  // 左侧
    }

    barrier();  // 再次等待所有线程

    // 步骤 3: 使用所有256个线程进行光源剔除（光源平面或球体-平面交集检测）
    // 如果场景中包含超过256个光源，则需要额外的传递
    uint n_pass = (n_lights + 255) / 256;

    for (uint i = 0; i < n_pass; i++) {
        uint light_index = i * 256 + gl_LocalInvocationIndex;
        if (light_index >= n_lights) {
            break;
        }

        float radius = pl_range[light_index];  // 光源的有效范围是一个球体
        vec4 position = camera.view * pl_position[light_index];  // 光源位置在视图空间中

        // 如果光源球体完全在远裁剪面后面或在近裁剪面前面，则不会与视锥体相交，因此可以提前退出
        // 注意，我们使用右手坐标系，其中前向矢量是 -z，因此需要取反z分量以获取深度
        float light_depth = -position.z;
        if ((light_depth + radius < min_depth_float) || (light_depth - radius > max_depth_float)) {
            continue;
        }

        // 计算从光源到视锥体每个侧面平面的 *有符号* 距离
        float d0 = dot(position.xyz, tile_frustum[0].xyz);
        float d1 = dot(position.xyz, tile_frustum[1].xyz);
        float d2 = dot(position.xyz, tile_frustum[2].xyz);
        float d3 = dot(position.xyz, tile_frustum[3].xyz);

        // 如果光源球体与视锥体的每个平面都 "相交"，则增加可见光源计数，并将其索引添加到共享的本地索引数组中
        // 注意，距离可能是负数，这种情况下光源在视锥体内部，因此测试仍应通过
        if ((d0 <= radius) && (d1 <= radius) && (d2 <= radius) && (d3 <= radius)) {
            uint offset = atomicAdd(n_visible_lights, 1);  // 原子操作返回更改前的值
            local_indices[offset] = int(light_index);
        }
    }

    barrier();  // 再次等待

    // 步骤 4: 一个线程应将本地索引数组推送到全局的 `Index` SSBO
    uint tile_id = gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x;  // 第i个瓦片
    if (gl_LocalInvocationIndex == 0) {
        uint offset = tile_id * n_lights;  // 在全局 `Index` SSBO 中此瓦片的起始索引
        for (uint i = 0; i < n_visible_lights; ++i) {
            pl_index[offset + i] = local_indices[i];
        }

        // 使用 -1 标记此瓦片的数组结束，这样读取SSBO的程序可以提前停止
        if (n_visible_lights != n_lights) {
            pl_index[offset + n_visible_lights] = -1;
        }
    }
}


#endif
