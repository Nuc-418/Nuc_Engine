/*
Autores: Francisco Aires (14884)
Data: 05/06/2019

Descrição: MeshRenderer.h
Ficheiro que cria e gere MeshRenderer.cpp
 */

#pragma once
#include "Mesh.h"
#include "Camera.h"

class MeshRenderer
{
public:
	

	MeshRenderer() {};
	

	Mesh mesh;

	Transform* transformPtr;

	GLuint program;
	
	void SetProgramShader(GLuint program);

	void Draw(GLenum mode,Camera* camera);
	
};