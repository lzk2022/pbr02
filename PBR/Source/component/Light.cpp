#include "Light.h"
#include "../utils/Log.h"
namespace component {
	Light::Light(const glm::vec3& color, float intensity)
		: Component(),mColor(color),mIntensity(intensity)
	{

	}
	void PointLight::SetAttenuation(float linear, float quadratic){
        LOG_TRACK;
        LOG_ASSERT(linear > 0, "线性衰减因子必须为正数...");
        LOG_ASSERT(quadratic > 0, "二次衰减因子必须为正数...");

        mLinear = linear;
        mQuadratic = quadratic;

        // 通过求解二次方程来找到最大范围，当衰减小于等于0.01时
        float a = quadratic;
        float b = linear;
        float c = -100.0f;
        float delta = b * b - 4.0f * a * c;

        LOG_ASSERT(delta > 0.0f, "你永远看不到这行，数学上是不可能的...");
        mRange = c / (-0.5f * (b + sqrt(delta)));  // 穆勒法（Muller's Method）
	}

    float PointLight::GetAttenuation(float distance) const{
        LOG_TRACK;
        LOG_ASSERT(distance >= 0.0f, "到光源的距离不能为负");
        if (distance >= mRange) return 0.0f;
        else return 1.0f / (1.0f + mLinear * distance + mQuadratic * pow(distance, 2.0f));
    }
    void Spotlight::SetCutoff(float range, float innerCutoff, float outerCutoff){
        LOG_TRACK;
        LOG_ASSERT(range > 0, "聚光灯范围必须为正数");
        LOG_ASSERT(innerCutoff > 0, "内部切光角度必须为正数");
        LOG_ASSERT(outerCutoff > 0, "外部切光角度必须为正数");

        mRange = range;
        mInnerCutoff = innerCutoff;
        mOuterCutoff = outerCutoff;
    }
    float Spotlight::GetInnerCosine() const
    {
        return glm::cos(glm::radians(mInnerCutoff));
    }
    float Spotlight::GetOuterCosine() const
    {
        return glm::cos(glm::radians(mOuterCutoff));
    }
    float Spotlight::GetAttenuation(float distance) const
    {
        // 简单起见，我们的聚光灯不遵循反比例平方定律，而是使用线性衰减。
        // 尽管如此，我们仍然有从内锥体到外锥体沿半径的淡出效果，因此整体效果足够真实而不失真实感。
        LOG_ASSERT(distance >= 0.0f, "到光源的距离不能为负");
        return 1.0f - std::clamp(distance / mRange, 0.0f, 1.0f);
    }
}
