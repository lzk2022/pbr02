#pragma once
#include <limits>
#include <glm/glm.hpp>
#include "../component/Component.h"
namespace component {
	// ��Դ���࣬�̳���������࣬�����˹�Դ�Ļ�������
	class Light : public Component {
	public:
		Light(const glm::vec3& color, float intensity = 1.0f);

	public:
		glm::vec3 mColor;		// ��Դ����ɫ
		float mIntensity;		// ����ǿ��
	};

	// ������࣬�̳��Թ�Դ���࣬û�ж���ĳ�Ա�򷽷�
	class DirectionLight : public Light {
	public:
		using Light::Light;		// �̳й��캯��
	};

	// ���Դ�࣬�̳��Թ�Դ���࣬�����˵��Դ���е�˥�����Ӻͷ���
	class PointLight : public Light {
	public:
		float mLinear, mQuadratic;							// ���ԺͶ���˥������
		float mRange = std::numeric_limits<float>::max();	// ��Դ�����Ӱ�췶Χ��Ĭ��Ϊ���޴�
		using Light::Light;  // �̳й��캯��

		// ���õ��Դ��˥������
		void SetAttenuation(float linear, float quadratic);

		// ��ȡ���Դ���ض������µ�˥��ֵ
		float GetAttenuation(float distance) const;
	};

	// �۹���࣬�̳��Թ�Դ���࣬�����˾۹�����е������й�Ǻͷ���
	class Spotlight : public Light {
	public:
		float mInnerCutoff;   // �ڲ��й�Ƕȣ���λΪ��
		float mOuterCutoff;   // �ⲿ�й�Ƕȣ���λΪ��
		float mRange = std::numeric_limits<float>::max();  // ��Դ�����Ӱ�췶Χ��Ĭ��Ϊ���޴�

		using Light::Light;  // �̳й��캯��

	public:
		// ���þ۹�Ƶ��й��
		void SetCutoff(float range, float innerCutoff = 15.0f, float outerCutoff = 30.0f);
		// ��ȡ�ڲ��й�ǵ�����ֵ
		float GetInnerCosine() const;
		// ��ȡ�ⲿ�й�ǵ�����ֵ
		float GetOuterCosine() const;
		// ��ȡ�۹�����ض������µ�˥��ֵ
		float GetAttenuation(float distance) const;
	};

	// ������࣬�̳��Թ�Դ���࣬Ŀǰ��ʵ��
	class AreaLight : public Light {
	public:
		using Light::Light;
		//todo ʹ�ñ��������߲�����LTC
	};

	// ������࣬�̳��Թ�Դ���࣬Ŀǰ��ʵ�֣���Ҫ�����·��׷��
	class VolumeLight : public Light {
	public:
		using Light::Light;
		// todo ��Ҫ�����·��׷��
	};
}
