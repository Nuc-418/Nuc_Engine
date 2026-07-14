// Framebuffer: render-to-texture target (color + depth/stencil renderbuffer).
// Color is RGBA8 by default; set `hdr = true` before the first Create/Resize for
// an RGBA16F float target (the scene pass renders linear HDR into one of these).

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

	// RGBA16F float color when true (set before the first Create); RGBA8 otherwise.
	bool hdr = false;

	GLuint colorTexture = 0;
	int width = 0;
	int height = 0;

private:
	GLuint fbo = 0;
	GLuint depthRbo = 0;
};
