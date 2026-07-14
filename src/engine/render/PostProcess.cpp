// PostProcess: tonemap pass implementation.

#include "engine/render/PostProcess.h"

bool PostProcess::Init()
{
	if (vao == 0)
		glGenVertexArrays(1, &vao); // empty VAO; the vertex shader synthesizes positions

	return tonemap.Load("assets/shaders/post/fullscreen.vert",
	                    "assets/shaders/post/tonemap.frag");
}

void PostProcess::Unload()
{
	tonemap.Unload();
	if (vao) {
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}
}

void PostProcess::Tonemap(GLuint hdrTexture, float exposure)
{
	GLuint program = tonemap.Program();
	if (program == 0)
		return;

	// Fullscreen pass: no depth test/write, no blending; the triangle covers the
	// whole target.
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_BLEND);

	glUseProgram(program);
	glProgramUniform1f(program, tonemap.Location("exposure"), exposure);
	glProgramUniform1i(program, tonemap.Location("hdrBuffer"), 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);

	// Restore state the scene pass relies on.
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}
