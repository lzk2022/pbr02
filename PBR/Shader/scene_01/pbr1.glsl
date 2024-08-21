#version 460 core
// 禁用优化指令
#pragma optimize(off)

layout(std140, binding = 0) uniform Camera {
    vec4 position;      // 相机位置
    vec4 direction;     // 相机方向
    mat4 view;          // 视图矩阵
    mat4 projection;    // 投影矩阵
} camera;
// 引入渲染器输入文件
#include "../core/renderer_input.glsl"

////////////////////////////////////////////////////////////////////////////////

#ifdef vertex_shader

// 顶点着色器
layout(location = 0) in vec3 position;    // 顶点位置输入
layout(location = 1) in vec3 normal;      // 顶点法线输入
layout(location = 2) in vec2 uv;          // 纹理坐标输入
layout(location = 3) in vec2 uv2;         // 第二组纹理坐标输入
layout(location = 4) in vec3 tangent;     // 切线输入
layout(location = 5) in vec3 binormal;    // 双切线输入

layout(location = 0) out _vtx {
    out vec3 _position;     // 传递到片段着色器的顶点位置
    out vec3 _normal;       // 传递到片段着色器的顶点法线
    out vec2 _uv;           // 传递到片段着色器的纹理坐标
    out vec2 _uv2;          // 传递到片段着色器的第二组纹理坐标
    out vec3 _tangent;      // 传递到片段着色器的切线
    out vec3 _binormal;     // 传递到片段着色器的双切线
};

void main() {
    // 计算顶点的裁剪空间坐标
    gl_Position = camera.projection * camera.view * self.transform * vec4(position, 1.0);

    // 将顶点位置、法线、切线、双切线、纹理坐标传递给下一阶段
    _position = vec3(self.transform * vec4(position, 1.0));
    _normal   = normalize(vec3(self.transform * vec4(normal, 0.0)));
    _tangent  = normalize(vec3(self.transform * vec4(tangent, 0.0)));
    _binormal = normalize(vec3(self.transform * vec4(binormal, 0.0)));
    _uv = uv;
}

#endif

////////////////////////////////////////////////////////////////////////////////


#ifdef fragment_shader

// 片段着色器、 引入PBR统一变量定义、 引入PBR着色函数
#include "../core/pbr_uniform.glsl"
#include "../core/pbr_shading.glsl"

layout(location = 0) in _vtx {
    in vec3 _position;   // 顶点位置输入
    in vec3 _normal;     // 顶点法线输入
    in vec2 _uv;         // 纹理坐标输入
    in vec2 _uv2;        // 第二组纹理坐标输入
    in vec3 _tangent;    // 切线输入
    in vec3 _binormal;   // 双切线输入
};

layout(location = 0) out vec4 color;   // 输出颜色
layout(location = 1) out vec4 bloom;   // 输出泛光

layout(std430, binding = 0) readonly buffer Color    { vec4  pl_color[];    };   // 光源颜色缓冲
layout(std430, binding = 1) readonly buffer Position { vec4  pl_position[]; };   // 光源位置缓冲
layout(std430, binding = 2) readonly buffer Range    { float pl_range[];    };   // 光源范围缓冲
layout(std430, binding = 3) readonly buffer Index    { int   pl_index[];    };   // 光源索引缓冲

layout(std140, binding = 1) uniform DL {
    vec4  color;         // 方向光颜色和强度
    vec4  direction;     // 方向光方向
    float intensity;     // 方向光强度
} dl;

layout(std140, binding = 2) uniform SL {
    vec4  color;         // 聚光灯颜色和强度
    vec4  position;      // 聚光灯位置
    vec4  direction;     // 聚光灯方向
    float intensity;     // 聚光灯强度
    float inner_cos;     // 内圆锥角余弦值
    float outer_cos;     // 外圆锥角余弦值
    float range;         // 聚光灯范围
} sl;

layout(std140, binding = 3) uniform OL {
    vec4  color;         // 轨道光颜色
    vec4  position;      // 轨道光位置
    float intensity;     // 轨道光强度
    float linear;        // 轨道光线性衰减系数
    float quadratic;     // 轨道光二次衰减系数
    float range;         // 轨道光范围
} ol;

layout(std140, binding = 4) uniform PL {
    float intensity;     // 点光源强度
    float linear;        // 点光源线性衰减系数
    float quadratic;     // 点光源二次衰减系数
} pl;

const uint n_pls = 28;      // 光源数量
const uint tile_size = 16;  // 瓦片大小

// 获取当前像素所在瓦片的偏移量及其起始索引
uint GetTileOffset() {
    ivec2 tile_id = ivec2(gl_FragCoord.xy) / ivec2(tile_size);   // 计算瓦片ID
    uint n_cols = rdr_in.resolution.x / tile_size;               // 计算每行的瓦片数
    uint tile_index = tile_id.y * n_cols + tile_id.x;            // 计算瓦片索引
    return tile_index * n_pls;                                   // 返回瓦片在索引缓冲中的起始偏移量
}

void main() {
    // 在深度预通道中，不执行任何绘制操作
    if (rdr_in.depth_prepass) {
        return;
    }

    Pixel px;
    px._position = _position;   // 设置像素位置
    px._normal   = _normal;     // 设置像素法线
    px._uv       = _uv;         // 设置像素纹理坐标
    px._has_tbn  = true;        // 像素是否具有切线空间

    InitPixel(px, camera.position.xyz);   // 初始化像素

    vec3 Lo = vec3(0.0);    // 漫反射光贡献
    vec3 Le = vec3(0.0);    // 自发光贡献

    // 计算方向光的贡献
    Lo += EvaluateADL(px, dl.direction.xyz, 1.0) * dl.color.rgb * dl.intensity;

    // 计算摄像机手电筒的贡献
    vec3 sc = EvaluateASL(px, sl.position.xyz, sl.direction.xyz, sl.range, sl.inner_cos, sl.outer_cos);
    Lo += sc * sl.color.rgb * sl.intensity;

    // 计算轨道光的贡献
    vec3 oc = EvaluateAPL(px, ol.position.xyz, ol.range, ol.linear, ol.quadratic, 1.0);
    Lo += oc * ol.color.rgb * ol.intensity;

    // 计算点光源 x 28 的贡献（如果未被剔除且可见）
    uint offset = GetTileOffset();
    for (uint i = 0; i < n_pls && pl_index[offset + i] != -1; ++i) {
        int index = pl_index[offset + i];
        vec3 pc = EvaluateAPL(px, pl_position[index].xyz, pl_range[index], pl.linear, pl.quadratic, 1.0);
        Lo += pc * pl_color[index].rgb * pl.intensity;
    }

    // 如果材质ID为6（符文台平台发光），计算自发光贡献
    if (self.material_id == 6) {
        Le = CircularEaseInOut(abs(sin(rdr_in.time * 2.0))) * px.emission.rgb;
    }

    color = vec4(Lo + Le, px.albedo.a);  // 设置输出颜色
    bloom = vec4(Le, 1.0);               // 设置泛光输出
}

#endif
