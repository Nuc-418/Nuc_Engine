#include"Controller.h"

void Controller::BasicMoviment(Transform* transform, float rotationOffset, float movmentOffset)
{
	userInputsPtr->UpdateKeyboard(userInputsPtr);
	userInputsPtr->UpdateMouse(true);

	deltaMouseV3 = { userInputsPtr->deltaMouse.x,userInputsPtr->deltaMouse.y,0 };
	transform->Rotate(deltaMouseV3 * rotationOffset);


	float offset = ((userInputsPtr->keyShift) ? (movmentOffset * 2) : movmentOffset);

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
void Controller::AssocieateUserInput(UserInputs* userInputs)
{
	userInputsPtr = userInputs;
}
void Controller::Exit(GLFWwindow* window)
{
	if (userInputsPtr->keyEsc) {
		//Quando for fechada, a janela será destruída
		glfwDestroyWindow(window);
		//Será terminado o contexto
		glfwTerminate();
		//E fechado o programa
		exit(0);
	}
}