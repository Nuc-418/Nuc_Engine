// EnginePlugin: the base interface every engine module/plugin implements.
//
// The engine treats features it does not itself depend on (physics, audio, ...)
// as plugins: self-contained modules under Plugins/ that hook into the engine
// through this interface. Engine core knows only this base class — never a
// concrete plugin — so a plugin can be added or removed without touching core.
//
// Layering: Plugins may include engine and third_party; engine core must never
// include a plugin. Game/scene code (which may include Plugins) is what
// registers concrete plugins with the Application's PluginManager.

#pragma once

#include <string>
#include <vector>

class Application;

class EnginePlugin
{
public:
	virtual ~EnginePlugin() = default;

	// Stable identifier: what Dependencies() entries refer to, and what logs
	// and tooling show.
	virtual const char* Name() const = 0;

	virtual const char* Version() const { return "0.1.0"; }

	// Names of plugins that must load and update before this one. Missing
	// names and cycles are reported loudly by the PluginManager (the plugin
	// still loads, in registration order).
	virtual std::vector<std::string> Dependencies() const { return {}; }

	// Called once, right after the plugin is registered (before any OnLoad).
	// Register component types, spawn types and other engine-facing hooks
	// here — explicit registration instead of static initializers, which a
	// static-library link can silently strip.
	virtual void RegisterTypes() {}

	// Called once after the scene has loaded, while the GL context is current.
	// Return false to report a failed initialization (the engine logs and
	// continues; the plugin should then no-op).
	virtual bool OnLoad(Application& app) { return true; }

	// Called every frame before the scene updates. `deltaTime` is seconds.
	// Simulation plugins should honor Application::simulating (see PhysicsWorld
	// stepping in JoltPhysicsPlugin) so the editor can freeze them in Edit mode.
	virtual void OnUpdate(Application& app, float deltaTime) {}

	// Called once after the main loop ends, while the GL context is still alive.
	virtual void OnUnload(Application& app) {}
};
