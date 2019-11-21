/*
Autores: Francisco Aires (14884) 
Data: 05/06/2019

Descrição: UserInputs.h
Ficheiro que cria e gere UserInputs.cpp
 */

#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp> // vec3, vec4, ivec4, mat4, ...
#include <glm/gtc/matrix_transform.hpp> // translate, rotate, scale, perspective, ...
#include <glm/gtc/type_ptr.hpp> // value_ptr
#include <iostream>
#include <vector>
#include <windows.h>



class UserInputs
{
public:
	UserInputs() {};
	UserInputs(GLFWwindow *window, int wWidth, int wHeight)
	{
		windowPtr = window;
		windowWidth = wWidth;
		windowHeight = wHeight;
	}
	void AssociateWindow(GLFWwindow *window, int wWidth, int wHeight);


	GLFWwindow *windowPtr;
	int windowWidth;
	int windowHeight;



	POINT mousePos = { NULL };
	glm::vec2 deltaMouse;
	glm::vec2 deltaMouseSum;

	bool keyW = false;
	bool keyA = false;
	bool keyS = false;
	bool keyD = false;
	bool keySpace = false;
	bool keyCtrl = false;
	bool keyShift = false;
	bool keyEsc = false;

	bool key0 = false;
	bool key1 = false;
	bool key2 = false;
	bool key3 = false;
	bool key4 = false;
	bool key5 = false;
	bool key6 = false;
	bool key7 = false;
	bool key8 = false;
	bool key9 = false;

	bool onceKey0 = false;
	bool onceKey1 = false;
	bool onceKey2 = false;
	bool onceKey3 = false;
	bool onceKey4 = false;
	bool onceKey5 = false;
	bool onceKey6 = false;
	bool onceKey7 = false;
	bool onceKey8 = false;
	bool onceKey9 = false;


	void UpdateKeyboard(UserInputs* userInputsPtr);

	void UpdateMouse(bool getDelta);

};
