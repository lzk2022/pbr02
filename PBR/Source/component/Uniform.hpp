#include "../utils/Log.h"
namespace component {
    // 判断是否为合法的GLSL uniform类型，包括基本类型和32位浮点向量/矩阵
    template<typename T>
    struct is_glsl_t {
        using type = typename std::enable_if_t<
            std::is_same_v<T, int> ||
            std::is_same_v<T, bool> ||
            std::is_same_v<T, float> ||
            std::is_same_v<T, GLuint> ||
            std::is_same_v<T, vec2> || std::is_same_v<T, uvec2> || std::is_same_v<T, ivec2> ||
            std::is_same_v<T, vec3> || std::is_same_v<T, uvec3> || std::is_same_v<T, ivec3> ||
            std::is_same_v<T, vec4> || std::is_same_v<T, uvec4> || std::is_same_v<T, ivec4> ||
            std::is_same_v<T, mat2> ||
            std::is_same_v<T, mat3> ||
            std::is_same_v<T, mat4>
        >;
    };

    template<typename T>
    Uniform<T>::Uniform(GLuint ownerId, GLuint location, const char* name) 
        :mOwnerId(ownerId), mLocation(location), mName(name),
         mValue(0),mSize(1),mpValue(nullptr),mpArray(nullptr)
    {
        //mSize = 1;
        //mValue = 0;
        //mpValue = nullptr;
        //mpArray = nullptr;
    }

    template<typename T>
    void Uniform<T>::operator<<(const T& value) 
    {
        mValue = value;
    }

    template<typename T>
    void Uniform<T>::operator<<=(const T* pValue) {
        mpValue = pValue;
    }

    template<typename T>
    void Uniform<T>::operator<<=(const std::vector<T>* pAarray) {
        mpArray = pAarray;
    }

    template<typename T>
    void Uniform<T>::Upload(T val, GLuint index) const {
        const GLuint& id = mOwnerId;
        const GLuint& lc = mLocation + index;

        // 根据Uniform的类型，调用不同的OpenGL函数上传值
        if constexpr (std::is_same_v<T, bool>)          { glProgramUniform1i(id, lc, static_cast<int>(val)); }
        else if constexpr (std::is_same_v<T, int>)      { glProgramUniform1i(id, lc, val); }
        else if constexpr (std::is_same_v<T, float>)    { glProgramUniform1f(id, lc, val); }
        else if constexpr (std::is_same_v<T, GLuint>)   { glProgramUniform1ui(id, lc, val); }
        else if constexpr (std::is_same_v<T, vec2>)     { glProgramUniform2fv(id, lc, 1, &val[0]); }
        else if constexpr (std::is_same_v<T, vec3>)     { glProgramUniform3fv(id, lc, 1, &val[0]); }
        else if constexpr (std::is_same_v<T, vec4>)     { glProgramUniform4fv(id, lc, 1, &val[0]); }
        else if constexpr (std::is_same_v<T, ivec2>)    { glProgramUniform2iv(id, lc, 1, &val[0]); }
        else if constexpr (std::is_same_v<T, ivec3>)    { glProgramUniform3iv(id, lc, 1, &val[0]); }
        else if constexpr (std::is_same_v<T, ivec4>)    { glProgramUniform4iv(id, lc, 1, &val[0]); }
        else if constexpr (std::is_same_v<T, uvec2>)    { glProgramUniform2uiv(id, lc, 1, &val[0]); }
        else if constexpr (std::is_same_v<T, uvec3>)    { glProgramUniform3uiv(id, lc, 1, &val[0]); }
        else if constexpr (std::is_same_v<T, uvec4>)    { glProgramUniform4uiv(id, lc, 1, &val[0]); }
        else if constexpr (std::is_same_v<T, mat2>)     { glProgramUniformMatrix2fv(id, lc, 1, GL_FALSE, &val[0][0]); }
        else if constexpr (std::is_same_v<T, mat3>)     { glProgramUniformMatrix3fv(id, lc, 1, GL_FALSE, &val[0][0]); }
        else if constexpr (std::is_same_v<T, mat4>)     { glProgramUniformMatrix4fv(id, lc, 1, GL_FALSE, &val[0][0]); }
        else                                            { LOG_ASSERT_TRUE(true, "未指定的模板Uniform类型T"); }
    }

    template<typename T>
    void Uniform<T>::Upload() const {
        if (mSize == 1) {
            T val = mpValue ? *(mpValue) : mValue;
            Upload(val);
            return;
        }
        for (GLuint i = 0; i < mSize; ++i) {
            T val = (*mpArray)[i];
            Upload(val, i);
        }
    }
}
