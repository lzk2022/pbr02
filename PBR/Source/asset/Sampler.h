#pragma once
#include "../asset/Asset.h"
#include <cstdint>
namespace asset {
    // Ԥ��Ĺ���ģʽö��
    enum class FilterMode : uint8_t {
        Point,      // �����
        Bilinear,   // ˫���Թ���
        Trilinear   // �����Թ���
    };

    // ������
	class Sampler : public Asset {
    public:
        Sampler(FilterMode mode = FilterMode::Point);
        ~Sampler();

        // ���ÿ������캯���͸�ֵ�����
        Sampler(const Sampler&) = delete;
        Sampler& operator=(const Sampler&) = delete;
        Sampler(Sampler&& other) noexcept = default;
        Sampler& operator=(Sampler&& other) noexcept = default;

    public:
        void Bind(GLuint index) const override;
        void UnBind(GLuint index) const override;

        template<typename T>
        void SetParam(GLenum name, T value) const;
        template<typename T>
        void SetParam(GLenum name, const T* value) const;
	};
    
}

#include "Sampler.hpp"

