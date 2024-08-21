#version 460 core

#ifdef vertex_shader

layout(location = 0) out vec2 _uv;  // 输出UV坐标

// 主函数，顶点着色器，计算位置和UV坐标
void main() {
    vec2 position = vec2(gl_VertexID % 2, gl_VertexID / 2) * 4.0 - 1;  // 计算顶点位置
    _uv = (position + 1) * 0.5;  // 计算UV坐标
    gl_Position = vec4(position, 0.0, 1.0);  // 设置顶点位置
}

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef fragment_shader

layout(location = 0) in vec2 _uv;  // 输入UV坐标
layout(location = 0) out vec4 color;  // 输出颜色

layout(binding = 0) uniform sampler2D color_depth_texture;  // 绑定颜色深度纹理
layout(binding = 1) uniform usampler2D stencil_texture;  // 绑定模板纹理

const float near = 0.1;  // 近裁剪面
const float far = 100.0;  // 远裁剪面

subroutine vec4 DrawBuffer(void);  // 定义子例程（类似函数指针）
layout(location = 0) subroutine uniform DrawBuffer drawbuffer;  // 子例程Uniform

// 子例程：绘制颜色缓冲区
layout(index = 0) subroutine(DrawBuffer)
vec4 DrawColorBuffer() {
    return texture(color_depth_texture, _uv);  // 返回颜色深度纹理采样结果
}

// 深度在屏幕空间中是非线性的，对于小的z值其精度很高，而对于大的z值其精度较低，
// 大多数像素会呈现白色，我们在绘制前必须对深度值进行线性化处理
// 子例程：绘制深度缓冲区
layout(index = 1) subroutine(DrawBuffer)
vec4 DrawDepthBuffer() {
    float depth = texture(color_depth_texture, _uv).r;  // 获取纹理中的深度值
    float ndc_depth = depth * 2.0 - 1.0;  // 转换为NDC空间的深度值
    float z = (2.0 * near * far) / (far + near - ndc_depth * (far - near));  // 转换为线性深度值
    float linear_depth = z / far;  // 归一化线性深度值到[0,1]范围
    return vec4(vec3(linear_depth), 1.0);  // 返回线性深度值作为颜色
}

// 子例程：绘制模板缓冲区
layout(index = 2) subroutine(DrawBuffer)
vec4 DrawStencilBuffer() {
    uint stencil = texture(stencil_texture, _uv).r;  // 获取模板纹理中的模板值
    return vec4(vec3(stencil), 1.0);  // 返回模板值作为颜色
}

// 主函数，片段着色器，根据子例程绘制颜色
void main() {
    color = drawbuffer();  // 调用选择的子例程，并将结果赋给颜色输出
}

#endif
