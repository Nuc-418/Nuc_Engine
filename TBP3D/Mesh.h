/*
Autores: Francisco Aires (14884)
Data: 05/06/2019

Descrição: Mesh.h
Ficheiro que cria e gere Mesh.cpp
 */

#pragma once
#include <GL\glew.h>
#include <GL\gl.h>
#include <glm/glm.hpp> // vec3, vec4, ivec4, mat4, ...
#include <glm/gtc/type_ptr.hpp> // value_ptr
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
using namespace std;

#define Positions 0
#define Normals 1
#define Colors 2
#define UVs 3

class Mesh
{

public:

	//Vertex info
	//OpenGL Var //1-Pos,2-Norm,3-Color,4-UVs
	int nVertex;
	vector<glm::vec3>* vertexArrayPtr[3] = { NULL };//Ver
	vector<glm::vec2>* vertexUvArrayPtr;


	//Element info
	int nElements = 0;
	vector<GLuint>* elementArrayPtr = { NULL };



	GLuint program;
	GLuint VAO;
	GLuint VBO[4] = { 0 };
	GLuint EBO = { 0 };
	const GLchar *resourceLocations[4] = { "vPosition","vNormal","vColor","vUV" };


	void AssignPosColor(vector<glm::vec3>* positionArray, vector<glm::vec3>*  colorArray);
	void AssignPosUvNorm(vector<glm::vec3>* positionArray, vector<glm::vec2>* uvArray, vector<glm::vec3>* normalArray);
	void AssignElementArray(vector<GLuint>* elementArray);

	void RewriteVertexPos(vector<glm::vec3>* positionArray);

private:
	void LoadVAO();
	void LoadVBOs();
	void LoadEBO();

	void Load();
};