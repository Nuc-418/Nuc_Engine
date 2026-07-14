// MaterialComponent: PBR (metallic/roughness) surface parameters for an actor.
//
// Attach alongside a MeshComponent; MeshComponent::OnRender uploads these to the
// PBR shader as the `pbrMaterial.*` uniforms each frame. Absent = sensible
// dielectric defaults. Parameters only for now (base color / metallic /
// roughness / emissive / ao); texture maps arrive with the texture-material
// milestone. Serialize drives the Details panel + undo like any component.

#pragma once

#include "engine/scene/Component.h"

#include <glm/glm.hpp>

class MaterialComponent : public Component
{
public:
	glm::vec3 baseColor = glm::vec3(1.0f);
	float metallic = 0.0f;
	float roughness = 0.6f;
	glm::vec3 emissive = glm::vec3(0.0f);
	float ao = 1.0f;

	static const char* StaticTypeId() { return "Material"; }
	const char* TypeId() const override { return StaticTypeId(); }
	const char* DisplayName() const override { return "Material"; }

	void Serialize(ISerializer& out) const override;
	void Deserialize(const IDeserializer& in) override;
};
