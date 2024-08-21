#ifndef _EASING_H
#define _EASING_H

#ifndef PI
#define PI      3.141592653589793  // 圆周率
#define HLF_PI  1.570796326794897  // π/2 的值，用于简化计算
#endif
// 引入扩展函数库
#include "extension.glsl"

// 一些简单的缓动函数（无分支）改编自 https://github.com/warrenm/AHEasing
// 这些缓动函数的效果可以在 https://easings.net/ 上进行可视化

float QuadraticEaseIn(float x) {
    return x * x;  // 二次缓入函数
}

float QuadraticEaseOut(float x) {
    return x * (2.0 - x);  // 二次缓出函数
}

float QuadraticEaseInOut(float x) {
    return x < 0.5 ? (2.0 * x * x) : (4.0 * x - 2.0 * x * x - 1.0);  // 二次缓入缓出函数
}

float CubicEaseIn(float x) {
    return x * x * x;  // 三次缓入函数
}

float CubicEaseOut(float x) {
    return 1.0 + pow(x - 1.0, 3.0);  // 三次缓出函数
}

float CubicEaseInOut(float x) {
    return x < 0.5 ? (4.0 * x * x * x) : (0.5 * pow(2.0 * x - 2.0, 3.0) + 1.0);  // 三次缓入缓出函数
}

float QuarticEaseIn(float x) {
    return x * x * x * x;  // 四次缓入函数
}

float QuarticEaseOut(float x) {
    return pow(x - 1.0, 3.0) * (1.0 - x) + 1.0;  // 四次缓出函数
}

float QuarticEaseInOut(float x) {
    return x < 0.5 ? (8.0 * x * x * x * x) : (1.0 - 8.0 * pow(2.0 * x - 1.0, 4.0));  // 四次缓入缓出函数
}

float QuinticEaseIn(float x) {
    return x * x * x * x * x;  // 五次缓入函数
}

float QuinticEaseOut(float x) {
    return pow(x - 1.0, 5.0) + 1.0;  // 五次缓出函数
}

float QuinticEaseInOut(float x) {
    return x < 0.5 ? (16.0 * x * x * x * x * x) : (0.5 * pow(2.0 * x - 2.0, 5.0) + 1.0);  // 五次缓入缓出函数
}

float SineEaseIn(float x) {
    return 1.0 - cos(x * HLF_PI);  // 正弦缓入函数
}

float SineEaseOut(float x) {
    return sin(x * HLF_PI);  // 正弦缓出函数
}

float SineEaseInOut(float x) {
    return 0.5 * (1.0 - cos(PI * x));  // 正弦缓入缓出函数
}

float CircularEaseIn(float x) {
    return 1.0 - sqrt(1.0 - x * x);  // 圆形缓入函数
}

float CircularEaseOut(float x) {
    return sqrt((2.0 - x) * x);  // 圆形缓出函数
}

float CircularEaseInOut(float x) {
    return x < 0.5 ? (0.5 * (1.0 - sqrt(1.0 - 4.0 * x * x))) : (0.5 * (sqrt((3.0 - 2.0 * x) * (2.0 * x - 1.0)) + 1.0));  // 圆形缓入缓出函数
}

float ExponentialEaseIn(float x) {
    return (x == 0.0) ? x : pow(2.0, 10.0 * (x - 1.0));  // 指数缓入函数
}

float ExponentialEaseOut(float x) {
    return (x == 1.0) ? x : 1.0 - pow(2.0, -10.0 * x);  // 指数缓出函数
}

float ExponentialEaseInOut(float x) {
    return (x == 0.0 || x == 1.0) ? x :
        ((x < 0.5) ? (0.5 * pow(2.0, (20.0 * x) - 10.0)) : (1.0 - 0.5 * pow(2.0, (-20.0 * x) + 10.0)));  // 指数缓入缓出函数
}

float ElasticEaseIn(float x) {
    return sin(13.0 * HLF_PI * x) * pow(2.0, 10.0 * (x - 1.0));  // 弹性缓入函数
}

float ElasticEaseOut(float x) {
    return 1.0 - sin(13.0 * HLF_PI * (x + 1.0)) * pow(2.0, -10.0 * x);  // 弹性缓出函数
}

float ElasticEaseInOut(float x) {
    return (x < 0.5)
        ? (0.5 * sin(26.0 * HLF_PI * x) * pow(2.0, 10.0 * (2.0 * x - 1.0)))
        : (0.5 * (2.0 - sin(26.0 * HLF_PI * x) * pow(2.0, -10.0 * (2.0 * x - 1.0))));  // 弹性缓入缓出函数
}

float BackEaseIn(float x) {
    return x * x * x - x * sin(x * PI);  // 背部缓入函数
}

float BackEaseOut(float x) {
    float f = 1.0 - x;
    return 1.0 - (f * f * f - f * sin(f * PI));  // 背部缓出函数
}

float BackEaseInOut(float x) {
    if (x < 0.5) {
        float f = 2.0 * x;
        return 0.5 * (f * f * f - f * sin(f * PI));  // 背部缓入缓出函数
    }

    float f = 2.0 - 2.0 * x;
    return 0.5 * (1.0 - (f * f * f - f * sin(f * PI))) + 0.5;
}

float BounceEaseOut(float x) {
    if (x < 0.36363636363) { return x * x * 7.5625; }
    if (x < 0.72727272727) { return x * x * 9.075 - x * 9.9 + 3.4; }
    if (x < 0.90000000000) { return x * x * 12.0664819945 - x * 19.6354570637 + 8.89806094183; }
    return x * x * 10.8 - x * 20.52 + 10.72;  // 反弹缓出函数
}

float BounceEaseIn(float x) {
    return 1.0 - BounceEaseOut(1.0 - x);  // 反弹缓入函数
}

float BounceEaseInOut(float x) {
    return (x < 0.5) ? (0.5 * BounceEaseIn(x * 2.0)) : (0.5 * BounceEaseOut(x * 2.0 - 1.0) + 0.5);  // 反弹缓入缓出函数
}

#endif
