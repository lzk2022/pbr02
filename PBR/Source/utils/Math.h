#pragma once
#include <type_traits>
#include <glm/glm.hpp>
#include <glad/glad.h>
namespace utils {
	class Math {
    public:
        // 判断整数是否是2的幂
        template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
        static constexpr bool IsPowerOfTwo(T value) {  // 隐式内联函数
            return value != 0 && (value & (value - 1)) == 0;
        }
        static GLuint CalMipmap(GLuint w, GLuint h);
        // 生成指定类型的随机数，返回随机数值
        template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
        [[nodiscard]] static T RandomGenerator();

        // 比较两个 glm::vec3 向量是否在给定 epsilon 范围内相等
        static bool Equals(const glm::vec3& a, const glm::vec3& b, float epsilon = 0.000001f);

        // 比较两个 glm::quat 四元数是否在给定 epsilon 范围内相等
        static bool Equals(const glm::quat& a, const glm::quat& b, float epsilon = 0.000001f);

        // 比较两个浮点数是否在给定 tolerance 范围内近似相等
        static bool Equals(float a, float b, float tolerance = std::numeric_limits<float>::epsilon());

        // 返回在 a 和 b 之间线性百分比的距离
        static float LinearPercent(float a, float b, float t);

        // 返回两个浮点数的线性插值值
        static float Lerp(float a, float b, float t);

        // 执行 Hermite 平滑插值，返回范围在 [0, 1] 的百分比
        static float SmoothStep(float a, float b, float t);

        // 执行二阶平滑插值，返回范围在 [0, 1] 的百分比
        static float SmootherStep(float a, float b, float t);

        // 根据每秒的百分比和增量时间返回一个与帧率无关的 t 值，用于 lerp/slerp
        static float EasePercent(float percent_per_second, float delta_time);

        // 根据锐度水平和增量时间返回一个与帧率无关的 t 值，用于 lerp/slerp
        static float EaseFactor(float sharpness, float delta_time);

        // 返回一个在 0.0 和 k 之间反弹的浮点数，随着 x 的变化单调递增
        static float Bounce(float x, float k);

        // 返回两个 glm::vec2 向量的线性插值值
        static glm::vec2 Lerp(const glm::vec2& a, const glm::vec2& b, float t);

        // 返回两个 glm::vec3 向量的线性插值值
        static glm::vec3 Lerp(const glm::vec3& a, const glm::vec3& b, float t);

        // 返回两个 glm::vec4 向量的线性插值值
        static glm::vec4 Lerp(const glm::vec4& a, const glm::vec4& b, float t);

        // 执行两个 glm::quat 四元数之间的球面插值，以最短路径为准
        static glm::quat Slerp(const glm::quat& a, const glm::quat& b, float t);

        // 执行两个 glm::quat 四元数之间的球面插值，以定向路径为准
        static glm::quat SlerpRaw(const glm::quat& a, const glm::quat& b, float t);

        // 根据 HSL 色彩模型创建 RGB 颜色
        static glm::vec3 HSL2RGB(float h, float s, float l);

        // 根据 HSV 色彩模型创建 RGB 颜色
        static glm::vec3 HSV2RGB(float h, float s, float v);

        // 根据 HSL 向量创建 RGB 颜色
        static glm::vec3 HSL2RGB(const glm::vec3& hsl);

        // 根据 HSV 向量创建 RGB 颜色
        static glm::vec3 HSV2RGB(const glm::vec3& hsv);
	};
}

