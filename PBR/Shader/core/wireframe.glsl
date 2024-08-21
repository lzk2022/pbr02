
#ifdef geometry_shader

/* 几何着色器将顶点数据直接传递到片段着色器
   在传递过程中，它将计算每个顶点到边缘的距离，用于定义线框
   通常，透视除法和视口变换由硬件自动完成
   但是由于我们在这里直接在屏幕空间中工作，我们需要自己完成这些步骤
   回想一下将顶点从局部空间转换到视口空间的ISO过程

   1. 矩阵 `M` 将顶点从局部空间转换到世界空间（模型矩阵）
   2. 矩阵 `V` 将顶点从世界空间转换到视图/相机空间（视图矩阵）
   3. 矩阵 `P` 将顶点从相机空间转换到裁剪空间（投影矩阵）
   4. 顶点位置经过 `MVP` 变换后赋给 `gl_Position`，从而在裁剪空间中
   5. 驱动程序执行透视除法 /w，将裁剪空间变换为NDC空间 [-1, 1]
   6. 驱动程序执行视口变换，将NDC空间变换为屏幕空间（1600x900）
   x. 注意，在某些上下文中，“屏幕/窗口”空间与视口空间不完全相同
*/

layout(triangles) in;  // 输入三角形
layout(triangle_strip, max_vertices = 3) out;  // 输出三角形带，最多输出3个顶点

layout(location = 0) in _vtx {
    in vec3 _position;   // 输入顶点位置
    in vec3 _normal;     // 输入顶点法线
    in vec2 _uv;         // 输入第一组纹理坐标
    in vec2 _uv2;        // 输入第二组纹理坐标
    in vec3 _tangent;    // 输入切线
    in vec3 _binormal;   // 输入双切线
} vtx[];

layout(location = 0) out _vtx {
    smooth out vec3 _position;   // 平滑输出顶点位置
    smooth out vec3 _normal;     // 平滑输出顶点法线
    smooth out vec2 _uv;         // 平滑输出第一组纹理坐标
    smooth out vec2 _uv2;        // 平滑输出第二组纹理坐标
    smooth out vec3 _tangent;    // 平滑输出切线
    smooth out vec3 _binormal;   // 平滑输出双切线
    noperspective out vec3 _distance;  // 到三角形边缘的距离，无透视插值
};

vec3 ComputeTriangleHeight(const vec2 p0, const vec2 p1, const vec2 p2) {
    // 计算三角形的边长
    float a = length(p1 - p2);  // 边a的长度
    float b = length(p2 - p0);  // 边b的长度
    float c = length(p1 - p0);  // 边c的长度

    // 计算三角形的角度
    float alpha = acos((b * b + c * c - a * a) / (2.0 * b * c));  // 角α
    float beta  = acos((a * a + c * c - b * b) / (2.0 * a * c));  // 角β

    // 计算三角形各边上的高度（高度）
    float ha = abs(c * sin(beta));  // 边a上的高度
    float hb = abs(c * sin(alpha));  // 边b上的高度
    float hc = abs(b * sin(alpha));  // 边c上的高度

    return vec3(ha, hb, hc);  // 返回三个高度值
}

const mat4 viewport = mat4(
    vec4(800, 0.0, 0.0, 0.0),  // NDC空间x ~ [-1,1] -> 缩放到视口宽度 -> x ~ [-800,800]
    vec4(0.0, 450, 0.0, 0.0),  // NDC空间y ~ [-1,1] -> 缩放到视口高度 -> y ~ [-450,450]
    vec4(0.0, 0.0, 1.0, 0.0),  // 在z轴保持缩放（深度缓冲区）
    vec4(800, 450, 0.0, 1.0)   // 在x/y轴上平移800/450 -> x ~ [0,1600], y ~ [0,900]
);

void main() {
    // 在屏幕空间中计算三角形的3个顶点
    vec2 p0 = vec2(viewport * (gl_in[0].gl_Position / gl_in[0].gl_Position.w));
    vec2 p1 = vec2(viewport * (gl_in[1].gl_Position / gl_in[1].gl_Position.w));
    vec2 p2 = vec2(viewport * (gl_in[2].gl_Position / gl_in[2].gl_Position.w));

    // 计算三角形的高度
    vec3 h = ComputeTriangleHeight(p0, p1, p2);

    for (uint i = 0; i < 3; i++) {
        // 将顶点数据传递给下一阶段
        _position = vtx[i]._position;
        _normal   = vtx[i]._normal;
        _uv       = vtx[i]._uv;
        _uv2      = vtx[i]._uv2;
        _tangent  = vtx[i]._tangent;
        _binormal = vtx[i]._binormal;

        // 根据顶点索引设置到三角形边缘的距离
        switch (i) {
            case 0: _distance = vec3(h.x, 0, 0); break;  // 对应边a的高度
            case 1: _distance = vec3(0, h.y, 0); break;  // 对应边b的高度
            case 2: _distance = vec3(0, 0, h.z); break;  // 对应边c的高度
        }

        gl_Position = gl_in[i].gl_Position;  // 设置顶点的裁剪空间位置
        EmitVertex();  // 发射顶点
    }

    EndPrimitive();  // 结束图元的生成
}


#endif