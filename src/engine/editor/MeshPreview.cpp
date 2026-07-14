// MeshPreview implementation. See MeshPreview.h.

#include "engine/editor/MeshPreview.h"
#include "engine/scene/GameObject.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>

namespace MeshPreview
{
	void Generate(std::map<std::string, Framebuffer>& out, const World& world,
	              const std::vector<std::string>& typeIds, int size)
	{
		// Save the GL state we touch so the surrounding UI pass is unaffected.
		GLint prevFbo = 0, prevVp[4] = { 0, 0, 0, 0 };
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
		glGetIntegerv(GL_VIEWPORT, prevVp);
		GLboolean prevDepth = glIsEnabled(GL_DEPTH_TEST);
		GLboolean prevBlend = glIsEnabled(GL_BLEND);
		GLboolean prevCull = glIsEnabled(GL_CULL_FACE);
		GLfloat prevClear[4] = { 0, 0, 0, 0 };
		glGetFloatv(GL_COLOR_CLEAR_VALUE, prevClear);

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);

		const glm::vec3 lightDir = glm::normalize(glm::vec3(-0.4f, -1.0f, -0.35f));
		const glm::vec3 white(1.0f);

		for (const std::string& type : typeIds) {
			std::unique_ptr<GameObject> object = world.Create(type);
			if (!object)
				continue;

			MeshComponent* meshComponent = object->GetMesh();
			if (!meshComponent)
				continue;
			Mesh& mesh = meshComponent->renderer.mesh;
			GLuint program = meshComponent->renderer.program;

			Framebuffer& fb = out[type];
			fb.Create(size, size);
			fb.Bind();
			glViewport(0, 0, size, size);
			glClearColor(0.14f, 0.14f, 0.16f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Frame the mesh from its local bounds so any size fits the thumbnail.
			glm::vec3 center = (mesh.aabbMin + mesh.aabbMax) * 0.5f;
			float radius = glm::length(mesh.aabbMax - mesh.aabbMin) * 0.5f;
			if (radius < 1e-4f)
				radius = 1.0f;
			glm::vec3 viewDir = glm::normalize(glm::vec3(1.0f, 0.85f, 1.0f));
			glm::vec3 camPos = center + viewDir * radius * 2.6f;

			glm::mat4 projection = glm::perspective(glm::radians(35.0f), 1.0f, radius * 0.05f, radius * 12.0f);
			glm::mat4 view = glm::lookAt(camPos, center, glm::vec3(0, 1, 0));
			glm::mat4 model(1.0f);
			glm::mat4 modelView = view * model;
			glm::mat4 mvp = projection * modelView;
			glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(modelView));

			// Uniforms not present in a given program resolve to -1 and are
			// silently ignored, so this covers both the PBR programs and the
			// unlit cube shader. Set the PBR uniforms explicitly (a single
			// key light + white dielectric material) so a thumbnail does not
			// depend on whatever the scene last uploaded to the shared program.
			glProgramUniformMatrix4fv(program, glGetUniformLocation(program, "MVP"), 1, GL_FALSE, glm::value_ptr(mvp));
			glProgramUniformMatrix4fv(program, glGetUniformLocation(program, "Model"), 1, GL_FALSE, glm::value_ptr(model));
			glProgramUniformMatrix4fv(program, glGetUniformLocation(program, "View"), 1, GL_FALSE, glm::value_ptr(view));
			glProgramUniformMatrix4fv(program, glGetUniformLocation(program, "ModelView"), 1, GL_FALSE, glm::value_ptr(modelView));
			glProgramUniformMatrix3fv(program, glGetUniformLocation(program, "NormalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
			glProgramUniform3fv(program, glGetUniformLocation(program, "CamPos"), 1, glm::value_ptr(camPos));

			// PBR material: white, matte dielectric.
			glProgramUniform3f(program, glGetUniformLocation(program, "pbrMaterial.baseColor"), 1.0f, 1.0f, 1.0f);
			glProgramUniform1f(program, glGetUniformLocation(program, "pbrMaterial.metallic"), 0.0f);
			glProgramUniform1f(program, glGetUniformLocation(program, "pbrMaterial.roughness"), 0.5f);
			glProgramUniform3f(program, glGetUniformLocation(program, "pbrMaterial.emissive"), 0.0f, 0.0f, 0.0f);
			glProgramUniform1f(program, glGetUniformLocation(program, "pbrMaterial.ao"), 1.0f);
			glProgramUniform1i(program, glGetUniformLocation(program, "uHasAlbedoTex"), 0);
			glProgramUniform1i(program, glGetUniformLocation(program, "uHasIBL"), 0);

			// One key directional light (boosted a little to compensate for the
			// diffuse 1/PI so the tonemapped thumbnail reads well) + soft ambient.
			glProgramUniform1i(program, glGetUniformLocation(program, "nDirectionalLights"), 1);
			glProgramUniform1i(program, glGetUniformLocation(program, "nPointLights"), 0);
			glProgramUniform1i(program, glGetUniformLocation(program, "nSpotLights"), 0);
			glProgramUniform1i(program, glGetUniformLocation(program, "directionalLight.switchL"), 1);
			glProgramUniform3fv(program, glGetUniformLocation(program, "directionalLight.direction"), 1, glm::value_ptr(lightDir));
			glProgramUniform3f(program, glGetUniformLocation(program, "directionalLight.diffuse"), 3.0f, 3.0f, 3.0f);
			glProgramUniform3fv(program, glGetUniformLocation(program, "directionalLight.specular"), 1, glm::value_ptr(white));
			glProgramUniform1i(program, glGetUniformLocation(program, "ambientLight.switchL"), 1);
			glProgramUniform3f(program, glGetUniformLocation(program, "ambientLight.ambient"), 0.12f, 0.12f, 0.14f);

			glUseProgram(program);
			glBindVertexArray(mesh.VAO);
			if (mesh.EBO == 0) {
				glDrawArrays(GL_TRIANGLES, 0, mesh.nVertex);
			} else {
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
				glDrawElements(GL_TRIANGLES, mesh.nElements, GL_UNSIGNED_INT, 0);
			}

			fb.Unbind();
			mesh.Unload(); // release the throwaway object's GPU buffers
		}

		glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);
		glViewport(prevVp[0], prevVp[1], prevVp[2], prevVp[3]);
		glClearColor(prevClear[0], prevClear[1], prevClear[2], prevClear[3]);
		if (prevDepth) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
		if (prevBlend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
		if (prevCull) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
	}
}
