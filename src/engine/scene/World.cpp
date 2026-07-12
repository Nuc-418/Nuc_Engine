// World: registry of scene objects plus lights, camera and render settings.

#include "engine/scene/World.h"

const char* ToString(ObjectType type)
{
	switch (type) {
	case ObjectType::Cube:        return "Cube";
	case ObjectType::IndexedCube: return "IndexedCube";
	case ObjectType::IronMan:     return "IronMan";
	}
	return "Unknown";
}

bool ObjectTypeFromString(const std::string& text, ObjectType& out)
{
	if (text == "Cube")        { out = ObjectType::Cube;        return true; }
	if (text == "IndexedCube") { out = ObjectType::IndexedCube; return true; }
	if (text == "IronMan")     { out = ObjectType::IronMan;     return true; }
	return false;
}

void World::RegisterType(ObjectType type, SpawnFn factory)
{
	factories[type] = factory;
}

bool World::CanSpawn(ObjectType type) const
{
	return factories.count(type) != 0;
}

GameObject* World::Spawn(ObjectType type, std::string name, unsigned long long forcedId)
{
	auto factory = factories.find(type);
	if (factory == factories.end())
		return nullptr;

	std::unique_ptr<GameObject> object = factory->second();
	if (!object)
		return nullptr;

	object->name = name.empty() ? UniqueName(ToString(type)) : name;

	WorldEntry entry;
	entry.type = type;
	entry.id = forcedId != 0 ? forcedId : nextId++;
	if (entry.id >= nextId)
		nextId = entry.id + 1;
	entry.object = std::move(object);
	entries.push_back(std::move(entry));

	return entries.back().object.get();
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
			if (onDestroyed)
				onDestroyed(object);
			object->meshRenderer.mesh.Unload();
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

	camera.transform.position = glm::vec3(1.0f, 1.0f, -10.0f);
	camera.transform.rotation = glm::vec3(0.0f);
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

std::string World::UniqueName(const char* base)
{
	int& counter = nameCounters[base];
	std::string name;
	bool taken = true;
	while (taken) {
		name = std::string(base) + "_" + std::to_string(counter++);
		taken = false;
		for (const WorldEntry& entry : entries) {
			if (entry.object->name == name) { taken = true; break; }
		}
	}
	return name;
}
