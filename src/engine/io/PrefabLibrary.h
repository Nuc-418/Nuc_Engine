// PrefabLibrary: GameObjects as reusable assets.
//
// A prefab is a JSON file in assets/prefabs: a base spawn type (whose factory
// provides the geometry/shader, exactly like scene files) plus the object's
// component states and rotation/scale. Position is not stored — it comes from
// placement, like any spawn.
//
// Every prefab registers as a World spawn type with id "prefab:<name>", so
// instances appear in the editor's Add menu / Content Browser, drag-drop
// spawn, undo respawn and scene files with zero extra plumbing. Spawning
// reconciles components through the same path as scene load
// (ReadComponentsInto). Prefabs may be based on other prefabs.

#pragma once

#include <string>

class GameObject;
class World;

namespace Prefabs
{
	constexpr const char* Directory = "assets/prefabs";

	// Saves `object` as Directory/<name>.prefab.json (creating the directory)
	// and registers/refreshes its spawn type in `world`.
	bool Save(World& world, const GameObject& object, const std::string& name);

	// Scans Directory and registers a spawn type per prefab file. Call after
	// the scene registered its base types (factories capture `world`, which
	// must outlive them).
	void RegisterAll(World& world);
}
