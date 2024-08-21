#ifndef _SAMPLING_H
#define _SAMPLING_H

// 参考资料:
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// https://www.pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations
// https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf

#include "extension.glsl"

// Van der Corput逆序列
float RadicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;  // (1 / 0x100000000)
}

// Hammersley点集（低差异随机序列）
vec2 Hammersley2D(uint i, uint N) {
    return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

// 均匀采样单位球面上的点（也是方向向量）
// 被采样点的概率为 1 / (4 * PI)，即无偏
vec3 UniformSampleSphere(float u, float v) {
    float phi = v * PI2;
    float cos_theta = 1.0 - 2.0 * u;  // ~ [-1, 1]
    float sin_theta = sqrt(max(0, 1 - cos_theta * cos_theta));
    return vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
}

// 均匀采样单位半球面上的点（也是方向向量）
// 被采样点的概率为 1 / (2 * PI)，即无偏
vec3 UniformSampleHemisphere(float u, float v) {
    float phi = v * PI2;
    float cos_theta = 1.0 - u;  // ~ [0, 1]
    float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
    return vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
}

// 余弦加权采样单位半球面上的点
// 被采样点的概率为 (cosine / PI)，即按余弦加权偏差
// 在余弦加权渲染方程中，此方法比均匀采样更受欢迎
vec3 CosineSampleHemisphere(float u, float v) {
    float phi = v * PI2;
    float cos_theta = sqrt(1.0 - u);  // 使用 `sqrt` 函数向余弦方向偏差
    float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
    return vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
}

// 使用GGX NDF进行重要性采样，给定alpha（粗糙度的平方）
// 这也是在单位半球面上操作的，PDF为 D_TRGGX() * cosine
// 此函数返回中间向量H（因为NDF在H处求值）
vec3 ImportanceSampleGGX(float u, float v, float alpha) {
    float a2 = alpha * alpha;
    float phi = u * PI2;
    float cos_theta = sqrt((1.0 - v) / (1.0 + (a2 - 1.0) * v));  // 向余弦和TRGGX NDF偏差
    float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
    return vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
}

// 在平面圆盘上均匀采样一个2D点
vec2 UniformSampleDisk(float u, float v) {
    float radius = sqrt(u);
    float theta = v * PI2;
    return vec2(radius * cos(theta), radius * sin(theta));
}

// 计算单位圆盘上的泊松样本数组，用于PCSS阴影
void PoissonSampleDisk(float seed, inout vec2 samples[16]) {
    const int n_samples = 16;
    float radius_step = 1.0 / float(n_samples);
    float angle_step = 3.883222077450933;  // PI2 * float(n_rings) / float(n_samples)

    float radius = radius_step;
    float angle = random1D(seed) * PI2;

    for (int i = 0; i < n_samples; ++i) {
        samples[i] = vec2(cos(angle), sin(angle)) * pow(radius, 0.75);  // 0.75是关键
        radius += radius_step;
        angle += angle_step;
    }
}

#endif
