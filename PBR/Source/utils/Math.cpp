#include "Math.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>                   // GLM：OpenGL数学库
#include <glm/ext.hpp>                   // GLM：OpenGL数学库 扩展
#include <glm/gtc/quaternion.hpp>        // GLM：OpenGL数学库 四元数
#include <glm/gtx/quaternion.hpp>        // GLM：OpenGL数学库 四元数 扩展
#include <glm/gtx/transform.hpp>         // GLM：OpenGL数学库 变换
#include <glm/gtc/type_ptr.hpp>          // GLM：OpenGL数学库 指针
#include <glm/gtx/string_cast.hpp>       // GLM：OpenGL数学库 字符串转换
#include <glm/gtc/matrix_transform.hpp>  // GLM：OpenGL数学库 矩阵变换
#include <glm/gtx/perpendicular.hpp>     // GLM：OpenGL数学库 垂直向量
#include <GLM/gtx/quaternion.hpp>
#include "../utils/Log.h"
#include <cmath>
#include <limits>
#include <random>
using namespace glm;
namespace utils
{
    // 静态随机数生成器引擎及状态设置
    static std::random_device gDevice;               // 从设备获取种子
    static std::mt19937       gEngine32(gDevice());  // 在启动时仅种子一次
    static std::mt19937_64    gEngine64(gDevice());  // 在启动时仅种子一次

    // 构造静态均匀分布器
    static std::uniform_int_distribution<uint32_t> gUint32NDistributor(0, std::numeric_limits<uint32_t>::max());
    static std::uniform_int_distribution<uint64_t> gUint64NDistributor(0, std::numeric_limits<uint64_t>::max());
    static std::uniform_real_distribution<double>  gDoubleNDistributor(0.0, 1.0);

	GLuint Math::CalMipmap(GLuint w, GLuint h)
	{
		return 1 + static_cast<GLuint>(floor(std::log2(std::max(w, h))));;
	}

    template<typename T, typename>
    T Math::RandomGenerator()
    {
        if constexpr (std::is_same_v<T, uint64_t>) {
            return gUint32NDistributor(gEngine64);
        }
        else if constexpr (std::is_same_v<T, uint32_t>) {
            return gUint32NDistributor(gEngine32);
        }
        else if constexpr (std::is_same_v<T, double>) {
            return gDoubleNDistributor(gEngine32);
        }
        else if constexpr (std::is_same_v<T, float>) {
            return static_cast<float>(gDoubleNDistributor(gEngine32));
        }
        else {
            LOG_EXCEPTION_TRUE(true, "不支持的整数类型 T {0}",T);
        }
    }

    // 显式模板函数实例化
    template uint64_t Math::RandomGenerator();
    template uint32_t Math::RandomGenerator();
    template double   Math::RandomGenerator();
    template float    Math::RandomGenerator();

    // 比较两个向量是否在给定阈值 epsilon 内相等
    bool Math::Equals(const vec3& a, const vec3& b, float epsilon) {
        return glm::length2(a - b) < epsilon;  // 使用 `length2` 可以避免 `sqrt()` 操作
    }

    // 比较两个四元数是否在给定阈值 epsilon 内相等
    // 相比向量，这里的 epsilon 应该更宽松，否则 slerp 可能难以收敛
    bool Math::Equals(const quat& a, const quat& b, float epsilon) {
        return abs(glm::dot(a, b) - 1.0f) < epsilon;
    }

    // 测试两个浮点数是否在给定公差 tolerance 内近似相等
    bool Math::Equals(float a, float b, float tolerance) {
        return abs(a - b) <= tolerance;
    }

    // 返回在 a 和 b 之间线性百分比的距离，本质上是均匀分布的累积分布函数，但可以是负数或大于1
    // 用于关键帧时间戳插值
    float Math::LinearPercent(float a, float b, float t) {
        return Equals(a, b) ? 1.0f : (t - a) / (b - a);
    }

    // 返回两个浮点数的线性混合值
    float Math::Lerp(float a, float b, float t) {
        return glm::lerp(a, b, t);
    }

    // 执行 Hermite 平滑插值，返回范围在 [0, 1] 的百分比
    // 参考 https://en.wikipedia.org/wiki/Smoothstep
    float Math::SmoothStep(float a, float b, float t) {
        t = std::clamp((t - a) / (b - a), 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }

    // 执行二阶平滑插值，返回范围在 [0, 1] 的百分比
    // 参考 https://en.wikipedia.org/wiki/Smoothstep
    float Math::SmootherStep(float a, float b, float t) {
        t = std::clamp((t - a) / (b - a), 0.0f, 1.0f);
        return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }

    // 根据每秒的百分比和增量时间返回一个与帧率无关的 t 值，用于 lerp/slerp
    float Math::EasePercent(float percent_per_second, float delta_time) {
        return 1.0f - pow(1.0f - percent_per_second, delta_time);
    }

    // 根据锐度水平和增量时间返回一个与帧率无关的 t 值，用于 lerp/slerp
    float Math::EaseFactor(float sharpness, float delta_time) {
        return 1.0f - exp(-sharpness * delta_time);
    }

    // 返回一个在 0.0 和 k 之间反弹的浮点数，随着 x 的变化单调递增
    float Math::Bounce(float x, float k) {
        return k - fabs(k - fmodf(x, k * 2));
    }

    // 返回两个向量的线性混合值
    vec2 Math::Lerp(const vec2& a, const vec2& b, float t) { return glm::lerp(a, b, t); }
    vec3 Math::Lerp(const vec3& a, const vec3& b, float t) { return glm::lerp(a, b, t); }
    vec4 Math::Lerp(const vec4& a, const vec4& b, float t) { return glm::lerp(a, b, t); }

    // 执行两个四元数之间的球面插值，以最短路径为准
    quat Math::Slerp(const quat& a, const quat& b, float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        return glm::normalize(glm::slerp(a, b, t));
    }

    // 执行两个四元数之间的球面插值，以定向路径为准
    quat Math::SlerpRaw(const quat& a, const quat& b, float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        return glm::normalize(glm::mix(a, b, t));
    }

    // 从HSL创建RGB颜色，参考 https://en.wikipedia.org/wiki/HSL_and_HSV
    vec3 Math::HSL2RGB(float h, float s, float l) {
        vec3 rgb = clamp(abs(mod(h * 6.0f + vec3(0.0f, 4.0f, 2.0f), 6.0f) - 3.0f) - 1.0f, 0.0f, 1.0f);
        return l + s * (rgb - 0.5f) * (1.0f - abs(2.0f * l - 1.0f));
    }

    // 从HSV创建RGB颜色，参考 https://en.wikipedia.org/wiki/HSL_and_HSV
    vec3 Math::HSV2RGB(float h, float s, float v) {
        if (s <= std::numeric_limits<float>::epsilon()) {
            return vec3(v);  // 零饱和度 = 灰度颜色
        }

        h = fmodf(h, 1.0f) * 6.0f;
        int i = static_cast<int>(h);

        float f = h - static_cast<float>(i);
        float p = v * (1.0f - s);
        float q = v * (1.0f - s * f);
        float t = v * (1.0f - s * (1.0f - f));

        switch (i) {
        case 0: return vec3(v, t, p);
        case 1: return vec3(q, v, p);
        case 2: return vec3(p, v, t);
        case 3: return vec3(p, q, v);
        case 4: return vec3(t, p, v);
        case 5: default: {
            return vec3(v, p, q);
        }
        }
    }

    // 从HSL向量创建RGB颜色
    vec3 Math::HSL2RGB(const vec3& hsl) {
        return HSL2RGB(hsl.x, hsl.y, hsl.z);
    }

    // 从HSV向量创建RGB颜色
    vec3 Math::HSV2RGB(const vec3& hsv) {
        return HSV2RGB(hsv.x, hsv.y, hsv.z);
    }
}
