#version 460 core

#ifdef vertex_shader

layout(location = 0) out vec2 _uv;  // 输出纹理坐标

void main() {
    vec2 position = vec2(gl_VertexID % 2, gl_VertexID / 2) * 4.0 - 1;  // 计算顶点位置
    _uv = (position + 1) * 0.5;  // 计算归一化纹理坐标
    gl_Position = vec4(position, 0.0, 1.0);  // 设置裁剪空间坐标
}

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef fragment_shader

layout(location = 0) in vec2 _uv;  // 输入纹理坐标
layout(location = 0) out vec4 color;  // 输出颜色

layout(location = 0) uniform int operator;  // 色调映射运算符
layout(binding = 0) uniform sampler2D hdr_map;  // HDR纹理
// 后处理函数库
#include "../utils/postprocess.glsl"

void main() {
    vec3 radiance = texture(hdr_map, _uv).rgb;  // 从HDR纹理获取辐射值
    vec3 ldr_color;

    switch (operator) {
        case 0: ldr_color = Reinhard(radiance);         break;  // Reinhard色调映射
        case 1: ldr_color = ReinhardJodie(radiance);    break;  // Reinhard-Jodie色调映射
        case 2: ldr_color = Uncharted2Filmic(radiance); break;  // Uncharted2 Filmic色调映射
        case 3: ldr_color = ApproxACES(radiance);       break;  // Approximate ACES色调映射
    }

    color = vec4(Linear2Gamma(ldr_color), 1.0);  // 线性到伽马校正，并输出最终颜色
}

#endif
