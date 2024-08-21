#include "Texture.h"
#include "../utils/Image.h"
#include "../utils/Global.h"
#include "../utils/Log.h"
#include "../utils/Math.h"
#include "../asset/Shader.h"

using namespace utils;
namespace asset {
	constexpr int gTextureUnitsNum = 32;						// ����Ԫ������
	static std::vector<GLuint> gTexturesBindingTable(32, 0);	// ��¼ÿ������Ԫ�󶨵�����

	Texture::Texture(const std::string& imgPath, GLuint level)
		:Asset(),mTarget(GL_TEXTURE_2D),mDepth(1),mLevel(level){
		LOG_TRACK;
		auto image = Image(imgPath);
		mWidth = image.Width();
		mHeight = image.Height();
		mFormat = image.Format();
		mIFormat = image.IFormat();

		// ���levelsΪ0�����Զ�������Ҫ��mipmap������ 512 * 1024
		// ȡ���ֵ 1024 �� ȡ��2Ϊ�׵Ķ��� 10.0 �� ����ȡ�� 10.0 ����1 11
		if (level == 0) {
			mLevel = 1 + static_cast<GLuint>(floor(std::log2(std::max(mWidth, mHeight))));
		}

		// ����һ��2D������󲢷���洢�ռ�
		glCreateTextures(GL_TEXTURE_2D, 1, &mId);
		glTextureStorage2D(mId, mLevel, mIFormat, mWidth, mHeight);

		// ����ͼ���Ƿ�ΪHDR��ʽѡ����ʵ����������ϴ���������
		if(image.IsHDR()) 
			glTextureSubImage2D(mId, 0, 0, 0, mWidth, mHeight, mFormat, GL_FLOAT, image.Pixels<float>());
		else 
			glTextureSubImage2D(mId, 0, 0, 0, mWidth, mHeight, mFormat, GL_UNSIGNED_BYTE, image.Pixels<uint8_t>());
		
		// ������ڶ༶��Զ�����������м����mipmap
		if (mLevel > 1) glGenerateTextureMipmap(mId);
		SetSampleState();
	}

	Texture::Texture(const std::string& imgPath, GLuint resolution, GLuint level)
		:Asset(),mWidth(resolution),mHeight(resolution),mLevel(level){
		LOG_TRACK;
		mTarget = GL_TEXTURE_CUBE_MAP;
		mDepth = 6;

		// �ֱ��ʱ�����2���ݣ��Ի�ø߱����Ӿ�Ч��
		LOG_ASSERT(Math::IsPowerOfTwo(resolution), "���Թ����ֱ��ʲ���2���ݵ���������ͼ");

		// ���ͼ��·������չ������.hdr���򾯸���ܻή���Ӿ�����
		if (auto path = std::filesystem::path(imgPath); path.extension() != ".hdr") {
			LOG_WARN("���Դӷ�HDRͼ�񹹽���������ͼ");
			LOG_WARN("ɫ��ӳ����Ӿ��������ܻ������½�");
		}

		// ͼ����ش洢������3ͨ����ʽ������ʹ��GL_RGBA
		mFormat = GL_RGBA;
		mIFormat = GL_RGB16F;

		// ���levelsΪ0�����Զ�������Ҫ��mipmap������
		if (level == 0) mLevel = Math::CalMipmap(mWidth, mHeight);

		// ���Ⱦ���ͼ����ص���ʱ2D�����У�����������mipmap��
		GLuint equirectangle = 0;
		glCreateTextures(GL_TEXTURE_2D, 1, &equirectangle);

		if (equirectangle > 0) {
			auto image = Image(imgPath, 3);

			GLuint imgW			= image.Width();
			GLuint imgH			= image.Height();
			GLuint imgFormat	= image.Format(); 
			GLenum imgIFormat	= image.IFormat(); 

			// ������ʱ�Ⱦ�������Ĳ���
			glTextureParameteri(equirectangle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(equirectangle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTextureParameteri(equirectangle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(equirectangle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			// ����ͼ���Ƿ�ΪHDR��ʽѡ����ʵ����������ϴ���������
			if (image.IsHDR()) {
				glTextureStorage2D(equirectangle, 1, imgIFormat, imgW, imgH);
				glTextureSubImage2D(equirectangle, 0, 0, 0, imgW, imgH, imgFormat, GL_FLOAT, image.Pixels<float>());
			}
			else {
				glTextureStorage2D(equirectangle, 1, imgIFormat, imgW, imgH);
				glTextureSubImage2D(equirectangle, 0, 0, 0, imgW, imgH, imgFormat, GL_UNSIGNED_BYTE, image.Pixels<uint8_t>());
			}
		}

		// ����һ���յ���������ͼ�����ɵȾ�������
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &mId);
		glTextureStorage2D(mId, mLevel, mIFormat, mWidth, mHeight);

		// ʹ�ü�����ɫ����2D�Ⱦ���ͶӰ����������ͼ����������
		LOG_INFO("������������ͼ:{0}", imgPath);
		auto shaderConvert = CShader("core\\equirect2cube.glsl");

		if (shaderConvert.Bind(); true) {
			glBindTextureUnit(0, equirectangle);
			glBindImageTexture(0, mId, 0, GL_TRUE, 0, GL_WRITE_ONLY, mIFormat);
			glDispatchCompute(resolution / 32, resolution / 32, 6);		// ������
			glMemoryBarrier(GL_ALL_BARRIER_BITS);						// �ȴ������ڴ���ʽ���
			glBindTextureUnit(0, 0);
			glBindImageTexture(0, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, mIFormat);
			shaderConvert.UnBind();
		}

		glDeleteTextures(1, &equirectangle);  // ɾ����ʱ2D�Ⱦ�������

		// ������ڶ༶��Զ�����������м����mipmap
		if (mLevel > 1)  glGenerateTextureMipmap(mId);

		SetSampleState();  // �����������״̬
	}

	Texture::Texture(const std::string& directory, const std::string& ext, GLuint resolution, GLuint level)
		:Asset(),mWidth(resolution),mHeight(resolution),mLevel(level){
		LOG_TRACK;
		mTarget = GL_TEXTURE_CUBE_MAP;
		mDepth = 6;
		mFormat = GL_RGBA;
		mIFormat = GL_RGBA16F;

		// �ֱ��ʱ�����2���ݣ��Ի�ø߱����Ӿ�Ч��
		LOG_ASSERT(Math::IsPowerOfTwo(resolution), "���Թ����ֱ��ʲ���2���ݵ���������ͼ");

		// ������캯������6��HDRͼ����Ϊ��������ͼ�������棬������������
		static const std::vector<std::string> faces{ "px", "nx", "py", "ny", "pz", "nz" };

		// STBͼ���Ŀǰ��֧��".exr"��ʽ
		LOG_ASSERT(ext == ".hdr", "��Ч���ļ���չ����Ԥ��ΪHDR��ʽ����");

		std::string filepath = gTexturePath + directory;
		// ���ÿ������ļ��Ƿ����
		for (const auto& face : faces) {
			std::string testFace = gResourcePath + directory + face + ext;
			if (!std::filesystem::exists(std::filesystem::path(testFace))) {
				LOG_ERROR("��Ŀ¼���Ҳ�����������ͼ��:{0}", testFace);
				return;
			}
		}

		// ���levelsΪ0�����Զ�������Ҫ��mipmap������
		if (level == 0)  mLevel = Math::CalMipmap(mWidth, mHeight);

		// ����һ����������ͼ������洢�ռ�
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &mId);
		glTextureStorage2D(mId, mLevel, mIFormat, mWidth, mHeight);

		// ��ÿ�����HDRͼ���ϴ�����������ͼ��
		for (GLuint face = 0; face < 6; face++) {
			auto image =Image(directory + faces[face] + ext, 3, false);
			glTextureSubImage3D(mId, 0, 0, 0, face, mWidth, mHeight, 1, mFormat, GL_FLOAT, image.Pixels<float>());
		}

		// ������ڶ༶��Զ�����������м����mipmap
		if (mLevel > 1) glGenerateTextureMipmap(mId);
		SetSampleState();  // �����������״̬
	}

	Texture::Texture(GLenum target, GLuint width, GLuint height, GLuint depth, GLenum iFormat, GLuint level)
		:Asset(),mTarget(target),mWidth(width),mHeight(height),mDepth(depth),mIFormat(iFormat),mLevel(level){
		LOG_TRACK;
		mFormat = 0;
		if (level == 0) mLevel = Math::CalMipmap(mWidth, mHeight);

		// ����Ŀ������ѡ����ʵ�����洢����
		glCreateTextures(mTarget, 1, &mId);

		switch (target) {
		case GL_TEXTURE_2D:
		case GL_TEXTURE_CUBE_MAP:   // depth����Ϊ6
			glTextureStorage2D(mId, mLevel, mIFormat, mWidth, mHeight);
			break;
		case GL_TEXTURE_2D_MULTISAMPLE: 
			glTextureStorage2DMultisample(mId, 4, mIFormat, mWidth, mHeight, GL_TRUE);
			break;
		case GL_TEXTURE_2D_ARRAY:
		case GL_TEXTURE_CUBE_MAP_ARRAY: // depth����Ϊ6 * n_layers
			glTextureStorage3D(mId, mLevel, mIFormat, mWidth, mHeight, mDepth);
			break;
		case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
			glTextureStorage3DMultisample(mId, 4, mIFormat, mWidth, mHeight, mDepth, GL_TRUE);
			break;
		default: 
			LOG_EXCEPTION(false, "��֧�ֵ�����Ŀ��");
		}
		SetSampleState();  // �����������״̬
	}

	Texture::~Texture(){
		LOG_TRACK;
		if (mId == 0) return;

		// ɾ�������������IDΪ0��һ��ȫ�ڵĻ��������ᱻ��Ĭ����
		glDeleteTextures(1, &mId);  

		// ��������󶨱������˵�ǰ���������Ԫ��Ϊ0
		for (int i = 0; i < gTextureUnitsNum; i++) {
			if (gTexturesBindingTable[i] == mId) {
				gTexturesBindingTable[i] = 0;
			}
		}
	}

	void Texture::Bind(GLuint index) const{
		LOG_TRACK;
		if (mId == gTexturesBindingTable[index]) return;
		gTexturesBindingTable[index] = mId;
		glBindTextureUnit(index, mId);  // ������ָ��������Ԫ
	}

	void Texture::UnBind(GLuint index) const{
		LOG_TRACK;
		if (gTexturesBindingTable[index] == 0) return;
		gTexturesBindingTable[index] = 0;
		glBindTextureUnit(index, 0);  // ��ָ������Ԫ�ϵ�������
	}

	void Texture::BindILS(GLuint level, GLuint index, GLenum access) const
	{
		LOG_ASSERT(level < mLevel, "Mipmap ���� {0} ����������Ч");  // ȷ�� mipmap ������Ч
		glBindImageTexture(index, mId, level, GL_TRUE, 0, access, mIFormat);  // ��ͼ������Ԫ
	}

	void Texture::UnbindILS(GLuint index) const
	{
		glBindImageTexture(index, 0, 0, GL_TRUE, 0, GL_READ_ONLY, mIFormat);  // ���ͼ������Ԫ

	}

	void Texture::Copy(const Texture& src, GLuint srcLevel, const Texture& dst, GLuint dstLevel)
	{
		// ���Դ�����Ŀ������ļ����Ƿ���Ч
		LOG_ASSERT(srcLevel < src.mLevel, "Mipmap ���� {0} ������ {1} ����Ч��", srcLevel, src.mId);
		LOG_ASSERT(dstLevel < dst.mLevel, "Mipmap ���� {0} ������ {1} ����Ч��", dstLevel, dst.mId);

		// ����Դ�����Ŀ����������ű���
		GLuint srcScale = static_cast<GLuint>(std::pow(2, srcLevel));
		GLuint dstScale = static_cast<GLuint>(std::pow(2, dstLevel));

		// ����Դ�����Ŀ������Ŀ�ȡ��߶Ⱥ����
		GLuint srcWidth = src.mWidth / srcScale;
		GLuint srcHeight = src.mHeight / srcScale;
		GLuint srcDepth = src.mDepth;

		GLuint dstWidth = dst.mWidth / dstScale;
		GLuint dstHeight = dst.mHeight / dstScale;
		GLuint dstDepth = dst.mDepth;

		// ���Դ�����Ŀ������ĳߴ��Ƿ�ƥ��
		if (srcWidth != dstWidth || srcHeight != dstHeight || srcDepth != dstDepth) {
			LOG_ERROR("�޷�����ͼ�����ݣ���ȡ��߶Ȼ���Ȳ�ƥ�䣡");
			return;
		}

		// ���Դ�����Ŀ������������Ƿ�ƥ��
		if (src.mTarget != dst.mTarget) {
			LOG_ERROR("�޷�����ͼ�����ݣ������ݵ�Ŀ�����ͣ�");
			return;
		}

		// ִ���������ݿ�������
		glCopyImageSubData(src.mId, src.mTarget, srcLevel, 0, 0, 0, dst.mId, dst.mTarget, dstLevel, 0, 0, 0, srcWidth, srcHeight, srcDepth);
	}

	void Texture::SetSampleState() const{
		LOG_TRACK;
		// GL_LINEAR: ���Թ��ˡ�GL_LINEAR_MIPMAP_LINEAR: mipmap���Թ���
		GLint magFilter = GL_LINEAR;										// ���÷Ŵ����ģʽ
		GLint minFilter = mLevel > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;	// ������С����ģʽ

		// ���֧�ָ������Թ��ˣ���ȡ����������ֵ��������1��8֮��
		static GLfloat sAnisotropy = -1.0f;
		if (sAnisotropy < 0) {
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &sAnisotropy);
			sAnisotropy = std::clamp(sAnisotropy, 1.0f, 8.0f);
		}

		// ����Ŀ���������ò�ͬ���������
		switch (mTarget){
		case GL_TEXTURE_2D:			// 2D����		ͨ������ƽ�����
		case GL_TEXTURE_2D_ARRAY:	// 2D�������飺	�������������������ͼ�����ӽ������
			switch (mIFormat){
			case GL_RG16F: {
				// ����S��T�����������ģʽΪGL_CLAMP_TO_EDGE
				// GL_CLAMP_TO_EDGE: �������곬������߽�ʱ������ı�Ե���ؽ��������������������
				// ������˵������������[0, 1]��Χ֮��Ĳ��ֻᱻ�����������Ե������ֵ
				glTextureParameteri(mId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTextureParameteri(mId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				break;
			}
			case GL_RGB16F:
			case GL_RGBA16F: {
				glTextureParameteri(mId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTextureParameteri(mId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTextureParameteri(mId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				break;
			}
			case GL_DEPTH_COMPONENT: {
				glTextureParameteri(mId, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTextureParameteri(mId, GL_TEXTURE_WRAP_T, GL_REPEAT);
				// ������С�ͷŴ����ģʽΪ GL_NEAREST : ����ָ���������ʱ���������˷�ʽ
				// �����������Ӧ�����ز�������ͼ�������ʱ��GL_NEAREST ��ѡ��������������ӽ����Ǹ����ص���ɫ��Ϊ���յĲ��������
				// ���ַ�ʽ������в�ֵ���㣬��˲�������ɫ����ӽ����Ǹ����ص���ɫ�����ܻᵼ��ͼ���ڷŴ�ʱ���־��״��Ч����
				glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				break;
			}
			default: {
				glTextureParameteri(mId, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTextureParameteri(mId, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, minFilter);
				glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, magFilter);
				// �������Թ������ڸ���������б�ӽ��µ����������������ģ����ʧ��
				// ͨ���������������Լ��𣬿������������б�ӽ��µ������ȣ��������Ӽ��㸺�����ڴ�ʹ��
				glTextureParameterf(mId, GL_TEXTURE_MAX_ANISOTROPY, sAnisotropy);
				break;
			}}
			break;
		case GL_TEXTURE_CUBE_MAP:			// ����������		�����ڻ���ӳ�䡢���������Ч���ȳ���
		case GL_TEXTURE_CUBE_MAP_ARRAY:{	// �������������飺	�����ڶ�̬����ӳ��
			glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, minFilter);
			glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, magFilter);
			// ��������S��T��R �����ģʽ
			// GL_CLAMP_TO_BORDER:ָ����������߽����������Ӧʹ�ñ߽���ɫ�������
			glTextureParameteri(mId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTextureParameteri(mId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTextureParameteri(mId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
			const float border[] = { 0.0f, 0.0f, 0.0f, 1.0f };
			glTextureParameterfv(mId, GL_TEXTURE_BORDER_COLOR, border);	// ���ñ߽���ɫ
			break;
		}
		case GL_TEXTURE_2D_MULTISAMPLE:
		case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
			// ���ز����������й��ˣ�Ӳ���ᴦ�����ж��ز�������
			return;
		default:
			LOG_ASSERT(false, "����Ŀ�겻֧��");
			break;
		}
	}

	/***********************************TexView**************************************/
	TexView::TexView(const Texture& texture) : Asset(), host(texture){
		LOG_TRACK;
		glGenTextures(1, &mId);  // ������ͼ����������������ʼ���������
	}
	TexView::~TexView(){
		LOG_TRACK;
		glDeleteTextures(1, &mId);  // ɾ����ͼ��������
	}
	void TexView::SetView(GLenum target, GLuint srcLevel, GLuint dstLevel, GLuint srcLayer, GLuint dstLayer) const{
		LOG_TRACK;
		glTextureView(mId, target, host.getId(), host.IFormat(), srcLevel, dstLevel, srcLayer, dstLayer);
	}
	void TexView::Bind(GLuint index) const{
		LOG_TRACK;
		if (mId != gTexturesBindingTable[index]) {
			glBindTextureUnit(index, mId);			// ��������ͼ��ָ��������Ԫ
			gTexturesBindingTable[index] = mId;		// ���¼�¼�������״̬
		}
	}
	void TexView::UnBind(GLuint index) const{
		LOG_TRACK;
		if (mId == gTexturesBindingTable[index]) {
			glBindTextureUnit(index, 0);  // ���ָ������Ԫ�ϵ�����
			gTexturesBindingTable[index] = 0;  // ���¼�¼�������״̬
		}
	}
}
