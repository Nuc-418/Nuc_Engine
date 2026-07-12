// MeshRenderer: draws a Mesh with a shader program and a Transform.

#pragma once
#include "engine/render/Mesh.h"
#include "engine/render/Camera.h"

class MeshRenderer
{
public:
	

	MeshRenderer() {};
	

	Mesh mesh;

	Transform* transformPtr;

	GLuint program;
	
	void SetProgramShader(GLuint program);

	void Draw(GLenum mode,Camera* camera);
	
};