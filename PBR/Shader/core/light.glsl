#version 460 core

layout(std140, binding = 0) uniform Camera {
    vec4 position;        // 相机位置
    vec4 direction;       // 相机方向
    mat4 view;            // 视图矩阵
    mat4 projection;      // 投影矩阵
} camera;

#include "../core/renderer_input.glsl"

////////////////////////////////////////////////////////////////////////////////

#ifdef vertex_shader

layout(location = 0) in vec3 position;  // 顶点位置

// 将顶点位置变换到裁剪空间
void main() {
    gl_Position = camera.projection * camera.view * self.transform * vec4(position, 1.0);
}

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef fragment_shader

layout(location = 0) out vec4 color;   // 输出颜色
layout(location = 1) out vec4 bloom;   // 输出辉光

layout(location = 3) uniform vec3  light_color;    // 光源颜色
layout(location = 4) uniform float light_intensity;    // 光源强度
layout(location = 5) uniform float bloom_factor;    // 辉光因子

// 光源通常使用辉光效果渲染，以模拟光线的扩散，因此我们总是写入第二个渲染目标，而不管亮度阈值检查，
// 辉光因子控制辉光的饱和度，> 1 = 放大，< 1 = 减少

// 主函数，计算片段的颜色和辉光
void main() {
    float fade_io = 0.3 + abs(cos(rdr_in.time));  // 根据时间计算一个淡入淡出效果的因子
    float intensity = light_intensity * fade_io;  // 计算最终的光源强度

    // 如果第二个MRT未启用，辉光将写入GL_NONE并被丢弃
    color = vec4(light_color * intensity, 1.0);  // 计算颜色输出
    bloom = intensity > 0.2 ? vec4(color.rgb * bloom_factor, 1.0) : vec4(0.0);  // 计算辉光输出
}

#endif
