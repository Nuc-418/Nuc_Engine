// PostProcess: fullscreen post passes. Currently just tonemapping the linear-HDR
// scene texture into the bound (LDR) target. Owns an attribute-less fullscreen
// triangle VAO and the tonemap program.

#pragma once

#include <GL/glew.h>

#include "engine/render/Shader.h"

class PostProcess
{
public:
	// Compiles the tonemap program and creates the fullscreen VAO. Returns false
	// (and logs) if the shader fails to build.
	bool Init();

	// Deletes GL objects (context must be current).
	void Unload();

	// Tonemaps `hdrTexture` into whatever framebuffer/viewport is currently
	// bound. Manages its own depth/blend state and restores depth testing.
	void Tonemap(GLuint hdrTexture, float exposure = 1.0f);

private:
	Shader tonemap;
	GLuint vao = 0;
};
