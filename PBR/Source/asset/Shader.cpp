#include "Shader.h"
#include "../utils/Log.h"
#include "../utils/Ext.h"
#include "../utils/Global.h"
#include <fstream>
#include <type_traits>

#include <glm/ext.hpp>                   // GLM：OpenGL数学库 扩展
#include <glm/gtc/quaternion.hpp>        // GLM：OpenGL数学库 四元数
#include <glm/gtx/quaternion.hpp>        // GLM：OpenGL数学库 四元数 扩展
#include <glm/gtx/transform.hpp>         // GLM：OpenGL数学库 变换
#include <glm/gtc/type_ptr.hpp>          // GLM：OpenGL数学库 指针
#include <glm/gtx/string_cast.hpp>       // GLM：OpenGL数学库 字符串转换
#include <glm/gtc/matrix_transform.hpp>  // GLM：OpenGL数学库 矩阵变换
#include <glm/gtx/perpendicular.hpp>     // GLM：OpenGL数学库 垂直向量
namespace asset {
	Shader::Shader(const std::string& sourcePath) : Asset(){
		LOG_TRACK;
		mSourcePath = gShaderPath + sourcePath;
		LOG_INFO("编译和链接着色器源代码：{0}", mSourcePath);

		// 加载各类着色器
		LoadShader(GL_VERTEX_SHADER);
		LoadShader(GL_TESS_CONTROL_SHADER);
		LoadShader(GL_TESS_EVALUATION_SHADER);
		LoadShader(GL_GEOMETRY_SHADER);
		LoadShader(GL_FRAGMENT_SHADER);

		// 链接各个着色器
		LinkShaders();
	}
	Shader::Shader(const std::string& binaryPath, GLenum format) : Asset(),mSourcePath(){
		LOG_TRACK;
		LOG_INFO("正在加载预编译的着色器程序：{0}", binaryPath);
		LOG_ASSERT_FILE(binaryPath);

		// 从预编译的着色器二进制文件中构建着色器程序
		std::ifstream in_stream(binaryPath.c_str(), std::ios::binary);
		std::istreambuf_iterator<char> iter_start(in_stream), iter_end;
		std::vector<char> buffer(iter_start, iter_end);
		in_stream.close();

		GLuint programId = glCreateProgram();
		glProgramBinary(programId, format, buffer.data(), buffer.size());

		// 检查着色器程序链接状态
		GLint status;
		glGetProgramiv(programId, GL_LINK_STATUS, &status);

		if (status == GL_FALSE) {
			GLint length;
			glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &length);

			std::string error;
			error.resize(length, ' ');
			glGetProgramInfoLog(programId, length, NULL, &error[0]);
			glDeleteProgram(programId);

			// 打印错误信息并释放着色器程序资源
			LOG_ERROR("加载着色器二进制文件失败，失败原因：{0}", error);
			LOG_ERROR("您确定着色器二进制文件的格式编号是正确的吗？");
			LOG_ERROR("您是否正在加载由不同驱动程序编译的二进制文件？");
			LOG_EXCEPTION(false, "着色器二进制文件已损坏：{0}", binaryPath);
		}

		mId = programId;
	}
	Shader::~Shader(){
		LOG_TRACK;
		// 析构函数：解绑着色器并删除着色器程序对象
		UnBind();
		glDeleteProgram(mId);
	}
	void Shader::Bind() const{
		LOG_TRACK;
		// 绑定着色器程序
		if (mId != msCurrBindShader) {
			glUseProgram(mId);
			msCurrBindShader = mId;
		}
	}
	void Shader::UnBind() const{
		LOG_TRACK;
		// 解绑着色器程序
		if (msCurrBindShader == mId) {
			msCurrBindShader = 0;
			glUseProgram(0);
		}
	}
	void Shader::Save() const{
		LOG_TRACK;
		// 将编译后的着色器二进制保存到源文件夹中
		LOG_ASSERT(mSourcePath.empty(), "着色器二进制文件已存在，请在保存前删除它");

		GLint formats;
		glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
		LOG_INFO("支持的着色器二进制格式数量：{0}", formats);

		if (formats <= 0) {
			LOG_WARN("不支持二进制格式，保存着色器二进制文件失败");
			return;
		}

		GLint binaryLength;
		glGetProgramiv(mId, GL_PROGRAM_BINARY_LENGTH, &binaryLength);
		LOG_INFO("正在获取着色器二进制文件长度:{0}", binaryLength);

		GLenum binaryFormat;
		std::vector<GLubyte> buffer(binaryLength);
		glGetProgramBinary(mId, binaryLength, NULL, &binaryFormat, buffer.data());

		// 构建保存路径
		std::string filepath = mSourcePath + "\\" + std::to_string(binaryFormat) + ".bin";
		LOG_INFO("将编译后的着色器程序保存到:{0}", filepath);

		// 写入二进制数据到文件
		std::ofstream out_stream(filepath.c_str(), std::ios::binary);
		out_stream.write(reinterpret_cast<char*>(buffer.data()), binaryLength);
		out_stream.close();
	}
	void Shader::Inspect() const{
		LOG_TRACK;
		// 检查着色器源代码
		if (mSourceCode.empty()) {
			if (mSourcePath.empty()) { LOG_WARN("从二进制文件加载着色器，源代码不可用"); }
			else { LOG_WARN("着色器编译有错误，找不到源代码"); }
			return;
		}

		// 打印源代码行号和内容
		LOG_INFO("检查着色器 {0} 的源代码：（示例着色器阶段）", mId);

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
		outbuff.reserve(8192);  // 预留足够的空间以容纳着色器代码

		switch (type) {
		case GL_COMPUTE_SHADER:          macro = "compute_shader";    break;
		case GL_VERTEX_SHADER:           macro = "vertex_shader";     break;
		case GL_TESS_CONTROL_SHADER:     macro = "tess_ctrl_shader";  break;
		case GL_TESS_EVALUATION_SHADER:  macro = "tess_eval_shader";  break;
		case GL_GEOMETRY_SHADER:         macro = "geometry_shader";   break;
		case GL_FRAGMENT_SHADER:         macro = "fragment_shader";   break;
		default: LOG_EXCEPTION(false, "着色器类型无效：{0}", type);
		}

		ReadShader(mSourcePath, macro, outbuff);

		// 如果源文件中不包含该类型的着色器代码，则跳过
		if (outbuff.find("#ifdef " + macro) == std::string::npos) return;  

		// 如果着色器源代码为空，则使用从文件中读取的代码
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
			glDeleteShader(shaderId);  // 编译失败，删除着色器对象以避免泄漏
			LOG_ERROR("编译着色器失败：{0}", logInfo);
			delete[] logInfo;
		}

		mShaders.push_back(shaderId);  // 编译成功，将着色器对象添加到列表中
	}

	void Shader::ReadShader(const std::string& path, const std::string& macro, std::string& output){
		LOG_TRACK;
		std::ifstream fileStream = std::ifstream(path, std::ios::in);
		LOG_ASSERT(fileStream.is_open(), "无法读取着色器文件:{0}", path);
		
		bool defined = false;
		std::string line;
		while (std::getline(fileStream, line)) {
			// 如果行中没有 "#ifdef" 和 "#include"，直接追加到输出字符串中并继续
			if (line.find("#include") == std::string::npos && line.find("#ifdef") == std::string::npos) {
				output.append(line + '\n');
				continue;
			}

			// 在第一次遇到 "#ifdef" 或 "#include" 前，只定义一次宏
			if (!defined && !macro.empty()) {
				output += ("#ifndef " + macro + '\n');
				output += ("#define " + macro + '\n');
				output += ("#endif\n\n");
				defined = true;
			}

			if (line.find("#include") != std::string::npos) {
				// 递归解析 "#include"，替换为头文件的内容
				auto directory = std::filesystem::path(path).parent_path();
				std::string relative_path = line.substr(10);	// 从第10个字符开始读取相对路径
				relative_path.pop_back();						// 移除最后的双引号
				std::string full_include_path = (directory / relative_path).string();

				ReadShader(full_include_path, "", output);		// 递归调用不定义宏
			}
			else {
				output.append(line + '\n');  // 宏已经定义，"#ifdef" 继续正常处理
			}
		}

		output.append("\n");
		fileStream.close();
	}

	void Shader::LinkShaders(){
		GLuint pid = glCreateProgram();
		LOG_ASSERT(pid > 0, "无法创建程序对象");

		// 将所有有效的着色器对象附加到程序对象上
		for (auto&& shader : mShaders) {
			if (shader > 0) {	 // 无效的着色器 ID 将被丢弃
				glAttachShader(pid, shader);
			}
		}

		// 链接着色器程序
		glLinkProgram(pid);

		GLint status;
		glGetProgramiv(pid, GL_LINK_STATUS, &status);

		if (status == GL_FALSE) {
			GLint length;
			glGetProgramiv(pid, GL_INFO_LOG_LENGTH, &length);

			GLchar* error = new GLchar[length + 1];
			glGetProgramInfoLog(pid, length, NULL, error);

			LOG_ERROR("链接着色器失败：{0}", error);
			delete[] error;
		}

		// 清理函数，用于分离并删除附加的着色器对象
		auto clearCache = [&pid](GLuint& shader) {
			if (shader > 0) {
				glDetachShader(pid, shader);
				glDeleteShader(shader);
			}
		};

		// 使用 range-based for_each 算法分离和删除所有附加的着色器对象
		utils::ranges::for_each(mShaders, clearCache);

		mShaders.clear();
		mId = pid;
	}

	/*******************************************************************************************************/
	CShader::CShader(const std::string& sourcePath) :Shader(){
		LOG_TRACK;
		mSourcePath = gShaderPath + sourcePath;
		mSourceCode = "";
		LOG_INFO("编译和链接计算着色器：{0}", mSourcePath);

		LoadShader(GL_COMPUTE_SHADER);
		LinkShaders();

		// 查询计算着色器的工作组尺寸
		GLint local_size[3];
		glGetProgramiv(mId, GL_COMPUTE_WORK_GROUP_SIZE, local_size);
		mLocalSizeX = local_size[0];
		mLocalSizeY = local_size[1];
		mLocalSizeZ = local_size[2];
	}

	CShader::CShader(const std::string& binaryPath, GLenum format) : Shader(binaryPath,format){
		LOG_TRACK;
		// 查询计算着色器的工作组尺寸
		GLint local_size[3];
		glGetProgramiv(mId, GL_COMPUTE_WORK_GROUP_SIZE, local_size);
		mLocalSizeX = local_size[0];
		mLocalSizeY = local_size[1];
		mLocalSizeZ = local_size[2];
	}

	void CShader::Dispatch(GLuint nx, GLuint ny, GLuint nz) const{
		LOG_TRACK;
		LOG_ASSERT(mLocalSizeX * mLocalSizeY * mLocalSizeZ <= gHardware.maxInvocations, "计算尺寸溢出！");
		LOG_ASSERT(mLocalSizeX <= gHardware.localSizeX, "局部工作组大小{0}超出最大值：{1}", "X", mLocalSizeX);
		LOG_ASSERT(mLocalSizeY <= gHardware.localSizeY, "局部工作组大小{0}超出最大值：{1}", "Y", mLocalSizeY);
		LOG_ASSERT(mLocalSizeZ <= gHardware.localSizeZ, "局部工作组大小{0}超出最大值：{1}", "Z", mLocalSizeZ);
		LOG_ASSERT(nx >= 1 && nx <= gHardware.workNumX, "无效的工作组数量{0}：{1}", "X", nx);
		LOG_ASSERT(ny >= 1 && ny <= gHardware.workNumY, "无效的工作组数量{0}：{1}", "Y", ny);
		LOG_ASSERT(nz >= 1 && nz <= gHardware.workNumZ, "无效的工作组数量{0}：{1}", "Z", nz);

		// 分派计算任务
		glDispatchCompute(nx, ny, nz);
	}
	void CShader::SyncWait(GLbitfield barriers) const{
		LOG_TRACK;
		// 同步等待函数，使用内存栅栏确保所有写操作完成
		glMemoryBarrier(barriers);
	}

}
