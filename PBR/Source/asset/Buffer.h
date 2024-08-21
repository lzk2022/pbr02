#pragma once
#include <vector>
#include <glad/glad.h>
namespace asset {
	class Buffer {
	protected:
		Buffer();

		/********************************************************************************
		* @brief        Buffer类的参数化构造函数
		*********************************************************************************
		* @param        size:   缓冲区的大小
		* @param        data:   缓冲区的数据指针
		* @param        access: 缓冲区访问标志
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
		
		// 获取整个缓冲区的数据，存储到提供的数据指针中
		void GetData(void* data) const;
		void* const GetData() const {
			return mpData;
		}

		// 获取指定偏移量和大小的缓冲区数据，存储到提供的数据指针中
		void GetData(GLintptr offset, GLsizeiptr size, void* data) const;

		// 设置整个缓冲区的数据，从提供的数据指针中拷贝数据
		void SetData(const void* data) const;

		// 设置指定偏移量和大小的缓冲区数据，从提供的数据指针中拷贝数据
		void SetData(GLintptr offset, GLsizeiptr size, const void* data) const;

		// 获取缓冲区的访问权限，以便对缓冲区进行操作
		void Acquire(GLbitfield access) const;

		// 释放缓冲区，使其不再可用
		void Release() const;

		void Clear() const;
		void Clear(GLintptr offset, GLsizeiptr size) const;

		// 刷新缓冲区数据，确保所有写入操作已经提交到缓冲区中
		void Flush() const;
		void Flush(GLintptr offset, GLsizeiptr size) const;

		// 使缓冲区数据失效，标记为需要重新初始化
		void Invalidate() const;
		void Invalidate(GLintptr offset, GLsizeiptr size) const;
	public:
		GLuint Id() const { return mId; }
		GLsizeiptr Size() const { return mSize; }
		void* const DataPtr() const { return mpData; }



	public:
		GLuint mId;				// 缓冲区对象 id
		GLsizeiptr mSize;		// 缓冲区的大小
		mutable void* mpData;	// 可变指针，用于存储数据的指针
	};

	/***********************************************VBO*****************************************************/
	// 顶点缓冲区对象:Vertex Buffer Object (VBO)
	class VBO :public Buffer {
	public:
		VBO(GLsizeiptr size,const void* data,GLbitfield access = 0) : Buffer(size,data,access){}
	};

	/***********************************************IBO*****************************************************/
	// 索引缓冲区对象:Index Buffer Object (IBO)
	class IBO : public Buffer {
	public:
		IBO(GLsizeiptr size, const void* data, GLbitfield access = 0) : Buffer(size, data, access) {}
	};

	/***********************************************PBO*****************************************************/
	// 像素缓冲区对象:Pixel Buffer Object (PBO)
	class PBO : public Buffer {
	public:
		PBO(GLsizeiptr size, const void* data, GLbitfield access = 0) : Buffer(size, data, access) {}
	};

	/******************************************IndexedBuffer************************************************/
	// 索引缓冲区
	class IndexedBuffer : public Buffer {
	public:
		void Reset(GLuint index);
	protected:
		IndexedBuffer() : Buffer(), mIndex(0), mTarget(0) {}
		IndexedBuffer(GLuint index, GLsizeiptr size, GLbitfield access) : Buffer(size, nullptr, access), mIndex(index), mTarget(0) {}

	protected:
		GLuint mIndex;   // 索引绑定点
		GLenum mTarget;  // 缓冲区目标
	};

	/***********************************************ATC****************************************************/
	// 原子计数器: Atomic Counters
	class ATC : public IndexedBuffer {
	public:
		ATC() = default;
		ATC(GLuint index, GLsizeiptr size, GLbitfield access = GL_DYNAMIC_STORAGE_BIT);
	};

	/***********************************************SSBO**************************************************/
	// 着色器存储缓冲区对象:Shader Storage Buffer Object
	class SSBO : public IndexedBuffer {
	public:
		SSBO() = default;
		SSBO(GLuint index, GLsizeiptr size, GLbitfield access = GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT);
	};

	/***********************************************UBO**************************************************/
	// 统一缓冲对象:Uniform Buffer Object
	class UBO : public IndexedBuffer {
		using u_vec = std::vector<GLuint>;
	public:
		UBO() = default;
		UBO(GLuint index, const u_vec& offset, const u_vec& length, const u_vec& stride);
		UBO(GLuint shader, GLuint blockId, GLbitfield access = GL_DYNAMIC_STORAGE_BIT);
		void SetUniform(GLuint uid, const void* data) const;
		void SetUniform(GLuint fr, GLuint to, const void* data) const;

	private:
		std::vector<GLuint> mOffset;  // 每个 uniform 的对齐字节偏移量
		std::vector<GLuint> mStride;  // 每个 uniform 的字节步长（带填充）
		std::vector<GLuint> mLength;  // 每个 uniform 的字节长度（不带填充）
	};
}

