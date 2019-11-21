/*
Autores: Francisco Aires (14884)
Data: 05/06/2019

Descrição: GameObject.h
Ficheiro que cria e gere GameObject.cpp
*/

#pragma once

#include "Camera.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "ObjLoader.h"
#include "Material.h"

#include <iostream>
using namespace std;

class GameObject
{
public:

	GameObject() {
		meshRenderer.transformPtr = &transform;
	};

	bool LoadObjFile(GLuint programShader, string folderPath, string fileName);

	void CreateObjPosColor(GLuint programShader, vector<glm::vec3>* positionArray, vector<glm::vec3>*  colorArray);

	void CreateObjPosUvNorm(GLuint programShader, vector<glm::vec3>* positionArray, vector<glm::vec2>* uvArray, vector<glm::vec3>* normalArray);

	string name;
	Transform transform;
	MeshRenderer meshRenderer;
	Material material;

};


