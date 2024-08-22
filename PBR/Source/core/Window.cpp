#include <windows.h>
//#include <gdiplus.h>
//#include <gdiplusinit.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Window.h"

#include "../utils/Global.h"
#include "../utils/Ext.h"
#include "../core/Input.h"
#include "../utils/Log.h"

namespace core {
	static HWND gHWND = nullptr;  // Win32�ڲ����ھ��
	void Window::Init()
	{
		LOG_TRACK;
		LOG_ASSERT(glfwInit(), "δ�ܳ�ʼ�� GLFW");
		mWidth = 1600;
		mHeight = 900;

		const GLFWvidmode* kVmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		mPosX = (kVmode->width - mWidth) / 2;
		mPosY = (kVmode->height - mHeight) / 2;

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_SAMPLES, 4);   // ÿ����4�������Ķ��ز���

		// ��ʾ���õ���������
		if constexpr (gIsDebug) glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);           

		mpWindow = glfwCreateWindow(mWidth, mHeight, utils::ToU8(mTitle).c_str(),
			NULL,	// monitor: ���ϣ������ȫ�����ڣ���ָ��Ҫȫ����ʾ�ļ�����������Ϊ NULL ��ʾ��������ģʽ
			NULL	// share: ָ��Ҫ������Դ����һ�����ڣ�����Ϊ NULL ��ʾ������
		);
		LOG_ASSERT(mpWindow != nullptr, "δ�ܴ���������....");

		glfwSetWindowPos(mpWindow, mPosX, mPosY);                     // ���ô���λ��
		glfwSetWindowAspectRatio(mpWindow, 16, 9);                    // ���ô��ڿ�߱�
		glfwSetWindowAttrib(mpWindow, GLFW_RESIZABLE, GLFW_FALSE);    // ��ֹ���ڴ�С����
		glfwMakeContextCurrent(mpWindow);                             // ���õ�ǰ������Ϊ�ô���
		glfwSwapInterval(0);                                            // ���ô�ֱͬ���Բ�������
		
		int ret = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);	// ʹ��GLFW��������ʼ��GLAD
		LOG_ASSERT(ret, "δ�ܳ�ʼ�� GLAD");

		gHWND = ::FindWindow(NULL, (LPCWSTR)(L"PBR"));
		LONG style = GetWindowLong(gHWND, GWL_STYLE) ^ WS_SYSMENU;
		SetWindowLong(gHWND, GWL_STYLE, style);
	}
	void Window::Clear()
	{
		LOG_TRACK;
		if (mpWindow) {
			glfwDestroyWindow(mpWindow);      // ���ٴ���
			glfwTerminate();                    // ��ֹGLFW��
			mpWindow = nullptr;
		}
	}
	void Window::Rename(const std::string& title)
	{
		LOG_TRACK;
		mTitle = title;
		glfwSetWindowTitle(mpWindow, utils::ToU8(mTitle).c_str());  // ���ô��ڱ���
	}
	void Window::Resize()
	{
		// �������ʾ�У����Ǽ򵥵���������λ�á���С�Ϳ�߱�
		glfwSetWindowPos(mpWindow, mPosX, mPosY);  // ���ô���λ��
		glfwSetWindowSize(mpWindow, mWidth, mHeight);  // ���ô��ڴ�С
		glfwSetWindowAspectRatio(mpWindow, 16, 9);  // ���ô��ڿ�߱�

		// �ӿ�λ��������Ϊ��λ������ڴ������½�
		glViewport(0, 0, mWidth, mHeight);  // ����OpenGL�ӿ�
	}
	GLFWwindow* Window::WindowPtr()
	{
		return mpWindow;
	}
	float Window::Width()
	{
		return mWidth;
	}
	float Window::Height()
	{
		return mHeight;
	}
	bool Window::OnExitRequest()
	{
		// ��ס��ǰͼ��
		auto cacheLayer = mLayer;

		// �л���Win32ͼ�㣬�ò���ϵͳ�ӹ�
		mLayer = Layer::Win32;
		Input::ShowCursor();

		// ������ƶ���ȡ����ťλ�ã�Ȼ�󵯳���Ϣ��
		glfwSetCursorPos(Window::mpWindow, 892, 515);

		// ����Windows�˳�ȷ����Ϣ��
		int buttonId = MessageBox(NULL,
			(LPCWSTR)L"��Ҫ�رմ�����",
			(LPCWSTR)L"PBR.exe",
			MB_OKCANCEL | MB_ICONQUESTION | MB_DEFBUTTON1 | MB_SETFOREGROUND
		);


		if (buttonId == IDOK) {
			// ���ؿ��Ƹ�Ӧ�ó����������˳������������л���������Դ
			// ���ֱ���������˳������ܻᵼ���ڴ�й©
			return true;
		}
		else if (buttonId == IDCANCEL) {
			mLayer = cacheLayer;  // �ָ�ͼ��
			if (cacheLayer == Layer::Scene) {
				Input::HideCursor();
			}
		}

		return false;
	}
	void Window::OnLayerSwitch()
	{
		mLayer = (mLayer == Layer::ImGui) ? Layer::Scene : Layer::ImGui;  // �л�ͼ��
		if (mLayer == Layer::ImGui) {
			Input::ShowCursor();  // ��ʾ�����
		}
		else {
			Input::HideCursor();  // ���������
			Input::Clear();  // �������״̬
		}
	}
}
