#pragma once

#include "Asset.h"
#include <string>
namespace asset {
	class Texture;	// 前向声明 Texture 类

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
	   * @brief        绑定纹理图像到图像单元
	   *********************************************************************************
	   * @param        level:  绑定的纹理级别
	   * @param        index:  绑定的图像单元索引
	   * @param        access: 访问方式（如 GL_READ_ONLY、GL_WRITE_ONLY、GL_READ_WRITE）
	   ********************************************************************************/
		void BindILS(GLuint level, GLuint index, GLenum access) const;

		/********************************************************************************
		* @brief        解绑图像单元
		*********************************************************************************
		* @param        index: 解绑的图像单元索引
		********************************************************************************/
		void UnbindILS(GLuint index) const;

		/********************************************************************************
		* @brief        将源纹理的指定层级复制到目标纹理的指定层级
		*********************************************************************************
		* @param        src:      源纹理对象
		* @param        srcLevel: 源纹理的级别
		* @param        dst:      目标纹理对象
		* @param        dstLevel: 目标纹理的级别
		********************************************************************************/
		static void Copy(const Texture& src, GLuint srcLevel, const Texture& dst, GLuint dstLevel);

	public:
		GLenum IFormat() const { return mIFormat; }

	private:
		// 设置采样状态
		void SetSampleState() const;

	public:
		GLuint mWidth, mHeight, mDepth;
		GLuint mLevel;
		GLenum mTarget;		// 纹理目标
		GLenum mFormat;		// 格式
		GLenum mIFormat;	// 内部格式
	};

	/*************************************TexView*************************************/
	class TexView : public Asset {
	public:
		const Texture& host;  // 引用原始纹理对象
		TexView(const Texture& texture);

		// 禁止使用右值引用创建对象，防止绑定到临时对象
		TexView(const Texture&& texture) = delete;
		~TexView();

		TexView(const TexView&) = delete;
		TexView& operator=(const TexView&) = delete;
		TexView(TexView&& other) = default;
		TexView& operator=(TexView&& other) = default;

		// 设置视图参数：目标、首层级、层级数、首层、层数
		void SetView(GLenum target, GLuint srcLevel, GLuint dstLevel, GLuint srcLayer, GLuint dstLayer) const;
		void Bind(GLuint index) const;
		void UnBind(GLuint index) const;

	};
}

