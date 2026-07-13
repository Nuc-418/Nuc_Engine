// PluginManager: owns the engine's plugins and drives their lifecycle.
//
// Plugins are keyed by their concrete type, so a scene fetches (or lazily
// registers) a plugin with GetOrAdd<T>() and later retrieves it with Get<T>().
// The Application owns one PluginManager and drives LoadAll/UpdateAll/UnloadAll
// around the main loop (see Application::Run).

#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "engine/plugin/Plugin.h"

class Application;

class PluginManager
{
public:
	// Returns the registered plugin of type T, constructing and registering one
	// (forwarding args to its constructor) if none exists yet. The reference is
	// stable for the manager's lifetime.
	template <class T, class... Args>
	T& GetOrAdd(Args&&... args)
	{
		std::type_index key(typeid(T));
		auto it = index.find(key);
		if (it != index.end())
			return static_cast<T&>(*plugins[it->second]);

		plugins.emplace_back(std::unique_ptr<EnginePlugin>(new T(std::forward<Args>(args)...)));
		index.emplace(key, plugins.size() - 1);
		loaded.push_back(false);
		orderDirty = true;
		plugins.back()->RegisterTypes(); // explicit registration, pre-OnLoad
		return static_cast<T&>(*plugins.back());
	}

	// Registered plugin of type T, or nullptr if none.
	template <class T>
	T* Get()
	{
		auto it = index.find(std::type_index(typeid(T)));
		return it == index.end() ? nullptr : static_cast<T*>(plugins[it->second].get());
	}

	// Calls OnLoad on every plugin not yet loaded, dependencies first (safe
	// to call again after new plugins are registered). Missing dependencies
	// and cycles are logged; the offenders still load in registration order.
	void LoadAll(Application& app);

	// Calls OnUpdate on every loaded plugin, dependencies first.
	void UpdateAll(Application& app, float deltaTime);

	// Calls OnUnload on every loaded plugin, reverse dependency order.
	void UnloadAll(Application& app);

private:
	// Recomputes the dependency order (SortByDependencies) when plugins were
	// added since the last computation.
	void RefreshOrder();

	std::vector<std::unique_ptr<EnginePlugin>> plugins;
	std::unordered_map<std::type_index, size_t> index;
	std::vector<bool> loaded;       // parallel to plugins
	std::vector<size_t> order;      // dependency-sorted indices into plugins
	bool orderDirty = false;
};
