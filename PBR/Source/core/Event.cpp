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
        glfwSetErrorCallback(GlfwError);                // GLFW����ص�
        glfwSetCursorEnterCallback(pWindow, GlfwCursorEnter);   // ������/�뿪�ص�
        glfwSetCursorPosCallback(pWindow, GlfwCursorPos);     // ���λ�ûص�
        glfwSetMouseButtonCallback(pWindow, GlfwMouseButton);   // ��갴ť�ص�
        glfwSetScrollCallback(pWindow, GlfwScroll);        // ���ֻص�
        glfwSetKeyCallback(pWindow, GlfwKey);           // �����¼��ص�
        glfwSetWindowSizeCallback(pWindow, GlfwWindowSize);    // ���ڴ�С�ı�ص�
        glfwSetFramebufferSizeCallback(pWindow, GlfwFramebufferSize); // ֡��������С�ı�ص�
        glfwSetWindowFocusCallback(pWindow, GlfwWindowFocus);   // ���ڽ���ı�ص�
	}

    void Event::GlfwError(int error, const char* description)
    {
        LOG_ERROR("GLFW error detected (code {0}): {1}", error, description);
    }
    void Event::GlfwCursorEnter(GLFWwindow* window, int entered)
    {
        if (entered) {
            LOG_INFO("�����봰��");
        }
        else{
            LOG_INFO("����뿪����");
        }
    }
    void Event::GlfwCursorPos(GLFWwindow* window, double xpos, double ypos)
    {
        // xpos��ypos�Դ������Ͻ�Ϊԭ�����Ļ����ϵ������
        if (Window::mLayer == Layer::Scene) {
            Input::SetCursor(xpos, ypos);
        }
        else if (Window::mLayer == Layer::ImGui) {
            // ����������ڴ���ʱ��GLFW����Ļ������λ�ú�ƫ�Ƽ��㡣
        }
    }
    void Event::GlfwMouseButton(GLFWwindow* window, int button, int action, int mods)
    {
        // GLFW��갴ť�¼��ص�����
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
        // GLFW�����¼��ص�����
        if (Window::mLayer == Layer::ImGui) {
            ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
        }
        else if (Window::mLayer == Layer::Scene) {
            // �봥���岻ͬ��������ֻ���մ�ֱƫ����
            Input::SetScroll(yoffset);
        }
    }
    void Event::GlfwKey(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        // GLFW���̰����¼��ص�����

        // ��Win32��λ�ڶ���ʱ�����������Ȩ�ø�����ϵͳ
        if (Window::mLayer == Layer::Win32) {
            return;
        }

        // ��GLFW�еļ�������ӳ��Ϊ���ǵı�׼���루Win32��ASCII��
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
            return;  // ���Գ�����ע�������ļ�
        }

        if (_key == KB_ESCAPE || _key == KB_ENTER) {
            // ���ܼ��İ����¼���Ӧ�ó�����
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
        // �����ڴ�С�ı�ʱ�����˻ص����������յ����³ߴ�����Ļ���꣬�������������꣬
        // ������ǲ�Ӧ�����������µ����ӿڴ�С��
    }
    void Event::GlfwFramebufferSize(GLFWwindow* window, int width, int height)
    {
        Window::Resize();
    }
    void Event::GlfwWindowFocus(GLFWwindow* window, int focused)
    {
    }
}
