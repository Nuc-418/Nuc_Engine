// MeshComponent: attach/render/unload + registration with the ComponentRegistry.

#include "engine/render/MeshComponent.h"
#include "engine/render/MaterialComponent.h"
#include "engine/scene/GameObject.h"
#include "engine/scene/ComponentRegistry.h"
#include "engine/io/ObjLoader.h"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace {
// Uploads the object's PBR material (a sibling MaterialComponent, or dielectric
// defaults) plus the albedo-texture flag to `program`. Locations are queried
// each call; the set is tiny and only lit objects reach here.
void UploadMaterial(GLuint program, GameObject* owner, bool hasAlbedoTexture)
{
	glm::vec3 baseColor(1.0f), emissive(0.0f);
	float metallic = 0.0f, roughness = 0.6f, ao = 1.0f;
	if (MaterialComponent* m = owner->GetComponent<MaterialComponent>()) {
		baseColor = m->baseColor;
		metallic = m->metallic;
		roughness = m->roughness;
		emissive = m->emissive;
		ao = m->ao;
	}
	glProgramUniform3fv(program, glGetProgramResourceLocation(program, GL_UNIFORM, "pbrMaterial.baseColor"), 1, glm::value_ptr(baseColor));
	glProgramUniform1f(program, glGetProgramResourceLocation(program, GL_UNIFORM, "pbrMaterial.metallic"), metallic);
	glProgramUniform1f(program, glGetProgramResourceLocation(program, GL_UNIFORM, "pbrMaterial.roughness"), roughness);
	glProgramUniform3fv(program, glGetProgramResourceLocation(program, GL_UNIFORM, "pbrMaterial.emissive"), 1, glm::value_ptr(emissive));
	glProgramUniform1f(program, glGetProgramResourceLocation(program, GL_UNIFORM, "pbrMaterial.ao"), ao);
	glProgramUniform1i(program, glGetProgramResourceLocation(program, GL_UNIFORM, "uHasAlbedoTex"), hasAlbedoTexture ? 1 : 0);
}
} // namespace

void MeshComponent::OnAttach()
{
	renderer.transformPtr = &owner->transform;
}

void MeshComponent::OnRender(GLenum mode, Camera* camera)
{
	// A component created without geometry (e.g. via the registry) has no
	// program bound; skip it rather than issuing an empty draw.
	if (renderer.program == 0)
		return;
	UploadMaterial(renderer.program, owner, hasAlbedoTexture);
	renderer.Draw(mode, camera, owner->WorldMatrix());
}

void MeshComponent::OnUnload()
{
	renderer.mesh.Unload();
}

bool MeshComponent::LoadObj(GLuint programShader, const std::string& folderPath, const std::string& fileName)
{
	std::string objPath = folderPath + fileName;

	ObjLoader objLoader((char*)objPath.data());
	if (objLoader.loaded)
	{
		renderer.SetProgramShader(programShader);
		renderer.mesh.AssignPosUvNorm(&objLoader.objInfo.vertexPos, &objLoader.objInfo.vertexUvs, &objLoader.objInfo.vertexNormals);
		std::cout << "Loaded : " << objPath << std::endl;

		std::string mtlPath = folderPath + objLoader.mtlFile.data();
		material.loadMaterial((char*)mtlPath.data());
		if (material.loaded)
		{
			material.materialStorage(renderer.program);
			hasAlbedoTexture = true; // model albedo comes from its bound texture
			std::cout << "Loaded : " << mtlPath << std::endl;
			return true;
		}
		std::cout << "Error Loading : " << mtlPath << std::endl;
		return false;
	}

	std::cout << "Error Loading : " << objPath << std::endl;
	return false;
}

void MeshComponent::CreatePosColor(GLuint programShader, std::vector<glm::vec3>* positionArray, std::vector<glm::vec3>* colorArray)
{
	renderer.SetProgramShader(programShader);
	renderer.mesh.AssignPosColor(positionArray, colorArray);
}

void MeshComponent::CreatePosNormColor(GLuint programShader, std::vector<glm::vec3>* positionArray, std::vector<glm::vec3>* normalArray, std::vector<glm::vec3>* colorArray)
{
	renderer.SetProgramShader(programShader);
	renderer.mesh.AssignPosNormColor(positionArray, normalArray, colorArray);
}

void MeshComponent::CreatePosUvNorm(GLuint programShader, std::vector<glm::vec3>* positionArray, std::vector<glm::vec2>* uvArray, std::vector<glm::vec3>* normalArray)
{
	renderer.SetProgramShader(programShader);
	renderer.mesh.AssignPosUvNorm(positionArray, uvArray, normalArray);
}

// Register so "Mesh" can be created by id (scene load / editor). The mesh's
// geometry comes from the spawn factory, so a registry-created MeshComponent
// starts empty and is populated by GameObject's authoring helpers.
namespace {
const bool kMeshRegistered = [] {
	ComponentRegistry::Register("Mesh", "Mesh",
		[] { return std::unique_ptr<Component>(new MeshComponent()); });
	return true;
}();
}
