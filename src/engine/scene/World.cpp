// World: registry of scene objects plus lights, camera and render settings.

#include "engine/scene/World.h"

void World::RegisterType(const std::string& id, const std::string& label, SpawnFn factory)
{
	if (types.find(id) == types.end())
		typeOrder.push_back(id);
	types[id] = { label.empty() ? id : label, std::move(factory) };
}

bool World::CanSpawn(const std::string& id) const
{
	return types.count(id) != 0;
}

const std::string& World::TypeLabel(const std::string& id) const
{
	auto it = types.find(id);
	return it != types.end() ? it->second.label : id;
}

GameObject* World::Spawn(const std::string& id, std::string name, unsigned long long forcedId)
{
	auto type = types.find(id);
	if (type == types.end())
		return nullptr;

	std::unique_ptr<GameObject> object = type->second.factory();
	if (!object)
		return nullptr;

	object->name = name.empty() ? UniqueName(type->second.label) : name;

	WorldEntry entry;
	entry.typeId = id;
	entry.id = forcedId != 0 ? forcedId : nextId++;
	if (entry.id >= nextId)
		nextId = entry.id + 1;
	entry.object = std::move(object);
	entries.push_back(std::move(entry));

	return entries.back().object.get();
}

std::unique_ptr<GameObject> World::Create(const std::string& id) const
{
	auto type = types.find(id);
	if (type == types.end())
		return nullptr;
	return type->second.factory();
}

GameObject* World::FindById(unsigned long long id)
{
	for (WorldEntry& entry : entries)
		if (entry.id == id)
			return entry.object.get();
	return nullptr;
}

unsigned long long World::IdOf(const GameObject* object) const
{
	const WorldEntry* entry = EntryOf(object);
	return entry ? entry->id : 0;
}

const WorldEntry* World::EntryOf(const GameObject* object) const
{
	for (const WorldEntry& entry : entries)
		if (entry.object.get() == object)
			return &entry;
	return nullptr;
}

bool World::Destroy(GameObject* object)
{
	for (size_t i = 0; i < entries.size(); i++) {
		if (entries[i].object.get() == object) {
			// Children survive: hand them to the destroyed object's parent
			// (or the root) without moving them in the world.
			std::vector<GameObject*> orphans = object->Children();
			for (GameObject* child : orphans)
				child->SetParent(object->Parent(), /*keepWorldTransform=*/true);
			object->SetParent(nullptr, /*keepWorldTransform=*/false);

			if (onDestroyed)
				onDestroyed(object);
			object->Unload();
			entries.erase(entries.begin() + i);
			return true;
		}
	}
	return false;
}

void World::Clear()
{
	// Destroy back-to-front so indices stay valid.
	while (!entries.empty())
		Destroy(entries.back().object.get());
	nameCounters.clear();
}

void World::ResetToDefaultMap()
{
	Clear();

	VectorLight& info = lights.lightInfo;
	info.ambientLight.clear();
	info.directionalLight.clear();
	info.pointLight.clear();
	info.spotLight.clear();

	AmbientLight ambient;
	ambient.switchL = true;
	ambient.ambient = glm::vec3(0.35f);
	info.ambientLight.push_back(ambient);

	DirectionalLight sun;
	sun.switchL = true;
	sun.direction = glm::vec3(1.0f, -1.0f, 0.5f);
	sun.ambient = glm::vec3(0.2f);
	sun.diffuse = glm::vec3(1.0f);
	sun.specular = glm::vec3(1.0f);
	info.directionalLight.push_back(sun);

	UploadLights();

	// A flat ground plane at the origin, UE5-style. Only if the scene that owns
	// this world registered the factory (the editor always has; a bare unit
	// test world may not).
	if (CanSpawn("Ground"))
		Spawn("Ground", "Ground");

	// Elevated view looking down onto the floor.
	camera.transform.position = glm::vec3(0.0f, 7.0f, -16.0f);
	camera.transform.rotation = glm::vec3(0.0f, 0.35f, 0.0f); // pitch down ~20 deg
	renderMode = GL_TRIANGLES;
}

void World::UploadLights()
{
	if (lightsProgram == 0)
		return;
	if (!lights.lightInfo.ambientLight.empty())
		lights.StoreAmbientLights(lightsProgram);
	if (!lights.lightInfo.directionalLight.empty())
		lights.StoreDirectionalLights(lightsProgram, (int)lights.lightInfo.directionalLight.size());
	lights.StorePointLights(lightsProgram, (int)lights.lightInfo.pointLight.size());
	lights.StoreSpotLights(lightsProgram, (int)lights.lightInfo.spotLight.size());
}

std::string World::UniqueName(const std::string& base)
{
	int& counter = nameCounters[base];
	std::string name;
	bool taken = true;
	while (taken) {
		name = base + "_" + std::to_string(counter++);
		taken = false;
		for (const WorldEntry& entry : entries) {
			if (entry.object->name == name) { taken = true; break; }
		}
	}
	return name;
}
