#pragma once
#include <vector>
#include <glad/glad.h>
namespace asset {
	class Buffer {
	protected:
		Buffer();

		/********************************************************************************
		* @brief        Buffer��Ĳ��������캯��
		*********************************************************************************
		* @param        size:   �������Ĵ�С
		* @param        data:   ������������ָ��
		* @param        access: ���������ʱ�־
		********************************************************************************/
		Buffer(GLsizeiptr size, const void* data, GLbitfield access);

		virtual ~Buffer();

		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;
		Buffer(Buffer&& other) noexcept;
		Buffer& operator=(Buffer&& other) noexcept;

	public:


	public:
		static void Copy(GLuint srcId, GLuint dstId, GLintptr srcOffset, GLintptr dstOffset, GLsizeiptr size);
		
		// ��ȡ���������������ݣ��洢���ṩ������ָ����
		void GetData(void* data) const;
		void* const GetData() const {
			return mpData;
		}

		// ��ȡָ��ƫ�����ʹ�С�Ļ��������ݣ��洢���ṩ������ָ����
		void GetData(GLintptr offset, GLsizeiptr size, void* data) const;

		// �������������������ݣ����ṩ������ָ���п�������
		void SetData(const void* data) const;

		// ����ָ��ƫ�����ʹ�С�Ļ��������ݣ����ṩ������ָ���п�������
		void SetData(GLintptr offset, GLsizeiptr size, const void* data) const;

		// ��ȡ�������ķ���Ȩ�ޣ��Ա�Ի��������в���
		void Acquire(GLbitfield access) const;

		// �ͷŻ�������ʹ�䲻�ٿ���
		void Release() const;

		void Clear() const;
		void Clear(GLintptr offset, GLsizeiptr size) const;

		// ˢ�»��������ݣ�ȷ������д������Ѿ��ύ����������
		void Flush() const;
		void Flush(GLintptr offset, GLsizeiptr size) const;

		// ʹ����������ʧЧ�����Ϊ��Ҫ���³�ʼ��
		void Invalidate() const;
		void Invalidate(GLintptr offset, GLsizeiptr size) const;
	public:
		GLuint Id() const { return mId; }
		GLsizeiptr Size() const { return mSize; }
		void* const DataPtr() const { return mpData; }



	public:
		GLuint mId;				// ���������� id
		GLsizeiptr mSize;		// �������Ĵ�С
		mutable void* mpData;	// �ɱ�ָ�룬���ڴ洢���ݵ�ָ��
	};

	/***********************************************VBO*****************************************************/
	// ���㻺��������:Vertex Buffer Object (VBO)
	class VBO :public Buffer {
	public:
		VBO(GLsizeiptr size,const void* data,GLbitfield access = 0) : Buffer(size,data,access){}
	};

	/***********************************************IBO*****************************************************/
	// ��������������:Index Buffer Object (IBO)
	class IBO : public Buffer {
	public:
		IBO(GLsizeiptr size, const void* data, GLbitfield access = 0) : Buffer(size, data, access) {}
	};

	/***********************************************PBO*****************************************************/
	// ���ػ���������:Pixel Buffer Object (PBO)
	class PBO : public Buffer {
	public:
		PBO(GLsizeiptr size, const void* data, GLbitfield access = 0) : Buffer(size, data, access) {}
	};

	/******************************************IndexedBuffer************************************************/
	// ����������
	class IndexedBuffer : public Buffer {
	public:
		void Reset(GLuint index);
	protected:
		IndexedBuffer() : Buffer(), mIndex(0), mTarget(0) {}
		IndexedBuffer(GLuint index, GLsizeiptr size, GLbitfield access) : Buffer(size, nullptr, access), mIndex(index), mTarget(0) {}

	protected:
		GLuint mIndex;   // �����󶨵�
		GLenum mTarget;  // ������Ŀ��
	};

	/***********************************************ATC****************************************************/
	// ԭ�Ӽ�����: Atomic Counters
	class ATC : public IndexedBuffer {
	public:
		ATC() = default;
		ATC(GLuint index, GLsizeiptr size, GLbitfield access = GL_DYNAMIC_STORAGE_BIT);
	};

	/***********************************************SSBO**************************************************/
	// ��ɫ���洢����������:Shader Storage Buffer Object
	class SSBO : public IndexedBuffer {
	public:
		SSBO() = default;
		SSBO(GLuint index, GLsizeiptr size, GLbitfield access = GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT);
	};

	/***********************************************UBO**************************************************/
	// ͳһ�������:Uniform Buffer Object
	class UBO : public IndexedBuffer {
		using u_vec = std::vector<GLuint>;
	public:
		UBO() = default;
		UBO(GLuint index, const u_vec& offset, const u_vec& length, const u_vec& stride);
		UBO(GLuint shader, GLuint blockId, GLbitfield access = GL_DYNAMIC_STORAGE_BIT);
		void SetUniform(GLuint uid, const void* data) const;
		void SetUniform(GLuint fr, GLuint to, const void* data) const;

	private:
		std::vector<GLuint> mOffset;  // ÿ�� uniform �Ķ����ֽ�ƫ����
		std::vector<GLuint> mStride;  // ÿ�� uniform ���ֽڲ���������䣩
		std::vector<GLuint> mLength;  // ÿ�� uniform ���ֽڳ��ȣ�������䣩
	};
}

