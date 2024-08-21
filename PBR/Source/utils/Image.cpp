#include "Image.h"
#define STB_IMAGE_IMPLEMENTATION
#ifdef STB_IMAGE_WRITE_IMPLEMENTATION
#undef STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#include <stb/stb_image.h>
#include <vector>
#include "Ext.h"
#include "../utils/Log.h"
#include <glm/glm.hpp>                   // GLM：OpenGL数学库
#include "../utils/Global.h"
namespace utils {
	void Image::deleter::operator()(uint8_t* buffer)
	{
		LOG_TRACK;
		if (buffer != nullptr) stbi_image_free(buffer);
	}

	Image::Image(const std::string& path, GLuint channel, bool flip)
		: mWidth(0),mHeight(0),mChannel(0)
	{
		LOG_TRACK;
		std::string filepath = gResourcePath + path;
		LOG_ASSERT_FILE(filepath);
		stbi_set_flip_vertically_on_load(flip);
		static const std::vector<std::string> kExt{ ".jpg", ".png", ".jpeg", ".bmp", ".hdr", ".exr" };

		auto ext = filepath.substr(filepath.rfind("."));
		LOG_ASSERT_TRUE(ranges::find(kExt, ext) == kExt.end(), "图像文件格式不受支持:" + filepath);

		LOG_INFO("加载图片：" + filepath);
		mIsHDR = stbi_is_hdr(filepath.c_str());
		if (mIsHDR) {
			float* buffer = stbi_loadf(filepath.c_str(), &mWidth, &mHeight, &mChannel, 4);
			LOG_ASSERT_TRUE(buffer == nullptr, "加载图片失败：{0}\t原因：{1}", filepath, stbi_failure_reason());
			PrintHDRInfo(buffer);
			mPixels.reset(reinterpret_cast<uint8_t*>(buffer));
		}
		else {
			LOG_ASSERT_TRUE(channel > 4, "当前通道{0}，最多只能读取4个通道图像资源",channel);
			uint8_t* buffer = stbi_load(filepath.c_str(), &mWidth, &mHeight, &mChannel, channel);
			LOG_ASSERT_TRUE(buffer == nullptr, "加载图片失败：{0}\t原因：{1}",filepath,stbi_failure_reason());
			mPixels.reset(buffer);
		}
		LOG_ASSERT_TRUE(mChannel > 4, "图像格式错误，通道数为{0}", mChannel);
		LOG_ASSERT_TRUE(mPixels == nullptr, "获取图像数据失败:{0}",filepath);
	}
	GLenum Image::Format() const
	{
		if (mIsHDR)  return GL_RGBA;
		switch (mChannel){
			case 1:  return GL_RED;		// 灰度
			case 2:  return GL_RG;		// 灰度 + alpha
			case 3:  return GL_RGB;
			case 4:  return GL_RGBA;
			default: return 0;
		}
	}
	GLenum Image::IFormat() const
	{
		if (mIsHDR) return GL_RGBA16F;
		switch (mChannel) {
			case 1:  return GL_R8;		// 灰度
			case 2:  return GL_RG8;		// 灰度 + alpha
			case 3:  return GL_RGB8;
			case 4:  return GL_RGBA8;
			default: return 0;
		}
	}

	// 获取图像像素数据的指针（模板特化版本）
	// 返回值：指向像素数据的const T*指针
	template<typename T>
	const T* Image::Pixels() const {
		return reinterpret_cast<const T*>(mPixels.get());
	}

	// 显式实例化模板函数
	template const uint8_t* Image::Pixels<uint8_t>() const;
	template const float* Image::Pixels<float>() const;

	void Image::PrintHDRInfo(float* buffer)
	{
		// 在重置缓冲指针之前报告HDR图像统计信息
		size_t pixels = mWidth * mHeight;
		float minLuminace = std::numeric_limits<float>::max();		// 最小亮度
		float maxLuminance = std::numeric_limits<float>::min();		// 最大亮度
		float sunLogLuminance = 0.0f;
		for (size_t i = 0; i < pixels; i++) {
			const float* pixel_ptr = buffer + (i * 3);				// 向前移动3(rgb)个通道

			auto color = glm::vec3(pixel_ptr[0], pixel_ptr[1], pixel_ptr[2]);
			auto luminance = glm::dot(color, glm::vec3(0.2126f, 0.7152f, 0.0722f));

			minLuminace = std::min(minLuminace, luminance);
			maxLuminance = std::max(maxLuminance, luminance);
			sunLogLuminance += std::max(log(luminance + 0.00001f), 0.0f);	// 避免除以零
		}
		float logAverageLuminance = exp(sunLogLuminance / pixels);

		LOG_INFO("HDR图像亮度报告:");
		LOG_INFO("最小值：" + std::to_string(minLuminace));
		LOG_INFO("最大值：" + std::to_string(maxLuminance));
		LOG_INFO("平均值：" + std::to_string(logAverageLuminance));

		float luminanceDiff = maxLuminance - minLuminace;
		if (luminanceDiff > 10000.0f) {
			LOG_WARN("输入的HDR图像太亮，一些像素的值接近无穷大!");
			LOG_WARN("这可能导致IBL中的严重伪影，甚至完全白色的图像!");
			LOG_WARN("请使用不同的图像或手动调整曝光值（EV）!");
		}
	}
}
