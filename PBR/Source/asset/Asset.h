#pragma once
#include <glad/glad.h>
namespace asset {
class Asset {
public:
	GLuint  getId()const { return mId; }

public:
	Asset();
	virtual ~Asset() {};
	Asset(const Asset&) = delete;				// 禁用拷贝构造函数
	Asset& operator = (const Asset&) = delete;	// 禁用拷贝赋值运算符
	Asset(Asset&& other) noexcept;				// 移动构造函数 noexcept 表示不会抛出异常
	Asset& operator = (Asset&& other) noexcept;	// 移动赋值运算符

public:
	virtual void Bind() const {}
	virtual void UnBind() const {}
	virtual void Bind(GLuint index) const {}
	virtual void UnBind(GLuint index) const {}
protected:
	GLuint mId;
};
}

