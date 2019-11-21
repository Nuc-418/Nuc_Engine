/*
Autores: Francisco Aires (14884)
Data: 05/06/2019

Descrição: Transform.h
Ficheiro que cria e gere Transform.cpp
 */

#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp> // vec3, vec4, ivec4, mat4, ...
#include <glm/gtc/matrix_transform.hpp>
#include<glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp> // translate, rotate, scale, perspective, ...
#include <glm/gtc/type_ptr.hpp> // value_ptr
#include "math.h"

class  Transform
{
public:
	
	Transform() {};

	/*Transformações*/
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

	glm::vec3 forward = { 1.0f ,0.0f ,0.0f };
	glm::vec3 up = { 0.0f ,1.0f ,0.0f };
	glm::vec3 right = { 0.0f ,0.0f ,1.0f };

	glm::mat4 model;

	/*Transformações*/
	//Rotation
	void Rotate(glm::vec3 deltaRotationVec);
	void SetRotation(glm::vec3 rotationVec);
	//Translation
	void Translate(glm::vec3 deltaPosVec);
	void SetPos(glm::vec3 posVec);

	//Update do modelo
	void UpdateModel();

	/*Criação de matrizes Translação, Rotação e Escala*/
	glm::mat4 translationMatrix;
	glm::mat4 rotationMatrix;
	glm::mat4 scaleMatrix;

	/*Cálculo das matrizes Translação, Rotação, Escala e Model e cálculo dos eixos do objeto e da câmara*/
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