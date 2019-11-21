/*
Autores: Beatriz Abreu (14874), Francisco Aires (14884) e Ronaldo Veloso (14850)
Data: 05/06/2019

Descri��o: Camera.cpp
Ficheiro que trata informa��es da c�mara
 */

#include "Camera.h"
#include <iostream>
#include <glm\gtc\matrix_inverse.hpp> // glm::inverseTranspose()
using namespace std;


/*Fun��o que cria uma c�mara dados uma determinada posi��o, um alvo e um vetor normal*/
Camera::Camera(glm::vec3 camPos, glm::vec3 lookAt, glm::vec3 vecUp)
{
	transform.position = camPos;

	projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 1000.0f);

	view = glm::lookAt(
		camPos,	//Posi��o
		lookAt,	//Alvo 
		vecUp	//Vetor normal
	);

	viewProjection = projection * view;
}

/*Fun��o que atualiza a c�mara e retorna o valor da matriz MVP*/
glm::mat4 Camera::GetMVP(glm::mat4 model)
{
	UpdateCam();

	return (viewProjection * model);
}

/*Fun��o que envia as diversas matrizes da c�mara para o programa shader*/
void Camera::CamToProgram(GLuint program, glm::mat4 model)
{
	
	/*Os valores Model, View e Projection da matriz MVP s�o enviados para o programa shader*/
	GLint modelId = glGetProgramResourceLocation(program, GL_UNIFORM, "Model");
	glProgramUniformMatrix4fv(program, modelId, 1, GL_FALSE, glm::value_ptr(model));

	GLint viewId = glGetProgramResourceLocation(program, GL_UNIFORM, "View");
	glProgramUniformMatrix4fv(program, viewId, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 modelView = view * model;
	normalMatrix = glm::inverseTranspose(glm::mat3(view * model));

	GLint modelViewId = glGetProgramResourceLocation(program, GL_UNIFORM, "ModelView");
	glProgramUniformMatrix4fv(program, modelViewId, 1, GL_FALSE, glm::value_ptr(modelView));

	GLint normalMatrixId = glGetProgramResourceLocation(program, GL_UNIFORM, "NormalMatrix");
	glProgramUniformMatrix3fv(program, normalMatrixId, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	GLint mvpId = glGetProgramResourceLocation(program, GL_UNIFORM, "MVP");
	glProgramUniformMatrix4fv(program, mvpId, 1, GL_FALSE, glm::value_ptr(GetMVP(model)));

}

/*Fun��o que atualiza a c�mara*/
void Camera::UpdateCam()
{

	transform.CalcRotationMatrix();
	transform.CalcLocalAxisCam();

	view = glm::lookAt(
		transform.position,	//Posi��o
		transform.forward + transform.position,	//Alvo
		transform.up	//Vetor normal
	);

	viewProjection = projection * view;
}