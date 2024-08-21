#pragma once
#include <type_traits>
#include <glm/glm.hpp>
#include <glad/glad.h>
namespace utils {
	class Math {
    public:
        // �ж������Ƿ���2����
        template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
        static constexpr bool IsPowerOfTwo(T value) {  // ��ʽ��������
            return value != 0 && (value & (value - 1)) == 0;
        }
        static GLuint CalMipmap(GLuint w, GLuint h);
        // ����ָ�����͵�����������������ֵ
        template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
        [[nodiscard]] static T RandomGenerator();

        // �Ƚ����� glm::vec3 �����Ƿ��ڸ��� epsilon ��Χ�����
        static bool Equals(const glm::vec3& a, const glm::vec3& b, float epsilon = 0.000001f);

        // �Ƚ����� glm::quat ��Ԫ���Ƿ��ڸ��� epsilon ��Χ�����
        static bool Equals(const glm::quat& a, const glm::quat& b, float epsilon = 0.000001f);

        // �Ƚ������������Ƿ��ڸ��� tolerance ��Χ�ڽ������
        static bool Equals(float a, float b, float tolerance = std::numeric_limits<float>::epsilon());

        // ������ a �� b ֮�����԰ٷֱȵľ���
        static float LinearPercent(float a, float b, float t);

        // �������������������Բ�ֵֵ
        static float Lerp(float a, float b, float t);

        // ִ�� Hermite ƽ����ֵ�����ط�Χ�� [0, 1] �İٷֱ�
        static float SmoothStep(float a, float b, float t);

        // ִ�ж���ƽ����ֵ�����ط�Χ�� [0, 1] �İٷֱ�
        static float SmootherStep(float a, float b, float t);

        // ����ÿ��İٷֱȺ�����ʱ�䷵��һ����֡���޹ص� t ֵ������ lerp/slerp
        static float EasePercent(float percent_per_second, float delta_time);

        // �������ˮƽ������ʱ�䷵��һ����֡���޹ص� t ֵ������ lerp/slerp
        static float EaseFactor(float sharpness, float delta_time);

        // ����һ���� 0.0 �� k ֮�䷴���ĸ����������� x �ı仯��������
        static float Bounce(float x, float k);

        // �������� glm::vec2 ���������Բ�ֵֵ
        static glm::vec2 Lerp(const glm::vec2& a, const glm::vec2& b, float t);

        // �������� glm::vec3 ���������Բ�ֵֵ
        static glm::vec3 Lerp(const glm::vec3& a, const glm::vec3& b, float t);

        // �������� glm::vec4 ���������Բ�ֵֵ
        static glm::vec4 Lerp(const glm::vec4& a, const glm::vec4& b, float t);

        // ִ������ glm::quat ��Ԫ��֮��������ֵ�������·��Ϊ׼
        static glm::quat Slerp(const glm::quat& a, const glm::quat& b, float t);

        // ִ������ glm::quat ��Ԫ��֮��������ֵ���Զ���·��Ϊ׼
        static glm::quat SlerpRaw(const glm::quat& a, const glm::quat& b, float t);

        // ���� HSL ɫ��ģ�ʹ��� RGB ��ɫ
        static glm::vec3 HSL2RGB(float h, float s, float l);

        // ���� HSV ɫ��ģ�ʹ��� RGB ��ɫ
        static glm::vec3 HSV2RGB(float h, float s, float v);

        // ���� HSL �������� RGB ��ɫ
        static glm::vec3 HSL2RGB(const glm::vec3& hsl);

        // ���� HSV �������� RGB ��ɫ
        static glm::vec3 HSV2RGB(const glm::vec3& hsv);
	};
}

