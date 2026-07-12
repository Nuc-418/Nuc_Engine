// Controller: predefined WASD/mouse-look movement built on UserInputs.

#pragma once
#include "engine/input/UserInputs.h"
#include "engine/scene/Transform.h"

class Controller
{
public:
	Controller() {};
	Controller(UserInputs* userInputs) { userInputsPtr = userInputs; }

	void AssocieateUserInput(UserInputs* userInputs);

	UserInputs* userInputsPtr;

	glm::vec3 deltaMouseV3;

	void BasicMoviment(Transform* transform, float rotationOffset, float movmentOffset);

	void Exit(GLFWwindow* window);
};