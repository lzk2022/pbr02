#pragma once
#include <glad/glad.h>
namespace asset {
class Asset {
public:
	GLuint  getId()const { return mId; }

public:
	Asset();
	virtual ~Asset() {};
	Asset(const Asset&) = delete;				// ���ÿ������캯��
	Asset& operator = (const Asset&) = delete;	// ���ÿ�����ֵ�����
	Asset(Asset&& other) noexcept;				// �ƶ����캯�� noexcept ��ʾ�����׳��쳣
	Asset& operator = (Asset&& other) noexcept;	// �ƶ���ֵ�����

public:
	virtual void Bind() const {}
	virtual void UnBind() const {}
	virtual void Bind(GLuint index) const {}
	virtual void UnBind(GLuint index) const {}
protected:
	GLuint mId;
};
}

