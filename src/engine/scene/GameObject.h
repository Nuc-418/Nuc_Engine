// GameObject: an actor — a Transform root plus a list of Components.
//
// Behaviour lives in components (MeshComponent, and later LightComponent,
// PhysicsBodyComponent, ...); the scene drives Update/Draw, which dispatch to
// them. The mesh authoring helpers (LoadObjFile, CreateObj*) are kept for the
// spawn factories and operate on the object's MeshComponent.
//
// A GameObject is heap-owned (unique_ptr in World) and never copied or moved,
// so component back-pointers into it (e.g. MeshRenderer::transformPtr) stay
// valid for its lifetime.

#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "engine/scene/Transform.h"
#include "engine/scene/Component.h"
#include "engine/render/MeshComponent.h"

using namespace std;

class GameObject
{
public:
	GameObject() = default;
	~GameObject() = default;
	GameObject(const GameObject&) = delete;
	GameObject& operator=(const GameObject&) = delete;

	string name;
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

	// First component of type T, or nullptr.
	template <class T>
	T* GetComponent() const
	{
		for (const std::unique_ptr<Component>& component : components)
			if (T* typed = dynamic_cast<T*>(component.get()))
				return typed;
		return nullptr;
	}

	// Create/find a component by its registered TypeId (scene load / editor).
	Component* AddComponentById(const std::string& typeId);
	Component* GetComponentById(const std::string& typeId) const;

	const std::vector<std::unique_ptr<Component>>& Components() const { return components; }

	// --- Per-frame dispatch ------------------------------------------------
	void Update(float deltaTime);
	void Draw(GLenum mode, Camera* camera);
	void Unload(); // frees component GL/external resources (GL context must be current)

	// --- Mesh convenience --------------------------------------------------
	MeshComponent* GetMesh() const { return GetComponent<MeshComponent>(); }
	MeshComponent& EnsureMesh(); // returns the mesh component, creating one if absent

	// Authoring helpers used by the spawn factories; populate the MeshComponent.
	bool LoadObjFile(GLuint programShader, string folderPath, string fileName);
	void CreateObjPosColor(GLuint programShader, vector<glm::vec3>* positionArray, vector<glm::vec3>* colorArray);
	void CreateObjPosNormColor(GLuint programShader, vector<glm::vec3>* positionArray, vector<glm::vec3>* normalArray, vector<glm::vec3>* colorArray);
	void CreateObjPosUvNorm(GLuint programShader, vector<glm::vec3>* positionArray, vector<glm::vec2>* uvArray, vector<glm::vec3>* normalArray);

private:
	std::vector<std::unique_ptr<Component>> components;
};
