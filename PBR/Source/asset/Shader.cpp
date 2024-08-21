#include "Shader.h"
#include "../utils/Log.h"
#include "../utils/Ext.h"
#include "../utils/Global.h"
#include <fstream>
#include <type_traits>

#include <glm/ext.hpp>                   // GLM��OpenGL��ѧ�� ��չ
#include <glm/gtc/quaternion.hpp>        // GLM��OpenGL��ѧ�� ��Ԫ��
#include <glm/gtx/quaternion.hpp>        // GLM��OpenGL��ѧ�� ��Ԫ�� ��չ
#include <glm/gtx/transform.hpp>         // GLM��OpenGL��ѧ�� �任
#include <glm/gtc/type_ptr.hpp>          // GLM��OpenGL��ѧ�� ָ��
#include <glm/gtx/string_cast.hpp>       // GLM��OpenGL��ѧ�� �ַ���ת��
#include <glm/gtc/matrix_transform.hpp>  // GLM��OpenGL��ѧ�� ����任
#include <glm/gtx/perpendicular.hpp>     // GLM��OpenGL��ѧ�� ��ֱ����
namespace asset {
	Shader::Shader(const std::string& sourcePath) : Asset(){
		LOG_TRACK;
		mSourcePath = gShaderPath + sourcePath;
		LOG_INFO("�����������ɫ��Դ���룺{0}", mSourcePath);

		// ���ظ�����ɫ��
		LoadShader(GL_VERTEX_SHADER);
		LoadShader(GL_TESS_CONTROL_SHADER);
		LoadShader(GL_TESS_EVALUATION_SHADER);
		LoadShader(GL_GEOMETRY_SHADER);
		LoadShader(GL_FRAGMENT_SHADER);

		// ���Ӹ�����ɫ��
		LinkShaders();
	}
	Shader::Shader(const std::string& binaryPath, GLenum format) : Asset(),mSourcePath(){
		LOG_TRACK;
		LOG_INFO("���ڼ���Ԥ�������ɫ������{0}", binaryPath);
		LOG_ASSERT_FILE(binaryPath);

		// ��Ԥ�������ɫ���������ļ��й�����ɫ������
		std::ifstream in_stream(binaryPath.c_str(), std::ios::binary);
		std::istreambuf_iterator<char> iter_start(in_stream), iter_end;
		std::vector<char> buffer(iter_start, iter_end);
		in_stream.close();

		GLuint programId = glCreateProgram();
		glProgramBinary(programId, format, buffer.data(), buffer.size());

		// �����ɫ����������״̬
		GLint status;
		glGetProgramiv(programId, GL_LINK_STATUS, &status);

		if (status == GL_FALSE) {
			GLint length;
			glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &length);

			std::string error;
			error.resize(length, ' ');
			glGetProgramInfoLog(programId, length, NULL, &error[0]);
			glDeleteProgram(programId);

			// ��ӡ������Ϣ���ͷ���ɫ��������Դ
			LOG_ERROR("������ɫ���������ļ�ʧ�ܣ�ʧ��ԭ��{0}", error);
			LOG_ERROR("��ȷ����ɫ���������ļ��ĸ�ʽ�������ȷ����");
			LOG_ERROR("���Ƿ����ڼ����ɲ�ͬ�����������Ķ������ļ���");
			LOG_EXCEPTION(false, "��ɫ���������ļ����𻵣�{0}", binaryPath);
		}

		mId = programId;
	}
	Shader::~Shader(){
		LOG_TRACK;
		// ���������������ɫ����ɾ����ɫ���������
		UnBind();
		glDeleteProgram(mId);
	}
	void Shader::Bind() const{
		LOG_TRACK;
		// ����ɫ������
		if (mId != msCurrBindShader) {
			glUseProgram(mId);
			msCurrBindShader = mId;
		}
	}
	void Shader::UnBind() const{
		LOG_TRACK;
		// �����ɫ������
		if (msCurrBindShader == mId) {
			msCurrBindShader = 0;
			glUseProgram(0);
		}
	}
	void Shader::Save() const{
		LOG_TRACK;
		// ����������ɫ�������Ʊ��浽Դ�ļ�����
		LOG_ASSERT(mSourcePath.empty(), "��ɫ���������ļ��Ѵ��ڣ����ڱ���ǰɾ����");

		GLint formats;
		glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
		LOG_INFO("֧�ֵ���ɫ�������Ƹ�ʽ������{0}", formats);

		if (formats <= 0) {
			LOG_WARN("��֧�ֶ����Ƹ�ʽ��������ɫ���������ļ�ʧ��");
			return;
		}

		GLint binaryLength;
		glGetProgramiv(mId, GL_PROGRAM_BINARY_LENGTH, &binaryLength);
		LOG_INFO("���ڻ�ȡ��ɫ���������ļ�����:{0}", binaryLength);

		GLenum binaryFormat;
		std::vector<GLubyte> buffer(binaryLength);
		glGetProgramBinary(mId, binaryLength, NULL, &binaryFormat, buffer.data());

		// ��������·��
		std::string filepath = mSourcePath + "\\" + std::to_string(binaryFormat) + ".bin";
		LOG_INFO("����������ɫ�����򱣴浽:{0}", filepath);

		// д����������ݵ��ļ�
		std::ofstream out_stream(filepath.c_str(), std::ios::binary);
		out_stream.write(reinterpret_cast<char*>(buffer.data()), binaryLength);
		out_stream.close();
	}
	void Shader::Inspect() const{
		LOG_TRACK;
		// �����ɫ��Դ����
		if (mSourceCode.empty()) {
			if (mSourcePath.empty()) { LOG_WARN("�Ӷ������ļ�������ɫ����Դ���벻����"); }
			else { LOG_WARN("��ɫ�������д����Ҳ���Դ����"); }
			return;
		}

		// ��ӡԴ�����кź�����
		LOG_INFO("�����ɫ�� {0} ��Դ���룺��ʾ����ɫ���׶Σ�", mId);

		std::istringstream isstream(mSourceCode);
		std::string line;
		int lineNumber = 1;

		while (std::getline(isstream, line)) {
			LOG_INFO("{0:03d} | {1}", lineNumber, line);
			lineNumber++;
		}
	}

	

	void Shader::LoadShader(GLenum type){
		LOG_TRACK;
		std::string macro;
		std::string outbuff;
		outbuff.reserve(8192);  // Ԥ���㹻�Ŀռ���������ɫ������

		switch (type) {
		case GL_COMPUTE_SHADER:          macro = "compute_shader";    break;
		case GL_VERTEX_SHADER:           macro = "vertex_shader";     break;
		case GL_TESS_CONTROL_SHADER:     macro = "tess_ctrl_shader";  break;
		case GL_TESS_EVALUATION_SHADER:  macro = "tess_eval_shader";  break;
		case GL_GEOMETRY_SHADER:         macro = "geometry_shader";   break;
		case GL_FRAGMENT_SHADER:         macro = "fragment_shader";   break;
		default: LOG_EXCEPTION(false, "��ɫ��������Ч��{0}", type);
		}

		ReadShader(mSourcePath, macro, outbuff);

		// ���Դ�ļ��в����������͵���ɫ�����룬������
		if (outbuff.find("#ifdef " + macro) == std::string::npos) return;  

		// �����ɫ��Դ����Ϊ�գ���ʹ�ô��ļ��ж�ȡ�Ĵ���
		if (mSourceCode.empty()) mSourceCode = outbuff; 

		const char* sourceCode = outbuff.c_str();
		GLuint shaderId = glCreateShader(type);
		glShaderSource(shaderId, 1, &sourceCode, nullptr);
		glCompileShader(shaderId);

		GLint status;
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);

		if (status == GL_FALSE) {
			GLint length;
			glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);

			GLchar* logInfo = new GLchar[length + 1];
			glGetShaderInfoLog(shaderId, length, NULL, logInfo);
			glDeleteShader(shaderId);  // ����ʧ�ܣ�ɾ����ɫ�������Ա���й©
			LOG_ERROR("������ɫ��ʧ�ܣ�{0}", logInfo);
			delete[] logInfo;
		}

		mShaders.push_back(shaderId);  // ����ɹ�������ɫ��������ӵ��б���
	}

	void Shader::ReadShader(const std::string& path, const std::string& macro, std::string& output){
		LOG_TRACK;
		std::ifstream fileStream = std::ifstream(path, std::ios::in);
		LOG_ASSERT(fileStream.is_open(), "�޷���ȡ��ɫ���ļ�:{0}", path);
		
		bool defined = false;
		std::string line;
		while (std::getline(fileStream, line)) {
			// �������û�� "#ifdef" �� "#include"��ֱ��׷�ӵ�����ַ����в�����
			if (line.find("#include") == std::string::npos && line.find("#ifdef") == std::string::npos) {
				output.append(line + '\n');
				continue;
			}

			// �ڵ�һ������ "#ifdef" �� "#include" ǰ��ֻ����һ�κ�
			if (!defined && !macro.empty()) {
				output += ("#ifndef " + macro + '\n');
				output += ("#define " + macro + '\n');
				output += ("#endif\n\n");
				defined = true;
			}

			if (line.find("#include") != std::string::npos) {
				// �ݹ���� "#include"���滻Ϊͷ�ļ�������
				auto directory = std::filesystem::path(path).parent_path();
				std::string relative_path = line.substr(10);	// �ӵ�10���ַ���ʼ��ȡ���·��
				relative_path.pop_back();						// �Ƴ�����˫����
				std::string full_include_path = (directory / relative_path).string();

				ReadShader(full_include_path, "", output);		// �ݹ���ò������
			}
			else {
				output.append(line + '\n');  // ���Ѿ����壬"#ifdef" ������������
			}
		}

		output.append("\n");
		fileStream.close();
	}

	void Shader::LinkShaders(){
		GLuint pid = glCreateProgram();
		LOG_ASSERT(pid > 0, "�޷������������");

		// ��������Ч����ɫ�����󸽼ӵ����������
		for (auto&& shader : mShaders) {
			if (shader > 0) {	 // ��Ч����ɫ�� ID ��������
				glAttachShader(pid, shader);
			}
		}

		// ������ɫ������
		glLinkProgram(pid);

		GLint status;
		glGetProgramiv(pid, GL_LINK_STATUS, &status);

		if (status == GL_FALSE) {
			GLint length;
			glGetProgramiv(pid, GL_INFO_LOG_LENGTH, &length);

			GLchar* error = new GLchar[length + 1];
			glGetProgramInfoLog(pid, length, NULL, error);

			LOG_ERROR("������ɫ��ʧ�ܣ�{0}", error);
			delete[] error;
		}

		// �����������ڷ��벢ɾ�����ӵ���ɫ������
		auto clearCache = [&pid](GLuint& shader) {
			if (shader > 0) {
				glDetachShader(pid, shader);
				glDeleteShader(shader);
			}
		};

		// ʹ�� range-based for_each �㷨�����ɾ�����и��ӵ���ɫ������
		utils::ranges::for_each(mShaders, clearCache);

		mShaders.clear();
		mId = pid;
	}

	/*******************************************************************************************************/
	CShader::CShader(const std::string& sourcePath) :Shader(){
		LOG_TRACK;
		mSourcePath = gShaderPath + sourcePath;
		mSourceCode = "";
		LOG_INFO("��������Ӽ�����ɫ����{0}", mSourcePath);

		LoadShader(GL_COMPUTE_SHADER);
		LinkShaders();

		// ��ѯ������ɫ���Ĺ�����ߴ�
		GLint local_size[3];
		glGetProgramiv(mId, GL_COMPUTE_WORK_GROUP_SIZE, local_size);
		mLocalSizeX = local_size[0];
		mLocalSizeY = local_size[1];
		mLocalSizeZ = local_size[2];
	}

	CShader::CShader(const std::string& binaryPath, GLenum format) : Shader(binaryPath,format){
		LOG_TRACK;
		// ��ѯ������ɫ���Ĺ�����ߴ�
		GLint local_size[3];
		glGetProgramiv(mId, GL_COMPUTE_WORK_GROUP_SIZE, local_size);
		mLocalSizeX = local_size[0];
		mLocalSizeY = local_size[1];
		mLocalSizeZ = local_size[2];
	}

	void CShader::Dispatch(GLuint nx, GLuint ny, GLuint nz) const{
		LOG_TRACK;
		LOG_ASSERT(mLocalSizeX * mLocalSizeY * mLocalSizeZ <= gHardware.maxInvocations, "����ߴ������");
		LOG_ASSERT(mLocalSizeX <= gHardware.localSizeX, "�ֲ��������С{0}�������ֵ��{1}", "X", mLocalSizeX);
		LOG_ASSERT(mLocalSizeY <= gHardware.localSizeY, "�ֲ��������С{0}�������ֵ��{1}", "Y", mLocalSizeY);
		LOG_ASSERT(mLocalSizeZ <= gHardware.localSizeZ, "�ֲ��������С{0}�������ֵ��{1}", "Z", mLocalSizeZ);
		LOG_ASSERT(nx >= 1 && nx <= gHardware.workNumX, "��Ч�Ĺ���������{0}��{1}", "X", nx);
		LOG_ASSERT(ny >= 1 && ny <= gHardware.workNumY, "��Ч�Ĺ���������{0}��{1}", "Y", ny);
		LOG_ASSERT(nz >= 1 && nz <= gHardware.workNumZ, "��Ч�Ĺ���������{0}��{1}", "Z", nz);

		// ���ɼ�������
		glDispatchCompute(nx, ny, nz);
	}
	void CShader::SyncWait(GLbitfield barriers) const{
		LOG_TRACK;
		// ͬ���ȴ�������ʹ���ڴ�դ��ȷ������д�������
		glMemoryBarrier(barriers);
	}

}
