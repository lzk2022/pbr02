#include "Texture.h"
#include "../utils/Image.h"
#include "../utils/Global.h"
#include "../utils/Log.h"
#include "../utils/Math.h"
#include "../asset/Shader.h"

using namespace utils;
namespace asset {
	constexpr int gTextureUnitsNum = 32;						// 纹理单元的数量
	static std::vector<GLuint> gTexturesBindingTable(32, 0);	// 记录每个纹理单元绑定的纹理

	Texture::Texture(const std::string& imgPath, GLuint level)
		:Asset(),mTarget(GL_TEXTURE_2D),mDepth(1),mLevel(level){
		LOG_TRACK;
		auto image = Image(imgPath);
		mWidth = image.Width();
		mHeight = image.Height();
		mFormat = image.Format();
		mIFormat = image.IFormat();

		// 如果levels为0，则自动计算需要的mipmap级别数 512 * 1024
		// 取最大值 1024 → 取以2为底的对数 10.0 → 向下取整 10.0 →加1 11
		if (level == 0) {
			mLevel = 1 + static_cast<GLuint>(floor(std::log2(std::max(mWidth, mHeight))));
		}

		// 创建一个2D纹理对象并分配存储空间
		glCreateTextures(GL_TEXTURE_2D, 1, &mId);
		glTextureStorage2D(mId, mLevel, mIFormat, mWidth, mHeight);

		// 根据图像是否为HDR格式选择合适的数据类型上传纹理数据
		if(image.IsHDR()) 
			glTextureSubImage2D(mId, 0, 0, 0, mWidth, mHeight, mFormat, GL_FLOAT, image.Pixels<float>());
		else 
			glTextureSubImage2D(mId, 0, 0, 0, mWidth, mHeight, mFormat, GL_UNSIGNED_BYTE, image.Pixels<uint8_t>());
		
		// 如果存在多级渐远纹理，生成所有级别的mipmap
		if (mLevel > 1) glGenerateTextureMipmap(mId);
		SetSampleState();
	}

	Texture::Texture(const std::string& imgPath, GLuint resolution, GLuint level)
		:Asset(),mWidth(resolution),mHeight(resolution),mLevel(level){
		LOG_TRACK;
		mTarget = GL_TEXTURE_CUBE_MAP;
		mDepth = 6;

		// 分辨率必须是2的幂，以获得高保真视觉效果
		LOG_ASSERT(Math::IsPowerOfTwo(resolution), "尝试构建分辨率不是2的幂的立方体贴图");

		// 如果图像路径的扩展名不是.hdr，则警告可能会降低视觉质量
		if (auto path = std::filesystem::path(imgPath); path.extension() != ".hdr") {
			LOG_WARN("尝试从非HDR图像构建立方体贴图");
			LOG_WARN("色调映射后视觉质量可能会严重下降");
		}

		// 图像加载存储不允许3通道格式，必须使用GL_RGBA
		mFormat = GL_RGBA;
		mIFormat = GL_RGB16F;

		// 如果levels为0，则自动计算需要的mipmap级别数
		if (level == 0) mLevel = Math::CalMipmap(mWidth, mHeight);

		// 将等矩形图像加载到临时2D纹理中（基本级别，无mipmap）
		GLuint equirectangle = 0;
		glCreateTextures(GL_TEXTURE_2D, 1, &equirectangle);

		if (equirectangle > 0) {
			auto image = Image(imgPath, 3);

			GLuint imgW			= image.Width();
			GLuint imgH			= image.Height();
			GLuint imgFormat	= image.Format(); 
			GLenum imgIFormat	= image.IFormat(); 

			// 设置临时等矩形纹理的参数
			glTextureParameteri(equirectangle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(equirectangle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTextureParameteri(equirectangle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(equirectangle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			// 根据图像是否为HDR格式选择合适的数据类型上传纹理数据
			if (image.IsHDR()) {
				glTextureStorage2D(equirectangle, 1, imgIFormat, imgW, imgH);
				glTextureSubImage2D(equirectangle, 0, 0, 0, imgW, imgH, imgFormat, GL_FLOAT, image.Pixels<float>());
			}
			else {
				glTextureStorage2D(equirectangle, 1, imgIFormat, imgW, imgH);
				glTextureSubImage2D(equirectangle, 0, 0, 0, imgW, imgH, imgFormat, GL_UNSIGNED_BYTE, image.Pixels<uint8_t>());
			}
		}

		// 创建一个空的立方体贴图来容纳等矩形纹理
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &mId);
		glTextureStorage2D(mId, mLevel, mIFormat, mWidth, mHeight);

		// 使用计算着色器将2D等矩形投影到立方体贴图的六个面上
		LOG_INFO("创建立方体贴图:{0}", imgPath);
		auto shaderConvert = CShader("core\\equirect2cube.glsl");

		if (shaderConvert.Bind(); true) {
			glBindTextureUnit(0, equirectangle);
			glBindImageTexture(0, mId, 0, GL_TRUE, 0, GL_WRITE_ONLY, mIFormat);
			glDispatchCompute(resolution / 32, resolution / 32, 6);		// 六个面
			glMemoryBarrier(GL_ALL_BARRIER_BITS);						// 等待所有内存访问结束
			glBindTextureUnit(0, 0);
			glBindImageTexture(0, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, mIFormat);
			shaderConvert.UnBind();
		}

		glDeleteTextures(1, &equirectangle);  // 删除临时2D等矩形纹理

		// 如果存在多级渐远纹理，生成所有级别的mipmap
		if (mLevel > 1)  glGenerateTextureMipmap(mId);

		SetSampleState();  // 设置纹理采样状态
	}

	Texture::Texture(const std::string& directory, const std::string& ext, GLuint resolution, GLuint level)
		:Asset(),mWidth(resolution),mHeight(resolution),mLevel(level){
		LOG_TRACK;
		mTarget = GL_TEXTURE_CUBE_MAP;
		mDepth = 6;
		mFormat = GL_RGBA;
		mIFormat = GL_RGBA16F;

		// 分辨率必须是2的幂，以获得高保真视觉效果
		LOG_ASSERT(Math::IsPowerOfTwo(resolution), "尝试构建分辨率不是2的幂的立方体贴图");

		// 这个构造函数期望6个HDR图像作为立方体贴图的六个面，命名规则如下
		static const std::vector<std::string> faces{ "px", "nx", "py", "ny", "pz", "nz" };

		// STB图像库目前不支持".exr"格式
		LOG_ASSERT(ext == ".hdr", "无效的文件扩展名，预期为HDR格式的面");

		std::string filepath = gTexturePath + directory;
		// 检查每个面的文件是否存在
		for (const auto& face : faces) {
			std::string testFace = gResourcePath + directory + face + ext;
			if (!std::filesystem::exists(std::filesystem::path(testFace))) {
				LOG_ERROR("在目录中找不到立方体贴图面:{0}", testFace);
				return;
			}
		}

		// 如果levels为0，则自动计算需要的mipmap级别数
		if (level == 0)  mLevel = Math::CalMipmap(mWidth, mHeight);

		// 创建一个立方体贴图并分配存储空间
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &mId);
		glTextureStorage2D(mId, mLevel, mIFormat, mWidth, mHeight);

		// 将每个面的HDR图像上传到立方体贴图中
		for (GLuint face = 0; face < 6; face++) {
			auto image =Image(directory + faces[face] + ext, 3, false);
			glTextureSubImage3D(mId, 0, 0, 0, face, mWidth, mHeight, 1, mFormat, GL_FLOAT, image.Pixels<float>());
		}

		// 如果存在多级渐远纹理，生成所有级别的mipmap
		if (mLevel > 1) glGenerateTextureMipmap(mId);
		SetSampleState();  // 设置纹理采样状态
	}

	Texture::Texture(GLenum target, GLuint width, GLuint height, GLuint depth, GLenum iFormat, GLuint level)
		:Asset(),mTarget(target),mWidth(width),mHeight(height),mDepth(depth),mIFormat(iFormat),mLevel(level){
		LOG_TRACK;
		mFormat = 0;
		if (level == 0) mLevel = Math::CalMipmap(mWidth, mHeight);

		// 根据目标类型选择合适的纹理存储函数
		glCreateTextures(mTarget, 1, &mId);

		switch (target) {
		case GL_TEXTURE_2D:
		case GL_TEXTURE_CUBE_MAP:   // depth必须为6
			glTextureStorage2D(mId, mLevel, mIFormat, mWidth, mHeight);
			break;
		case GL_TEXTURE_2D_MULTISAMPLE: 
			glTextureStorage2DMultisample(mId, 4, mIFormat, mWidth, mHeight, GL_TRUE);
			break;
		case GL_TEXTURE_2D_ARRAY:
		case GL_TEXTURE_CUBE_MAP_ARRAY: // depth必须为6 * n_layers
			glTextureStorage3D(mId, mLevel, mIFormat, mWidth, mHeight, mDepth);
			break;
		case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
			glTextureStorage3DMultisample(mId, 4, mIFormat, mWidth, mHeight, mDepth, GL_TRUE);
			break;
		default: 
			LOG_EXCEPTION(false, "不支持的纹理目标");
		}
		SetSampleState();  // 设置纹理采样状态
	}

	Texture::~Texture(){
		LOG_TRACK;
		if (mId == 0) return;

		// 删除纹理对象，纹理ID为0（一个全黑的回退纹理）会被静默忽略
		glDeleteTextures(1, &mId);  

		// 更新纹理绑定表，将绑定了当前纹理的纹理单元置为0
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
		glBindTextureUnit(index, mId);  // 绑定纹理到指定的纹理单元
	}

	void Texture::UnBind(GLuint index) const{
		LOG_TRACK;
		if (gTexturesBindingTable[index] == 0) return;
		gTexturesBindingTable[index] = 0;
		glBindTextureUnit(index, 0);  // 将指定纹理单元上的纹理解绑
	}

	void Texture::BindILS(GLuint level, GLuint index, GLenum access) const
	{
		LOG_ASSERT(level < mLevel, "Mipmap 级别 {0} 在纹理中无效");  // 确保 mipmap 级别有效
		glBindImageTexture(index, mId, level, GL_TRUE, 0, access, mIFormat);  // 绑定图像纹理单元
	}

	void Texture::UnbindILS(GLuint index) const
	{
		glBindImageTexture(index, 0, 0, GL_TRUE, 0, GL_READ_ONLY, mIFormat);  // 解绑图像纹理单元

	}

	void Texture::Copy(const Texture& src, GLuint srcLevel, const Texture& dst, GLuint dstLevel)
	{
		// 检查源纹理和目标纹理的级别是否有效
		LOG_ASSERT(srcLevel < src.mLevel, "Mipmap 级别 {0} 在纹理 {1} 中无效！", srcLevel, src.mId);
		LOG_ASSERT(dstLevel < dst.mLevel, "Mipmap 级别 {0} 在纹理 {1} 中无效！", dstLevel, dst.mId);

		// 计算源纹理和目标纹理的缩放比例
		GLuint srcScale = static_cast<GLuint>(std::pow(2, srcLevel));
		GLuint dstScale = static_cast<GLuint>(std::pow(2, dstLevel));

		// 计算源纹理和目标纹理的宽度、高度和深度
		GLuint srcWidth = src.mWidth / srcScale;
		GLuint srcHeight = src.mHeight / srcScale;
		GLuint srcDepth = src.mDepth;

		GLuint dstWidth = dst.mWidth / dstScale;
		GLuint dstHeight = dst.mHeight / dstScale;
		GLuint dstDepth = dst.mDepth;

		// 检查源纹理和目标纹理的尺寸是否匹配
		if (srcWidth != dstWidth || srcHeight != dstHeight || srcDepth != dstDepth) {
			LOG_ERROR("无法拷贝图像数据，宽度、高度或深度不匹配！");
			return;
		}

		// 检查源纹理和目标纹理的类型是否匹配
		if (src.mTarget != dst.mTarget) {
			LOG_ERROR("无法拷贝图像数据，不兼容的目标类型！");
			return;
		}

		// 执行纹理数据拷贝操作
		glCopyImageSubData(src.mId, src.mTarget, srcLevel, 0, 0, 0, dst.mId, dst.mTarget, dstLevel, 0, 0, 0, srcWidth, srcHeight, srcDepth);
	}

	void Texture::SetSampleState() const{
		LOG_TRACK;
		// GL_LINEAR: 线性过滤、GL_LINEAR_MIPMAP_LINEAR: mipmap线性过滤
		GLint magFilter = GL_LINEAR;										// 设置放大过滤模式
		GLint minFilter = mLevel > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;	// 设置缩小过滤模式

		// 如果支持各向异性过滤，获取最大各向异性值并限制在1到8之间
		static GLfloat sAnisotropy = -1.0f;
		if (sAnisotropy < 0) {
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &sAnisotropy);
			sAnisotropy = std::clamp(sAnisotropy, 1.0f, 8.0f);
		}

		// 根据目标类型设置不同的纹理参数
		switch (mTarget){
		case GL_TEXTURE_2D:			// 2D纹理：		通常用于平面表面
		case GL_TEXTURE_2D_ARRAY:	// 2D纹理数组：	常用于体积纹理、光照贴图、多视角纹理等
			switch (mIFormat){
			case GL_RG16F: {
				// 设置S和T方向的纹理环绕模式为GL_CLAMP_TO_EDGE
				// GL_CLAMP_TO_EDGE: 纹理坐标超出纹理边界时，纹理的边缘像素将被复制以填充额外的区域
				// 具体来说，纹理坐标在[0, 1]范围之外的部分会被限制在纹理边缘的像素值
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
				// 设置缩小和放大过滤模式为 GL_NEAREST : 用于指定纹理采样时的最近点过滤方式
				// 当纹理坐标对应的像素不在纹理图像的中心时，GL_NEAREST 会选择与纹理坐标最接近的那个像素的颜色作为最终的采样结果。
				// 这种方式不会进行插值计算，因此采样的颜色是最接近的那个像素的颜色，可能会导致图像在放大时出现锯齿状的效果。
				glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				break;
			}
			default: {
				glTextureParameteri(mId, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTextureParameteri(mId, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, minFilter);
				glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, magFilter);
				// 各向异性过滤用于改善纹理在斜视角下的质量，减少纹理的模糊和失真
				// 通过增加最大各向异性级别，可以提高纹理在斜视角下的清晰度，但会增加计算负担和内存使用
				glTextureParameterf(mId, GL_TEXTURE_MAX_ANISOTROPY, sAnisotropy);
				break;
			}}
			break;
		case GL_TEXTURE_CUBE_MAP:			// 立方体纹理：		常用于环境映射、反射和折射效果等场景
		case GL_TEXTURE_CUBE_MAP_ARRAY:{	// 立方体纹理数组：	常用于动态环境映射
			glTextureParameteri(mId, GL_TEXTURE_MIN_FILTER, minFilter);
			glTextureParameteri(mId, GL_TEXTURE_MAG_FILTER, magFilter);
			// 设置纹理S、T、R 轴包裹模式
			// GL_CLAMP_TO_BORDER:指定超出纹理边界的纹理坐标应使用边界颜色进行填充
			glTextureParameteri(mId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTextureParameteri(mId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTextureParameteri(mId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
			const float border[] = { 0.0f, 0.0f, 0.0f, 1.0f };
			glTextureParameterfv(mId, GL_TEXTURE_BORDER_COLOR, border);	// 设置边界颜色
			break;
		}
		case GL_TEXTURE_2D_MULTISAMPLE:
		case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
			// 多重采样纹理不进行过滤，硬件会处理所有多重采样操作
			return;
		default:
			LOG_ASSERT(false, "纹理目标不支持");
			break;
		}
	}

	/***********************************TexView**************************************/
	TexView::TexView(const Texture& texture) : Asset(), host(texture){
		LOG_TRACK;
		glGenTextures(1, &mId);  // 创建视图的纹理名，但不初始化纹理对象
	}
	TexView::~TexView(){
		LOG_TRACK;
		glDeleteTextures(1, &mId);  // 删除视图的纹理名
	}
	void TexView::SetView(GLenum target, GLuint srcLevel, GLuint dstLevel, GLuint srcLayer, GLuint dstLayer) const{
		LOG_TRACK;
		glTextureView(mId, target, host.getId(), host.IFormat(), srcLevel, dstLevel, srcLayer, dstLayer);
	}
	void TexView::Bind(GLuint index) const{
		LOG_TRACK;
		if (mId != gTexturesBindingTable[index]) {
			glBindTextureUnit(index, mId);			// 绑定纹理视图到指定的纹理单元
			gTexturesBindingTable[index] = mId;		// 更新记录的纹理绑定状态
		}
	}
	void TexView::UnBind(GLuint index) const{
		LOG_TRACK;
		if (mId == gTexturesBindingTable[index]) {
			glBindTextureUnit(index, 0);  // 解绑指定纹理单元上的纹理
			gTexturesBindingTable[index] = 0;  // 更新记录的纹理绑定状态
		}
	}
}
