#include "Light.h"
#include "../utils/Log.h"
namespace component {
	Light::Light(const glm::vec3& color, float intensity)
		: Component(),mColor(color),mIntensity(intensity)
	{

	}
	void PointLight::SetAttenuation(float linear, float quadratic){
        LOG_TRACK;
        LOG_ASSERT(linear > 0, "����˥�����ӱ���Ϊ����...");
        LOG_ASSERT(quadratic > 0, "����˥�����ӱ���Ϊ����...");

        mLinear = linear;
        mQuadratic = quadratic;

        // ͨ�������η������ҵ����Χ����˥��С�ڵ���0.01ʱ
        float a = quadratic;
        float b = linear;
        float c = -100.0f;
        float delta = b * b - 4.0f * a * c;

        LOG_ASSERT(delta > 0.0f, "����Զ���������У���ѧ���ǲ����ܵ�...");
        mRange = c / (-0.5f * (b + sqrt(delta)));  // ���շ���Muller's Method��
	}

    float PointLight::GetAttenuation(float distance) const{
        LOG_TRACK;
        LOG_ASSERT(distance >= 0.0f, "����Դ�ľ��벻��Ϊ��");
        if (distance >= mRange) return 0.0f;
        else return 1.0f / (1.0f + mLinear * distance + mQuadratic * pow(distance, 2.0f));
    }
    void Spotlight::SetCutoff(float range, float innerCutoff, float outerCutoff){
        LOG_TRACK;
        LOG_ASSERT(range > 0, "�۹�Ʒ�Χ����Ϊ����");
        LOG_ASSERT(innerCutoff > 0, "�ڲ��й�Ƕȱ���Ϊ����");
        LOG_ASSERT(outerCutoff > 0, "�ⲿ�й�Ƕȱ���Ϊ����");

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
        // ����������ǵľ۹�Ʋ���ѭ������ƽ�����ɣ�����ʹ������˥����
        // ������ˣ�������Ȼ�д���׶�嵽��׶���ذ뾶�ĵ���Ч�����������Ч���㹻��ʵ����ʧ��ʵ�С�
        LOG_ASSERT(distance >= 0.0f, "����Դ�ľ��벻��Ϊ��");
        return 1.0f - std::clamp(distance / mRange, 0.0f, 1.0f);
    }
}
