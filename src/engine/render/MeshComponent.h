// MeshComponent: renders a Mesh with a Material and shader program.
//
// This is the component form of the old GameObject::meshRenderer / material.
// GameObject's authoring helpers (LoadObjFile, CreateObj*) populate the mesh
// component; the rest of the engine reaches it via GameObject::GetMesh().

#pragma once

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
};
