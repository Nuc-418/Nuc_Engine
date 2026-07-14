// IblEnvironment: image-based lighting precompute. Renders a procedural HDR sky
// into an environment cubemap, then derives the three maps PBR ambient needs —
// a diffuse irradiance cubemap, a roughness-mipped specular prefilter cubemap,
// and the BRDF integration LUT. Built once; the PBR shader samples the maps for
// ambient diffuse + specular. No external .hdr asset required.

#pragma once

#include <GL/glew.h>

#include "engine/render/Shader.h"

class IblEnvironment
{
public:
	// Builds all maps from the procedural sky. Returns false (and leaves Ready()
	// false, so the PBR shader falls back to flat ambient) if a shader fails.
	bool Build();

	void Unload();
	bool Ready() const { return ready; }

	// One-time per lit program: point the IBL samplers at their units and enable
	// uHasIBL / uMaxReflectionLod. Call after Build().
	void ConfigureProgram(GLuint program, int irradianceUnit, int prefilterUnit, int brdfUnit);

	// Per-frame: bind the three IBL textures to their units before lit draws.
	void BindForFrame(int irradianceUnit, int prefilterUnit, int brdfUnit);

private:
	void RenderCube();
	void RenderQuad();

	bool ready = false;

	GLuint envCubemap = 0;
	GLuint irradianceMap = 0;
	GLuint prefilterMap = 0;
	GLuint brdfLUT = 0;

	GLuint captureFBO = 0;
	GLuint captureRBO = 0;
	GLuint cubeVAO = 0;
	GLuint cubeVBO = 0;
	GLuint emptyVAO = 0;

	float maxReflectionLod = 4.0f; // prefilter mip count - 1

	Shader skyShader;
	Shader irradianceShader;
	Shader prefilterShader;
	Shader brdfShader;
};
