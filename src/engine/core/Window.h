// Window: thin wrapper around a GLFW window.

#pragma once
#define GLFW_USE_DWM_SWAP_INTERVAL
#include <GLFW/glfw3.h>

class Window
{
public :

  bool NewWindow(int width, int height, char* windowName, GLFWmonitor* monitor,GLFWwindow* share);

  void SetWindowPos(int x, int y);

  void MakeContextCurrent();
	GLFWwindow* windowPtr;

};