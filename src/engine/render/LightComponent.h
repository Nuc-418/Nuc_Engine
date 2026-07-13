// LightComponent: a scene light attached to a GameObject.
//
// Kind selects directional/point/spot. Placement comes from the owner:
// point/spot lights sit at the owner's world position; directional/spot
// direction is the owner's world forward axis — rotate the object to aim the
// light. World::SyncComponentLights collects every LightComponent each frame,
// merges them after the world-level authored lights, and re-uploads through
// the existing Lights uniform path when anything changed (shaders unchanged).
//
// Editing: Details panel (kind, colors, attenuation, cone). Component edits
// are not yet undoable (that arrives with generalized component undo).

#pragma once

#include "engine/scene/Component.h"

#include <glm/glm.hpp>

class LightComponent : public Component
{
public:
	enum class Kind { Directional = 0, Point = 1, Spot = 2 };

	Kind kind = Kind::Point;
	bool on = true;

	glm::vec3 ambient = glm::vec3(0.05f);
	glm::vec3 diffuse = glm::vec3(1.0f);
	glm::vec3 specular = glm::vec3(1.0f);

	// Point/Spot attenuation coefficients.
	float constant = 1.0f;
	float linear = 0.09f;
	float quadratic = 0.032f;

	// Spot cone half-angle, radians (matches SpotLight::cutOff).
	float cutOff = 0.26f;

	static const char* StaticTypeId() { return "Light"; }
	const char* TypeId() const override { return StaticTypeId(); }
	const char* DisplayName() const override { return "Light"; }

	void Serialize(ISerializer& out) const override;
	void Deserialize(const IDeserializer& in) override;
};
