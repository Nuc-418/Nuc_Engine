// PluginManager: lifecycle dispatch for registered plugins, dependency-first.

#include "engine/plugin/PluginManager.h"
#include "engine/plugin/PluginSort.h"

#include <cstdio>

void PluginManager::RefreshOrder()
{
	if (!orderDirty)
		return;
	std::vector<std::string> names;
	std::vector<std::vector<std::string>> dependencies;
	for (const std::unique_ptr<EnginePlugin>& plugin : plugins) {
		names.push_back(plugin->Name());
		dependencies.push_back(plugin->Dependencies());
	}
	PluginSortResult result = SortByDependencies(names, dependencies);
	for (const std::string& error : result.errors)
		fprintf(stderr, "[plugin] %s\n", error.c_str());
	order = result.order;
	orderDirty = false;
}

void PluginManager::LoadAll(Application& app)
{
	RefreshOrder();
	for (size_t i : order) {
		if (loaded[i])
			continue;
		loaded[i] = true;
		EnginePlugin& plugin = *plugins[i];
		if (!plugin.OnLoad(app))
			fprintf(stderr, "[plugin] %s %s failed to load\n", plugin.Name(), plugin.Version());
	}
}

void PluginManager::UpdateAll(Application& app, float deltaTime)
{
	RefreshOrder();
	for (size_t i : order)
		if (loaded[i])
			plugins[i]->OnUpdate(app, deltaTime);
}

void PluginManager::UnloadAll(Application& app)
{
	RefreshOrder();
	// Reverse dependency order so plugins tear down opposite to how they loaded.
	for (size_t n = order.size(); n-- > 0;) {
		size_t i = order[n];
		if (loaded[i]) {
			plugins[i]->OnUnload(app);
			loaded[i] = false;
		}
	}
}
