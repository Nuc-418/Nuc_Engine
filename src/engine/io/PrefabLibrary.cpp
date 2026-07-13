#include "engine/io/PrefabLibrary.h"

#include "engine/io/JsonSerialization.h"
#include "engine/scene/World.h"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;
using nlohmann::json;

namespace {

std::string PrefabPath(const std::string& name)
{
	return std::string(Prefabs::Directory) + "/" + name + ".prefab.json";
}

// Registers (or refreshes) one prefab spawn type from its parsed file.
void RegisterPrefab(World& world, const std::string& name, const json& root)
{
	std::string baseType = root.value("type", "");
	if (baseType.empty())
		return;

	json components = root.value("components", json::array());
	glm::vec3 rotation = Vec3FromJson(root.value("rotation", json()));
	glm::vec3 scale = Vec3FromJson(root.value("scale", json()), glm::vec3(1.0f));

	World* worldPtr = &world;
	world.RegisterType("prefab:" + name, name,
		[worldPtr, baseType, components, rotation, scale]() -> std::unique_ptr<GameObject> {
			std::unique_ptr<GameObject> object = worldPtr->Create(baseType);
			if (!object) {
				std::cout << "Prefab spawn failed: base type '" << baseType << "' unknown" << std::endl;
				return nullptr;
			}
			ReadComponentsInto(*object, components);
			object->transform.rotation = rotation;
			object->transform.scale = scale;
			return object;
		});
}

} // namespace

bool Prefabs::Save(World& world, const GameObject& object, const std::string& name)
{
	const WorldEntry* entry = world.EntryOf(&object);
	if (!entry || entry->typeId.empty() || name.empty())
		return false;

	json root;
	root["version"] = 1;
	root["type"] = entry->typeId; // may itself be a prefab (nesting is fine)
	root["rotation"] = Vec3ToJson(object.transform.rotation);
	root["scale"] = Vec3ToJson(object.transform.scale);
	root["components"] = ComponentsToJson(object);

	std::error_code ec;
	fs::create_directories(Directory, ec);
	std::string path = PrefabPath(name);
	std::ofstream file(path);
	if (!file.is_open()) {
		std::cout << "Prefab save failed, cannot write: " << path << std::endl;
		return false;
	}
	file << root.dump(2);
	std::cout << "Prefab saved: " << path << std::endl;

	RegisterPrefab(world, name, root);
	return true;
}

void Prefabs::RegisterAll(World& world)
{
	std::error_code ec;
	for (fs::directory_iterator it(Directory, ec), end; !ec && it != end; it.increment(ec)) {
		std::string filename = it->path().filename().string();
		const std::string suffix = ".prefab.json";
		if (filename.size() <= suffix.size()
		    || filename.compare(filename.size() - suffix.size(), suffix.size(), suffix) != 0)
			continue;
		std::string name = filename.substr(0, filename.size() - suffix.size());

		std::ifstream file(it->path());
		if (!file.is_open())
			continue;
		json root = json::parse(file, nullptr, /*allow_exceptions=*/false);
		if (root.is_discarded() || !root.is_object() || root.value("version", 0) != 1) {
			std::cout << "Prefab skipped, invalid file: " << filename << std::endl;
			continue;
		}
		RegisterPrefab(world, name, root);
	}
}
