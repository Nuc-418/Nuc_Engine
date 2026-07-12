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

GameObject* World::Spawn(ObjectType type, std::string name)
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
	entry.object = std::move(object);
	entries.push_back(std::move(entry));

	return entries.back().object.get();
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
