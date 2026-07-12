// SceneSerializer: JSON save/load of a World (objects, lights, camera).

#pragma once

#include <string>

class World;

namespace SceneSerializer
{
	// The scene a packaged game build loads at startup; the editor's
	// Package Game action saves the current world here.
	constexpr const char* StartupScenePath = "assets/scenes/startup.json";

	bool Save(const World& world, const std::string& path);

	// Clears the world and respawns objects through its registered factories,
	// then restores transforms, lights and camera.
	bool Load(World& world, const std::string& path);
}
