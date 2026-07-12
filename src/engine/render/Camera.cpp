// Camera: perspective camera; uploads Model/View/MVP matrices to shader programs.

#include "engine/render/Camera.h"
#include <iostream>
#include <glm/gtc/matrix_inverse.hpp> // glm::inverseTranspose()
#include <glm/gtc/type_ptr.hpp> // value_ptr
using namespace std;


/*Função que cria uma câmara dados uma determinada posição, um alvo e um vetor normal*/
Camera::Camera(glm::vec3 camPos, glm::vec3 lookAt, glm::vec3 vecUp)
{
	transform.position = camPos;

	projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 1000.0f);

	view = glm::lookAt(
		camPos,	//Posição
		lookAt,	//Alvo 
		vecUp	//Vetor normal
	);

	viewProjection = projection * view;
}

/*Função que atualiza a câmara e retorna o valor da matriz MVP*/
glm::mat4 Camera::GetMVP(glm::mat4 model)
{
	UpdateCam();

	return (viewProjection * model);
}

/*Função que envia as diversas matrizes da câmara para o programa shader*/
void Camera::CamToProgram(GLuint program, glm::mat4 model)
{
	auto cached = locationCache.find(program);
	if (cached == locationCache.end()) {
		UniformLocations locations;
		locations.model = glGetProgramResourceLocation(program, GL_UNIFORM, "Model");
		locations.view = glGetProgramResourceLocation(program, GL_UNIFORM, "View");
		locations.modelView = glGetProgramResourceLocation(program, GL_UNIFORM, "ModelView");
		locations.normalMatrix = glGetProgramResourceLocation(program, GL_UNIFORM, "NormalMatrix");
		locations.mvp = glGetProgramResourceLocation(program, GL_UNIFORM, "MVP");
		cached = locationCache.emplace(program, locations).first;
	}
	const UniformLocations& loc = cached->second;

	/*Os valores Model, View e Projection da matriz MVP são enviados para o programa shader*/
	glProgramUniformMatrix4fv(program, loc.model, 1, GL_FALSE, glm::value_ptr(model));

	glProgramUniformMatrix4fv(program, loc.view, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 modelView = view * model;
	normalMatrix = glm::inverseTranspose(glm::mat3(view * model));

	glProgramUniformMatrix4fv(program, loc.modelView, 1, GL_FALSE, glm::value_ptr(modelView));

	glProgramUniformMatrix3fv(program, loc.normalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	glProgramUniformMatrix4fv(program, loc.mvp, 1, GL_FALSE, glm::value_ptr(GetMVP(model)));

}

/*Função que atualiza a câmara*/
void Camera::UpdateCam()
{

	transform.CalcRotationMatrix();
	transform.CalcLocalAxisCam();

	view = glm::lookAt(
		transform.position,	//Posição
		transform.forward + transform.position,	//Alvo
		transform.up	//Vetor normal
	);

	viewProjection = projection * view;
}