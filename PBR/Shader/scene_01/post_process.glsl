#version 460 core
#ifdef vertex_shader

layout(location = 0) out vec2 _uv; // 输出UV坐标

void main() {
    // 计算顶点位置
    vec2 position = vec2(gl_VertexID % 2, gl_VertexID / 2) * 4.0 - 1;
    _uv = (position + 1) * 0.5; // 计算UV坐标
    gl_Position = vec4(position, 0.0, 1.0); // 设置顶点位置
}

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef fragment_shader

layout(location = 0) in vec2 _uv; // 输入UV坐标
layout(location = 0) out vec4 color; // 输出颜色

layout(location = 0) uniform int operator; // 色调映射操作符
layout(binding = 0) uniform sampler2D hdr_map; // HDR贴图
layout(binding = 1) uniform sampler2D bloom_map; // 泛光贴图
// 引入后处理函数库
#include "../utils/postprocess.glsl"

void main() {
    vec3 radiance = texture(hdr_map, _uv).rgb; // 从HDR贴图中获取辐射度
    vec3 ldr_color;

    // 根据操作符选择色调映射算法
    switch (operator) {
        case 0: ldr_color = Reinhard(radiance);         break; // 使用Reinhard算法
        case 1: ldr_color = ReinhardJodie(radiance);    break; // 使用Reinhard-Jodie算法
        case 2: ldr_color = Uncharted2Filmic(radiance); break; // 使用Uncharted 2 Filmic算法
        case 3: ldr_color = ApproxACES(radiance);       break; // 使用Approx ACES算法
    }

    // 色调映射通常不适用于泛光部分，因为泛光已模糊且不包含细节
    // 只需将泛光加到色调映射后的像素颜色上，并进行RGB裁剪
    ldr_color += texture(bloom_map, _uv).rgb;
    color = vec4(Linear2Gamma(ldr_color), 1.0); // 将线性颜色转换为gamma校正颜色并输出
}

#endif
