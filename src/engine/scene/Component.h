// Component: the base for everything attachable to a GameObject (Actor).
//
// A GameObject owns a list of Components; the engine dispatches per-frame update
// and render to them and (de)serializes them. Engine core knows only this base
// class, so plugins can define their own components (e.g. the physics body) and
// register them with the ComponentRegistry — mirroring the plugin system.

#pragma once

#include <GL/glew.h> // GLenum

class GameObject;
class Camera;
class ISerializer;
class IDeserializer;

class Component
{
public:
	virtual ~Component() = default;

	// Set by GameObject when the component is attached; valid for the
	// component's lifetime.
	GameObject* owner = nullptr;

	// Stable identifier used for serialization and the ComponentRegistry
	// (e.g. "Mesh", "Light", "PhysicsBody"). Must be unique per component type.
	virtual const char* TypeId() const = 0;

	// Human-readable label for the editor; defaults to the TypeId.
	virtual const char* DisplayName() const { return TypeId(); }

	// Called once, right after `owner` is set.
	virtual void OnAttach() {}

	// Per-frame hooks. OnUpdate runs before OnRender (see GameObject::Update /
	// GameObject::Draw, driven by the scene).
	virtual void OnUpdate(float deltaTime) {}
	virtual void OnRender(GLenum mode, Camera* camera) {}

	// Release GL / external resources while the context is still current
	// (called from GameObject::Unload on destroy).
	virtual void OnUnload() {}

	// Persist / restore component state. Defaults to no state.
	virtual void Serialize(ISerializer& out) const {}
	virtual void Deserialize(const IDeserializer& in) {}
};
