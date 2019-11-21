/*
Autores: Francisco Aires (14884)
Data: 05/06/2019

Descrição: ObjLoader.h
Ficheiro que cria e gere ObjLoader.cpp
 */

#pragma once
#include <GL\glew.h>
#include <GL\gl.h>

#include <glm/glm.hpp> // vec3, vec4, ivec4, mat4, ...
#include <glm/gtc/matrix_transform.hpp> // translate, rotate, scale, perspective, ...
#include <glm/gtc/type_ptr.hpp> // value_ptr
#include <iostream>
#include <vector>

typedef struct {
public:
	std::vector<glm::vec3>vertexPos;
	std::vector<glm::vec2>vertexUvs;
	std::vector<glm::vec3>vertexNormals;
} ObjInfo;

class ObjLoader
{
public:
	ObjLoader(char* path) { filePath = path; LoadObj(); }
	bool loaded = false;
	std::string mtlFile;
	ObjInfo objInfo;

private:
	const char* filePath;
	bool LoadObj();
};

