#pragma once
#include <memory>
#include <string>
#include <glad/glad.h>
namespace utils {
	class Image {
	public:
		Image(const std::string& filepath, GLuint channel = 0, bool flip = false);

		// 复制构造函数已删除，因为std::unique_ptr是移动语义
		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;

		// 移动构造函数和移动赋值运算符默认实现
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
		GLint mChannel;		// 图像通道
		bool mIsHDR;
		// 自定义删除器结构体，用于释放图像数据内存
		struct deleter
		{
			void operator()(uint8_t* buffer);		// 删除器操作符重载函数
		};

		std::unique_ptr<uint8_t, deleter> mPixels;	// 使用自定义删除器的unique_ptr，管理图像像素数据
	};
}

