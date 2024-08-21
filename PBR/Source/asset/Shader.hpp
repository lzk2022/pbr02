#include <glm/glm.hpp>                   // GLM：OpenGL数学库
#include "../utils/Log.h"
namespace asset {
	template<typename T>
	void Shader::SetUniform(GLuint location, const T& val) const {
		using namespace glm;
		LOG_TRACK;
		// 根据模板类型设置不同类型的 uniform 变量
		if constexpr (std::is_same_v<T, bool>) { glProgramUniform1i(mId, location, static_cast<int>(val)); }
		else if constexpr (std::is_same_v<T, int>) { glProgramUniform1i(mId, location, val); }
		else if constexpr (std::is_same_v<T, float>) { glProgramUniform1f(mId, location, val); }
		else if constexpr (std::is_same_v<T, GLuint>) { glProgramUniform1ui(mId, location, val); }
		else if constexpr (std::is_same_v<T, vec2>) { glProgramUniform2fv(mId, location, 1, &val[0]); }
		else if constexpr (std::is_same_v<T, vec3>) { glProgramUniform3fv(mId, location, 1, &val[0]); }
		else if constexpr (std::is_same_v<T, vec4>) { glProgramUniform4fv(mId, location, 1, &val[0]); }
		else if constexpr (std::is_same_v<T, mat2>) { glProgramUniformMatrix2fv(mId, location, 1, GL_FALSE, &val[0][0]); }
		else if constexpr (std::is_same_v<T, mat3>) { glProgramUniformMatrix3fv(mId, location, 1, GL_FALSE, &val[0][0]); }
		else if constexpr (std::is_same_v<T, mat4>) { glProgramUniformMatrix4fv(mId, location, 1, GL_FALSE, &val[0][0]); }
		else {
			LOG_EXCEPTION(false, "未指定的模板统一类型 T");
		}
	}

	template<typename T>
	void Shader::SetUniformArray(GLuint location, GLuint size, const std::vector<T>& vec) const {
		LOG_TRACK;
		// 设置 uniform 数组
		for (GLuint i = 0; i < size; ++i) {
			const T& val = vec[i];
			SetUniform(location + i, val);
		}
	}
}