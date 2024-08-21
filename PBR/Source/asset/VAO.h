#pragma once
#include "../asset/Asset.h"

namespace asset {
	// �����������Vertex Array Object (VAO) 
	class VAO : public Asset {
	public:
		VAO();
		~VAO();

		// ɾ���������캯���͸�ֵ���������ֹ���󿽱�
		VAO(const VAO&) = delete;
		VAO& operator=(const VAO&) = delete;

		// �ƶ����캯�����ƶ���ֵ�������Ĭ��ʵ��֧���ƶ�����
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
