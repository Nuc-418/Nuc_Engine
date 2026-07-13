// Controller: predefined movement/mouse-look built on named input actions.

#pragma once
#include "engine/input/UserInputs.h"
#include "engine/input/InputActions.h"
#include "engine/scene/Transform.h"

class Controller
{
public:
	Controller() {};

	void AssociateUserInput(UserInputs* userInputs, InputActions* inputActions);

	UserInputs* userInputsPtr = nullptr;
	InputActions* actionsPtr = nullptr;

	glm::vec3 deltaMouseV3;

	// Mouse-look plus the MoveForward/MoveRight/MoveUp axes ("Sprint" doubles
	// the speed) — bindings installed by Application::Init.
	void BasicMovement(Transform* transform, float rotationOffset, float movementOffset);

	// Requests a clean shutdown (window close) while "Exit" is held.
	void RequestExit(GLFWwindow* window);
};
