// PhysicsBodyComponent: a Jolt rigid body attached to a GameObject.
//
// Add it (in code, the editor's Add Component menu, or a scene file) and the
// JoltPhysicsPlugin realizes a box body for the object on its next update:
// half-extents come from the mesh's local AABB scaled by the object's world
// scale (a 0.5 half-extent unit box when there is no mesh), at the object's
// world position. Dynamic bodies write their simulated pose back into the
// owner's transform every step (via the plugin's existing binding sync);
// static bodies act as obstacles. In Edit mode (app.simulating == false) the
// body teleports along with its object, so what you place is what simulates
// when Play starts.
//
// v1 constraints (documented, revisit with the render/undo phases): bodies
// assume root-level objects (a parented owner's local transform receives
// world poses), the shape ignores the object's rotation at creation time,
// and Details-panel parameter editing arrives with generalized component
// reflection — until then bodies are Dynamic unless code sets `dynamic`.
//
// This header stays Jolt-free (PhysicsWorld pimpl), like the plugin's.

#pragma once

#include "engine/scene/Component.h"
#include "JoltPhysics/PhysicsWorld.h"

#include <vector>

class PhysicsBodyComponent : public Component
{
public:
	~PhysicsBodyComponent() override;

	bool dynamic = true;

	static const char* StaticTypeId() { return "PhysicsBody"; }
	const char* TypeId() const override { return StaticTypeId(); }
	const char* DisplayName() const override { return "Physics Body"; }

	void OnAttach() override;
	void OnUnload() override; // removes the body from the physics world

	void Serialize(ISerializer& out) const override;
	void Deserialize(const IDeserializer& in) override;

	// --- Plugin-facing state (managed by JoltPhysicsPlugin::OnUpdate) -------
	PhysicsWorld::BodyId body = PhysicsWorld::InvalidBody;
	// Last pose pushed to the body in edit mode; the plugin teleports the
	// body only when the owner actually moved.
	glm::vec3 lastSyncedPosition = glm::vec3(0.0f);
	glm::quat lastSyncedRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

	// Every live component, in attach order; the plugin walks this to realize
	// bodies and keep edit-mode poses in sync.
	static const std::vector<PhysicsBodyComponent*>& Instances();

private:
	void ReleaseBody();
};
