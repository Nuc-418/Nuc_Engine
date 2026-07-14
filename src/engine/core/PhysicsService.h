// IPhysicsService: the engine-owned contract for the physics backend.
//
// The interface lives in engine core so both sides can share it; the concrete
// backend (the JoltPhysics plugin) implements it and Provides it into the
// Application's ServiceRegistry in OnLoad. Engine and editor code then adjust
// physics through this interface — with no dependency on the concrete plugin
// or on Jolt. This is the "physics query interface" the plugin-system-v2 phase
// calls for; it grows (raycasts, overlaps) as consumers need it.

#pragma once

#include <glm/glm.hpp>

class IPhysicsService
{
public:
	virtual ~IPhysicsService() = default;

	// World gravity in units/s^2 (default is earth-like, {0, -9.81, 0}).
	virtual void SetGravity(const glm::vec3& gravity) = 0;
	virtual glm::vec3 GetGravity() const = 0;
};
