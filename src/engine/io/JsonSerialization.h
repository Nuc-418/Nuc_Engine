// JSON adapters for the engine's serialization interfaces, shared by
// SceneSerializer and PrefabLibrary. Only io-module .cpp files include this
// (it pulls in nlohmann); components keep seeing ISerializer/IDeserializer.

#pragma once

#include <string>

#include <glm/glm.hpp>
#include "nlohmann/json.hpp"

#include "engine/scene/GameObject.h"
#include "engine/scene/Serialization.h"

inline nlohmann::json Vec3ToJson(const glm::vec3& v)
{
	return nlohmann::json::array({ v.x, v.y, v.z });
}

inline glm::vec3 Vec3FromJson(const nlohmann::json& j, glm::vec3 fallback = glm::vec3(0.0f))
{
	if (!j.is_array() || j.size() != 3)
		return fallback;
	return glm::vec3(j[0].get<float>(), j[1].get<float>(), j[2].get<float>());
}

// Each component writes/reads its fields into one JSON object.
class JsonWriter : public ISerializer
{
public:
	explicit JsonWriter(nlohmann::json& target) : j(target) {}
	void Write(const char* key, float value) override { j[key] = value; }
	void Write(const char* key, int value) override { j[key] = value; }
	void Write(const char* key, bool value) override { j[key] = value; }
	void Write(const char* key, const glm::vec3& value) override { j[key] = Vec3ToJson(value); }
	void Write(const char* key, const std::string& value) override { j[key] = value; }
private:
	nlohmann::json& j;
};

class JsonReader : public IDeserializer
{
public:
	explicit JsonReader(const nlohmann::json& source) : j(source) {}
	float ReadFloat(const char* key, float fallback) const override { return j.value(key, fallback); }
	int ReadInt(const char* key, int fallback) const override { return j.value(key, fallback); }
	bool ReadBool(const char* key, bool fallback) const override { return j.value(key, fallback); }
	glm::vec3 ReadVec3(const char* key, const glm::vec3& fallback) const override
	{
		return j.contains(key) ? Vec3FromJson(j[key], fallback) : fallback;
	}
	std::string ReadString(const char* key, const std::string& fallback) const override { return j.value(key, fallback); }
private:
	const nlohmann::json& j;
};

// Writes every component of `object` as [{ "type", ...state }, ...].
inline nlohmann::json ComponentsToJson(const GameObject& object)
{
	nlohmann::json components = nlohmann::json::array();
	for (const std::unique_ptr<Component>& component : object.Components()) {
		nlohmann::json record;
		record["type"] = component->TypeId();
		JsonWriter writer(record);
		component->Serialize(writer);
		components.push_back(record);
	}
	return components;
}

// Reconciliation used by scene load and prefab spawn: for each saved record,
// reuse a matching component if the object already has one (the spawn factory
// creates the defaults, e.g. the mesh), else create it via the registry, then
// let it read its state.
inline void ReadComponentsInto(GameObject& object, const nlohmann::json& componentsArray)
{
	for (const nlohmann::json& record : componentsArray) {
		std::string componentType = record.value("type", "");
		if (componentType.empty())
			continue;
		Component* component = object.GetComponentById(componentType);
		if (!component)
			component = object.AddComponentById(componentType);
		if (component) {
			JsonReader reader(record);
			component->Deserialize(reader);
		}
	}
}
