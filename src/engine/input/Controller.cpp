#include "engine/input/Controller.h"

void Controller::BasicMovement(Transform* transform, float rotationOffset, float movementOffset)
{
	userInputsPtr->UpdateKeyboard(userInputsPtr);
	userInputsPtr->UpdateMouse(true);

	deltaMouseV3 = { userInputsPtr->deltaMouse.x,userInputsPtr->deltaMouse.y,0 };
	transform->Rotate(deltaMouseV3 * rotationOffset);


	float offset = ((userInputsPtr->keyShift) ? (movementOffset * 2) : movementOffset);

	if (userInputsPtr->keyW)
		transform->Translate(transform->forward * offset);
	if (userInputsPtr->keyA)
		transform->Translate(-transform->right * offset);
	if (userInputsPtr->keyS)
		transform->Translate(-transform->forward * offset);
	if (userInputsPtr->keyD)
		transform->Translate(transform->right * offset);
	if (userInputsPtr->keySpace)
		transform->Translate(transform->up * offset);
	if (userInputsPtr->keyCtrl)
		transform->Translate(-transform->up * offset);

}
void Controller::AssociateUserInput(UserInputs* userInputs)
{
	userInputsPtr = userInputs;
}
void Controller::RequestExit(GLFWwindow* window)
{
	if (userInputsPtr->keyEsc)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}