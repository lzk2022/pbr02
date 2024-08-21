#include "../utils/Log.h"
namespace component {

    template<typename T, typename>
    void Material::SetUniform(GLuint location, const T& value) 
    {
        LOG_TRACK;
        // �����uniformλ�ô����ڲ��ʵ�uniform�б��У���������ֵ
        if (mUniforms.count(location) > 0) {
            auto& unifVariant = mUniforms[location];
            LOG_ASSERT(std::holds_alternative<Uniform<T>>(unifVariant), "��ƥ��� uniform ���ͣ�");
            std::get<Uniform<T>>(unifVariant) << value;
        }
    }

    template<typename T, typename>
    void Material::SetUniform(pbr_u attribute, const T& value) {
        LOG_TRACK;
        SetUniform<T>(static_cast<GLuint>(attribute), value);
    }

    template<typename T, typename>
    void Material::BindUniform(GLuint location, const T* pValue) {
        LOG_TRACK;
        // �����uniformλ�ô����ڲ��ʵ�uniform�б��У����ָ��ָ���ֵ
        if (mUniforms.count(location) > 0) {
            auto& unifVariant = mUniforms[location];
            LOG_ASSERT(std::holds_alternative<Uniform<T>>(unifVariant), "��ƥ��� uniform ���ͣ�");
            std::get<Uniform<T>>(unifVariant) <<= pValue;
        }
    }

    template<typename T, typename>
    void Material::BindUniform(pbr_u attribute, const T* pValue) {
        LOG_TRACK;
        BindUniform<T>(static_cast<GLuint>(attribute), pValue);
    }

    // ����һ��uniform�����ֵ
    template<typename T, typename>
    void Material::SetUniformArray(GLuint location, GLuint size, const std::vector<T>* pArray) {
        LOG_TRACK;
        // �����uniformλ�ô����ڲ��ʵ�uniform�б��У������������ֵ
        if (mUniforms.count(location) > 0) {
            auto& unifVariant = mUniforms[location];
            LOG_ASSERT(std::holds_alternative<Uniform<T>>(unifVariant), "��ƥ��� uniform ���ͣ�");
            auto& uniform = std::get<Uniform<T>>(unifVariant);
            uniform.mSize = size;
            uniform <<= pArray;
        }
    }

    // ��ʽģ��ʵ������ʹ��X��
#define INSTANTIATE_TEMPLATE(T) \
    template class Uniform<T>; \
    template void Material::SetUniform<T>(GLuint location, const T& value); \
    template void Material::SetUniform<T>(pbr_u attribute, const T& value); \
    template void Material::BindUniform<T>(GLuint location, const T* pValue); \
    template void Material::BindUniform<T>(pbr_u attribute, const T* pValue); \
    template void Material::SetUniformArray(GLuint location, GLuint size, const std::vector<T>* pArray);

    // ʵ���������ģ������
    INSTANTIATE_TEMPLATE(int)
    INSTANTIATE_TEMPLATE(GLuint)
    INSTANTIATE_TEMPLATE(bool)
    INSTANTIATE_TEMPLATE(float)
    INSTANTIATE_TEMPLATE(vec2)
    INSTANTIATE_TEMPLATE(vec3)
    INSTANTIATE_TEMPLATE(vec4)
    INSTANTIATE_TEMPLATE(mat2)
    INSTANTIATE_TEMPLATE(mat3)
    INSTANTIATE_TEMPLATE(mat4)
    INSTANTIATE_TEMPLATE(ivec2)
    INSTANTIATE_TEMPLATE(ivec3)
    INSTANTIATE_TEMPLATE(ivec4)
    INSTANTIATE_TEMPLATE(uvec2)
    INSTANTIATE_TEMPLATE(uvec3)
    INSTANTIATE_TEMPLATE(uvec4)

#undef INSTANTIATE_TEMPLATE
}
