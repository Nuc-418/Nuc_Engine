// Framebuffer: render-to-texture target (RGBA8 color + depth/stencil renderbuffer).

#pragma once

#include <GL/glew.h>

class Framebuffer
{
public:
	bool Create(int width, int height);

	// No-op when the size is unchanged; otherwise recreates the attachments.
	void Resize(int width, int height);

	void Bind();
	void Unbind();

	// Deletes the GL objects. Must run while the GL context is alive.
	void Unload();

	GLuint colorTexture = 0;
	int width = 0;
	int height = 0;

private:
	GLuint fbo = 0;
	GLuint depthRbo = 0;
};
