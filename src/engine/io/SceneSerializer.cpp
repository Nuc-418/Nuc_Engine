// SceneSerializer: JSON save/load of a World (objects, lights, camera).

#include "engine/io/SceneSerializer.h"
#include "engine/io/JsonSerialization.h"
#include "engine/scene/World.h"

#include <fstream>
#include <iostream>
#include <map>
#include <utility>
#include <vector>

using nlohmann::json;

bool SceneSerializer::Save(const World& world, const std::string& path)
{
	json root;
	root["version"] = 1;

	root["camera"] = {
		{ "position", Vec3ToJson(world.camera.transform.position) },
		{ "rotation", Vec3ToJson(world.camera.transform.rotation) },
	};
	root["renderMode"] = (int)world.renderMode;
	// References an object's "id" below (0 = none); resolved after load.
	root["activeCamera"] = world.activeCameraId;

	json objects = json::array();
	for (const WorldEntry& entry : world.entries) {
		const Transform& transform = entry.object->transform;

		// Each component records its type + state; the mesh's geometry still
		// comes from the spawn factory (keyed by "type"), so MeshComponent has
		// no state of its own here.
		json components = ComponentsToJson(*entry.object);

		// position/rotation/scale are the LOCAL transform; "parent" (0 = root)
		// references another object's "id" and is resolved after load.
		objects.push_back({
			{ "type", entry.typeId },
			{ "name", entry.object->name },
			{ "id", entry.id },
			{ "parent", entry.object->Parent() ? world.IdOf(entry.object->Parent()) : 0 },
			{ "position", Vec3ToJson(transform.position) },
			{ "rotation", Vec3ToJson(transform.rotation) },
			{ "scale", Vec3ToJson(transform.scale) },
			{ "components", components },
		});
	}
	root["objects"] = objects;

	const VectorLight& info = world.lights.lightInfo;
	json lights;
	lights["ambient"] = json::array();
	for (const AmbientLight& light : info.ambientLight)
		lights["ambient"].push_back({ { "on", light.switchL }, { "ambient", Vec3ToJson(light.ambient) } });
	lights["directional"] = json::array();
	for (const DirectionalLight& light : info.directionalLight)
		lights["directional"].push_back({
			{ "on", light.switchL }, { "direction", Vec3ToJson(light.direction) },
			{ "ambient", Vec3ToJson(light.ambient) }, { "diffuse", Vec3ToJson(light.diffuse) },
			{ "specular", Vec3ToJson(light.specular) } });
	lights["point"] = json::array();
	for (const PointLight& light : info.pointLight)
		lights["point"].push_back({
			{ "on", light.switchL }, { "position", Vec3ToJson(light.position) },
			{ "ambient", Vec3ToJson(light.ambient) }, { "diffuse", Vec3ToJson(light.diffuse) },
			{ "specular", Vec3ToJson(light.specular) }, { "constant", light.constant },
			{ "linear", light.linear }, { "quadratic", light.quadratic } });
	lights["spot"] = json::array();
	for (const SpotLight& light : info.spotLight)
		lights["spot"].push_back({
			{ "on", light.switchL }, { "position", Vec3ToJson(light.position) },
			{ "direction", Vec3ToJson(light.direction) }, { "cutOff", light.cutOff },
			{ "ambient", Vec3ToJson(light.ambient) }, { "diffuse", Vec3ToJson(light.diffuse) },
			{ "specular", Vec3ToJson(light.specular) }, { "constant", light.constant },
			{ "linear", light.linear }, { "quadratic", light.quadratic } });
	root["lights"] = lights;

	std::ofstream file(path);
	if (!file.is_open()) {
		std::cout << "Scene save failed, cannot write: " << path << std::endl;
		return false;
	}
	file << root.dump(2);
	std::cout << "Scene saved: " << path << std::endl;
	return true;
}

bool SceneSerializer::Load(World& world, const std::string& path)
{
	std::ifstream file(path);
	if (!file.is_open()) {
		std::cout << "Scene load failed, cannot read: " << path << std::endl;
		return false;
	}

	json root = json::parse(file, nullptr, /*allow_exceptions=*/false);
	if (root.is_discarded() || !root.is_object()) {
		std::cout << "Scene load failed, invalid JSON: " << path << std::endl;
		return false;
	}
	if (root.value("version", 0) != 1) {
		std::cout << "Scene load failed, unsupported version in: " << path << std::endl;
		return false;
	}

	world.Clear();

	// Saved id -> spawned object, plus deferred parent links (a parent may be
	// saved after its child, so links resolve only after every spawn).
	std::map<unsigned long long, GameObject*> bySavedId;
	std::vector<std::pair<GameObject*, unsigned long long>> parentLinks;

	for (const json& item : root.value("objects", json::array())) {
		std::string typeId = item.value("type", "");
		if (!world.CanSpawn(typeId)) {
			std::cout << "Scene load: skipping unknown object type '" << typeId << "'" << std::endl;
			continue;
		}
		GameObject* object = world.Spawn(typeId, item.value("name", ""));
		if (!object)
			continue;
		unsigned long long savedId = item.value("id", 0ULL);
		if (savedId != 0)
			bySavedId[savedId] = object;
		unsigned long long parentId = item.value("parent", 0ULL);
		if (parentId != 0)
			parentLinks.push_back({ object, parentId });
		object->transform.position = Vec3FromJson(item.value("position", json()));
		object->transform.rotation = Vec3FromJson(item.value("rotation", json()));
		object->transform.scale = Vec3FromJson(item.value("scale", json()), glm::vec3(1.0f));

		// Restore components through the shared reconciliation path (the
		// spawn factory already created this type's defaults, e.g. the mesh).
		ReadComponentsInto(*object, item.value("components", json::array()));
	}

	// The saved position/rotation/scale are local values, so links restore
	// with keepWorldTransform=false.
	for (const std::pair<GameObject*, unsigned long long>& link : parentLinks) {
		auto it = bySavedId.find(link.second);
		if (it != bySavedId.end())
			link.first->SetParent(it->second, /*keepWorldTransform=*/false);
	}

	world.activeCameraId = 0;
	unsigned long long savedActiveCamera = root.value("activeCamera", 0ULL);
	if (savedActiveCamera != 0) {
		auto it = bySavedId.find(savedActiveCamera);
		if (it != bySavedId.end())
			world.activeCameraId = world.IdOf(it->second);
	}

	world.camera.transform.position = Vec3FromJson(root["camera"].value("position", json()), world.camera.transform.position);
	world.camera.transform.rotation = Vec3FromJson(root["camera"].value("rotation", json()), world.camera.transform.rotation);
	world.renderMode = (GLenum)root.value("renderMode", (int)GL_TRIANGLES);

	VectorLight& info = world.lights.lightInfo;
	const json lights = root.value("lights", json::object());

	info.ambientLight.clear();
	for (const json& item : lights.value("ambient", json::array())) {
		AmbientLight light;
		light.switchL = item.value("on", 1);
		light.ambient = Vec3FromJson(item.value("ambient", json()));
		info.ambientLight.push_back(light);
	}
	info.directionalLight.clear();
	for (const json& item : lights.value("directional", json::array())) {
		DirectionalLight light;
		light.switchL = item.value("on", 1);
		light.direction = Vec3FromJson(item.value("direction", json()));
		light.ambient = Vec3FromJson(item.value("ambient", json()));
		light.diffuse = Vec3FromJson(item.value("diffuse", json()));
		light.specular = Vec3FromJson(item.value("specular", json()));
		info.directionalLight.push_back(light);
	}
	info.pointLight.clear();
	for (const json& item : lights.value("point", json::array())) {
		PointLight light;
		light.switchL = item.value("on", 1);
		light.position = Vec3FromJson(item.value("position", json()));
		light.ambient = Vec3FromJson(item.value("ambient", json()));
		light.diffuse = Vec3FromJson(item.value("diffuse", json()));
		light.specular = Vec3FromJson(item.value("specular", json()));
		light.constant = item.value("constant", 1.0f);
		light.linear = item.value("linear", 0.0f);
		light.quadratic = item.value("quadratic", 0.0f);
		info.pointLight.push_back(light);
	}
	info.spotLight.clear();
	for (const json& item : lights.value("spot", json::array())) {
		SpotLight light;
		light.switchL = item.value("on", 1);
		light.position = Vec3FromJson(item.value("position", json()));
		light.direction = Vec3FromJson(item.value("direction", json()));
		light.cutOff = item.value("cutOff", 0.26f);
		light.ambient = Vec3FromJson(item.value("ambient", json()));
		light.diffuse = Vec3FromJson(item.value("diffuse", json()));
		light.specular = Vec3FromJson(item.value("specular", json()));
		light.constant = item.value("constant", 1.0f);
		light.linear = item.value("linear", 0.0f);
		light.quadratic = item.value("quadratic", 0.0f);
		info.spotLight.push_back(light);
	}

	/* Re-upload everything (counts included) to the lit program. */
	world.UploadLights();

	std::cout << "Scene loaded: " << path << std::endl;
	return true;
}
