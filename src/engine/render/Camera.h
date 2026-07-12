// Camera: perspective camera; uploads Model/View/MVP matrices to shader programs.

#pragma once
#include <GL/glew.h>
#include <unordered_map>
#include "engine/scene/Transform.h"

class Camera 
{
public:

	Camera(glm::vec3 camPos, glm::vec3 lookAt, glm::vec3 vecUp);

	glm::mat4 GetMVP(glm::mat4 model);

	// Rebuild the projection matrix. The constructor default matches the
	// original hardcoded perspective(45deg, 4/3, 0.1, 1000).
	void SetPerspective(float fovDegrees, float aspect, float nearPlane, float farPlane);
	void SetAspect(float aspect);

	void CamToProgram(GLuint program, glm::mat4 model);

	// Current matrices (view refreshed by UpdateCam during rendering).
	const glm::mat4& GetView() const { return view; }
	const glm::mat4& GetProjection() const { return projection; }

	
	Transform transform;

	//Update
	void UpdateCam();

private:

	// Uniform locations cached per shader program. Locations that do not
	// exist in a program are stored as -1 and silently ignored by GL.
	// Programs are created once at load time and never recreated, so the
	// cache is never invalidated.
	struct UniformLocations
	{
		GLint model;
		GLint view;
		GLint modelView;
		GLint normalMatrix;
		GLint mvp;
	};
	std::unordered_map<GLuint, UniformLocations> locationCache;

	float fovDegrees = 45.0f;
	float aspectRatio = 4.0f / 3.0f;
	float nearPlane = 0.1f;
	float farPlane = 1000.0f;

	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 viewProjection;
	glm::mat4 normalMatrix;

};


