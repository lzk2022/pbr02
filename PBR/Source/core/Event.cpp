#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui_impl_glut.h>
#include <imgui/imgui_impl_glfw.h>
#include "Event.h"
#include "../core/Window.h"
#include "../utils/Log.h"
#include "../core/Input.h"

namespace core {
	void Event::RegisterCallBacks(){
		auto* pWindow = Window::mpWindow;
        glfwSetErrorCallback(GlfwError);                // GLFW错误回调
        glfwSetCursorEnterCallback(pWindow, GlfwCursorEnter);   // 光标进入/离开回调
        glfwSetCursorPosCallback(pWindow, GlfwCursorPos);     // 光标位置回调
        glfwSetMouseButtonCallback(pWindow, GlfwMouseButton);   // 鼠标按钮回调
        glfwSetScrollCallback(pWindow, GlfwScroll);        // 滚轮回调
        glfwSetKeyCallback(pWindow, GlfwKey);           // 键盘事件回调
        glfwSetWindowSizeCallback(pWindow, GlfwWindowSize);    // 窗口大小改变回调
        glfwSetFramebufferSizeCallback(pWindow, GlfwFramebufferSize); // 帧缓冲区大小改变回调
        glfwSetWindowFocusCallback(pWindow, GlfwWindowFocus);   // 窗口焦点改变回调
	}

    void Event::GlfwError(int error, const char* description)
    {
        LOG_ERROR("GLFW error detected (code {0}): {1}", error, description);
    }
    void Event::GlfwCursorEnter(GLFWwindow* window, int entered)
    {
        if (entered) {
            LOG_INFO("光标进入窗口");
        }
        else{
            LOG_INFO("光标离开窗口");
        }
    }
    void Event::GlfwCursorPos(GLFWwindow* window, double xpos, double ypos)
    {
        // xpos和ypos以窗口左上角为原点的屏幕坐标系来衡量
        if (Window::mLayer == Layer::Scene) {
            Input::SetCursor(xpos, ypos);
        }
        else if (Window::mLayer == Layer::ImGui) {
            // 当光标锁定在窗口时，GLFW会在幕后处理光标位置和偏移计算。
        }
    }
    void Event::GlfwMouseButton(GLFWwindow* window, int button, int action, int mods)
    {
        // GLFW鼠标按钮事件回调函数
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            Input::SetMouseDown(MouseButton::Left, action == GLFW_PRESS);
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            Input::SetMouseDown(MouseButton::Right, action == GLFW_PRESS);
        }
        else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
            Input::SetMouseDown(MouseButton::Middle, action == GLFW_PRESS);
        }
    }
    void Event::GlfwScroll(GLFWwindow* window, double xoffset, double yoffset)
    {
        // GLFW滚轮事件回调函数
        if (Window::mLayer == Layer::ImGui) {
            ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
        }
        else if (Window::mLayer == Layer::Scene) {
            // 与触摸板不同，鼠标滚轮只接收垂直偏移量
            Input::SetScroll(yoffset);
        }
    }
    void Event::GlfwKey(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        // GLFW键盘按键事件回调函数

        // 当Win32层位于顶部时，将输入控制权让给操作系统
        if (Window::mLayer == Layer::Win32) {
            return;
        }

        // 将GLFW中的键码重新映射为我们的标准键码（Win32和ASCII）
        unsigned char _key = '0';
        switch (key) {
        case GLFW_KEY_UP:     case GLFW_KEY_W:         _key = 'w';        break;
        case GLFW_KEY_DOWN:   case GLFW_KEY_S:         _key = 's';        break;
        case GLFW_KEY_LEFT:   case GLFW_KEY_A:         _key = 'a';        break;
        case GLFW_KEY_RIGHT:  case GLFW_KEY_D:         _key = 'd';        break;
        case GLFW_KEY_Z:                               _key = 'z';        break;
        case GLFW_KEY_R:                               _key = 'r';        break;
        case GLFW_KEY_SPACE:                           _key = KB_SPACE;   break;
        case GLFW_KEY_ENTER:  case GLFW_KEY_KP_ENTER:  _key = KB_ENTER;  break;
        case GLFW_KEY_ESCAPE:                          _key = KB_ESCAPE;  break;
        default:
            return;  // 忽略除上述注册键以外的键
        }

        if (_key == KB_ESCAPE || _key == KB_ENTER) {
            // 功能键的按下事件由应用程序处理
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                Input::SetKeyDown(_key, true);
            }
            return;
        }

        if (Window::mLayer == Layer::Scene) {
            Input::SetKeyDown(_key, action != GLFW_RELEASE);
        }
        else if (Window::mLayer == Layer::ImGui) {
            ImGui_ImplGlfw_KeyCallback(Window::mpWindow, key, scancode, action, mods);
        }
    }
    void Event::GlfwWindowSize(GLFWwindow* window, int width, int height)
    {
        // 当窗口大小改变时触发此回调，但它接收到的新尺寸是屏幕坐标，而不是像素坐标，
        // 因此我们不应该在这里重新调整视口大小。
    }
    void Event::GlfwFramebufferSize(GLFWwindow* window, int width, int height)
    {
        Window::Resize();
    }
    void Event::GlfwWindowFocus(GLFWwindow* window, int focused)
    {
    }
}
