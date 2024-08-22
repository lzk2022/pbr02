#include "Input.h"
#include "../utils/Log.h"
#include "../core/Window.h"
#include <GLFW/glfw3.h>

namespace core {

	std::unordered_map<unsigned char, bool> Input::mKeybook{
		{'w',0},{'s',0},{'a',0},{'d',0},{'z',0},{'r',0},
		{VK_SPACE,0},{VK_RETURN,0},{VK_ESCAPE,0}
	};
	void Input::Clear()
	{
		LOG_TRACK;
		// ������а���״̬
		for (auto& keystate : mKeybook) {
			keystate.second = 0;
		}

		// �����λ������Ϊ��������
		mCursorPosX = Window::Width() * 0.5f;
		mCursorPosY = Window::Height() * 0.5f;
		SetCursor(mCursorPosX, mCursorPosY);

		// ���ù��ƫ�ơ������״̬�͹���ƫ��
		mCursorDeltaX = mCursorDeltaY = 0.0f;
		mMouseButtonL = mMouseButtonR = mMouseButtonR = false;
		mScrollOffset = 0.0f;
		
	}
	void Input::HideCursor()
	{
		LOG_TRACK;
		glfwSetInputMode(Window::mpWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	void Input::ShowCursor()
	{
		LOG_TRACK;
		glfwSetInputMode(Window::mpWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	void Input::SetKeyDown(unsigned char key, bool isPressed)
	{
		LOG_TRACK;
		if(mKeybook.find(key) == mKeybook.end()) {
			return;
		}
		mKeybook[key] = isPressed;
	}
	bool Input::GetMouseDown(MouseButton button)
	{
		LOG_TRACK;
		switch (button) {
		case MouseButton::Left:   return mMouseButtonL;
		case MouseButton::Right:  return mMouseButtonR;
		case MouseButton::Middle: return mMouseButtonM;
		default:
			return false;
		}
	}
	float Input::GetCursorOffset(MouseAxis axis)
	{
		LOG_TRACK;
		float offset = 0.0f;
		if (axis == MouseAxis::Horizontal) {
			offset = mCursorDeltaX;
			mCursorDeltaX = 0;  // ���ù��ƫ����
		}
		else if (axis == MouseAxis::Vertical) {
			offset = mCursorDeltaY;
			mCursorDeltaY = 0;  // ���ù��ƫ����
		}

		return offset;
	}
	bool Input::GetKeyDown(unsigned char key)
	{
		// ����δע����keybook�еİ���
		if (mKeybook.find(key) == mKeybook.end()) {
			return false;
		}
		return mKeybook[key];
	}
	glm::ivec2 Input::GetCursorPosition()
	{
		return glm::ivec2(mCursorPosX, mCursorPosY);
	}
	void Input::SetCursor(float newX, float newY)
	{
		LOG_TRACK;
		mCursorDeltaX = newX - mCursorPosX;
		mCursorDeltaY = newY - mCursorPosY;
		glfwSetCursorPos(Window::mpWindow, mCursorPosX, mCursorPosY);
	}
	void Input::SetMouseDown(MouseButton button, bool pressed)
	{
		switch (button) {
		case MouseButton::Left:   mMouseButtonL = pressed;  break;
		case MouseButton::Right:  mMouseButtonR = pressed;  break;
		case MouseButton::Middle: mMouseButtonM = pressed;  break;
		default:
			return;
		}
	}
	void Input::SetScroll(float offset)
	{
		mScrollOffset += offset;
	}
}