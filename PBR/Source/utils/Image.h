#pragma once
#include <memory>
#include <string>
#include <glad/glad.h>
namespace utils {
	class Image {
	public:
		Image(const std::string& filepath, GLuint channel = 0, bool flip = false);

		// ���ƹ��캯����ɾ������Ϊstd::unique_ptr���ƶ�����
		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;

		// �ƶ����캯�����ƶ���ֵ�����Ĭ��ʵ��
		Image(Image&& other) noexcept = default;
		Image& operator = (Image&& other) noexcept = default;

		bool IsHDR() const { return mIsHDR; }
		GLuint Width() const { return static_cast<GLuint>(mWidth); }
		GLuint Height() const { return static_cast<GLuint>(mHeight); };
		GLenum Format() const;
		GLenum IFormat() const;

		template<typename T>
		const T* Pixels() const;

	private:
		void PrintHDRInfo(float* buffer);

	private:
		GLint mWidth;
		GLint mHeight;
		GLint mChannel;		// ͼ��ͨ��
		bool mIsHDR;
		// �Զ���ɾ�����ṹ�壬�����ͷ�ͼ�������ڴ�
		struct deleter
		{
			void operator()(uint8_t* buffer);		// ɾ�������������غ���
		};

		std::unique_ptr<uint8_t, deleter> mPixels;	// ʹ���Զ���ɾ������unique_ptr������ͼ����������
	};
}

