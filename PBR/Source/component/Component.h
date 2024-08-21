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

		// ��ȡ�����ΨһID
		uint64_t ID() const { return mId; }

		// ��ʽת��Ϊbool���ͣ����ڼ������Ƿ�����
		explicit operator bool() const { return mEnable; }

		// �������
		void Enable() { mEnable = true; }

		// �������
		void Disable() { mEnable = false; }
	protected:
		uint64_t mId;			// ȫ��Ψһʵ��ID
		bool mEnable;			// ����Ƿ����õı�־
	};

    // ��ǩö�٣����ڱ�ʶʵ��Ĳ�ͬ��ɫ������
    enum class ETag : uint16_t {
        Untagged = 1 << 0,  // �ޱ�ǩ
        Static = 1 << 1,  // ��̬����
        MainCamera = 1 << 2,  // �������
        WorldPlane = 1 << 3,  // ����ƽ��
        Skybox = 1 << 4,  // ��պ�
        Water = 1 << 5,  // ˮ��
        Particle = 1 << 6   // ����ϵͳ
    };

    // λ��������أ����ڶԱ�ǩ����λ����
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

    // λ��������ϸ�ֵ����
    inline ETag& operator|=(ETag& lhs, ETag rhs) { return lhs = lhs | rhs; }
    inline ETag& operator&=(ETag& lhs, ETag rhs) { return lhs = lhs & rhs; }
    inline ETag& operator^=(ETag& lhs, ETag rhs) { return lhs = lhs ^ rhs; }

    // ��ǩ����࣬�̳��� Component ��
    class Tag : public Component {
    private:
        ETag tag;  // ��ǩֵ

    public:
        // ���캯������ʼ����ǩֵ
        explicit Tag(ETag tag) : Component(), tag(tag) {}

        // ��ӱ�ǩ
        void Add(ETag t) { tag |= t; }

        // ɾ����ǩ
        void Del(ETag t) { tag &= ~t; }

        // ����Ƿ����ĳ����ǩ
        constexpr bool Contains(ETag t) const {
            return utils::ToIntegral(tag & t) > 0;
        }
    };
}
