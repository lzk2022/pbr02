#include <type_traits>
#include "../utils/Log.h"
namespace asset {
    template<typename T>
    inline void Sampler::SetParam(GLenum name, T value) const {
        LOG_TRACK;
        // ���ò���������Ĳ�����֧�ֲ�ͬ���͵Ĳ�������
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
        // ���ò���������Ĳ�����֧�ֲ�ͬ���͵Ĳ������ã�����汾��
        if constexpr (std::is_same_v<T, int>) {
            glSamplerParameteriv(mId, name, value);
        }
        else if constexpr (std::is_same_v<T, float>) {
            glSamplerParameterfv(mId, name, value);
        }
    }

    // ��ʽʵ����ģ�庯����ʹ��X��
    #define INSTANTIATE_TEMPLATE(T) \
        template void Sampler::SetParam<T>(GLenum name, T value) const; \
        template void Sampler::SetParam<T>(GLenum name, const T* value) const;

    INSTANTIATE_TEMPLATE(int)
    INSTANTIATE_TEMPLATE(float)

    #undef INSTANTIATE_TEMPLATE
}
