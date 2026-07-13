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
//
// The hinted overloads (WriteColor, WriteEnum) exist for consumers that care
// about presentation — the editor's auto-generated Details UI draws a color
// picker or a labeled combo instead of raw drags. They default to the plain
// Write, so serializers (JSON) and existing implementations are unaffected;
// deserialization reads the same plain vec3/int back.
class ISerializer
{
public:
	virtual ~ISerializer() = default;
	virtual void Write(const char* key, float value) = 0;
	virtual void Write(const char* key, int value) = 0;
	virtual void Write(const char* key, bool value) = 0;
	virtual void Write(const char* key, const glm::vec3& value) = 0;
	virtual void Write(const char* key, const std::string& value) = 0;

	// A vec3 that is a color (UI: color picker).
	virtual void WriteColor(const char* key, const glm::vec3& value) { Write(key, value); }
	// An int that indexes a fixed label set (UI: combo). `labels` must have
	// static storage duration (string literals).
	virtual void WriteEnum(const char* key, int value, const char* const* labels, int labelCount)
	{
		(void)labels; (void)labelCount;
		Write(key, value);
	}
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
