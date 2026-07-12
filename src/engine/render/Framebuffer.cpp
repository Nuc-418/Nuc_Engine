// Framebuffer: render-to-texture target (RGBA8 color + depth/stencil renderbuffer).

#include "engine/render/Framebuffer.h"
#include <iostream>

bool Framebuffer::Create(int w, int h)
{
	if (w < 1) w = 1;
	if (h < 1) h = 1;
	width = w;
	height = h;

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &colorTexture);
	glBindTexture(GL_TEXTURE_2D, colorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

	glGenRenderbuffers(1, &depthRbo);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRbo);

	bool complete = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	if (!complete)
		std::cout << "Framebuffer incomplete (" << width << "x" << height << ")" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return complete;
}

void Framebuffer::Resize(int w, int h)
{
	if (w < 1) w = 1;
	if (h < 1) h = 1;
	if (w == width && h == height && fbo != 0)
		return;

	Unload();
	Create(w, h);
}

void Framebuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void Framebuffer::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Unload()
{
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &colorTexture);
	glDeleteRenderbuffers(1, &depthRbo);
	fbo = 0;
	colorTexture = 0;
	depthRbo = 0;
	width = 0;
	height = 0;
}
