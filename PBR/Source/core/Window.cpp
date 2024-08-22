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
	static HWND gHWND = nullptr;  // Win32内部窗口句柄
	void Window::Init()
	{
		LOG_TRACK;
		LOG_ASSERT(glfwInit(), "未能初始化 GLFW");
		mWidth = 1600;
		mHeight = 900;

		const GLFWvidmode* kVmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		mPosX = (kVmode->width - mWidth) / 2;
		mPosY = (kVmode->height - mHeight) / 2;

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_SAMPLES, 4);   // 每像素4个样本的多重采样

		// 提示启用调试上下文
		if constexpr (gIsDebug) glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);           

		mpWindow = glfwCreateWindow(mWidth, mHeight, utils::ToU8(mTitle).c_str(),
			NULL,	// monitor: 如果希望创建全屏窗口，则指定要全屏显示的监视器，否则为 NULL 表示创建窗口模式
			NULL	// share: 指定要共享资源的另一个窗口，否则为 NULL 表示不共享
		);
		LOG_ASSERT(mpWindow != nullptr, "未能创建主窗体....");

		glfwSetWindowPos(mpWindow, mPosX, mPosY);                     // 设置窗口位置
		glfwSetWindowAspectRatio(mpWindow, 16, 9);                    // 设置窗口宽高比
		glfwSetWindowAttrib(mpWindow, GLFW_RESIZABLE, GLFW_FALSE);    // 禁止窗口大小调整
		glfwMakeContextCurrent(mpWindow);                             // 设置当前上下文为该窗口
		glfwSwapInterval(0);                                            // 禁用垂直同步以测试性能
		
		int ret = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);	// 使用GLFW加载器初始化GLAD
		LOG_ASSERT(ret, "未能初始化 GLAD");

		gHWND = ::FindWindow(NULL, (LPCWSTR)(L"PBR"));
		LONG style = GetWindowLong(gHWND, GWL_STYLE) ^ WS_SYSMENU;
		SetWindowLong(gHWND, GWL_STYLE, style);
	}
	void Window::Clear()
	{
		LOG_TRACK;
		if (mpWindow) {
			glfwDestroyWindow(mpWindow);      // 销毁窗口
			glfwTerminate();                    // 终止GLFW库
			mpWindow = nullptr;
		}
	}
	void Window::Rename(const std::string& title)
	{
		LOG_TRACK;
		mTitle = title;
		glfwSetWindowTitle(mpWindow, utils::ToU8(mTitle).c_str());  // 设置窗口标题
	}
	void Window::Resize()
	{
		// 在这个演示中，我们简单地锁定窗口位置、大小和宽高比
		glfwSetWindowPos(mpWindow, mPosX, mPosY);  // 设置窗口位置
		glfwSetWindowSize(mpWindow, mWidth, mHeight);  // 设置窗口大小
		glfwSetWindowAspectRatio(mpWindow, 16, 9);  // 设置窗口宽高比

		// 视口位置以像素为单位，相对于窗口左下角
		glViewport(0, 0, mWidth, mHeight);  // 设置OpenGL视口
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
		// 记住当前图层
		auto cacheLayer = mLayer;

		// 切换到Win32图层，让操作系统接管
		mLayer = Layer::Win32;
		Input::ShowCursor();

		// 将光标移动到取消按钮位置，然后弹出消息框
		glfwSetCursorPos(Window::mpWindow, 892, 515);

		// 弹出Windows退出确认消息框
		int buttonId = MessageBox(NULL,
			(LPCWSTR)L"你要关闭窗口吗？",
			(LPCWSTR)L"PBR.exe",
			MB_OKCANCEL | MB_ICONQUESTION | MB_DEFBUTTON1 | MB_SETFOREGROUND
		);


		if (buttonId == IDOK) {
			// 返回控制给应用程序并在那里退出，这样我们有机会清理资源
			// 如果直接在这里退出，可能会导致内存泄漏
			return true;
		}
		else if (buttonId == IDCANCEL) {
			mLayer = cacheLayer;  // 恢复图层
			if (cacheLayer == Layer::Scene) {
				Input::HideCursor();
			}
		}

		return false;
	}
	void Window::OnLayerSwitch()
	{
		mLayer = (mLayer == Layer::ImGui) ? Layer::Scene : Layer::ImGui;  // 切换图层
		if (mLayer == Layer::ImGui) {
			Input::ShowCursor();  // 显示鼠标光标
		}
		else {
			Input::HideCursor();  // 隐藏鼠标光标
			Input::Clear();  // 清除输入状态
		}
	}
}
