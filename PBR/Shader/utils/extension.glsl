#ifndef _EXT_H
#define _EXT_H

////////////////////////////////////////////////////////////////////////////////

#define EPS      1e-5              // 极小值，用于比较浮点数是否接近
#define PI       3.141592653589793 // 圆周率
#define PI2      6.283185307179586 // 圆周率的两倍
#define INV_PI   0.318309886183791 // 圆周率的倒数，1/π
#define HLF_PI   1.570796326794897 // π的一半
#define SQRT2    1.414213562373095 // 2的平方根
#define SQRT3    1.732050807568877 // 3的平方根
#define SQRT5    2.236067977499789 // 5的平方根
#define CBRT2    1.259921049894873 // 2的立方根
#define CBRT3    1.442249570307408 // 3的立方根
#define G_PHI    1.618033988749894 // 黄金比例
#define EULER_E  2.718281828459045 // 自然对数e
#define LN2      0.693147180559945 // 2的自然对数
#define LN10     2.302585092994046 // 10的自然对数
#define INV_LN2  1.442695040888963 // 自然对数2的倒数，1/ln2
#define INV_LN10 0.434294481903252 // 自然对数10的倒数，1/ln10

#define clamp01(x) clamp(x, 0.0, 1.0) // 将x限制在0到1之间的宏函数

////////////////////////////////////////////////////////////////////////////////

// 计算vec2/vec3/vec4的最小/最大分量
float min2(const vec2 v) { return min(v.x, v.y); }    // 计算vec2的最小分量
float max2(const vec2 v) { return max(v.x, v.y); }    // 计算vec2的最大分量
float min3(const vec3 v) { return min(min(v.x, v.y), v.z); } // 计算vec3的最小分量
float max3(const vec3 v) { return max(max(v.x, v.y), v.z); } // 计算vec3的最大分量
float min4(const vec4 v) { return min(min3(v.xyz), v.w); }    // 计算vec4的最小分量
float max4(const vec4 v) { return max(max3(v.xyz), v.w); }    // 计算vec4的最大分量

// 以10为底的对数和以2为底的对数
float log10(float x) { return log(x) * INV_LN10; } // 计算以10为底的对数
float log2(float x) { return log(x) * INV_LN2; }   // 计算以2为底的对数

// 检查值x是否在区间(a, b)内，避免分支，返回值类型以避免分支
float step3(float a, float x, float b) { return step(a, x) * step(x, b); } // 检查float值x是否在区间(a, b)内，返回0或1
vec2 step3(const vec2 a, const vec2 x, const vec2 b) { return step(a, x) - step(b, x); } // 检查vec2值x是否在区间(a, b)内，返回0或1
vec3 step3(const vec3 a, const vec3 x, const vec3 b) { return step(a, x) - step(b, x); } // 检查vec3值x是否在区间(a, b)内，返回0或1
vec4 step3(const vec4 a, const vec4 x, const vec4 b) { return step(a, x) - step(b, x); } // 检查vec4值x是否在区间(a, b)内，返回0或1

// 优化的低次幂函数，去除了隐式的`exp/log`调用
float pow2(float x) { return x * x; }   // 计算x的平方
float pow3(float x) { return x * x * x; } // 计算x的立方
float pow4(float x) { return x * x * x * x; } // 计算x的四次方
float pow5(float x) { return x * x * x * x * x; } // 计算x的五次方

// 计算线性RGB颜色的亮度，sRGB必须先转换为线性RGB
float luminance(const vec3 linear_rgb) {
    return dot(linear_rgb, vec3(0.2126, 0.7152, 0.0722)); // 使用亮度权重计算亮度
}

// HSL颜色空间到RGB颜色空间的转换
vec3 hsl2rgb(float h, float s, float l) {
    vec3 u = mod(h * 6.0 + vec3(0.0, 4.0, 2.0), 6.0); // 计算HSL到RGB的中间值
    vec3 v = abs(u - 3.0) - 1.0; // 计算HSL到RGB的中间值
    return l + s * (clamp01(v) - 0.5) * (1.0 - abs(2.0 * l - 1.0)); // 计算HSL到RGB的转换
}

// HSV颜色空间到RGB颜色空间的转换
vec3 hsv2rgb(float h, float s, float v) {
    if (s <= 1e-4) return vec3(v);  // 零饱和度，返回灰度颜色

    float x = fract(h) * 6.0; // 计算HSV到RGB的中间值
    float f = fract(x);
    uint  i = uint(x);

    float p = v * (1.0f - s); // 计算HSV到RGB的中间值
    float q = v * (1.0f - s * f); // 计算HSV到RGB的中间值
    float t = v * (1.0f - s * (1.0f - f)); // 计算HSV到RGB的中间值

    switch (i) {
        case 0u: return vec3(v, t, p);
        case 1u: return vec3(q, v, p);
        case 2u: return vec3(p, v, t);
        case 3u: return vec3(p, q, v);
        case 4u: return vec3(t, p, v);
        default: return vec3(v, p, q);
    }
}

// HSL颜色空间到RGB颜色空间的转换（重载函数）
vec3 hsl2rgb(const vec3 hsl) {
    return hsl2rgb(hsl.x, hsl.y, hsl.z); // 调用重载函数
}

// HSV颜色空间到RGB颜色空间的转换（重载函数）
vec3 hsv2rgb(const vec3 hsv) {
    return hsv2rgb(hsv.x, hsv.y, hsv.z); // 调用重载函数
}

// 根据连续的彩虹色带返回RGB颜色，基于色相参数（0 ~ 1）
// 为了创建平滑的颜色过渡，色相必须是单向的，因此一旦色相达到1
// 它必须跳回到0，这样的值可以通过模运算或fract函数创建
vec3 rainbow(float hue) {
    return hsv2rgb(hue, 1.0, 1.0); // 使用HSV颜色空间计算彩虹颜色
}

// 当x的值单调变化时，返回一个在0.0和k之间弹跳的浮点数
float bounce(float x, float k) {
    return k - abs(k - mod(x, k * 2)); // 计算弹跳值
}

// 将32位浮点数打包成vec4（RGBA格式），每个RGBA组件都是8位和1/256精度
// 这经常用于不支持高精度格式如GL_DEPTH_COMPONENT32的移动设备上
// 例如，在WebGL中实现阴影映射时广泛使用这段代码
vec4 pack(float x) {
    const vec4 bit_shift = vec4(1.0, 255.0, 255.0 * 255.0, 255.0 * 255.0 * 255.0); // 位移向量
    const vec4 bit_mask = vec4(vec3(1.0 / 255.0), 0.0); // 位掩码
    vec4 v = fract(x * bit_shift); // 计算打包的值
    return v - v.gbaa * bit_mask; // 截断超出8位的值
}

// 将vec4 RGBA解包成32位精度浮点数，详细解释请参见：
// https://stackoverflow.com/questions/9882716/packing-float-into-vec4-how-does-this-code-work
float unpack(vec4 v) {
    const vec4 bit_shift = vec4(1.0, 1.0 / 255.0, 1.0 / (255.0 * 255.0), 1.0 / (255.0 * 255.0 * 255.0)); // 位移向量
    return dot(v, bit_shift); // 计算解包的值
}

// 返回一个0到1之间的朴素伪随机数，种子x可以是(-inf, +inf)内的任何数字
// 请注意，这种随机性在0.5附近更集中，而在0.0和1.0的两端更稀疏
// 在正弦波的高峰处非常有缺陷，因此种子x不应接近PI/2的倍数
// 参考资料：https://thebookofshaders.com/10/
float random1D(float x) {
    return fract(sin(x) * 100000.0); // 计算朴素伪随机数
}

// 基于2D种子向量返回一个0到1之间的朴素伪随机数
// 可以通过不同的a、b和c值改变随机模式，但是
// 种子值（每个分量）必须是介于0到1之间的浮点数
float random2D(const vec2 uv) {
    const float a = 12.9898, b = 78.233, c = 43758.5453123; // 常数
    float dt = dot(uv.xy, vec2(a, b)); // 点乘
    float sn = mod(dt, PI); // 模运算
    return fract(sin(sn) * c); // 计算朴素伪随机数
}

#endif
