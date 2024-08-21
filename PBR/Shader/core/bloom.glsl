#version 460 core

// 简单的辉光效果计算着色器（1次下采样 + 6次高斯模糊 + 1次上采样）

// 对于图像加载存储操作，从边界外读取将返回0，写入边界外将不起作用，因此可以安全地忽略所有边界检查。
// 这个着色器可以通过使用共享的本地存储来缓存获取的像素来进行轻微优化，但由于imageLoad()已经非常快，
// 因此性能提升很小。

// 注意：请不要对Alpha通道进行模糊处理！

#ifdef compute_shader

// 导入高斯权重定义文件
#define k11x11
#include "../utils/gaussian.glsl"

layout(local_size_x = 32, local_size_y = 18, local_size_z = 1) in;  // 适应16:9的纵横比

layout(binding = 0, rgba16f) uniform image2D ping;  // 读写绑定的ping图像
layout(binding = 1, rgba16f) uniform image2D pong;  // 读写绑定的pong图像

layout(location = 0) uniform bool horizontal;  // 水平方向标志

// 高斯模糊的水平方向处理函数
void GaussianBlurH(const ivec2 coord) {
    vec3 color = imageLoad(ping, coord).rgb * weight[0];
    for (int i = 1; i < 6; i++) {
        ivec2 offset = ivec2(i, 0);
        color += imageLoad(ping, coord + offset).rgb * weight[i];
        color += imageLoad(ping, coord - offset).rgb * weight[i];
    }
    imageStore(pong, coord, vec4(color, 1.0));
}

// 高斯模糊的垂直方向处理函数
void GaussianBlurV(const ivec2 coord) {
    vec3 color = imageLoad(pong, coord).rgb * weight[0];
    for (int i = 1; i < 6; i++) {
        ivec2 offset = ivec2(0, i);
        color += imageLoad(pong, coord + offset).rgb * weight[i];
        color += imageLoad(pong, coord - offset).rgb * weight[i];
    }
    imageStore(ping, coord, vec4(color, 1.0));
}

// 主函数，根据水平标志调用相应的高斯模糊函数
void main() {
    ivec2 ils_coord = ivec2(gl_GlobalInvocationID.xy);

    if (horizontal) {
        GaussianBlurH(ils_coord);  // 水平模糊
    }
    else {
        GaussianBlurV(ils_coord);  // 垂直模糊
    }
}

#endif
