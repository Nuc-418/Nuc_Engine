/*
Autores: RonaldoVeloso
Data: 05/06/2019

Descrição: Lights.h
Ficheiro que cria e gere Lights.cpp
 */
#include "Time.h"


float Time::time = 0;
float Time::deltaTime = 0;
double Time::preciseTime = 0;
double Time::preciseDeltaTime = 0;

void Time::Update(double dTime) {
	preciseTime += dTime;
	preciseDeltaTime = dTime;

	deltaTime = (float)preciseDeltaTime;
	time = (float)preciseTime;

}

void Time::TimeToProgram(GLuint program)
{
	GLint timeId = glGetProgramResourceLocation(program, GL_UNIFORM, "Time");
	glProgramUniform1f(program, timeId, time);
}
