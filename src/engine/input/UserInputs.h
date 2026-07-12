// UserInputs: keyboard and mouse state captured from GLFW callbacks.

#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp> // vec2
#include <windows.h> // POINT, GetCursorPos/SetCursorPos



class UserInputs
{
public:
	UserInputs() {};
	UserInputs(GLFWwindow *window, int wWidth, int wHeight)
	{
		windowPtr = window;
		windowWidth = wWidth;
		windowHeight = wHeight;
	}
	// Stores the window, updates the recenter math, and installs the key
	// callback once. Install any other callback consumers (e.g. ImGui's
	// GLFW backend with install_callbacks=true) AFTER this so they chain.
	void AssociateWindow(GLFWwindow *window, int wWidth, int wHeight);

	// Show or hide+capture the OS cursor (GLFW input mode, not Win32).
	void SetCursorCaptured(bool captured);

	// Keeps the mouse recenter math in sync after a window resize.
	void SetWindowSize(int wWidth, int wHeight);

	// Warps the cursor to the window center so the next look delta is zero.
	void CenterCursor();


	GLFWwindow *windowPtr = nullptr;
	int windowWidth = 0;
	int windowHeight = 0;



	POINT mousePos = { NULL };
	glm::vec2 deltaMouse = glm::vec2(0.0f);
	glm::vec2 deltaMouseSum = glm::vec2(0.0f);

	bool keyW = false;
	bool keyA = false;
	bool keyS = false;
	bool keyD = false;
	bool keySpace = false;
	bool keyCtrl = false;
	bool keyShift = false;
	bool keyEsc = false;

	bool key0 = false;
	bool key1 = false;
	bool key2 = false;
	bool key3 = false;
	bool key4 = false;
	bool key5 = false;
	bool key6 = false;
	bool key7 = false;
	bool key8 = false;
	bool key9 = false;

	bool onceKey0 = false;
	bool onceKey1 = false;
	bool onceKey2 = false;
	bool onceKey3 = false;
	bool onceKey4 = false;
	bool onceKey5 = false;
	bool onceKey6 = false;
	bool onceKey7 = false;
	bool onceKey8 = false;
	bool onceKey9 = false;


	void UpdateMouse(bool getDelta);

};
