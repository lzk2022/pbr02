#pragma once

#include "Asset.h"
#include <string>
namespace asset {
	class Texture;	// ǰ������ Texture ��

	class Texture : public Asset {
	public:
		Texture(const std::string& imgPath, GLuint level = 0);
		Texture(const std::string& imgPath, GLuint resolution, GLuint level);
		Texture(const std::string& directory, const std::string& ext, GLuint resolution, GLuint level);
		Texture(GLenum target, GLuint width, GLuint height, GLuint depth, GLenum iFormat, GLuint level);
		~Texture();

		Texture(const Texture&) = delete;
		Texture& operator = (const Texture&) = delete;
		Texture(Texture&& other) noexcept = default;
		Texture& operator = (Texture&& other) noexcept = default;

	public:
		void Bind(GLuint index) const override;
		void UnBind(GLuint index) const override;

		/********************************************************************************
	   * @brief        ������ͼ��ͼ��Ԫ
	   *********************************************************************************
	   * @param        level:  �󶨵�������
	   * @param        index:  �󶨵�ͼ��Ԫ����
	   * @param        access: ���ʷ�ʽ���� GL_READ_ONLY��GL_WRITE_ONLY��GL_READ_WRITE��
	   ********************************************************************************/
		void BindILS(GLuint level, GLuint index, GLenum access) const;

		/********************************************************************************
		* @brief        ���ͼ��Ԫ
		*********************************************************************************
		* @param        index: ����ͼ��Ԫ����
		********************************************************************************/
		void UnbindILS(GLuint index) const;

		/********************************************************************************
		* @brief        ��Դ�����ָ���㼶���Ƶ�Ŀ�������ָ���㼶
		*********************************************************************************
		* @param        src:      Դ�������
		* @param        srcLevel: Դ����ļ���
		* @param        dst:      Ŀ���������
		* @param        dstLevel: Ŀ������ļ���
		********************************************************************************/
		static void Copy(const Texture& src, GLuint srcLevel, const Texture& dst, GLuint dstLevel);

	public:
		GLenum IFormat() const { return mIFormat; }

	private:
		// ���ò���״̬
		void SetSampleState() const;

	public:
		GLuint mWidth, mHeight, mDepth;
		GLuint mLevel;
		GLenum mTarget;		// ����Ŀ��
		GLenum mFormat;		// ��ʽ
		GLenum mIFormat;	// �ڲ���ʽ
	};

	/*************************************TexView*************************************/
	class TexView : public Asset {
	public:
		const Texture& host;  // ����ԭʼ�������
		TexView(const Texture& texture);

		// ��ֹʹ����ֵ���ô������󣬷�ֹ�󶨵���ʱ����
		TexView(const Texture&& texture) = delete;
		~TexView();

		TexView(const TexView&) = delete;
		TexView& operator=(const TexView&) = delete;
		TexView(TexView&& other) = default;
		TexView& operator=(TexView&& other) = default;

		// ������ͼ������Ŀ�ꡢ�ײ㼶���㼶�����ײ㡢����
		void SetView(GLenum target, GLuint srcLevel, GLuint dstLevel, GLuint srcLayer, GLuint dstLayer) const;
		void Bind(GLuint index) const;
		void UnBind(GLuint index) const;

	};
}

