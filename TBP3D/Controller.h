/*
Autores: Francisco Aires (14884)
Data: 05/06/2019

Descrição: Controller.h
Ficheiro com controladores predefinidos
 */

#pragma once
#include "UserInputs.h"
#include "Transform.h"

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