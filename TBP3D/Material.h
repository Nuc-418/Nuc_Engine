/*
Autores: Francisco Aires (14884)
Data: 05/06/2019

Descrição: Material.h
Ficheiro que cria e gere Material.cpp
 */


#pragma once
#include <iostream>
#include <string>
#include <vector>
using namespace std;
#define GLEW_STATIC
#include <GL\glew.h>

#include <glm/glm.hpp> // vec3, vec4, ivec4, mat4, ...

struct MaterialInfo
{
	GLuint  illum;
	GLfloat shininess;
	GLfloat opticalDensity;
	GLfloat alpha;
	glm::vec3 emissive;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

class Material
{
public:
	MaterialInfo materialInfo = {NULL};

	void materialStorage(GLuint program);

	void loadMaterial(char* path);

	bool loaded = false;
};







