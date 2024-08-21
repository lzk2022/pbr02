#pragma once
#include <limits>
#include <glm/glm.hpp>
#include "../component/Component.h"
namespace component {
	// 光源基类，继承自组件基类，定义了光源的基本属性
	class Light : public Component {
	public:
		Light(const glm::vec3& color, float intensity = 1.0f);

	public:
		glm::vec3 mColor;		// 光源的颜色
		float mIntensity;		// 光照强度
	};

	// 方向光类，继承自光源基类，没有额外的成员或方法
	class DirectionLight : public Light {
	public:
		using Light::Light;		// 继承构造函数
	};

	// 点光源类，继承自光源基类，定义了点光源特有的衰减因子和方法
	class PointLight : public Light {
	public:
		float mLinear, mQuadratic;							// 线性和二次衰减因子
		float mRange = std::numeric_limits<float>::max();	// 光源的最大影响范围，默认为无限大
		using Light::Light;  // 继承构造函数

		// 设置点光源的衰减因子
		void SetAttenuation(float linear, float quadratic);

		// 获取点光源在特定距离下的衰减值
		float GetAttenuation(float distance) const;
	};

	// 聚光灯类，继承自光源基类，定义了聚光灯特有的内外切光角和方法
	class Spotlight : public Light {
	public:
		float mInnerCutoff;   // 内部切光角度，单位为度
		float mOuterCutoff;   // 外部切光角度，单位为度
		float mRange = std::numeric_limits<float>::max();  // 光源的最大影响范围，默认为无限大

		using Light::Light;  // 继承构造函数

	public:
		// 设置聚光灯的切光角
		void SetCutoff(float range, float innerCutoff = 15.0f, float outerCutoff = 30.0f);
		// 获取内部切光角的余弦值
		float GetInnerCosine() const;
		// 获取外部切光角的余弦值
		float GetOuterCosine() const;
		// 获取聚光灯在特定距离下的衰减值
		float GetAttenuation(float distance) const;
	};

	// 区域光类，继承自光源基类，目前待实现
	class AreaLight : public Light {
	public:
		using Light::Light;
		//todo 使用贝塞尔曲线采样和LTC
	};

	// 体积光类，继承自光源基类，目前待实现，需要体积光路径追踪
	class VolumeLight : public Light {
	public:
		using Light::Light;
		// todo 需要体积光路径追踪
	};
}
