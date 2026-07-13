// MeshComponent: renders a Mesh with a Material and shader program.
//
// This is the component form of the old GameObject::meshRenderer / material.
// The authoring helpers (LoadObj, Create*) live here; spawn factories reach
// them via GameObject::EnsureMesh(), and the rest of the engine reads the
// component via GameObject::GetMesh().

#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "engine/scene/Component.h"
#include "engine/render/MeshRenderer.h"
#include "engine/render/Material.h"

class MeshComponent : public Component
{
public:
	MeshRenderer renderer;
	Material material;

	static const char* StaticTypeId() { return "Mesh"; } // GetComponent<T> matches on this
	const char* TypeId() const override { return StaticTypeId(); }
	const char* DisplayName() const override { return "Mesh"; }

	void OnAttach() override;                          // wire renderer to owner's transform
	void OnRender(GLenum mode, Camera* camera) override;
	void OnUnload() override;                          // free the mesh's GL objects

	// --- Authoring (used by the spawn factories) ---------------------------
	// Loads an .obj (+ its .mtl) from folderPath+fileName onto this component.
	bool LoadObj(GLuint programShader, const std::string& folderPath, const std::string& fileName);
	// Uploads raw vertex arrays. The arrays must outlive the upload call only
	// (Mesh copies to the GPU immediately) but are also kept as pointers for
	// RewriteVertexPos-style edits — pass storage with stable addresses.
	void CreatePosColor(GLuint programShader, std::vector<glm::vec3>* positionArray, std::vector<glm::vec3>* colorArray);
	void CreatePosNormColor(GLuint programShader, std::vector<glm::vec3>* positionArray, std::vector<glm::vec3>* normalArray, std::vector<glm::vec3>* colorArray);
	void CreatePosUvNorm(GLuint programShader, std::vector<glm::vec3>* positionArray, std::vector<glm::vec2>* uvArray, std::vector<glm::vec3>* normalArray);
};
