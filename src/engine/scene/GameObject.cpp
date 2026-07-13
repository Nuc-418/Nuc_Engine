#include "engine/scene/GameObject.h"
#include "engine/scene/ComponentRegistry.h"
#include "engine/render/Camera.h"

#include <algorithm>
#include <iostream>
using namespace std;

Component* GameObject::AddComponentById(const std::string& typeId)
{
	std::unique_ptr<Component> component = ComponentRegistry::Create(typeId);
	if (!component)
		return nullptr;
	Component* raw = component.get();
	raw->owner = this;
	components.push_back(std::move(component));
	raw->OnAttach();
	return raw;
}

Component* GameObject::GetComponentById(const std::string& typeId) const
{
	for (const std::unique_ptr<Component>& component : components)
		if (typeId == component->TypeId())
			return component.get();
	return nullptr;
}

bool GameObject::SetParent(GameObject* newParent, bool keepWorldTransform)
{
	if (newParent == parent)
		return true;
	if (newParent == this || (newParent && IsAncestorOf(newParent)))
		return false; // would create a cycle

	glm::mat4 world;
	if (keepWorldTransform)
		world = WorldMatrix();

	if (parent) {
		std::vector<GameObject*>& siblings = parent->children;
		siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
	}
	parent = newParent;
	if (parent)
		parent->children.push_back(this);

	if (keepWorldTransform) {
		glm::mat4 local = parent ? glm::inverse(parent->WorldMatrix()) * world : world;
		transform.SetFromMatrix(local);
	}
	return true;
}

bool GameObject::IsAncestorOf(const GameObject* other) const
{
	for (const GameObject* walk = other ? other->parent : nullptr; walk; walk = walk->parent)
		if (walk == this)
			return true;
	return false;
}

glm::mat4 GameObject::WorldMatrix()
{
	transform.UpdateModel();
	return parent ? parent->WorldMatrix() * transform.model : transform.model;
}

void GameObject::Update(float deltaTime)
{
	for (const std::unique_ptr<Component>& component : components)
		component->OnUpdate(deltaTime);
}

void GameObject::Simulate(float deltaTime)
{
	for (const std::unique_ptr<Component>& component : components)
		component->OnSimulate(deltaTime);
}

void GameObject::PlayBegin()
{
	for (const std::unique_ptr<Component>& component : components)
		component->OnPlayBegin();
}

void GameObject::PlayEnd()
{
	for (const std::unique_ptr<Component>& component : components)
		component->OnPlayEnd();
}

void GameObject::Draw(GLenum mode, Camera* camera)
{
	for (const std::unique_ptr<Component>& component : components)
		component->OnRender(mode, camera);
}

void GameObject::Unload()
{
	for (const std::unique_ptr<Component>& component : components)
		component->OnUnload();
}

MeshComponent& GameObject::EnsureMesh()
{
	if (MeshComponent* mesh = GetMesh())
		return *mesh;
	return *AddComponent<MeshComponent>();
}
