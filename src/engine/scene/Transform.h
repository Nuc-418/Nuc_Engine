// Transform: position/rotation/scale and model-matrix computation.

#pragma once
#include <glm/glm.hpp> // vec3, vec4, ivec4, mat4, ...
#include <glm/gtc/matrix_transform.hpp> // translate, rotate, scale, perspective, ...
#include <glm/gtx/transform.hpp>
#include "engine/core/EngineMath.h"

class  Transform
{
public:
	
	Transform() {};

	/*Transforma��es*/
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

	// In-class values match what CalcLocalAxis computes at identity rotation
	// (note: CalcLocalAxis negates defaultRight), so an un-updated Transform
	// agrees with an updated one.
	glm::vec3 forward = { 0.0f ,0.0f ,1.0f };
	glm::vec3 up = { 0.0f ,1.0f ,0.0f };
	glm::vec3 right = { -1.0f ,0.0f ,0.0f };

	glm::mat4 model;

	/*Transforma��es*/
	//Rotation
	void Rotate(glm::vec3 deltaRotationVec);
	void SetRotation(glm::vec3 rotationVec);
	//Translation
	void Translate(glm::vec3 deltaPosVec);
	void SetPos(glm::vec3 posVec);

	// Decomposes a TRS matrix into position/rotation/scale (rotation via
	// EulerYXZFromMatrix, matching CalcRotationMatrix's convention) and
	// refreshes the cached matrices. Used to keep an object's world transform
	// when it is reparented (local = inverse(parentWorld) * world).
	void SetFromMatrix(const glm::mat4& matrix);

	//Update do modelo
	void UpdateModel();

	/*Cria��o de matrizes Transla��o, Rota��o e Escala*/
	glm::mat4 translationMatrix;
	glm::mat4 rotationMatrix;
	glm::mat4 scaleMatrix;

	/*C�lculo das matrizes Transla��o, Rota��o, Escala e Model e c�lculo dos eixos do objeto e da c�mara*/
	void CalcTranslationMatrix();
	void CalcRotationMatrix();
	void CalcScaleMatrix();
	void CalcModel();
	void CalcLocalAxis();
	void CalcLocalAxisCam();

	glm::vec4 defaultForward = { 0.0f ,0.0f ,1.0f ,0 };
	glm::vec4 defaultUp = { 0.0f ,1.0f ,0.0f ,0 };
	glm::vec4 defaultRight = { 1.0f ,0.0f ,0.0f, 0 };

	
};