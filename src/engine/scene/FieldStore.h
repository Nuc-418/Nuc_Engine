// FieldStore: an in-memory ISerializer/IDeserializer pair.
//
// component->Serialize(store) snapshots its state; store fed to
// component->Deserialize restores it. Copyable, so the editor's undo stack
// records component edits as before/after FieldStores, and tests round-trip
// components without touching JSON.

#pragma once

#include <map>
#include <string>

#include "engine/scene/Serialization.h"

class FieldStore : public ISerializer, public IDeserializer
{
public:
	std::map<std::string, float> floats;
	std::map<std::string, int> ints;
	std::map<std::string, bool> bools;
	std::map<std::string, glm::vec3> vec3s;
	std::map<std::string, std::string> strings;

	void Write(const char* key, float value) override { floats[key] = value; }
	void Write(const char* key, int value) override { ints[key] = value; }
	void Write(const char* key, bool value) override { bools[key] = value; }
	void Write(const char* key, const glm::vec3& value) override { vec3s[key] = value; }
	void Write(const char* key, const std::string& value) override { strings[key] = value; }

	float ReadFloat(const char* key, float fallback) const override
	{ auto it = floats.find(key); return it != floats.end() ? it->second : fallback; }
	int ReadInt(const char* key, int fallback) const override
	{ auto it = ints.find(key); return it != ints.end() ? it->second : fallback; }
	bool ReadBool(const char* key, bool fallback) const override
	{ auto it = bools.find(key); return it != bools.end() ? it->second : fallback; }
	glm::vec3 ReadVec3(const char* key, const glm::vec3& fallback) const override
	{ auto it = vec3s.find(key); return it != vec3s.end() ? it->second : fallback; }
	std::string ReadString(const char* key, const std::string& fallback) const override
	{ auto it = strings.find(key); return it != strings.end() ? it->second : fallback; }

	bool operator==(const FieldStore& other) const
	{
		return floats == other.floats && ints == other.ints && bools == other.bools
		    && vec3s == other.vec3s && strings == other.strings;
	}
	bool operator!=(const FieldStore& other) const { return !(*this == other); }
};
