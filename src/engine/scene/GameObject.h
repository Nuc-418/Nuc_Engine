// GameObject: an actor — a Transform root plus a list of Components.
//
// Behaviour lives in components (MeshComponent, LightComponent,
// CameraComponent, ...); the scene drives Update/Draw, which dispatch to
// them. Mesh authoring lives on MeshComponent (spawn factories reach it via
// EnsureMesh()).
//
// A GameObject is heap-owned (unique_ptr in World) and never copied or moved,
// so component back-pointers into it (e.g. MeshRenderer::transformPtr) stay
// valid for its lifetime.

#pragma once

#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "engine/scene/Transform.h"
#include "engine/scene/Component.h"
#include "engine/render/MeshComponent.h"

class GameObject
{
public:
	GameObject() = default;
	~GameObject() = default;
	GameObject(const GameObject&) = delete;
	GameObject& operator=(const GameObject&) = delete;

	std::string name;
	Transform transform;

	// --- Components --------------------------------------------------------
	// Constructs a component of type T, attaches it, and returns it.
	template <class T, class... Args>
	T* AddComponent(Args&&... args)
	{
		T* component = new T(std::forward<Args>(args)...);
		component->owner = this;
		components.emplace_back(component);
		component->OnAttach();
		return component;
	}

	// First component of type T, or nullptr. Matches on the component's
	// stable TypeId (T must expose StaticTypeId(), as MeshComponent does)
	// instead of RTTI.
	template <class T>
	T* GetComponent() const
	{
		for (const std::unique_ptr<Component>& component : components)
			if (std::strcmp(component->TypeId(), T::StaticTypeId()) == 0)
				return static_cast<T*>(component.get());
		return nullptr;
	}

	// Create/find a component by its registered TypeId (scene load / editor).
	Component* AddComponentById(const std::string& typeId);
	Component* GetComponentById(const std::string& typeId) const;

	const std::vector<std::unique_ptr<Component>>& Components() const { return components; }

	// --- Hierarchy -----------------------------------------------------------
	// The transform is LOCAL to the parent; world = parentWorld * local.
	// Parent/child links are non-owning (World owns every object); World::Destroy
	// reparents children to the destroyed object's parent.

	// Reparents this object (nullptr = scene root). Refuses self/descendant
	// parents (returns false). With keepWorldTransform the local transform is
	// rewritten so the object does not move in the world.
	bool SetParent(GameObject* newParent, bool keepWorldTransform = true);

	GameObject* Parent() const { return parent; }
	const std::vector<GameObject*>& Children() const { return children; }
	bool IsAncestorOf(const GameObject* other) const;

	// Composes the world matrix up the parent chain (refreshes each local
	// model on the way).
	glm::mat4 WorldMatrix();

	// --- Per-frame dispatch ------------------------------------------------
	void Update(float deltaTime);
	void Simulate(float deltaTime); // OnSimulate; only called while simulating
	void PlayBegin();
	void PlayEnd();
	void Draw(GLenum mode, Camera* camera);
	void Unload(); // frees component GL/external resources (GL context must be current)

	// --- Mesh convenience --------------------------------------------------
	MeshComponent* GetMesh() const { return GetComponent<MeshComponent>(); }
	MeshComponent& EnsureMesh(); // returns the mesh component, creating one if absent

private:
	std::vector<std::unique_ptr<Component>> components;
	GameObject* parent = nullptr;
	std::vector<GameObject*> children;
};
