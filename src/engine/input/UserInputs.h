// UserInputs: raw keyboard and mouse state captured from GLFW callbacks.
//
// Keys are a plain state table (held + this-frame press edges) instead of
// one bool per hardcoded key; gameplay code reads named actions through
// InputActions rather than this raw state. The key callback finds its
// instance through the GLFW window user pointer, so there is no file-static
// single-handler restriction. Install other callback consumers (e.g. ImGui's
// GLFW backend with install_callbacks=true) AFTER AssociateWindow so they
// chain; ImGui does not touch the window user pointer.

#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp> // vec2, dvec2

class UserInputs
{
public:
	static const int KeyCount = GLFW_KEY_LAST + 1;

	UserInputs() {}

	// Stores the window (as its user pointer), updates the recenter math,
	// and installs the key callback once.
	void AssociateWindow(GLFWwindow *window, int wWidth, int wHeight);

	// Show or hide+capture the OS cursor (GLFW input mode, not Win32).
	void SetCursorCaptured(bool captured);

	// Keeps the mouse recenter math in sync after a window resize.
	void SetWindowSize(int wWidth, int wHeight);

	// Warps the cursor to the window center so the next look delta is zero.
	void CenterCursor();

	// Clears the per-frame press edges; Application::Run calls this once per
	// frame after InputActions snapshots the state (and before the next
	// glfwPollEvents accumulates new edges).
	void ClearPressed();

	GLFWwindow *windowPtr = nullptr;
	int windowWidth = 0;
	int windowHeight = 0;

	// Raw key state, indexed by GLFW_KEY_*. keyDown = held right now;
	// keyPressed = went down since the last ClearPressed.
	bool keyDown[KeyCount] = {};
	bool keyPressed[KeyCount] = {};

	// Cursor position in GLFW content-area coordinates (top-left origin).
	glm::dvec2 mousePos = glm::dvec2(0.0);
	glm::vec2 deltaMouse = glm::vec2(0.0f);
	glm::vec2 deltaMouseSum = glm::vec2(0.0f);

	void UpdateMouse(bool getDelta);
};
