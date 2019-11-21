/*
Autor: Professor Duarte Duque
Data: 14/03/2019

Descrição: LoadShaders.h
Ficheiro que cria e gere LoadShaders.cpp
 */

#pragma once

#include <GL\gl.h>

// Descomentar para debug

#define _DEBUG

typedef struct {
	GLenum       type;
	const char*  filename;
	GLuint       shader;
} ShaderInfo;


GLuint LoadShaders(ShaderInfo*);
