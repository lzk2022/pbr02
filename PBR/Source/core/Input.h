#pragma once
#include <unordered_map>
#include <GLM/glm.hpp>
namespace core {
	enum class MouseAxis : char {
		Horizontal,  // ˮƽ��
		Vertical     // ��ֱ��
	};

	enum class MouseButton : char {
		Left,    // ���
		Middle,  // �м�
		Right    // �Ҽ�
	};
	class Input {
	public:
		static void Clear();
		static void HideCursor();
		static void ShowCursor();
		static void SetKeyDown(unsigned char key, bool isPressed);
		static bool GetMouseDown(MouseButton button);
		// ��ȡ���ƫ����
		static float GetCursorOffset(MouseAxis axis);	
		static bool GetKeyDown(unsigned char key);
		static glm::ivec2 GetCursorPosition();
		static void SetCursor(float newX,float newY);
		static void SetMouseDown(MouseButton button, bool pressed);
		static void SetScroll(float offset);
	private:
		static std::unordered_map<uint8_t, bool> mKeybook;
		inline static float mCursorPosX = 0;		// ��굱ǰλ�õ�X����
		inline static float mCursorPosY = 0;		// ��굱ǰλ�õ�Y����
		inline static float mCursorDeltaX = 0;		// ���X����ı仯��
		inline static float mCursorDeltaY = 0;		// ���Y����ı仯��

		inline static bool mMouseButtonL = false;	// ����������״̬
		inline static bool mMouseButtonR = false;	// ����Ҽ�����״̬
		inline static bool mMouseButtonM = false;	// ����м�����״̬
		inline static bool mScrollOffset = 0.0f;	// ������ƫ����
	};
}
