#pragma once
#include "../asset/Asset.h"

namespace asset {
	// 顶点数组对象：Vertex Array Object (VAO) 
	class VAO : public Asset {
	public:
		VAO();
		~VAO();

		// 删除拷贝构造函数和赋值运算符，禁止对象拷贝
		VAO(const VAO&) = delete;
		VAO& operator=(const VAO&) = delete;

		// 移动构造函数和移动赋值运算符，默认实现支持移动语义
		VAO(VAO&& other) noexcept = default;
		VAO& operator=(VAO&& other) noexcept = default;

	public:
		void Bind() const override;
		void UnBind() const override;
		void SetVBO(GLuint vbo, GLuint attr_id, GLint offset, GLint size, GLint stride, GLenum type) const;
		void SetIBO(GLuint ibo) const;
		void Draw(GLenum mode, GLsizei count);
	};
}
