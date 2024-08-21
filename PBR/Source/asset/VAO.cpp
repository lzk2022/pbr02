#include "VAO.h"
#include "../utils/Log.h"
namespace asset {
	// 当前绑定的顶点数组对象（VAO）的全局变量，用于智能绑定管理
	static GLuint gCurrBoundVertexArray = 0;

	VAO::VAO():Asset(){
		LOG_TRACK;
		glCreateVertexArrays(1, &mId);
	}
	VAO::~VAO(){
		LOG_TRACK;
		UnBind();  // 确保在删除之前解绑
		glDeleteVertexArrays(1, &mId);
	}
	void VAO::Bind() const{
		LOG_TRACK;
		if (mId == gCurrBoundVertexArray) return;
		glBindVertexArray(mId);			// 绑定当前VAO
		gCurrBoundVertexArray = mId;	// 更新当前绑定的VAO ID
	}
	void VAO::UnBind() const{
		LOG_TRACK;
		if (gCurrBoundVertexArray != mId) return;
		gCurrBoundVertexArray = 0;  // 重置当前绑定的VAO ID
		glBindVertexArray(0);		// 解绑当前VAO
	}
	void VAO::SetVBO(GLuint vbo, GLuint attrId, GLint offset, GLint size, GLint stride, GLenum type) const{
		LOG_TRACK;
		// 绑定顶点缓冲区对象到顶点数组对象
		glVertexArrayVertexBuffer(    
			mId,    // vaobj : 顶点数组对象的ID
			attrId, // bindingindex : 绑定顶点缓冲区对象的属性索引
			vbo,    // buffer : 顶点缓冲区对象的ID
			offset, // offset : 顶点缓冲区对象中数据的起始偏移量（以字节为单位）
			stride  // stride : 顶点缓冲区对象中每个顶点属性的步长（以字节为单位）
		);
		// 启用VAO中的顶点属性
		glEnableVertexArrayAttrib(    
			mId,    // vaobj : 顶点数组对象的ID
			attrId  // index : 要启用的顶点属性索引
		);
		// 将属性绑定到指定的绑定索引
		glVertexArrayAttribBinding(    
			mId,    // vaobj : 顶点数组对象的ID
			attrId, // attribindex : 要绑定的顶点属性索引
			attrId  // bindingindex : 要绑定到的绑定索引
		);

		// 根据数据类型设置顶点属性格式
		switch (type) {
		case GL_HALF_FLOAT:
		case GL_FLOAT:
			// 指定顶点属性的格式
			glVertexArrayAttribFormat(    
				mId,        // vaobj : 顶点数组对象的ID
				attrId,     // attribindex : 顶点属性索引
				size,       // size : 每个顶点属性的组件数量
				type,       // type : 每个组件的数据类型
				GL_FALSE,   // normalized : 是否将定点数值归一化 (GL_FALSE表示不归一化)
				0           // relativeoffset : 相对于绑定点的偏移量（以字节为单位）
			);
			break;
		case GL_UNSIGNED_INT:	
		case GL_INT:		// 整数格式（注意使用I）
			// 指定整数顶点属性的格式
			glVertexArrayAttribIFormat(    
				mId,        // vaobj : 顶点数组对象的ID
				attrId,     // attribindex : 顶点属性索引
				size,       // size : 每个顶点属性的组件数量
				type,       // type : 每个组件的数据类型
				0           // relativeoffset : 相对于绑定点的偏移量（以字节为单位）
			);
			break;
		case GL_DOUBLE:		// 双精度浮点格式（注意使用L）
			// 指定双精度浮点顶点属性的格式（注意使用L）
			glVertexArrayAttribLFormat(    
				mId,        // vaobj : 顶点数组对象的ID
				attrId,     // attribindex : 顶点属性索引
				size,       // size : 每个顶点属性的组件数量
				type,       // type : 每个组件的数据类型
				0           // relativeoffset : 相对于绑定点的偏移量（以字节为单位）
			); 
			break;
		default: { LOG_ERROR("不支持的顶点属性类型！"); }
		}
	}
	void VAO::SetIBO(GLuint ibo) const{
		LOG_TRACK;
		glVertexArrayElementBuffer(mId, ibo);  // 将索引缓冲对象绑定到VAO
	}
	void VAO::Draw(GLenum mode, GLsizei count){
		LOG_TRACK;
		Bind();				// 绑定当前VAO
		// 使用当前VAO绘制元素
		glDrawElements(    
			mode,				// mode : 绘制的图元类型，例如 GL_TRIANGLES、GL_LINES 等
			count,				// count : 绘制的元素数量
			GL_UNSIGNED_INT,	// type : 索引数据的类型，这里是无符号整数
			0					// indices : 指向元素索引的偏移量（以字节为单位），0表示从开始位置开始
		);
		// UnBind();
	}
}