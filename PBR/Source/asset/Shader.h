#pragma once
#include "Asset.h"
#include <string>
#include <vector>
namespace asset {
	class Shader : public Asset {
	public:
		Shader() : Asset() {};
		Shader(const std::string& sourcePath);
		Shader(const std::string& binaryPath, GLenum format);
		virtual ~Shader();

		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete; 
		Shader(Shader&& other) = default;
		Shader& operator=(Shader&& other) = default;

	public:
		void Bind() const override;
		void UnBind() const override;
		void Save() const;

		// 检查和显示着色器程序的信息
		void Inspect() const;

		// 设置单个uniform变量的值
		template<typename T>
		void SetUniform(GLuint location, const T& val) const;

		// 设置uniform数组变量的值
		template<typename T>
		void SetUniformArray(GLuint location, GLuint size, const std::vector<T>& vec) const;

	protected:
		void LoadShader(GLenum type);
		void ReadShader(const std::string& path, const std::string& macro, std::string& output);
		void LinkShaders();

	protected:
		using Asset::mId;
		std::string mSourcePath;		// 着色器源文件路径
		std::string mSourceCode;		// 着色器源代码
		std::vector<GLuint> mShaders;	// 着色器对象列表
		inline static GLuint msCurrBindShader = 0;	// 用于跟踪当前的渲染状态
	};

	/******************************************************************************************************/
	class CShader : public Shader {
	public:
		CShader(const std::string& sourcePath);
		CShader(const std::string& binaryPath, GLenum format);

		// 分派计算着色器任务
		void Dispatch(GLuint nx, GLuint ny, GLuint nz = 1) const;

		// 等待计算着色器任务完成并同步
		void SyncWait(GLbitfield barriers = GL_SHADER_STORAGE_BARRIER_BIT) const;
	private:
		GLint mLocalSizeX;
		GLint mLocalSizeY;
		GLint mLocalSizeZ;
	};
}
#include "Shader.hpp"