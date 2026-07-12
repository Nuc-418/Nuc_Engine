// Time: static frame/elapsed time, pushed to shaders via the "Time" uniform.

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

