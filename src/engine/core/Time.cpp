// Time: static frame/elapsed time, pushed to shaders via the "Time" uniform.

#include "engine/core/Time.h"


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
