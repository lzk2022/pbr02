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
	// ������������Ⱦ�д洢��Ⱦ���
	// ֡�������: Frame Buffer Object 
	class FBO : public Asset {
	public:
		FBO() = default;
		// �������Ŀ�Ⱥ͸߶�
		FBO(GLuint width, GLuint height);
		~FBO();

		// ���ÿ������캯���͸�ֵ�����
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
		GLenum mStatus;                      // ֡�����״̬
		GLuint mWidth, mHeight;               // ֡����Ŀ�Ⱥ͸߶�

		std::vector<Texture> mColorAttachments;			// �洢��ɫ����������
		std::unique_ptr<RBO>       mDepstRenderbuffer;  // ��Ⱥ�ģ����Ϊ������Ⱦ����
		std::unique_ptr<Texture>   mDepstTexture;       // ��Ⱥ�ģ����Ϊ��������
		std::unique_ptr<TexView>   mStencilView;        // ģ����Ϊ��ʱ������ͼ
	};
}


