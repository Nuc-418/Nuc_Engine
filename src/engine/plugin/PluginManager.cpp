// PluginManager: lifecycle dispatch for registered plugins.

#include "engine/plugin/PluginManager.h"

#include <cstdio>

void PluginManager::LoadAll(Application& app)
{
	for (; loadedCount < plugins.size(); ++loadedCount) {
		EnginePlugin& plugin = *plugins[loadedCount];
		if (!plugin.OnLoad(app))
			fprintf(stderr, "[plugin] %s failed to load\n", plugin.Name());
	}
}

void PluginManager::UpdateAll(Application& app, float deltaTime)
{
	for (size_t i = 0; i < loadedCount; ++i)
		plugins[i]->OnUpdate(app, deltaTime);
}

void PluginManager::UnloadAll(Application& app)
{
	// Reverse order so plugins tear down opposite to how they loaded.
	for (size_t i = loadedCount; i-- > 0;)
		plugins[i]->OnUnload(app);
	loadedCount = 0;
}
