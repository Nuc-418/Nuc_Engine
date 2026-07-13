// Time: static frame/elapsed time, pushed to shaders via the "Time" uniform.

#include "engine/core/Time.h"
#include "engine/render/Shader.h"
#include <unordered_map>


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
	// Location cached per program; cleared when a shader reload bumps the
	// generation (a relink can move locations).
	static std::unordered_map<GLuint, GLint> locationCache;
	static unsigned cacheGeneration = 0;
	if (cacheGeneration != Shader::GlobalGeneration()) {
		locationCache.clear();
		cacheGeneration = Shader::GlobalGeneration();
	}
	auto it = locationCache.find(program);
	if (it == locationCache.end())
		it = locationCache.emplace(program, glGetProgramResourceLocation(program, GL_UNIFORM, "Time")).first;

	glProgramUniform1f(program, it->second, time);
}
