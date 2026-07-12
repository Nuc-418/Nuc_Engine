/*
Autores: Francisco Aires (14884)
Data: 05/06/2019

Descrição: Texture.h
Ficheiro que cria e gere Texture.cpp
 */

#pragma once
#include <iostream>
#define GLEW_STATIC
#include <GL\glew.h>



using namespace std;
class Texture
{
public:

	void TesxureToProgram(GLuint program,string textureFile);

private:
	void load_texture(string textureFile);

	
};
