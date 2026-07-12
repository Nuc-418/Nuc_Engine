// World: registry of scene objects plus the lights, camera and render settings
// that belong to the running scene. The editor enumerates, spawns and deletes
// objects through this; game scenes populate it and keep raw handles.

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "engine/scene/GameObject.h"
#include "engine/render/Lights.h"
#include "engine/render/Camera.h"

enum class ObjectType { Cube, IndexedCube, IronMan };

const char* ToString(ObjectType type);
bool ObjectTypeFromString(const std::string& text, ObjectType& out);

struct WorldEntry
{
	ObjectType type;
	// GameObject wires meshRenderer.transformPtr to its own transform, so the
	// object itself must never be copied or moved; unique_ptr storage keeps
	// the address stable while the entries vector grows.
	std::unique_ptr<GameObject> object;
};

class World
{
public:
	using SpawnFn = std::function<std::unique_ptr<GameObject>()>;

	// Scenes register how each ObjectType is created (capturing their shader
	// programs and geometry); the editor spawns through the same factories.
	void RegisterType(ObjectType type, SpawnFn factory);
	bool CanSpawn(ObjectType type) const;

	// Spawns an object; an empty name is replaced by a unique "<Type>_<n>".
	GameObject* Spawn(ObjectType type, std::string name = "");

	// Fires onDestroyed, frees the mesh, and removes the entry.
	bool Destroy(GameObject* object);

	// Destroys every object (scene load / shutdown).
	void Clear();

	std::vector<WorldEntry> entries;

	// Scenes hook this to null their raw handles when an object goes away.
	std::function<void(GameObject*)> onDestroyed;

	Lights lights;
	GLuint lightsProgram = 0; // the program the lights are uploaded to
	Camera camera{ glm::vec3(1.0f, 1.0f, -10.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f) };
	GLenum renderMode = GL_TRIANGLES;

private:
	std::string UniqueName(const char* base);

	std::map<ObjectType, SpawnFn> factories;
	std::map<std::string, int> nameCounters;
};
