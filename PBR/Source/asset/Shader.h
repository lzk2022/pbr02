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

		// ������ʾ��ɫ���������Ϣ
		void Inspect() const;

		// ���õ���uniform������ֵ
		template<typename T>
		void SetUniform(GLuint location, const T& val) const;

		// ����uniform���������ֵ
		template<typename T>
		void SetUniformArray(GLuint location, GLuint size, const std::vector<T>& vec) const;

	protected:
		void LoadShader(GLenum type);
		void ReadShader(const std::string& path, const std::string& macro, std::string& output);
		void LinkShaders();

	protected:
		using Asset::mId;
		std::string mSourcePath;		// ��ɫ��Դ�ļ�·��
		std::string mSourceCode;		// ��ɫ��Դ����
		std::vector<GLuint> mShaders;	// ��ɫ�������б�
		inline static GLuint msCurrBindShader = 0;	// ���ڸ��ٵ�ǰ����Ⱦ״̬
	};

	/******************************************************************************************************/
	class CShader : public Shader {
	public:
		CShader(const std::string& sourcePath);
		CShader(const std::string& binaryPath, GLenum format);

		// ���ɼ�����ɫ������
		void Dispatch(GLuint nx, GLuint ny, GLuint nz = 1) const;

		// �ȴ�������ɫ��������ɲ�ͬ��
		void SyncWait(GLbitfield barriers = GL_SHADER_STORAGE_BARRIER_BIT) const;
	private:
		GLint mLocalSizeX;
		GLint mLocalSizeY;
		GLint mLocalSizeZ;
	};
}
#include "Shader.hpp"