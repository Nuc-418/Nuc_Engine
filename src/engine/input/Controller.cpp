#include "engine/input/Controller.h"

void Controller::BasicMovement(Transform* transform, float rotationOffset, float movementOffset)
{
	userInputsPtr->UpdateMouse(true);

	deltaMouseV3 = { userInputsPtr->deltaMouse.x,userInputsPtr->deltaMouse.y,0 };
	transform->Rotate(deltaMouseV3 * rotationOffset);

	float offset = actionsPtr->IsDown("Sprint") ? (movementOffset * 2) : movementOffset;

	transform->Translate(transform->forward * (offset * actionsPtr->Axis("MoveForward")));
	transform->Translate(transform->right * (offset * actionsPtr->Axis("MoveRight")));
	transform->Translate(transform->up * (offset * actionsPtr->Axis("MoveUp")));
}

void Controller::AssociateUserInput(UserInputs* userInputs, InputActions* inputActions)
{
	userInputsPtr = userInputs;
	actionsPtr = inputActions;
}

void Controller::RequestExit(GLFWwindow* window)
{
	if (actionsPtr->IsDown("Exit"))
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}
