/*
Autores: Beatriz Abreu (14874), Francisco Aires (14884) e Ronaldo Veloso (14850)
Data: 05/06/2019

Descrição: UserInputs.cpp
Ficheiro que gere inputs do utilizador
 */

#include "UserInputs.h"

static UserInputs *selectedUserInput = NULL;

void UserInputs::AssociateWindow(GLFWwindow *window, int wWidth, int wHeight)
{
	windowPtr = window;
	windowWidth = wWidth;
	windowHeight = wHeight;
}


/*Função que recebe valores do input do utilizador*/
void GetKeyInfo(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_W && action == GLFW_PRESS)
		selectedUserInput->keyW = true;
	if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		selectedUserInput->keyW = false;

	if (key == GLFW_KEY_A && action == GLFW_PRESS)
		selectedUserInput->keyA = true;
	if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		selectedUserInput->keyA = false;

	if (key == GLFW_KEY_S && action == GLFW_PRESS)
		selectedUserInput->keyS = true;
	if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		selectedUserInput->keyS = false;

	if (key == GLFW_KEY_D && action == GLFW_PRESS)
		selectedUserInput->keyD = true;
	if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		selectedUserInput->keyD = false;

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		selectedUserInput->keySpace = true;
	if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
		selectedUserInput->keySpace = false;

	if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS)
		selectedUserInput->keyCtrl = true;
	if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE)
		selectedUserInput->keyCtrl = false;

	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
		selectedUserInput->keyShift = true;
	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
		selectedUserInput->keyShift = false;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		selectedUserInput->keyEsc = true;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
		selectedUserInput->keyEsc = false;


	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		selectedUserInput->key1 = true;
		((selectedUserInput->onceKey1) ? selectedUserInput->onceKey1 = false : selectedUserInput->onceKey1 = true);
	}

	if (key == GLFW_KEY_1 && action == GLFW_RELEASE)
		selectedUserInput->key1 = false;

	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		selectedUserInput->key2 = true;
		((selectedUserInput->onceKey2) ? selectedUserInput->onceKey2 = false : selectedUserInput->onceKey2 = true);
	}

	if (key == GLFW_KEY_2 && action == GLFW_RELEASE)
		selectedUserInput->key2 = false;

	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
	{
		selectedUserInput->key3 = true;
		((selectedUserInput->onceKey3) ? selectedUserInput->onceKey3 = false : selectedUserInput->onceKey3 = true);
	}

	if (key == GLFW_KEY_3 && action == GLFW_RELEASE)
		selectedUserInput->key3 = false;

	if (key == GLFW_KEY_4 && action == GLFW_PRESS)
	{
		selectedUserInput->key4 = true;
		((selectedUserInput->onceKey4) ? selectedUserInput->onceKey4 = false : selectedUserInput->onceKey4 = true);
	}

	if (key == GLFW_KEY_4 && action == GLFW_RELEASE)
		selectedUserInput->key4 = false;

	if (key == GLFW_KEY_5 && action == GLFW_PRESS)
	{
		selectedUserInput->key5 = true;

		((selectedUserInput->onceKey5) ? selectedUserInput->onceKey5 = false : selectedUserInput->onceKey5 = true);
	}
	if (key == GLFW_KEY_5 && action == GLFW_RELEASE)
		selectedUserInput->key5 = false;

	if (key == GLFW_KEY_6 && action == GLFW_PRESS)
	{
		selectedUserInput->key6 = true;

		((selectedUserInput->onceKey6) ? selectedUserInput->onceKey6 = false : selectedUserInput->onceKey6 = true);
	}
	if (key == GLFW_KEY_6 && action == GLFW_RELEASE)
		selectedUserInput->key6 = false;

	if (key == GLFW_KEY_7 && action == GLFW_PRESS)
	{
		selectedUserInput->key7 = true;

		((selectedUserInput->onceKey7) ? selectedUserInput->onceKey7 = false : selectedUserInput->onceKey7 = true);
	}
	if (key == GLFW_KEY_7 && action == GLFW_RELEASE)
		selectedUserInput->key7 = false;

	if (key == GLFW_KEY_8 && action == GLFW_PRESS)
	{
		selectedUserInput->key8 = true;

		((selectedUserInput->onceKey8) ? selectedUserInput->onceKey8 = false : selectedUserInput->onceKey8 = true);
	}
	if (key == GLFW_KEY_8 && action == GLFW_RELEASE)
		selectedUserInput->key8 = false;

	if (key == GLFW_KEY_9 && action == GLFW_PRESS)
	{
		selectedUserInput->key9 = true;

		((selectedUserInput->onceKey9) ? selectedUserInput->onceKey9 = false : selectedUserInput->onceKey9 = true);
	}
	if (key == GLFW_KEY_9 && action == GLFW_RELEASE)
		selectedUserInput->key9 = false;




}

/*Atualização dos inputs do teclado*/
void UserInputs::UpdateKeyboard(UserInputs*userInputsPtr)
{
	selectedUserInput = userInputsPtr;
	glfwSetKeyCallback(windowPtr, GetKeyInfo);
}

/*Atualização da posição do cursor*/
void UserInputs::UpdateMouse(bool getDelta)
{
	int x = 0, y = 0;


	GetCursorPos(&mousePos);

	if (getDelta)
	{
		//Obter pos da window
		glfwGetWindowPos(windowPtr, &x, &y);

		//Obter o centro da window
		x += (int)fabs(windowWidth / 2);
		y += (int)fabs(windowHeight / 2);

		deltaMouse.x = (float)mousePos.x - x;
		deltaMouse.y = (float)mousePos.y - y;

		deltaMouseSum += deltaMouse;

		if (deltaMouse.x != 0 || deltaMouse.y != 0)
			SetCursorPos(x, y);
	}
}