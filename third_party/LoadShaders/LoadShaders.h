// LoadShaders: shader compilation/linking helper.
// Derived from the OpenGL Programming Guide ("Red Book") LoadShaders utility.

#pragma once

#include <GL/gl.h>

typedef struct {
	GLenum       type;
	const char*  filename;
	GLuint       shader;
} ShaderInfo;


GLuint LoadShaders(ShaderInfo*);
