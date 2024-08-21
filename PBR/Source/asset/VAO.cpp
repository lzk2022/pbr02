#include "VAO.h"
#include "../utils/Log.h"
namespace asset {
	// ��ǰ�󶨵Ķ����������VAO����ȫ�ֱ������������ܰ󶨹���
	static GLuint gCurrBoundVertexArray = 0;

	VAO::VAO():Asset(){
		LOG_TRACK;
		glCreateVertexArrays(1, &mId);
	}
	VAO::~VAO(){
		LOG_TRACK;
		UnBind();  // ȷ����ɾ��֮ǰ���
		glDeleteVertexArrays(1, &mId);
	}
	void VAO::Bind() const{
		LOG_TRACK;
		if (mId == gCurrBoundVertexArray) return;
		glBindVertexArray(mId);			// �󶨵�ǰVAO
		gCurrBoundVertexArray = mId;	// ���µ�ǰ�󶨵�VAO ID
	}
	void VAO::UnBind() const{
		LOG_TRACK;
		if (gCurrBoundVertexArray != mId) return;
		gCurrBoundVertexArray = 0;  // ���õ�ǰ�󶨵�VAO ID
		glBindVertexArray(0);		// ���ǰVAO
	}
	void VAO::SetVBO(GLuint vbo, GLuint attrId, GLint offset, GLint size, GLint stride, GLenum type) const{
		LOG_TRACK;
		// �󶨶��㻺�������󵽶����������
		glVertexArrayVertexBuffer(    
			mId,    // vaobj : ������������ID
			attrId, // bindingindex : �󶨶��㻺�����������������
			vbo,    // buffer : ���㻺���������ID
			offset, // offset : ���㻺�������������ݵ���ʼƫ���������ֽ�Ϊ��λ��
			stride  // stride : ���㻺����������ÿ���������ԵĲ��������ֽ�Ϊ��λ��
		);
		// ����VAO�еĶ�������
		glEnableVertexArrayAttrib(    
			mId,    // vaobj : ������������ID
			attrId  // index : Ҫ���õĶ�����������
		);
		// �����԰󶨵�ָ���İ�����
		glVertexArrayAttribBinding(    
			mId,    // vaobj : ������������ID
			attrId, // attribindex : Ҫ�󶨵Ķ�����������
			attrId  // bindingindex : Ҫ�󶨵��İ�����
		);

		// ���������������ö������Ը�ʽ
		switch (type) {
		case GL_HALF_FLOAT:
		case GL_FLOAT:
			// ָ���������Եĸ�ʽ
			glVertexArrayAttribFormat(    
				mId,        // vaobj : ������������ID
				attrId,     // attribindex : ������������
				size,       // size : ÿ���������Ե��������
				type,       // type : ÿ���������������
				GL_FALSE,   // normalized : �Ƿ񽫶�����ֵ��һ�� (GL_FALSE��ʾ����һ��)
				0           // relativeoffset : ����ڰ󶨵��ƫ���������ֽ�Ϊ��λ��
			);
			break;
		case GL_UNSIGNED_INT:	
		case GL_INT:		// ������ʽ��ע��ʹ��I��
			// ָ�������������Եĸ�ʽ
			glVertexArrayAttribIFormat(    
				mId,        // vaobj : ������������ID
				attrId,     // attribindex : ������������
				size,       // size : ÿ���������Ե��������
				type,       // type : ÿ���������������
				0           // relativeoffset : ����ڰ󶨵��ƫ���������ֽ�Ϊ��λ��
			);
			break;
		case GL_DOUBLE:		// ˫���ȸ����ʽ��ע��ʹ��L��
			// ָ��˫���ȸ��㶥�����Եĸ�ʽ��ע��ʹ��L��
			glVertexArrayAttribLFormat(    
				mId,        // vaobj : ������������ID
				attrId,     // attribindex : ������������
				size,       // size : ÿ���������Ե��������
				type,       // type : ÿ���������������
				0           // relativeoffset : ����ڰ󶨵��ƫ���������ֽ�Ϊ��λ��
			); 
			break;
		default: { LOG_ERROR("��֧�ֵĶ����������ͣ�"); }
		}
	}
	void VAO::SetIBO(GLuint ibo) const{
		LOG_TRACK;
		glVertexArrayElementBuffer(mId, ibo);  // �������������󶨵�VAO
	}
	void VAO::Draw(GLenum mode, GLsizei count){
		LOG_TRACK;
		Bind();				// �󶨵�ǰVAO
		// ʹ�õ�ǰVAO����Ԫ��
		glDrawElements(    
			mode,				// mode : ���Ƶ�ͼԪ���ͣ����� GL_TRIANGLES��GL_LINES ��
			count,				// count : ���Ƶ�Ԫ������
			GL_UNSIGNED_INT,	// type : �������ݵ����ͣ��������޷�������
			0					// indices : ָ��Ԫ��������ƫ���������ֽ�Ϊ��λ����0��ʾ�ӿ�ʼλ�ÿ�ʼ
		);
		// UnBind();
	}
}