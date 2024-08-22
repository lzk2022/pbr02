#pragma once
#include <unordered_map>
#include <GLM/glm.hpp>
namespace core {
	enum class MouseAxis : char {
		Horizontal,  // 水平轴
		Vertical     // 垂直轴
	};

	enum class MouseButton : char {
		Left,    // 左键
		Middle,  // 中键
		Right    // 右键
	};
	class Input {
	public:
		static void Clear();
		static void HideCursor();
		static void ShowCursor();
		static void SetKeyDown(unsigned char key, bool isPressed);
		static bool GetMouseDown(MouseButton button);
		// 获取光标偏移量
		static float GetCursorOffset(MouseAxis axis);	
		static bool GetKeyDown(unsigned char key);
		static glm::ivec2 GetCursorPosition();
		static void SetCursor(float newX,float newY);
		static void SetMouseDown(MouseButton button, bool pressed);
		static void SetScroll(float offset);
	private:
		static std::unordered_map<uint8_t, bool> mKeybook;
		inline static float mCursorPosX = 0;		// 光标当前位置的X坐标
		inline static float mCursorPosY = 0;		// 光标当前位置的Y坐标
		inline static float mCursorDeltaX = 0;		// 光标X坐标的变化量
		inline static float mCursorDeltaY = 0;		// 光标Y坐标的变化量

		inline static bool mMouseButtonL = false;	// 鼠标左键按下状态
		inline static bool mMouseButtonR = false;	// 鼠标右键按下状态
		inline static bool mMouseButtonM = false;	// 鼠标中键按下状态
		inline static bool mScrollOffset = 0.0f;	// 鼠标滚轮偏移量
	};
}
