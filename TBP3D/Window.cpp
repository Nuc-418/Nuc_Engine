#include"Window.h"

bool Window::NewWindow(int width, int height, char* windowName, GLFWmonitor* monitor, GLFWwindow* share)
{
	windowPtr = glfwCreateWindow(width, height, windowName, monitor, share);
	if (!windowPtr) {
		glfwTerminate();
		return false;
	}
	return true;
}

void  Window::SetWindowPos(int x, int y)
{
	glfwSetWindowPos(windowPtr, x, y);
}

void Window::MakeContextCurrent()
{
	glfwMakeContextCurrent(windowPtr);
}