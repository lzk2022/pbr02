#include "Math.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>                   // GLM��OpenGL��ѧ��
#include <glm/ext.hpp>                   // GLM��OpenGL��ѧ�� ��չ
#include <glm/gtc/quaternion.hpp>        // GLM��OpenGL��ѧ�� ��Ԫ��
#include <glm/gtx/quaternion.hpp>        // GLM��OpenGL��ѧ�� ��Ԫ�� ��չ
#include <glm/gtx/transform.hpp>         // GLM��OpenGL��ѧ�� �任
#include <glm/gtc/type_ptr.hpp>          // GLM��OpenGL��ѧ�� ָ��
#include <glm/gtx/string_cast.hpp>       // GLM��OpenGL��ѧ�� �ַ���ת��
#include <glm/gtc/matrix_transform.hpp>  // GLM��OpenGL��ѧ�� ����任
#include <glm/gtx/perpendicular.hpp>     // GLM��OpenGL��ѧ�� ��ֱ����
#include <GLM/gtx/quaternion.hpp>
#include "../utils/Log.h"
#include <cmath>
#include <limits>
#include <random>
using namespace glm;
namespace utils
{
    // ��̬��������������漰״̬����
    static std::random_device gDevice;               // ���豸��ȡ����
    static std::mt19937       gEngine32(gDevice());  // ������ʱ������һ��
    static std::mt19937_64    gEngine64(gDevice());  // ������ʱ������һ��

    // ���쾲̬���ȷֲ���
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
            LOG_EXCEPTION_TRUE(true, "��֧�ֵ��������� T {0}",T);
        }
    }

    // ��ʽģ�庯��ʵ����
    template uint64_t Math::RandomGenerator();
    template uint32_t Math::RandomGenerator();
    template double   Math::RandomGenerator();
    template float    Math::RandomGenerator();

    // �Ƚ����������Ƿ��ڸ�����ֵ epsilon �����
    bool Math::Equals(const vec3& a, const vec3& b, float epsilon) {
        return glm::length2(a - b) < epsilon;  // ʹ�� `length2` ���Ա��� `sqrt()` ����
    }

    // �Ƚ�������Ԫ���Ƿ��ڸ�����ֵ epsilon �����
    // �������������� epsilon Ӧ�ø����ɣ����� slerp ������������
    bool Math::Equals(const quat& a, const quat& b, float epsilon) {
        return abs(glm::dot(a, b) - 1.0f) < epsilon;
    }

    // ���������������Ƿ��ڸ������� tolerance �ڽ������
    bool Math::Equals(float a, float b, float tolerance) {
        return abs(a - b) <= tolerance;
    }

    // ������ a �� b ֮�����԰ٷֱȵľ��룬�������Ǿ��ȷֲ����ۻ��ֲ��������������Ǹ��������1
    // ���ڹؼ�֡ʱ�����ֵ
    float Math::LinearPercent(float a, float b, float t) {
        return Equals(a, b) ? 1.0f : (t - a) / (b - a);
    }

    // �������������������Ի��ֵ
    float Math::Lerp(float a, float b, float t) {
        return glm::lerp(a, b, t);
    }

    // ִ�� Hermite ƽ����ֵ�����ط�Χ�� [0, 1] �İٷֱ�
    // �ο� https://en.wikipedia.org/wiki/Smoothstep
    float Math::SmoothStep(float a, float b, float t) {
        t = std::clamp((t - a) / (b - a), 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }

    // ִ�ж���ƽ����ֵ�����ط�Χ�� [0, 1] �İٷֱ�
    // �ο� https://en.wikipedia.org/wiki/Smoothstep
    float Math::SmootherStep(float a, float b, float t) {
        t = std::clamp((t - a) / (b - a), 0.0f, 1.0f);
        return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }

    // ����ÿ��İٷֱȺ�����ʱ�䷵��һ����֡���޹ص� t ֵ������ lerp/slerp
    float Math::EasePercent(float percent_per_second, float delta_time) {
        return 1.0f - pow(1.0f - percent_per_second, delta_time);
    }

    // �������ˮƽ������ʱ�䷵��һ����֡���޹ص� t ֵ������ lerp/slerp
    float Math::EaseFactor(float sharpness, float delta_time) {
        return 1.0f - exp(-sharpness * delta_time);
    }

    // ����һ���� 0.0 �� k ֮�䷴���ĸ����������� x �ı仯��������
    float Math::Bounce(float x, float k) {
        return k - fabs(k - fmodf(x, k * 2));
    }

    // �����������������Ի��ֵ
    vec2 Math::Lerp(const vec2& a, const vec2& b, float t) { return glm::lerp(a, b, t); }
    vec3 Math::Lerp(const vec3& a, const vec3& b, float t) { return glm::lerp(a, b, t); }
    vec4 Math::Lerp(const vec4& a, const vec4& b, float t) { return glm::lerp(a, b, t); }

    // ִ��������Ԫ��֮��������ֵ�������·��Ϊ׼
    quat Math::Slerp(const quat& a, const quat& b, float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        return glm::normalize(glm::slerp(a, b, t));
    }

    // ִ��������Ԫ��֮��������ֵ���Զ���·��Ϊ׼
    quat Math::SlerpRaw(const quat& a, const quat& b, float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        return glm::normalize(glm::mix(a, b, t));
    }

    // ��HSL����RGB��ɫ���ο� https://en.wikipedia.org/wiki/HSL_and_HSV
    vec3 Math::HSL2RGB(float h, float s, float l) {
        vec3 rgb = clamp(abs(mod(h * 6.0f + vec3(0.0f, 4.0f, 2.0f), 6.0f) - 3.0f) - 1.0f, 0.0f, 1.0f);
        return l + s * (rgb - 0.5f) * (1.0f - abs(2.0f * l - 1.0f));
    }

    // ��HSV����RGB��ɫ���ο� https://en.wikipedia.org/wiki/HSL_and_HSV
    vec3 Math::HSV2RGB(float h, float s, float v) {
        if (s <= std::numeric_limits<float>::epsilon()) {
            return vec3(v);  // �㱥�Ͷ� = �Ҷ���ɫ
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

    // ��HSL��������RGB��ɫ
    vec3 Math::HSL2RGB(const vec3& hsl) {
        return HSL2RGB(hsl.x, hsl.y, hsl.z);
    }

    // ��HSV��������RGB��ɫ
    vec3 Math::HSV2RGB(const vec3& hsv) {
        return HSV2RGB(hsv.x, hsv.y, hsv.z);
    }
}
