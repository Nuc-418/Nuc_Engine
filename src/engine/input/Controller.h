// Controller: predefined WASD/mouse-look movement built on UserInputs.

#pragma once
#include "engine/input/UserInputs.h"
#include "engine/scene/Transform.h"

class Controller
{
public:
	Controller() {};
	Controller(UserInputs* userInputs) { userInputsPtr = userInputs; }

	void AssociateUserInput(UserInputs* userInputs);

	UserInputs* userInputsPtr = nullptr;

	glm::vec3 deltaMouseV3;

	void BasicMovement(Transform* transform, float rotationOffset, float movementOffset);

	// Requests a clean shutdown (window close) when ESC is pressed.
	void RequestExit(GLFWwindow* window);
};