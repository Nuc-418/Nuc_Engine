// MeshRenderer: draws a Mesh with a shader program and a Transform.

#pragma once
#include "engine/render/Mesh.h"
#include "engine/render/Camera.h"

class MeshRenderer
{
public:
	

	MeshRenderer() {};
	

	Mesh mesh;

	Transform* transformPtr = nullptr;

	GLuint program = 0;

	void SetProgramShader(GLuint program);

	// Draws with an explicit model matrix (the owner's WORLD matrix; with
	// hierarchy the local transform alone is not enough).
	void Draw(GLenum mode, Camera* camera, const glm::mat4& model);
	
};