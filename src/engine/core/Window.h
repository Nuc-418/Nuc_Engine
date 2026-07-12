// Window: thin wrapper around a GLFW window.

#pragma once
#define GLFW_USE_DWM_SWAP_INTERVAL
#include <GLFW/glfw3.h>

class Window
{
public :

  // Requests a 4.4 compatibility context (the shaders are #version 440 and
  // the engine uses glGetProgramResourceLocation, i.e. GL 4.3+); falls back
  // to a default, hint-less context if creation fails.
  bool NewWindow(int width, int height, const char* windowName, GLFWmonitor* monitor, GLFWwindow* share);

  void SetWindowSize(int width, int height);

  void SetWindowPos(int x, int y);

  void MakeContextCurrent();
	GLFWwindow* windowPtr;

};