#pragma once
#include "../asset/Asset.h"
#include "Texture.h"
#include <vector>
#include <memory>
namespace asset {
	/***************************************RBO***************************************/
	class RBO : public Asset {
	public:
		RBO(GLuint width, GLuint height, bool isMultisample = false);
		~RBO();
		void Bind() const override;
		void UnBind() const override;
	};

	/***************************************FBO***************************************/
	// 用于在离屏渲染中存储渲染结果
	// 帧缓冲对象: Frame Buffer Object 
	class FBO : public Asset {
	public:
		FBO() = default;
		// 缓冲区的宽度和高度
		FBO(GLuint width, GLuint height);
		~FBO();

		// 禁用拷贝构造函数和赋值运算符
		FBO(const FBO&) = delete;
		FBO& operator=(const FBO&) = delete;
		FBO(FBO&& other) noexcept = default;
		FBO& operator=(FBO&& other) noexcept = default;

	public:
		void AddColorTexture(GLuint count, bool multisample = false);
		void SetColorTexture(GLenum index, GLuint texture_2d);
		void SetColorTexture(GLenum index, GLuint textureCubemap, GLuint face);
		void AddDepStTexture(bool multisample = false);
		void AddDepStRenderBuffer(bool multisample = false);
		void AddDepthCubemap();
		const Texture& GetColorTexture(GLenum index) const;
		const Texture& GetDepthTexture() const;
		const TexView& GetStencilTexView() const;
		void Bind() const override;
		void UnBind() const override;
		void SetDrawBuffer(GLuint index) const;
		void SetDrawBuffers(std::vector<GLuint> indices) const;
		void SetDrawBuffers() const;
		void Draw(GLint index) const;
		void Clear(GLint index) const;
		void Clear() const;

	public:
		static void CopyColor(const FBO& src, GLuint srcIndex, const FBO& dst, GLuint dstIndex);
		static void CopyDepth(const FBO& src, const FBO& dst);
		static void CopyStencil(const FBO& src, const FBO& dst);

	public:
		GLenum mStatus;                      // 帧缓冲的状态
		GLuint mWidth, mHeight;               // 帧缓冲的宽度和高度

		std::vector<Texture> mColorAttachments;			// 存储颜色附件的向量
		std::unique_ptr<RBO>       mDepstRenderbuffer;  // 深度和模板作为单个渲染缓冲
		std::unique_ptr<Texture>   mDepstTexture;       // 深度和模板作为单个纹理
		std::unique_ptr<TexView>   mStencilView;        // 模板作为临时纹理视图
	};
}


