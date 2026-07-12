// Camera: perspective camera; uploads Model/View/MVP matrices to shader programs.

#pragma once
#include <GL/glew.h>
#include "engine/scene/Transform.h"

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


