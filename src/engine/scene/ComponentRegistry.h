// ComponentRegistry: creates components by their string TypeId.
//
// Used to reconstruct components on scene load and to drive the editor's
// "Add Component" list. Core registers its components (Mesh, ...); plugins
// register theirs at startup, so the registry is the seam that lets a plugin
// add a component type without core knowing it.

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

class Component;

class ComponentRegistry
{
public:
	using Factory = std::function<std::unique_ptr<Component>()>;

	// Registers a component type. `label` is shown in the editor. Re-registering
	// an id replaces its factory/label but keeps its order.
	static void Register(const std::string& typeId, const std::string& label, Factory factory);

	// Creates a component of the given type, or nullptr if the id is unknown.
	static std::unique_ptr<Component> Create(const std::string& typeId);

	static bool IsRegistered(const std::string& typeId);

	// Registered type ids in registration order (drives the Add Component menu).
	static const std::vector<std::string>& TypeIds();
	static const std::string& Label(const std::string& typeId);
};
