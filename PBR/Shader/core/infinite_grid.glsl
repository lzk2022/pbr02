
#version 460 core
#pragma optimize(off)

// 注意：如果网格在中途渲染，内部的单元格可能会接收附加帧缓冲区的清除颜色，
// 因此随后渲染的网格可能会由于深度测试而被阻挡。请记住，该网格只是一个透明的
// 平面上的四边形，除了线条上的像素外，它依赖于alpha混合才能正常工作，因此
// 需要在最后绘制。

// 参考：
// https://ourmachinery.com/post/borderland-between-rendering-and-editor-part-1/
// https://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/

const float scale = 100.0;  // 网格的缩放因子
const float lod_floor = 4.0;  // 线条之间的最小像素数，用于决定LOD是否切换
const vec4 x_axis_color = vec4(220, 20, 60, 255) / 255.0;  // X轴颜色
const vec4 z_axis_color = vec4(0, 46, 255, 255) / 255.0;   // Z轴颜色

layout(location = 0) uniform float cell_size = 2.0;  // 单元格大小
layout(location = 1) uniform vec4 thin_line_color = vec4(vec3(0.1), 1.0);  // 细线颜色
layout(location = 2) uniform vec4 wide_line_color = vec4(vec3(0.2), 1.0);  // 每10条线为粗线

////////////////////////////////////////////////////////////////////////////////

#ifdef vertex_shader

layout(std140, binding = 0) uniform Camera {
    vec4 position;   // 相机位置
    vec4 direction;  // 相机方向
    mat4 view;       // 视图矩阵
    mat4 projection; // 投影矩阵
} camera;  // 相机Uniform

layout(location = 0) out vec2 _uv;  // 输出UV坐标

// 平面四边形的顶点，顺时针顺序
const vec3 positions[6] = vec3[] (
    vec3(-1, 0, -1), vec3(-1, 0, 1), vec3(1, 0, 1),
    vec3(1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1)
);

// 主函数，顶点着色器，计算位置和UV坐标
void main() {
    uint index = camera.position.y >= 0 ? gl_VertexID : (5 - gl_VertexID);  // 当y < 0时，反转顶点顺序
    vec3 position = positions[index] * scale;  // 缩放顶点位置
    gl_Position = camera.projection * camera.view * vec4(position, 1.0);  // 计算最终位置
    _uv = position.xz;  // 将UV限制在X-Z平面上（y == 0）
}

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef fragment_shader
// 导入扩展库
#include "../utils/extension.glsl"

layout(location = 0) in vec2 _uv;  // 输入UV坐标
layout(location = 0) out vec4 color;  // 输出颜色
layout(location = 1) out vec4 bloom;  // 输出到泛光通道


// 主函数，片段着色器，计算网格线的颜色和透明度
void main() {
    // higher derivative = farther cell = smaller LOD = less details = more transparent
    vec2 derivative = fwidth(_uv);  // 计算UV坐标的导数
    float lod = max(0.0, log10(length(derivative) * lod_floor / cell_size) + 1.0);  // 计算LOD级别
    float fade = fract(lod);  // LOD渐变因子

    // cell size at LOD level 0, 1 and 2, each higher level is 10 times larger
    float cell_size_0 = cell_size * pow(10.0, floor(lod));  // LOD级别0的单元格大小
    float cell_size_1 = cell_size_0 * 10.0;  // LOD级别1的单元格大小
    float cell_size_2 = cell_size_1 * 10.0;  // LOD级别2的单元格大小

    derivative *= 4.0;  // 每个抗锯齿线条涵盖最多4个像素

    // compute absolute distance to cell line centers for each LOD and pick max x/y to be the alpha
    // alpha_0 >= alpha_1 >= alpha_2
    float alpha_0 = max2(1.0 - abs(clamp01(mod(_uv, cell_size_0) / derivative) * 2.0 - 1.0));  // LOD级别0的透明度
    float alpha_1 = max2(1.0 - abs(clamp01(mod(_uv, cell_size_1) / derivative) * 2.0 - 1.0));  // LOD级别1的透明度
    float alpha_2 = max2(1.0 - abs(clamp01(mod(_uv, cell_size_2) / derivative) * 2.0 - 1.0));  // LOD级别2的透明度

    // line margins can be used to check where the current line is (e.g. x = 0, or y = 3, etc)
    vec2 margin = min(derivative, 1.0);  // 计算边界
    vec2 basis = step3(vec2(0.0), _uv, margin);  // 计算基础参数

    // blend between falloff colors to handle LOD transition and highlight world axis X and Z
    vec4 c = alpha_2 > 0.0
        ? (basis.y > 0.0 ? x_axis_color : (basis.x > 0.0 ? z_axis_color : wide_line_color))
        : (alpha_1 > 0.0 ? mix(wide_line_color, thin_line_color, fade) : thin_line_color);  // 根据条件混合颜色

    // calculate opacity falloff based on distance to grid extents
    float opacity_falloff = 1.0 - clamp01(length(_uv) / scale);  // 根据距离网格边界的距离计算透明度衰减

    // blend between LOD level alphas and scale with opacity falloff
    c.a *= (alpha_2 > 0.0 ? alpha_2 : alpha_1 > 0.0 ? alpha_1 : (alpha_0 * (1.0 - fade))) * opacity_falloff;  // 根据透明度和透明度衰减混合计算最终透明度

    color = c;  // 输出颜色
    bloom = c;  // 输出到泛光通道，如果MRT被启用，否则写入GL_NONE
}

#endif
