#ifndef _POSTPROCESS_H
#define _POSTPROCESS_H

#include "extension.glsl"

/* 常用色调映射操作符（TMO） */
// 参考资料：
// https://64.github.io/tonemapping/
// https://docs.unrealengine.com/4.26/en-US/RenderingAndGraphics/PostProcessEffects/ColorGrading/

// Reinhard色调映射
vec3 Reinhard(vec3 radiance) {
    return radiance / (1.0 + radiance);
}

// Reinhard色调映射（基于亮度）
// radiance: 待处理的辐照度
// max_luminance: 最大亮度值
vec3 ReinhardLuminance(vec3 radiance, float max_luminance) {
    float li = luminance(radiance);  // 计算辐照度的亮度
    float numerator = li * (1.0 + (li / (max_luminance * max_luminance)));
    float lo = numerator / (1.0 + li);
    return radiance * (lo / li);
}

// Reinhard-Jodie色调映射
// radiance: 待处理的辐照度
vec3 ReinhardJodie(vec3 radiance) {
    vec3 t = radiance / (1.0 + radiance);
    vec3 x = radiance / (1.0 + luminance(radiance));
    return vec3(mix(x.r, t.r, t.r), mix(x.g, t.g, t.g), mix(x.b, t.b, t.b));
}

// Uncharted2Partial色调映射
// x: 待处理的颜色向量
vec3 Uncharted2Partial(vec3 x) {
    float a = 0.15;
    float b = 0.50;
    float c = 0.10;
    float d = 0.20;
    float e = 0.02;
    float f = 0.30;
    return ((x * (a * x + c * b) + d * e) / (x * (a * x + b) + d * f)) - e / f;
}

// Uncharted2Filmic色调映射
// radiance: 待处理的辐照度
vec3 Uncharted2Filmic(vec3 radiance) {
    float exposure_bias = 2.0;
    vec3 white_scale = vec3(1.0) / Uncharted2Partial(vec3(11.2));
    vec3 c = Uncharted2Partial(radiance * exposure_bias);
    return c * white_scale;
}

// ApproxACES色调映射
// radiance: 待处理的辐照度
vec3 ApproxACES(vec3 radiance) {
    vec3 v = radiance * 0.6;
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((v * (a * v + b)) / (v * (c * v + d) + e), 0.0, 1.0);
}

/* 简单的伽马校正，使用近似的2.2次幂 */

// 从伽马校正到线性空间
// color: 待处理的颜色向量
vec3 Gamma2Linear(vec3 color) {
    return pow(color, vec3(2.2));  // 逐分量操作
}

// 从线性空间到伽马校正
// color: 待处理的颜色向量
vec3 Linear2Gamma(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));  // 逐分量操作
}

// 从伽马校正到线性空间（灰度）
// grayscale: 待处理的灰度值
float Gamma2Linear(float grayscale) {
    return pow(grayscale, 2.2);
}

// 从线性空间到伽马校正（灰度）
// grayscale: 待处理的灰度值
float Linear2Gamma(float grayscale) {
    return pow(grayscale, 1.0 / 2.2);
}

#endif
