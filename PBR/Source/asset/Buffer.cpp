#include "Buffer.h"
#include <map>
#include <numeric>
#include <GLM/ext/vector_uint2.hpp>
#include "../utils/Log.h"

namespace asset {
	Buffer::Buffer() : mId(0), mSize(0), mpData(nullptr) { LOG_TRACK; }

	Buffer::Buffer(GLsizeiptr size, const void* data, GLbitfield access) :mSize(size), mpData(nullptr) {
		LOG_TRACK;
		glCreateBuffers(1, &mId);
		glNamedBufferStorage(mId, size, data, access);	 // 分配不可变存储的缓冲区
	}

	Buffer::~Buffer() {
		LOG_TRACK;
		// 删除缓冲区对象，绑定的缓冲区将会被解绑，`id = 0`将会被忽略
		glDeleteBuffers(1, &mId);
	}

	Buffer::Buffer(Buffer&& other) noexcept {
		LOG_TRACK;
		mId = std::exchange(other.mId, 0);
		mSize = std::exchange(other.mSize, 0);
		mpData = std::exchange(other.mpData, nullptr);
	}

	void Buffer::Copy(GLuint srcId, GLuint dstId, GLintptr srcOffset, GLintptr dstOffset, GLsizeiptr size){
		LOG_TRACK;
		glCopyNamedBufferSubData(srcId, dstId, srcOffset, dstOffset, size);
	}

	void Buffer::GetData(void* data) const{
		LOG_TRACK;
		glGetNamedBufferSubData(mId, 0, mSize, data);
	}
	void Buffer::GetData(GLintptr offset, GLsizeiptr size, void* data) const{
		LOG_TRACK;
		glGetNamedBufferSubData(mId, offset, size, data);
	}
	void Buffer::SetData(const void* data) const{
		LOG_TRACK;
		glNamedBufferSubData(mId, 0, mSize, data);
	}
	void Buffer::SetData(GLintptr offset, GLsizeiptr size, const void* data) const{
		LOG_TRACK;
		glNamedBufferSubData(mId, offset, size, data);
	}
	void Buffer::Acquire(GLbitfield access) const{
		LOG_TRACK;
		if (!mpData) {
			// 映射指定范围的缓冲区对象到客户端地址空间
			mpData = glMapNamedBufferRange(    
				mId,       // buffer : 要映射的缓冲区对象的ID
				0,         // offset : 映射的起始偏移量（以字节为单位）
				mSize,     // length : 映射的长度（以字节为单位）
				access     // access : 映射的访问权限，例如 GL_MAP_READ_BIT、GL_MAP_WRITE_BIT 等
			);
			LOG_ASSERT(mpData, "无法将缓冲区映射到客户端内存");
		}
	}
	void Buffer::Release() const{
		LOG_TRACK;
		if (!mpData) return;
		mpData == nullptr;
		GLboolean ret = glUnmapNamedBuffer(mId);
		LOG_ASSERT(ret == GL_FALSE, "数据存储内容损坏");
	}
	void Buffer::Clear() const{
		LOG_TRACK;
		glClearNamedBufferSubData(    // 清除缓冲区对象的指定范围的数据
			mId,				// buffer : 要清除数据的缓冲区对象的ID
			GL_R8UI,			// internalformat : 缓冲区数据的内部格式，这里是无符号8位红色分量
			0,					// offset : 要清除的起始偏移量（以字节为单位）
			mSize,				// size : 要清除的长度（以字节为单位）
			GL_RED,				// format : 数据的格式，这里表示红色分量
			GL_UNSIGNED_BYTE,	// type : 数据的类型，这里是无符号字节
			NULL				// data : 指向要用来填充缓冲区的数据的指针，这里是 NULL 表示清除数据
		);
	}
	void Buffer::Clear(GLintptr offset, GLsizeiptr size) const{
		LOG_TRACK;
		glClearNamedBufferSubData(mId, GL_R8UI, offset, size, GL_RED, GL_UNSIGNED_BYTE, NULL);
	}
	void Buffer::Flush() const{
		LOG_TRACK;
		glFlushMappedNamedBufferRange(mId, 0, mSize);
	}
	void Buffer::Flush(GLintptr offset, GLsizeiptr size) const{
		LOG_TRACK;
		glFlushMappedNamedBufferRange(mId, offset, size);
	}
	void Buffer::Invalidate() const{
		LOG_TRACK;
		glInvalidateBufferSubData(mId, 0, mSize);
	}
	void Buffer::Invalidate(GLintptr offset, GLsizeiptr size) const{
		LOG_TRACK;
		glInvalidateBufferSubData(mId, offset, size);
	}
	

	Buffer& Buffer::operator=(Buffer&& other) noexcept{
		LOG_TRACK;
		if (this != &other) {
			this->mId = std::exchange(other.mId, 0);
			this->mSize = std::exchange(other.mSize, 0);
			this->mpData = std::exchange(other.mpData, nullptr);
		}
		return *this;
	}

	/******************************************IndexedBuffer************************************************/
	void IndexedBuffer::Reset(GLuint index){
		LOG_TRACK;
		mIndex = index;
		// 将缓冲区对象绑定到指定的绑定点
		glBindBufferBase(
			mTarget,  // target : 缓冲区的绑定目标，例如 GL_UNIFORM_BUFFER、GL_TRANSFORM_FEEDBACK_BUFFER 等
			mIndex,   // index : 绑定点的索引
			mId       // buffer : 要绑定的缓冲区对象的ID
		);
	}

	/***********************************************ATC****************************************************/
	ATC::ATC(GLuint index, GLsizeiptr size, GLbitfield access) : IndexedBuffer(index,size,access){
		LOG_TRACK;
		mTarget = GL_ATOMIC_COUNTER_BUFFER;
		glBindBufferBase(mTarget, index, mId);		// 将缓冲区绑定到指定索引上
	}

	/***********************************************SSBO**************************************************/
	SSBO::SSBO(GLuint index, GLsizeiptr size, GLbitfield access) : IndexedBuffer(index,size,access){
		LOG_TRACK;
		mTarget = GL_SHADER_STORAGE_BUFFER;
		glBindBufferBase(mTarget, index, mId);
	}

	/***********************************************UBO**************************************************/
	UBO::UBO(GLuint index, const u_vec& offset, const u_vec& length, const u_vec& stride)
		: IndexedBuffer(), mOffset(offset), mLength(length), mStride(stride){
		LOG_TRACK;
		// 计算缓冲区总大小
		mSize = std::accumulate(stride.begin(), stride.end(), 0);
		mpData = nullptr;
		mIndex = index;
		mTarget = GL_UNIFORM_BUFFER;

		glCreateBuffers(1, &mId);
		// 分配动态存储的缓冲区
		glNamedBufferStorage(		// 为缓冲区对象分配存储
			mId,					// buffer : 要分配存储的缓冲区对象的ID
			mSize,					// size : 要分配的存储大小（以字节为单位）
			NULL,					// data : 指向初始化数据的指针，这里是 NULL 表示不初始化数据
			GL_DYNAMIC_STORAGE_BIT  // flags : 存储的标志，GL_DYNAMIC_STORAGE_BIT，表示存储可以被动态更新
		);

		// 将缓冲区绑定到指定索引上
		glBindBufferBase(mTarget, mIndex, mId);
	}

	UBO::UBO(GLuint shader, GLuint blockId, GLbitfield access) : IndexedBuffer(){
		LOG_TRACK;
		LOG_ASSERT(glIsProgram(shader) == GL_TRUE, "对象 {0} 不是一个有效的着色器程序", shader);
		// OpenGL标准140和430版本中每种类型的基本对齐和字节大小（不允许3组件向量/矩阵）
		using uvec2 = glm::uvec2;
		// std_140_430 映射表
		static const std::map<GLenum, uvec2> std_140_430{
			{ GL_INT,               uvec2(4U, 4U) },
			{ GL_UNSIGNED_INT,      uvec2(4U, 4U) },
			{ GL_BOOL,              uvec2(4U, 4U) },
			{ GL_FLOAT,             uvec2(4U, 4U) },
			{ GL_INT_VEC2,          uvec2(8U, 8U) },
			{ GL_INT_VEC4,          uvec2(16, 16) },
			{ GL_UNSIGNED_INT_VEC2, uvec2(8U, 8U) },
			{ GL_UNSIGNED_INT_VEC4, uvec2(16, 16) },
			{ GL_FLOAT_VEC2,        uvec2(8U, 8U) },
			{ GL_FLOAT_VEC4,        uvec2(16, 16) },
			{ GL_FLOAT_MAT2,        uvec2(16, 16) },
			{ GL_FLOAT_MAT4,        uvec2(16, 64) }
			// { GL_FLOAT_VEC3,     uvec2(16, 12) }
			// { GL_FLOAT_MAT3,     uvec2(16, 48) }
		};

		// 查询着色器属性
		static const GLenum props1[] = {
			GL_NUM_ACTIVE_VARIABLES,   // 查询着色器中活跃变量的数量
			GL_NAME_LENGTH,            // 查询着色器变量名称的长度
			GL_BUFFER_BINDING,         // 查询与着色器相关联的缓冲区对象的绑定点
			GL_BUFFER_DATA_SIZE        // 查询缓冲区对象的数据大小
		};
		static const GLenum props2[] = {
			GL_ACTIVE_VARIABLES        // 查询当前着色器程序中所有活跃变量的列表
		};
		static const GLenum unifProps[] = {
			GL_NAME_LENGTH,            // 查询统一变量名称的长度
			GL_TYPE,                   // 查询统一变量的数据类型
			GL_OFFSET,                 // 查询统一变量在缓冲区中的偏移量
			GL_ARRAY_SIZE,             // 查询统一变量数组的大小
			GL_ARRAY_STRIDE            // 查询统一变量数组的步长（即相邻数组元素之间的字节距离）
		};

		// 获取uniform块的信息
		GLint blockInfo[4];
		//glGetProgramResourceiv(shader, GL_UNIFORM_BLOCK, blockId, 4, props1, 4, NULL, blockInfo);
		glGetProgramResourceiv(
			shader,           // program : 着色器程序的ID
			GL_UNIFORM_BLOCK, // type : 查询的资源类型，这里是统一变量块
			blockId,          // index : 要查询的统一变量块的索引
			4,                // propCount : 要查询的属性数量
			props1,           // props : 要查询的属性列表
			4,                // bufSize : 返回值的缓冲区大小
			NULL,             // length : 返回的属性数量，这里是 NULL 表示不使用
			blockInfo         // params : 存储查询结果的缓冲区
		);
		mIndex = static_cast<GLuint>(blockInfo[2]);		// uniform块的绑定点
		mSize = static_cast<GLsizeiptr>(blockInfo[3]);  // 分配的块缓冲区大小（字节）
		mTarget = GL_UNIFORM_BUFFER;

		// 获取uniform块的名称
		char* name = new char[blockInfo[1]];
		//glGetProgramResourceName(shader, GL_UNIFORM_BLOCK, blockId, blockInfo[1], NULL, name);
		glGetProgramResourceName(
			shader,           // program : 着色器程序的ID
			GL_UNIFORM_BLOCK, // type : 查询的资源类型，这里是统一变量块
			blockId,          // index : 要查询的统一变量块的索引
			blockInfo[1],     // bufSize : 用于存储名称的缓冲区大小
			NULL,             // length : 返回的名称长度，这里是 NULL 表示不使用
			name              // name : 存储资源名称的缓冲区
		);
		std::string blockName(name);  // uniform块的名称
		delete[] name;

		GLint nUnifs = blockInfo[0];
		GLint* indices = new GLint[nUnifs];
		// 获取uniform块中变量的信息
		//glGetProgramResourceiv(shader, GL_UNIFORM_BLOCK, blockId, 1, props2, nUnifs, NULL, indices);
		glGetProgramResourceiv(
			shader,           // program : 着色器程序的ID
			GL_UNIFORM_BLOCK, // type : 查询的资源类型，这里是统一变量块
			blockId,          // index : 要查询的统一变量块的索引
			1,                // propCount : 要查询的属性数量，这里是 1，表示查询一个属性
			props2,           // props : 要查询的属性列表，这里包含一个属性 GL_ACTIVE_VARIABLES
			nUnifs,           // bufSize : 用于存储结果的缓冲区大小，指定 indices 缓冲区可以接收的最大数量
			NULL,             // length : 返回的属性数量，这里是 NULL 表示不使用
			indices           // params : 存储查询结果的缓冲区，用于存储活跃变量的索引
		);
		std::vector<GLint> unifIndices(indices, indices + nUnifs);
		delete[] indices;

		LOG_INFO("计算统一变量块 {0} 的 std140 对齐偏移", blockName);

		// 遍历uniform块中的每个变量
		for (auto& index : unifIndices) {
			GLint unifInfo[5];
			// 获取uniform变量的信息
			glGetProgramResourceiv(shader, GL_UNIFORM, index, 5, unifProps, 5, NULL, unifInfo);

			char* name = new char[unifInfo[0]];
			// 获取uniform变量的名称
			glGetProgramResourceName(shader, GL_UNIFORM, index, unifInfo[0], NULL, name);
			std::string unifName(name);  // uniform变量的名称（在调试中有用）
			delete[] name;

			GLenum type			= unifInfo[1];  // uniform数据类型
			GLuint offset		= unifInfo[2];  // uniform偏移量（相对于uniform块的基址）
			GLuint arrSize		= unifInfo[3];  // 数组中的元素数（如果不是数组则为0或1）
			GLuint arrStride	= unifInfo[4];  // 连续元素之间的偏移量（如果不是数组则为0）

			// 检查uniform类型是否支持
			LOG_ASSERT(std_140_430.find(type) != std_140_430.end(), "不支持的统一变量类型 {0}", unifName);

			// 计算uniform变量的字节大小
			GLuint byte_size = arrSize <= 1 ? std_140_430.at(type).y : 16U * arrSize;

			// 检查数组元素是否按照vec4大小对齐
			if (arrSize > 1) {
				LOG_ASSERT(arrStride == 16U, "数组元素没有填充到 vec4 的大小！");
			}

			// 将偏移量和字节大小添加到对应的向量中
			mOffset.push_back(offset);
			mLength.push_back(byte_size);
		}

		// 根据查询的偏移量对uniform变量进行排序（按递增顺序）
		std::vector<GLuint> sortedIdx(nUnifs);
		std::iota(sortedIdx.begin(), sortedIdx.end(), 0);
		std::stable_sort(sortedIdx.begin(), sortedIdx.end(), [this](GLuint i, GLuint j) {
			return mOffset[i] < mOffset[j];
		});

		// 使用排序后的索引重新排序偏移量和字节大小向量
		for (GLuint i = 0; i < nUnifs - 1; ++i) {
			if (sortedIdx[i] == i) continue;
			GLuint o;
			for (o = i + 1; o < sortedIdx.size(); ++o) {
				if (sortedIdx[o] == i) break;
			}
			std::swap(mOffset[i], mOffset[sortedIdx[i]]);
			std::swap(mLength[i], mLength[sortedIdx[i]]);
			std::swap(sortedIdx[i], sortedIdx[o]);
		}

		// 计算步长向量
		for (GLint i = 1; i < nUnifs; i++) {
			mStride.push_back(mOffset[i] - mOffset[i - 1]);
		}
		this->mStride.push_back(mLength.back());

		// 确保驱动程序为块分配了足够的空间（至少不小于紧凑大小）
		GLuint packed_size = mOffset.back() + mStride.back();
		LOG_ASSERT(packed_size <= static_cast<GLuint>(mSize), "块缓冲区大小不正确！");

		glCreateBuffers(1, &mId);
		// 分配块缓冲区的存储空间
		glNamedBufferStorage(mId, mSize, NULL, access);
		glBindBufferBase(mTarget, mIndex, mId);
	}

	void UBO::SetUniform(GLuint uid, const void* data) const{
		LOG_TRACK;
		glNamedBufferSubData(mId, mOffset[uid], mLength[uid], data);  // 更新单个uniform变量数据
	}

	void UBO::SetUniform(GLuint src, GLuint dst, const void* data) const{
		LOG_TRACK;
		auto it = mStride.begin();
		auto bytes = std::accumulate(it + src, it + dst + 1, decltype(mStride)::value_type(0));
		glNamedBufferSubData(mId, mOffset[src], bytes, data);  // 更新一段uniform变量的数据
	}

}