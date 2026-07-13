// MeshComponent: attach/render/unload + registration with the ComponentRegistry.

#include "engine/render/MeshComponent.h"
#include "engine/scene/GameObject.h"
#include "engine/scene/ComponentRegistry.h"

void MeshComponent::OnAttach()
{
	renderer.transformPtr = &owner->transform;
}

void MeshComponent::OnRender(GLenum mode, Camera* camera)
{
	// A component created without geometry (e.g. via the registry) has no
	// program bound; skip it rather than issuing an empty draw.
	if (renderer.program != 0)
		renderer.Draw(mode, camera);
}

void MeshComponent::OnUnload()
{
	renderer.mesh.Unload();
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
