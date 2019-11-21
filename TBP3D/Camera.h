/*
Autores: Francisco Aires (14884)
Data: 05/06/2019

Descrição: Camera.h
Ficheiro que cria e gere Camera.cpp
 */

#pragma once
#include "Transform.h"

class Camera 
{
public:

	Camera(glm::vec3 camPos, glm::vec3 lookAt, glm::vec3 vecUp);

	glm::mat4 GetMVP(glm::mat4 model);

	void CamToProgram(GLuint program, glm::mat4 model);

	
	Transform transform;

	//Update
	void UpdateCam();

private:
	
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 viewProjection;
	glm::mat4 normalMatrix;

};


