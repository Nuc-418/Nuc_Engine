// IblEnvironment: procedural-sky IBL precompute. See header.

#include "engine/render/IblEnvironment.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <iostream>

namespace {

const int kEnvSize = 256;        // environment cubemap face size
const int kIrradianceSize = 32;  // diffuse irradiance face size
const int kPrefilterSize = 128;  // specular prefilter base face size
const int kBrdfSize = 512;       // BRDF LUT size
const int kPrefilterMips = 5;    // -> maxReflectionLod = 4

glm::mat4 CaptureProjection()
{
	return glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
}

void CaptureViews(glm::mat4 out[6])
{
	const glm::vec3 o(0.0f);
	out[0] = glm::lookAt(o, glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
	out[1] = glm::lookAt(o, glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
	out[2] = glm::lookAt(o, glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
	out[3] = glm::lookAt(o, glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
	out[4] = glm::lookAt(o, glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
	out[5] = glm::lookAt(o, glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
}

GLuint MakeCubemap(int size, bool mip)
{
	GLuint id = 0;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
	for (int i = 0; i < 6; i++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, mip ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (mip)
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	return id;
}

} // namespace

bool IblEnvironment::Build()
{
	bool ok = skyShader.Load("assets/shaders/ibl/cubemap.vert", "assets/shaders/ibl/sky.frag")
	        && irradianceShader.Load("assets/shaders/ibl/cubemap.vert", "assets/shaders/ibl/irradiance.frag")
	        && prefilterShader.Load("assets/shaders/ibl/cubemap.vert", "assets/shaders/ibl/prefilter.frag")
	        && brdfShader.Load("assets/shaders/post/fullscreen.vert", "assets/shaders/ibl/brdf.frag");
	if (!ok) {
		std::cout << "IBL: shader load failed; using flat ambient." << std::endl;
		return false;
	}

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// Save the state we clobber.
	GLint prevFbo = 0, prevVp[4] = { 0, 0, 0, 0 };
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
	glGetIntegerv(GL_VIEWPORT, prevVp);
	GLboolean prevCull = glIsEnabled(GL_CULL_FACE);
	GLboolean prevDepth = glIsEnabled(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE); // cube is captured from the inside

	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	const glm::mat4 proj = CaptureProjection();
	glm::mat4 views[6];
	CaptureViews(views);

	// --- Environment cubemap: render the procedural sky into all six faces. ---
	envCubemap = MakeCubemap(kEnvSize, true);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, kEnvSize, kEnvSize);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	glEnable(GL_DEPTH_TEST);
	GLuint sky = skyShader.Program();
	glUseProgram(sky);
	glUniformMatrix4fv(glGetUniformLocation(sky, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
	glViewport(0, 0, kEnvSize, kEnvSize);
	for (int i = 0; i < 6; i++) {
		glUniformMatrix4fv(glGetUniformLocation(sky, "view"), 1, GL_FALSE, glm::value_ptr(views[i]));
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		RenderCube();
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// --- Irradiance cubemap: cosine-convolve the environment. ---
	irradianceMap = MakeCubemap(kIrradianceSize, false);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, kIrradianceSize, kIrradianceSize);

	GLuint irr = irradianceShader.Program();
	glUseProgram(irr);
	glUniformMatrix4fv(glGetUniformLocation(irr, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
	glUniform1i(glGetUniformLocation(irr, "environmentMap"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	glViewport(0, 0, kIrradianceSize, kIrradianceSize);
	for (int i = 0; i < 6; i++) {
		glUniformMatrix4fv(glGetUniformLocation(irr, "view"), 1, GL_FALSE, glm::value_ptr(views[i]));
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		RenderCube();
	}

	// --- Prefiltered specular cubemap: GGX per roughness mip. ---
	prefilterMap = MakeCubemap(kPrefilterSize, true);
	GLuint pre = prefilterShader.Program();
	glUseProgram(pre);
	glUniformMatrix4fv(glGetUniformLocation(pre, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
	glUniform1i(glGetUniformLocation(pre, "environmentMap"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	for (int mip = 0; mip < kPrefilterMips; mip++) {
		int mipSize = (int)(kPrefilterSize * std::pow(0.5, mip));
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipSize, mipSize);
		glViewport(0, 0, mipSize, mipSize);
		float roughness = (float)mip / (float)(kPrefilterMips - 1);
		glUniform1f(glGetUniformLocation(pre, "roughness"), roughness);
		for (int i = 0; i < 6; i++) {
			glUniformMatrix4fv(glGetUniformLocation(pre, "view"), 1, GL_FALSE, glm::value_ptr(views[i]));
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			RenderCube();
		}
	}
	maxReflectionLod = (float)(kPrefilterMips - 1);

	// --- BRDF integration LUT (RG16F 2D). ---
	glGenTextures(1, &brdfLUT);
	glBindTexture(GL_TEXTURE_2D, brdfLUT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, kBrdfSize, kBrdfSize, 0, GL_RG, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, kBrdfSize, kBrdfSize);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUT, 0);
	glViewport(0, 0, kBrdfSize, kBrdfSize);
	glDisable(GL_DEPTH_TEST);
	glUseProgram(brdfShader.Program());
	glClear(GL_COLOR_BUFFER_BIT);
	RenderQuad();

	// Restore.
	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)prevFbo);
	glViewport(prevVp[0], prevVp[1], prevVp[2], prevVp[3]);
	if (prevCull) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
	if (prevDepth) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);

	ready = true;
	std::cout << "IBL: environment built (procedural sky)." << std::endl;
	return true;
}

void IblEnvironment::ConfigureProgram(GLuint program, int irradianceUnit, int prefilterUnit, int brdfUnit)
{
	if (!ready || program == 0)
		return;
	glProgramUniform1i(program, glGetUniformLocation(program, "irradianceMap"), irradianceUnit);
	glProgramUniform1i(program, glGetUniformLocation(program, "prefilterMap"), prefilterUnit);
	glProgramUniform1i(program, glGetUniformLocation(program, "brdfLUT"), brdfUnit);
	glProgramUniform1f(program, glGetUniformLocation(program, "uMaxReflectionLod"), maxReflectionLod);
	glProgramUniform1i(program, glGetUniformLocation(program, "uHasIBL"), 1);
}

void IblEnvironment::BindForFrame(int irradianceUnit, int prefilterUnit, int brdfUnit)
{
	if (!ready)
		return;
	glActiveTexture(GL_TEXTURE0 + irradianceUnit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	glActiveTexture(GL_TEXTURE0 + prefilterUnit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
	glActiveTexture(GL_TEXTURE0 + brdfUnit);
	glBindTexture(GL_TEXTURE_2D, brdfLUT);
	glActiveTexture(GL_TEXTURE0);
}

void IblEnvironment::RenderCube()
{
	if (cubeVAO == 0) {
		const float v[] = {
			-1,-1,-1,  1, 1,-1,  1,-1,-1,  1, 1,-1, -1,-1,-1, -1, 1,-1,
			-1,-1, 1,  1,-1, 1,  1, 1, 1,  1, 1, 1, -1, 1, 1, -1,-1, 1,
			-1, 1, 1, -1, 1,-1, -1,-1,-1, -1,-1,-1, -1,-1, 1, -1, 1, 1,
			 1, 1, 1,  1,-1,-1,  1, 1,-1,  1,-1,-1,  1, 1, 1,  1,-1, 1,
			-1,-1,-1,  1,-1,-1,  1,-1, 1,  1,-1, 1, -1,-1, 1, -1,-1,-1,
			-1, 1,-1,  1, 1, 1,  1, 1,-1,  1, 1, 1, -1, 1,-1, -1, 1, 1,
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		glBindVertexArray(cubeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	}
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void IblEnvironment::RenderQuad()
{
	if (emptyVAO == 0)
		glGenVertexArrays(1, &emptyVAO); // fullscreen.vert synthesizes positions
	glBindVertexArray(emptyVAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
}

void IblEnvironment::Unload()
{
	glDeleteTextures(1, &envCubemap);
	glDeleteTextures(1, &irradianceMap);
	glDeleteTextures(1, &prefilterMap);
	glDeleteTextures(1, &brdfLUT);
	glDeleteFramebuffers(1, &captureFBO);
	glDeleteRenderbuffers(1, &captureRBO);
	if (cubeVBO) glDeleteBuffers(1, &cubeVBO);
	if (cubeVAO) glDeleteVertexArrays(1, &cubeVAO);
	if (emptyVAO) glDeleteVertexArrays(1, &emptyVAO);
	skyShader.Unload();
	irradianceShader.Unload();
	prefilterShader.Unload();
	brdfShader.Unload();
	envCubemap = irradianceMap = prefilterMap = brdfLUT = 0;
	captureFBO = captureRBO = cubeVAO = cubeVBO = emptyVAO = 0;
	ready = false;
}
