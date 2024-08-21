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
		glNamedBufferStorage(mId, size, data, access);	 // ���䲻�ɱ�洢�Ļ�����
	}

	Buffer::~Buffer() {
		LOG_TRACK;
		// ɾ�����������󣬰󶨵Ļ��������ᱻ���`id = 0`���ᱻ����
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
			// ӳ��ָ����Χ�Ļ��������󵽿ͻ��˵�ַ�ռ�
			mpData = glMapNamedBufferRange(    
				mId,       // buffer : Ҫӳ��Ļ����������ID
				0,         // offset : ӳ�����ʼƫ���������ֽ�Ϊ��λ��
				mSize,     // length : ӳ��ĳ��ȣ����ֽ�Ϊ��λ��
				access     // access : ӳ��ķ���Ȩ�ޣ����� GL_MAP_READ_BIT��GL_MAP_WRITE_BIT ��
			);
			LOG_ASSERT(mpData, "�޷���������ӳ�䵽�ͻ����ڴ�");
		}
	}
	void Buffer::Release() const{
		LOG_TRACK;
		if (!mpData) return;
		mpData == nullptr;
		GLboolean ret = glUnmapNamedBuffer(mId);
		LOG_ASSERT(ret == GL_FALSE, "���ݴ洢������");
	}
	void Buffer::Clear() const{
		LOG_TRACK;
		glClearNamedBufferSubData(    // ��������������ָ����Χ������
			mId,				// buffer : Ҫ������ݵĻ����������ID
			GL_R8UI,			// internalformat : ���������ݵ��ڲ���ʽ���������޷���8λ��ɫ����
			0,					// offset : Ҫ�������ʼƫ���������ֽ�Ϊ��λ��
			mSize,				// size : Ҫ����ĳ��ȣ����ֽ�Ϊ��λ��
			GL_RED,				// format : ���ݵĸ�ʽ�������ʾ��ɫ����
			GL_UNSIGNED_BYTE,	// type : ���ݵ����ͣ��������޷����ֽ�
			NULL				// data : ָ��Ҫ������仺���������ݵ�ָ�룬������ NULL ��ʾ�������
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
		// ������������󶨵�ָ���İ󶨵�
		glBindBufferBase(
			mTarget,  // target : �������İ�Ŀ�꣬���� GL_UNIFORM_BUFFER��GL_TRANSFORM_FEEDBACK_BUFFER ��
			mIndex,   // index : �󶨵������
			mId       // buffer : Ҫ�󶨵Ļ����������ID
		);
	}

	/***********************************************ATC****************************************************/
	ATC::ATC(GLuint index, GLsizeiptr size, GLbitfield access) : IndexedBuffer(index,size,access){
		LOG_TRACK;
		mTarget = GL_ATOMIC_COUNTER_BUFFER;
		glBindBufferBase(mTarget, index, mId);		// ���������󶨵�ָ��������
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
		// ���㻺�����ܴ�С
		mSize = std::accumulate(stride.begin(), stride.end(), 0);
		mpData = nullptr;
		mIndex = index;
		mTarget = GL_UNIFORM_BUFFER;

		glCreateBuffers(1, &mId);
		// ���䶯̬�洢�Ļ�����
		glNamedBufferStorage(		// Ϊ�������������洢
			mId,					// buffer : Ҫ����洢�Ļ����������ID
			mSize,					// size : Ҫ����Ĵ洢��С�����ֽ�Ϊ��λ��
			NULL,					// data : ָ���ʼ�����ݵ�ָ�룬������ NULL ��ʾ����ʼ������
			GL_DYNAMIC_STORAGE_BIT  // flags : �洢�ı�־��GL_DYNAMIC_STORAGE_BIT����ʾ�洢���Ա���̬����
		);

		// ���������󶨵�ָ��������
		glBindBufferBase(mTarget, mIndex, mId);
	}

	UBO::UBO(GLuint shader, GLuint blockId, GLbitfield access) : IndexedBuffer(){
		LOG_TRACK;
		LOG_ASSERT(glIsProgram(shader) == GL_TRUE, "���� {0} ����һ����Ч����ɫ������", shader);
		// OpenGL��׼140��430�汾��ÿ�����͵Ļ���������ֽڴ�С��������3�������/����
		using uvec2 = glm::uvec2;
		// std_140_430 ӳ���
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

		// ��ѯ��ɫ������
		static const GLenum props1[] = {
			GL_NUM_ACTIVE_VARIABLES,   // ��ѯ��ɫ���л�Ծ����������
			GL_NAME_LENGTH,            // ��ѯ��ɫ���������Ƶĳ���
			GL_BUFFER_BINDING,         // ��ѯ����ɫ��������Ļ���������İ󶨵�
			GL_BUFFER_DATA_SIZE        // ��ѯ��������������ݴ�С
		};
		static const GLenum props2[] = {
			GL_ACTIVE_VARIABLES        // ��ѯ��ǰ��ɫ�����������л�Ծ�������б�
		};
		static const GLenum unifProps[] = {
			GL_NAME_LENGTH,            // ��ѯͳһ�������Ƶĳ���
			GL_TYPE,                   // ��ѯͳһ��������������
			GL_OFFSET,                 // ��ѯͳһ�����ڻ������е�ƫ����
			GL_ARRAY_SIZE,             // ��ѯͳһ��������Ĵ�С
			GL_ARRAY_STRIDE            // ��ѯͳһ��������Ĳ���������������Ԫ��֮����ֽھ��룩
		};

		// ��ȡuniform�����Ϣ
		GLint blockInfo[4];
		//glGetProgramResourceiv(shader, GL_UNIFORM_BLOCK, blockId, 4, props1, 4, NULL, blockInfo);
		glGetProgramResourceiv(
			shader,           // program : ��ɫ�������ID
			GL_UNIFORM_BLOCK, // type : ��ѯ����Դ���ͣ�������ͳһ������
			blockId,          // index : Ҫ��ѯ��ͳһ�����������
			4,                // propCount : Ҫ��ѯ����������
			props1,           // props : Ҫ��ѯ�������б�
			4,                // bufSize : ����ֵ�Ļ�������С
			NULL,             // length : ���ص����������������� NULL ��ʾ��ʹ��
			blockInfo         // params : �洢��ѯ����Ļ�����
		);
		mIndex = static_cast<GLuint>(blockInfo[2]);		// uniform��İ󶨵�
		mSize = static_cast<GLsizeiptr>(blockInfo[3]);  // ����Ŀ黺������С���ֽڣ�
		mTarget = GL_UNIFORM_BUFFER;

		// ��ȡuniform�������
		char* name = new char[blockInfo[1]];
		//glGetProgramResourceName(shader, GL_UNIFORM_BLOCK, blockId, blockInfo[1], NULL, name);
		glGetProgramResourceName(
			shader,           // program : ��ɫ�������ID
			GL_UNIFORM_BLOCK, // type : ��ѯ����Դ���ͣ�������ͳһ������
			blockId,          // index : Ҫ��ѯ��ͳһ�����������
			blockInfo[1],     // bufSize : ���ڴ洢���ƵĻ�������С
			NULL,             // length : ���ص����Ƴ��ȣ������� NULL ��ʾ��ʹ��
			name              // name : �洢��Դ���ƵĻ�����
		);
		std::string blockName(name);  // uniform�������
		delete[] name;

		GLint nUnifs = blockInfo[0];
		GLint* indices = new GLint[nUnifs];
		// ��ȡuniform���б�������Ϣ
		//glGetProgramResourceiv(shader, GL_UNIFORM_BLOCK, blockId, 1, props2, nUnifs, NULL, indices);
		glGetProgramResourceiv(
			shader,           // program : ��ɫ�������ID
			GL_UNIFORM_BLOCK, // type : ��ѯ����Դ���ͣ�������ͳһ������
			blockId,          // index : Ҫ��ѯ��ͳһ�����������
			1,                // propCount : Ҫ��ѯ������������������ 1����ʾ��ѯһ������
			props2,           // props : Ҫ��ѯ�������б��������һ������ GL_ACTIVE_VARIABLES
			nUnifs,           // bufSize : ���ڴ洢����Ļ�������С��ָ�� indices ���������Խ��յ��������
			NULL,             // length : ���ص����������������� NULL ��ʾ��ʹ��
			indices           // params : �洢��ѯ����Ļ����������ڴ洢��Ծ����������
		);
		std::vector<GLint> unifIndices(indices, indices + nUnifs);
		delete[] indices;

		LOG_INFO("����ͳһ������ {0} �� std140 ����ƫ��", blockName);

		// ����uniform���е�ÿ������
		for (auto& index : unifIndices) {
			GLint unifInfo[5];
			// ��ȡuniform��������Ϣ
			glGetProgramResourceiv(shader, GL_UNIFORM, index, 5, unifProps, 5, NULL, unifInfo);

			char* name = new char[unifInfo[0]];
			// ��ȡuniform����������
			glGetProgramResourceName(shader, GL_UNIFORM, index, unifInfo[0], NULL, name);
			std::string unifName(name);  // uniform���������ƣ��ڵ��������ã�
			delete[] name;

			GLenum type			= unifInfo[1];  // uniform��������
			GLuint offset		= unifInfo[2];  // uniformƫ�����������uniform��Ļ�ַ��
			GLuint arrSize		= unifInfo[3];  // �����е�Ԫ�������������������Ϊ0��1��
			GLuint arrStride	= unifInfo[4];  // ����Ԫ��֮���ƫ�������������������Ϊ0��

			// ���uniform�����Ƿ�֧��
			LOG_ASSERT(std_140_430.find(type) != std_140_430.end(), "��֧�ֵ�ͳһ�������� {0}", unifName);

			// ����uniform�������ֽڴ�С
			GLuint byte_size = arrSize <= 1 ? std_140_430.at(type).y : 16U * arrSize;

			// �������Ԫ���Ƿ���vec4��С����
			if (arrSize > 1) {
				LOG_ASSERT(arrStride == 16U, "����Ԫ��û����䵽 vec4 �Ĵ�С��");
			}

			// ��ƫ�������ֽڴ�С��ӵ���Ӧ��������
			mOffset.push_back(offset);
			mLength.push_back(byte_size);
		}

		// ���ݲ�ѯ��ƫ������uniform�����������򣨰�����˳��
		std::vector<GLuint> sortedIdx(nUnifs);
		std::iota(sortedIdx.begin(), sortedIdx.end(), 0);
		std::stable_sort(sortedIdx.begin(), sortedIdx.end(), [this](GLuint i, GLuint j) {
			return mOffset[i] < mOffset[j];
		});

		// ʹ��������������������ƫ�������ֽڴ�С����
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

		// ���㲽������
		for (GLint i = 1; i < nUnifs; i++) {
			mStride.push_back(mOffset[i] - mOffset[i - 1]);
		}
		this->mStride.push_back(mLength.back());

		// ȷ����������Ϊ��������㹻�Ŀռ䣨���ٲ�С�ڽ��մ�С��
		GLuint packed_size = mOffset.back() + mStride.back();
		LOG_ASSERT(packed_size <= static_cast<GLuint>(mSize), "�黺������С����ȷ��");

		glCreateBuffers(1, &mId);
		// ����黺�����Ĵ洢�ռ�
		glNamedBufferStorage(mId, mSize, NULL, access);
		glBindBufferBase(mTarget, mIndex, mId);
	}

	void UBO::SetUniform(GLuint uid, const void* data) const{
		LOG_TRACK;
		glNamedBufferSubData(mId, mOffset[uid], mLength[uid], data);  // ���µ���uniform��������
	}

	void UBO::SetUniform(GLuint src, GLuint dst, const void* data) const{
		LOG_TRACK;
		auto it = mStride.begin();
		auto bytes = std::accumulate(it + src, it + dst + 1, decltype(mStride)::value_type(0));
		glNamedBufferSubData(mId, mOffset[src], bytes, data);  // ����һ��uniform����������
	}

}