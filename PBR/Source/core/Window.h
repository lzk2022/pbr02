#pragma once
#include <glad/glad.h>
#include <string>
struct GLFWwindow;		// 前向声明
namespace core {
	enum class Layer : char {
		Scene, ImGui, Win32
	};
	class Window {
	public:
		static void Init();
		static void Clear();
		static void Rename(const std::string& title);
		static void Resize();
		static GLFWwindow* WindowPtr();
		static float Width();
		static float Height();
		static bool OnExitRequest();
	public:
		inline static std::string mTitle = "PBR";
		inline static GLuint mWidth = 0;
		inline static GLuint mHeight = 0;
		inline static GLuint mPosX = 0;
		inline static GLuint mPosY = 0;
		inline static float mAspectRatio = 16.0f / 9.0f;
		static inline Layer mLayer = Layer::Scene;  // 当前窗口图层
	public:
		inline static GLFWwindow* mpWindow = nullptr;
	};
}

