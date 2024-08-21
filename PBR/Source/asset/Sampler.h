#pragma once
#include "../asset/Asset.h"
#include <cstdint>
namespace asset {
    // 预设的过滤模式枚举
    enum class FilterMode : uint8_t {
        Point,      // 点采样
        Bilinear,   // 双线性过滤
        Trilinear   // 三线性过滤
    };

    // 采样器
	class Sampler : public Asset {
    public:
        Sampler(FilterMode mode = FilterMode::Point);
        ~Sampler();

        // 禁用拷贝构造函数和赋值运算符
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

