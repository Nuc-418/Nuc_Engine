// World: registry of scene objects plus the lights, camera and render settings
// that belong to the running scene. The editor enumerates, spawns and deletes
// objects through this; game scenes populate it and keep raw handles.
//
// Spawnable types are registered dynamically by string id (see RegisterType),
// so adding a mesh or discovering a model at runtime needs no enum or switch.

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "engine/scene/GameObject.h"
#include "engine/render/Lights.h"
#include "engine/render/Camera.h"

struct WorldEntry
{
	// Stable key of the type this object was spawned from (serialized as-is and
	// used to respawn on undo). Empty only for objects added outside Spawn.
	std::string typeId;
	unsigned long long id = 0; // stable identity for undo history
	// GameObject wires meshRenderer.transformPtr to its own transform, so the
	// object itself must never be copied or moved; unique_ptr storage keeps
	// the address stable while the entries vector grows.
	std::unique_ptr<GameObject> object;
};

class World
{
public:
	using SpawnFn = std::function<std::unique_ptr<GameObject>()>;

	// Registers a spawnable type. `id` is the stable key (also what gets
	// serialized); `label` is the human-readable name shown in the editor.
	// Re-registering an id replaces its factory/label but keeps its order.
	void RegisterType(const std::string& id, const std::string& label, SpawnFn factory);
	bool CanSpawn(const std::string& id) const;

	// Spawns an object of the given type; an empty name is replaced by a unique
	// "<Label>_<n>". forcedId re-attaches a previous identity (undo of a
	// delete); 0 = new id.
	GameObject* Spawn(const std::string& id, std::string name = "", unsigned long long forcedId = 0);

	// Runs a type's factory without adding it to the world. Used to build
	// throwaway objects (e.g. Content Browser thumbnails). Null if unregistered.
	std::unique_ptr<GameObject> Create(const std::string& id) const;

	// Registered type ids, in registration order (drives the editor palettes).
	const std::vector<std::string>& TypeIds() const { return typeOrder; }
	const std::string& TypeLabel(const std::string& id) const;

	GameObject* FindById(unsigned long long id);
	unsigned long long IdOf(const GameObject* object) const; // 0 if absent
	const WorldEntry* EntryOf(const GameObject* object) const;

	// Fires onDestroyed, frees the mesh, and removes the entry.
	bool Destroy(GameObject* object);

	// Destroys every object (scene load / shutdown).
	void Clear();

	// Empties the world into a fresh default map: no objects (bar a ground
	// plane if registered), a soft ambient and one directional light, camera
	// and render mode back to defaults.
	void ResetToDefaultMap();

	std::vector<WorldEntry> entries;

	// Scenes hook this to null their raw handles when an object goes away.
	std::function<void(GameObject*)> onDestroyed;

	// World-level authored lights (scene file / Lights panel / demo setup).
	Lights lights;
	GLuint lightsProgram = 0; // the program the lights are uploaded to

	// Authored lights merged with every LightComponent in the world (their
	// position/direction follow the owning object's world transform). This is
	// what is actually uploaded — read it (not `lights`) for "what's lit now",
	// e.g. StorePrimitiveLight.
	Lights combinedLights;

	// Rebuilds combinedLights and re-uploads to lightsProgram; safe to call
	// with any vector empty. Used after wholesale light-state changes
	// (scene load, undo/redo).
	void UploadLights();

	// Rebuilds combinedLights and uploads only if something changed (a light
	// component was added/removed/edited or its object moved). Call once per
	// frame before drawing.
	void SyncComponentLights();
	Camera camera{ glm::vec3(1.0f, 1.0f, -10.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f) };
	GLenum renderMode = GL_TRIANGLES;

private:
	std::string UniqueName(const std::string& base);
	VectorLight BuildCombinedLights();

	struct TypeInfo { std::string label; SpawnFn factory; };
	std::map<std::string, TypeInfo> types;
	std::vector<std::string> typeOrder;
	std::map<std::string, int> nameCounters;
	unsigned long long nextId = 1;
};
