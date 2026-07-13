// UserInputs: keyboard and mouse state captured from GLFW callbacks.

#include "engine/input/UserInputs.h"

static void GetKeyInfo(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	UserInputs* inputs = (UserInputs*)glfwGetWindowUserPointer(window);
	if (!inputs || key < 0 || key >= UserInputs::KeyCount)
		return;
	if (action == GLFW_PRESS) {
		inputs->keyDown[key] = true;
		inputs->keyPressed[key] = true;
	}
	if (action == GLFW_RELEASE)
		inputs->keyDown[key] = false;
}

void UserInputs::AssociateWindow(GLFWwindow *window, int wWidth, int wHeight)
{
	windowPtr = window;
	windowWidth = wWidth;
	windowHeight = wHeight;

	glfwSetWindowUserPointer(windowPtr, this);
	glfwSetKeyCallback(windowPtr, GetKeyInfo);
}

void UserInputs::SetCursorCaptured(bool captured)
{
	glfwSetInputMode(windowPtr, GLFW_CURSOR, captured ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
}

void UserInputs::SetWindowSize(int wWidth, int wHeight)
{
	windowWidth = wWidth;
	windowHeight = wHeight;
}

void UserInputs::CenterCursor()
{
	// Content-area center in GLFW cursor coordinates (top-left origin).
	glfwSetCursorPos(windowPtr, windowWidth / 2.0, windowHeight / 2.0);
}

void UserInputs::ClearPressed()
{
	for (int i = 0; i < KeyCount; i++)
		keyPressed[i] = false;
}

/*Atualização da posição do cursor*/
void UserInputs::UpdateMouse(bool getDelta)
{
	glfwGetCursorPos(windowPtr, &mousePos.x, &mousePos.y);

	if (getDelta)
	{
		// Window center in content-area coordinates.
		double centerX = windowWidth / 2.0;
		double centerY = windowHeight / 2.0;

		deltaMouse.x = (float)(mousePos.x - centerX);
		deltaMouse.y = (float)(mousePos.y - centerY);

		deltaMouseSum += deltaMouse;

		if (deltaMouse.x != 0 || deltaMouse.y != 0)
			glfwSetCursorPos(windowPtr, centerX, centerY);
	}
}
