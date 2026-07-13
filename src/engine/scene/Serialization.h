// Serialization interfaces: an engine-owned abstraction over the concrete
// serializer (JSON, today). Components serialize through these so they never
// depend on the JSON library directly — the same reason plugins can define
// serializable components without pulling in nlohmann.
//
// SceneSerializer implements ISerializer/IDeserializer over nlohmann::json.

#pragma once

#include <string>
#include <glm/glm.hpp>

// Writes named fields into the current component's record.
class ISerializer
{
public:
	virtual ~ISerializer() = default;
	virtual void Write(const char* key, float value) = 0;
	virtual void Write(const char* key, int value) = 0;
	virtual void Write(const char* key, bool value) = 0;
	virtual void Write(const char* key, const glm::vec3& value) = 0;
	virtual void Write(const char* key, const std::string& value) = 0;
};

// Reads named fields, returning the supplied default when a key is absent.
class IDeserializer
{
public:
	virtual ~IDeserializer() = default;
	virtual float ReadFloat(const char* key, float fallback) const = 0;
	virtual int ReadInt(const char* key, int fallback) const = 0;
	virtual bool ReadBool(const char* key, bool fallback) const = 0;
	virtual glm::vec3 ReadVec3(const char* key, const glm::vec3& fallback) const = 0;
	virtual std::string ReadString(const char* key, const std::string& fallback) const = 0;
};
