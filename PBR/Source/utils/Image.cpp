#include "Image.h"
#define STB_IMAGE_IMPLEMENTATION
#ifdef STB_IMAGE_WRITE_IMPLEMENTATION
#undef STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#include <stb/stb_image.h>
#include <vector>
#include "Ext.h"
#include "../utils/Log.h"
#include <glm/glm.hpp>                   // GLM��OpenGL��ѧ��
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
		LOG_ASSERT_TRUE(ranges::find(kExt, ext) == kExt.end(), "ͼ���ļ���ʽ����֧��:" + filepath);

		LOG_INFO("����ͼƬ��" + filepath);
		mIsHDR = stbi_is_hdr(filepath.c_str());
		if (mIsHDR) {
			float* buffer = stbi_loadf(filepath.c_str(), &mWidth, &mHeight, &mChannel, 4);
			LOG_ASSERT_TRUE(buffer == nullptr, "����ͼƬʧ�ܣ�{0}\tԭ��{1}", filepath, stbi_failure_reason());
			PrintHDRInfo(buffer);
			mPixels.reset(reinterpret_cast<uint8_t*>(buffer));
		}
		else {
			LOG_ASSERT_TRUE(channel > 4, "��ǰͨ��{0}�����ֻ�ܶ�ȡ4��ͨ��ͼ����Դ",channel);
			uint8_t* buffer = stbi_load(filepath.c_str(), &mWidth, &mHeight, &mChannel, channel);
			LOG_ASSERT_TRUE(buffer == nullptr, "����ͼƬʧ�ܣ�{0}\tԭ��{1}",filepath,stbi_failure_reason());
			mPixels.reset(buffer);
		}
		LOG_ASSERT_TRUE(mChannel > 4, "ͼ���ʽ����ͨ����Ϊ{0}", mChannel);
		LOG_ASSERT_TRUE(mPixels == nullptr, "��ȡͼ������ʧ��:{0}",filepath);
	}
	GLenum Image::Format() const
	{
		if (mIsHDR)  return GL_RGBA;
		switch (mChannel){
			case 1:  return GL_RED;		// �Ҷ�
			case 2:  return GL_RG;		// �Ҷ� + alpha
			case 3:  return GL_RGB;
			case 4:  return GL_RGBA;
			default: return 0;
		}
	}
	GLenum Image::IFormat() const
	{
		if (mIsHDR) return GL_RGBA16F;
		switch (mChannel) {
			case 1:  return GL_R8;		// �Ҷ�
			case 2:  return GL_RG8;		// �Ҷ� + alpha
			case 3:  return GL_RGB8;
			case 4:  return GL_RGBA8;
			default: return 0;
		}
	}

	// ��ȡͼ���������ݵ�ָ�루ģ���ػ��汾��
	// ����ֵ��ָ���������ݵ�const T*ָ��
	template<typename T>
	const T* Image::Pixels() const {
		return reinterpret_cast<const T*>(mPixels.get());
	}

	// ��ʽʵ����ģ�庯��
	template const uint8_t* Image::Pixels<uint8_t>() const;
	template const float* Image::Pixels<float>() const;

	void Image::PrintHDRInfo(float* buffer)
	{
		// �����û���ָ��֮ǰ����HDRͼ��ͳ����Ϣ
		size_t pixels = mWidth * mHeight;
		float minLuminace = std::numeric_limits<float>::max();		// ��С����
		float maxLuminance = std::numeric_limits<float>::min();		// �������
		float sunLogLuminance = 0.0f;
		for (size_t i = 0; i < pixels; i++) {
			const float* pixel_ptr = buffer + (i * 3);				// ��ǰ�ƶ�3(rgb)��ͨ��

			auto color = glm::vec3(pixel_ptr[0], pixel_ptr[1], pixel_ptr[2]);
			auto luminance = glm::dot(color, glm::vec3(0.2126f, 0.7152f, 0.0722f));

			minLuminace = std::min(minLuminace, luminance);
			maxLuminance = std::max(maxLuminance, luminance);
			sunLogLuminance += std::max(log(luminance + 0.00001f), 0.0f);	// ���������
		}
		float logAverageLuminance = exp(sunLogLuminance / pixels);

		LOG_INFO("HDRͼ�����ȱ���:");
		LOG_INFO("��Сֵ��" + std::to_string(minLuminace));
		LOG_INFO("���ֵ��" + std::to_string(maxLuminance));
		LOG_INFO("ƽ��ֵ��" + std::to_string(logAverageLuminance));

		float luminanceDiff = maxLuminance - minLuminace;
		if (luminanceDiff > 10000.0f) {
			LOG_WARN("�����HDRͼ��̫����һЩ���ص�ֵ�ӽ������!");
			LOG_WARN("����ܵ���IBL�е�����αӰ��������ȫ��ɫ��ͼ��!");
			LOG_WARN("��ʹ�ò�ͬ��ͼ����ֶ������ع�ֵ��EV��!");
		}
	}
}
