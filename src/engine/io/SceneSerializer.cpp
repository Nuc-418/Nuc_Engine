// SceneSerializer: JSON save/load of a World (objects, lights, camera).

#include "engine/io/SceneSerializer.h"
#include "engine/scene/World.h"

#include "nlohmann/json.hpp"

#include <fstream>
#include <iostream>

using nlohmann::json;

static json ToJson(const glm::vec3& v)
{
	return json::array({ v.x, v.y, v.z });
}

static glm::vec3 Vec3FromJson(const json& j, glm::vec3 fallback = glm::vec3(0.0f))
{
	if (!j.is_array() || j.size() != 3)
		return fallback;
	return glm::vec3(j[0].get<float>(), j[1].get<float>(), j[2].get<float>());
}

bool SceneSerializer::Save(const World& world, const std::string& path)
{
	json root;
	root["version"] = 1;

	root["camera"] = {
		{ "position", ToJson(world.camera.transform.position) },
		{ "rotation", ToJson(world.camera.transform.rotation) },
	};
	root["renderMode"] = (int)world.renderMode;

	json objects = json::array();
	for (const WorldEntry& entry : world.entries) {
		const Transform& transform = entry.object->transform;
		objects.push_back({
			{ "type", ToString(entry.type) },
			{ "name", entry.object->name },
			{ "position", ToJson(transform.position) },
			{ "rotation", ToJson(transform.rotation) },
			{ "scale", ToJson(transform.scale) },
		});
	}
	root["objects"] = objects;

	const VectorLight& info = world.lights.lightInfo;
	json lights;
	lights["ambient"] = json::array();
	for (const AmbientLight& light : info.ambientLight)
		lights["ambient"].push_back({ { "on", light.switchL }, { "ambient", ToJson(light.ambient) } });
	lights["directional"] = json::array();
	for (const DirectionalLight& light : info.directionalLight)
		lights["directional"].push_back({
			{ "on", light.switchL }, { "direction", ToJson(light.direction) },
			{ "ambient", ToJson(light.ambient) }, { "diffuse", ToJson(light.diffuse) },
			{ "specular", ToJson(light.specular) } });
	lights["point"] = json::array();
	for (const PointLight& light : info.pointLight)
		lights["point"].push_back({
			{ "on", light.switchL }, { "position", ToJson(light.position) },
			{ "ambient", ToJson(light.ambient) }, { "diffuse", ToJson(light.diffuse) },
			{ "specular", ToJson(light.specular) }, { "constant", light.constant },
			{ "linear", light.linear }, { "quadratic", light.quadratic } });
	lights["spot"] = json::array();
	for (const SpotLight& light : info.spotLight)
		lights["spot"].push_back({
			{ "on", light.switchL }, { "position", ToJson(light.position) },
			{ "direction", ToJson(light.direction) }, { "cutOff", light.cutOff },
			{ "ambient", ToJson(light.ambient) }, { "diffuse", ToJson(light.diffuse) },
			{ "specular", ToJson(light.specular) }, { "constant", light.constant },
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

	for (const json& item : root.value("objects", json::array())) {
		ObjectType type;
		std::string typeName = item.value("type", "");
		if (!ObjectTypeFromString(typeName, type) || !world.CanSpawn(type)) {
			std::cout << "Scene load: skipping unknown object type '" << typeName << "'" << std::endl;
			continue;
		}
		GameObject* object = world.Spawn(type, item.value("name", ""));
		if (!object)
			continue;
		object->transform.position = Vec3FromJson(item.value("position", json()));
		object->transform.rotation = Vec3FromJson(item.value("rotation", json()));
		object->transform.scale = Vec3FromJson(item.value("scale", json()), glm::vec3(1.0f));
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
	if (world.lightsProgram != 0) {
		if (!info.ambientLight.empty())
			world.lights.StoreAmbientLights(world.lightsProgram);
		if (!info.directionalLight.empty())
			world.lights.StoreDirectionalLights(world.lightsProgram, (int)info.directionalLight.size());
		world.lights.StorePointLights(world.lightsProgram, (int)info.pointLight.size());
		world.lights.StoreSpotLights(world.lightsProgram, (int)info.spotLight.size());
	}

	std::cout << "Scene loaded: " << path << std::endl;
	return true;
}
