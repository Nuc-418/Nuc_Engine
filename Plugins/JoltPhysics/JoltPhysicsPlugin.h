// JoltPhysicsPlugin: the engine plugin that owns a PhysicsWorld and drives it.
//
// Lifecycle (see PluginManager / Application::Run):
//   - constructed and its world initialized when a scene first requests it via
//     app.plugins.GetOrAdd<JoltPhysicsPlugin>();
//   - OnUpdate steps the simulation each frame *only while app.simulating*, then
//     writes each bound body's transform back into its GameObject Transform;
//   - OnUnload tears the world down.
//
// This header stays Jolt-free (PhysicsWorld hides Jolt behind a pimpl), so
// scene/game code can use it without pulling in Jolt or C++17.

#pragma once

#include <vector>

#include "engine/plugin/Plugin.h"
#include "JoltPhysics/PhysicsWorld.h"

class Transform;

class JoltPhysicsPlugin : public EnginePlugin
{
public:
	JoltPhysicsPlugin();
	~JoltPhysicsPlugin() override;

	// The registered plugin, or null before a scene registers one. Lets
	// PhysicsBodyComponent manage its body without an Application in scope
	// (a proper service registry is a later roadmap phase).
	static JoltPhysicsPlugin* Instance();

	const char* Name() const override { return "JoltPhysics"; }
	const char* Version() const override { return "0.2.0"; }

	// Registers PhysicsBodyComponent with the ComponentRegistry — explicit
	// registration (called by PluginManager::GetOrAdd), not a static
	// initializer a static-library link could strip.
	void RegisterTypes() override;

	bool OnLoad(Application& app) override;
	void OnUpdate(Application& app, float deltaTime) override;
	void OnUnload(Application& app) override;

	// The physics world, for creating and querying bodies directly.
	PhysicsWorld& World() { return world; }

	// After each step, copy `id`'s world position/rotation into `transform`.
	// A transform may be bound to at most one body; binding again replaces it.
	void Bind(PhysicsWorld::BodyId id, Transform* transform);

	// Stops syncing `transform` (call before the Transform/GameObject is freed).
	// Does not remove the body from the world — use World().RemoveBody for that.
	void UnbindTransform(Transform* transform);

private:
	// Creates bodies for PhysicsBodyComponents that don't have one yet and,
	// in edit mode, teleports bodies to follow their objects.
	void SyncBodyComponents(bool simulating);

	PhysicsWorld world;

	struct Binding { PhysicsWorld::BodyId id; Transform* transform; };
	std::vector<Binding> bindings;
};
