#pragma once

struct GLFWwindow;
namespace core {
	class Event {
	public:
		static void RegisterCallBacks();
	private:
        static void GlfwError(int error, const char* description);
        static void GlfwCursorEnter(GLFWwindow* window, int entered);
        static void GlfwCursorPos(GLFWwindow* window, double xpos, double ypos);
        static void GlfwMouseButton(GLFWwindow* window, int button, int action, int mods);
        static void GlfwScroll(GLFWwindow* window, double xoffset, double yoffset);
        static void GlfwKey(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void GlfwWindowSize(GLFWwindow* window, int width, int height);
        static void GlfwFramebufferSize(GLFWwindow* window, int width, int height);
        static void GlfwWindowFocus(GLFWwindow* window, int focused);
	};
}