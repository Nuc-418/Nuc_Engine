// RotatorComponent: spins its owner while the simulation runs — the first
// behavior component (UE's RotatingMovement equivalent). Speed is in the
// engine's Euler convention (x = yaw about Y, y = pitch about X, z = roll),
// radians per second, applied through Transform::Rotate. Serializable and
// editable in Details via property reflection.
//
// If `activeWhile` names an input action, the rotation only applies while that
// action is held — the first behavior to read input through the
// BehaviorContext. Empty (the default) means always spin.

#pragma once

#include "engine/scene/Component.h"

#include <glm/glm.hpp>
#include <string>

class RotatorComponent : public Component
{
public:
	glm::vec3 radiansPerSecond = glm::vec3(1.0f, 0.0f, 0.0f);
	std::string activeWhile;  // input action gating rotation; empty => always

	static const char* StaticTypeId() { return "Rotator"; }
	const char* TypeId() const override { return StaticTypeId(); }
	const char* DisplayName() const override { return "Rotator"; }

	void OnSimulate(float deltaTime, const BehaviorContext& ctx) override;

	void Serialize(ISerializer& out) const override;
	void Deserialize(const IDeserializer& in) override;
};
