#pragma once
#include "../utils/Math.h"
#include "../utils/Ext.h"
namespace component {
	using namespace utils;
	class Component {
	public:
		Component()
		{
			mId = Math::RandomGenerator<uint64_t>();
			mEnable = true;
		}

		virtual ~Component() {};

		Component(const Component&) = default;
		Component& operator=(const Component&) = default;
		Component(Component&& other) noexcept = default;
		Component& operator=(Component&& other) noexcept = default;

		// 获取组件的唯一ID
		uint64_t ID() const { return mId; }

		// 隐式转换为bool类型，用于检查组件是否启用
		explicit operator bool() const { return mEnable; }

		// 启用组件
		void Enable() { mEnable = true; }

		// 禁用组件
		void Disable() { mEnable = false; }
	protected:
		uint64_t mId;			// 全局唯一实例ID
		bool mEnable;			// 组件是否启用的标志
	};

    // 标签枚举，用于标识实体的不同角色或类型
    enum class ETag : uint16_t {
        Untagged = 1 << 0,  // 无标签
        Static = 1 << 1,  // 静态物体
        MainCamera = 1 << 2,  // 主摄像机
        WorldPlane = 1 << 3,  // 世界平面
        Skybox = 1 << 4,  // 天空盒
        Water = 1 << 5,  // 水体
        Particle = 1 << 6   // 粒子系统
    };

    // 位运算符重载，用于对标签进行位操作
    inline constexpr ETag operator|(ETag a, ETag b) {
        return static_cast<ETag>(ToIntegral(a) | utils::ToIntegral(b));
    }

    inline constexpr ETag operator&(ETag a, ETag b) {
        return static_cast<ETag>(ToIntegral(a) & ToIntegral(b));
    }

    inline constexpr ETag operator^(ETag a, ETag b) {
        return static_cast<ETag>(ToIntegral(a) ^ ToIntegral(b));
    }

    inline constexpr ETag operator~(ETag a) {
        return static_cast<ETag>(~ToIntegral(a));
    }

    // 位运算符复合赋值重载
    inline ETag& operator|=(ETag& lhs, ETag rhs) { return lhs = lhs | rhs; }
    inline ETag& operator&=(ETag& lhs, ETag rhs) { return lhs = lhs & rhs; }
    inline ETag& operator^=(ETag& lhs, ETag rhs) { return lhs = lhs ^ rhs; }

    // 标签组件类，继承自 Component 类
    class Tag : public Component {
    private:
        ETag tag;  // 标签值

    public:
        // 构造函数，初始化标签值
        explicit Tag(ETag tag) : Component(), tag(tag) {}

        // 添加标签
        void Add(ETag t) { tag |= t; }

        // 删除标签
        void Del(ETag t) { tag &= ~t; }

        // 检查是否包含某个标签
        constexpr bool Contains(ETag t) const {
            return utils::ToIntegral(tag & t) > 0;
        }
    };
}
