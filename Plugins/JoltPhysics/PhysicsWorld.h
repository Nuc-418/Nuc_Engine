// PhysicsWorld: a thin, engine-facing wrapper around a Jolt PhysicsSystem.
//
// This header is deliberately Jolt-free: Jolt lives entirely behind an opaque
// Impl (pimpl), so only PhysicsWorld.cpp needs the Jolt headers (and C++17).
// The rest of the engine can create/step bodies and read their transforms
// using only GLM types.

#pragma once

#include <cstdint>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class PhysicsWorld
{
public:
	// A dynamic body is moved by the simulation; a static body never moves and
	// only acts as an obstacle (floors, walls).
	enum class Motion { Static, Dynamic };

	// Opaque handle to a body (Jolt's BodyID packed into a uint32). Compare
	// against InvalidBody to detect a failed creation.
	using BodyId = uint32_t;
	static const BodyId InvalidBody = 0xffffffffu;

	PhysicsWorld();
	~PhysicsWorld();

	PhysicsWorld(const PhysicsWorld&) = delete;
	PhysicsWorld& operator=(const PhysicsWorld&) = delete;

	// Brings up the Jolt globals (once per process) and this world's system.
	// Idempotent: safe to call more than once. Returns false on failure.
	bool Init();
	void Shutdown();
	bool IsInitialized() const;

	void SetGravity(const glm::vec3& gravity);

	// Advances the simulation by deltaTime seconds. No-op until Init() succeeds.
	void Step(float deltaTime);

	// Creates a body and adds it to the world. halfExtents/radius are half-sizes
	// (a unit cube is halfExtents {0.5,0.5,0.5}). Returns InvalidBody on failure.
	BodyId CreateBox(const glm::vec3& position, const glm::vec3& halfExtents, Motion motion);
	BodyId CreateSphere(const glm::vec3& position, float radius, Motion motion);

	void RemoveBody(BodyId id);

	glm::vec3 GetPosition(BodyId id) const;
	glm::quat GetRotation(BodyId id) const;

	// Teleports a body (zeroing its velocity). Used to keep edit-mode bodies
	// following their objects; activates dynamic bodies so they resettle.
	void SetPose(BodyId id, const glm::vec3& position, const glm::quat& rotation);

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};
