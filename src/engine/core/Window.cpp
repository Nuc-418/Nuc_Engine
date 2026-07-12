#include "engine/core/Window.h"

bool Window::NewWindow(int width, int height, const char* windowName, GLFWmonitor* monitor, GLFWwindow* share)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	windowPtr = glfwCreateWindow(width, height, windowName, monitor, share);
	if (!windowPtr) {
		// Retry with the pre-hint behavior (driver picks the version).
		glfwDefaultWindowHints();
		windowPtr = glfwCreateWindow(width, height, windowName, monitor, share);
	}
	if (!windowPtr) {
		glfwTerminate();
		return false;
	}
	return true;
}

void Window::SetWindowSize(int width, int height)
{
	glfwSetWindowSize(windowPtr, width, height);
}

void  Window::SetWindowPos(int x, int y)
{
	glfwSetWindowPos(windowPtr, x, y);
}

void Window::MakeContextCurrent()
{
	glfwMakeContextCurrent(windowPtr);
}