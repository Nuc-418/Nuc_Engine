/*
Autores: Francisco Aires (14884)
Data: 05/06/2019

Descrição: Time.h
Ficheiro que cria e gere Time.cpp
 */

#pragma once

#include <GL/glew.h>
#include <ctime>

class Time
{
public:
	static void Update(double time);
	static float time;
	static float deltaTime;
	static double preciseTime;
	static double preciseDeltaTime;
	
	static void TimeToProgram(GLuint program);

};

