#include <type_traits>
#include "../utils/Log.h"
namespace asset {
    template<typename T>
    inline void Sampler::SetParam(GLenum name, T value) const {
        LOG_TRACK;
        // 设置采样器对象的参数，支持不同类型的参数设置
        if constexpr (std::is_same_v<T, int>) {
            glSamplerParameteri(mId, name, value);
        }
        else if constexpr (std::is_same_v<T, float>) {
            glSamplerParameterf(mId, name, value);
        }
    }

    template<typename T>
    inline void Sampler::SetParam(GLenum name, const T* value) const{
        LOG_TRACK;
        // 设置采样器对象的参数，支持不同类型的参数设置（数组版本）
        if constexpr (std::is_same_v<T, int>) {
            glSamplerParameteriv(mId, name, value);
        }
        else if constexpr (std::is_same_v<T, float>) {
            glSamplerParameterfv(mId, name, value);
        }
    }

    // 显式实例化模板函数，使用X宏
    #define INSTANTIATE_TEMPLATE(T) \
        template void Sampler::SetParam<T>(GLenum name, T value) const; \
        template void Sampler::SetParam<T>(GLenum name, const T* value) const;

    INSTANTIATE_TEMPLATE(int)
    INSTANTIATE_TEMPLATE(float)

    #undef INSTANTIATE_TEMPLATE
}
