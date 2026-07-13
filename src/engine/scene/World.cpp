// World: registry of scene objects plus lights, camera and render settings.

#include "engine/scene/World.h"
#include "engine/core/EngineMath.h"
#include "engine/render/CameraComponent.h"
#include "engine/render/LightComponent.h"

void World::RegisterType(const std::string& id, const std::string& label, SpawnFn factory)
{
	if (types.find(id) == types.end())
		typeOrder.push_back(id);
	types[id] = { label.empty() ? id : label, std::move(factory) };
}

bool World::CanSpawn(const std::string& id) const
{
	return types.count(id) != 0;
}

const std::string& World::TypeLabel(const std::string& id) const
{
	auto it = types.find(id);
	return it != types.end() ? it->second.label : id;
}

GameObject* World::Spawn(const std::string& id, std::string name, unsigned long long forcedId)
{
	auto type = types.find(id);
	if (type == types.end())
		return nullptr;

	std::unique_ptr<GameObject> object = type->second.factory();
	if (!object)
		return nullptr;

	object->name = name.empty() ? UniqueName(type->second.label) : name;

	WorldEntry entry;
	entry.typeId = id;
	entry.id = forcedId != 0 ? forcedId : nextId++;
	if (entry.id >= nextId)
		nextId = entry.id + 1;
	entry.object = std::move(object);
	entries.push_back(std::move(entry));

	return entries.back().object.get();
}

std::unique_ptr<GameObject> World::Create(const std::string& id) const
{
	auto type = types.find(id);
	if (type == types.end())
		return nullptr;
	return type->second.factory();
}

GameObject* World::FindById(unsigned long long id)
{
	for (WorldEntry& entry : entries)
		if (entry.id == id)
			return entry.object.get();
	return nullptr;
}

unsigned long long World::IdOf(const GameObject* object) const
{
	const WorldEntry* entry = EntryOf(object);
	return entry ? entry->id : 0;
}

const WorldEntry* World::EntryOf(const GameObject* object) const
{
	for (const WorldEntry& entry : entries)
		if (entry.object.get() == object)
			return &entry;
	return nullptr;
}

bool World::Destroy(GameObject* object)
{
	for (size_t i = 0; i < entries.size(); i++) {
		if (entries[i].object.get() == object) {
			// Children survive: hand them to the destroyed object's parent
			// (or the root) without moving them in the world.
			std::vector<GameObject*> orphans = object->Children();
			for (GameObject* child : orphans)
				child->SetParent(object->Parent(), /*keepWorldTransform=*/true);
			object->SetParent(nullptr, /*keepWorldTransform=*/false);

			if (entries[i].id == activeCameraId)
				activeCameraId = 0;

			if (onDestroyed)
				onDestroyed(object);
			object->Unload();
			entries.erase(entries.begin() + i);
			return true;
		}
	}
	return false;
}

void World::Clear()
{
	// Destroy back-to-front so indices stay valid.
	while (!entries.empty())
		Destroy(entries.back().object.get());
	nameCounters.clear();
}

void World::ResetToDefaultMap()
{
	Clear();

	VectorLight& info = lights.lightInfo;
	info.ambientLight.clear();
	info.directionalLight.clear();
	info.pointLight.clear();
	info.spotLight.clear();

	AmbientLight ambient;
	ambient.switchL = true;
	ambient.ambient = glm::vec3(0.35f);
	info.ambientLight.push_back(ambient);

	DirectionalLight sun;
	sun.switchL = true;
	sun.direction = glm::vec3(1.0f, -1.0f, 0.5f);
	sun.ambient = glm::vec3(0.2f);
	sun.diffuse = glm::vec3(1.0f);
	sun.specular = glm::vec3(1.0f);
	info.directionalLight.push_back(sun);

	UploadLights();

	// A flat ground plane at the origin, UE5-style. Only if the scene that owns
	// this world registered the factory (the editor always has; a bare unit
	// test world may not).
	if (CanSpawn("Ground"))
		Spawn("Ground", "Ground");

	// Elevated view looking down onto the floor.
	camera.transform.position = glm::vec3(0.0f, 7.0f, -16.0f);
	camera.transform.rotation = glm::vec3(0.0f, 0.35f, 0.0f); // pitch down ~20 deg
	renderMode = GL_TRIANGLES;
	activeCameraId = 0;
}

// Appends each LightComponent to the authored lights. Point/spot position is
// the owner's world position; directional/spot direction is the owner's world
// forward axis (+Z rotated by the object).
VectorLight World::BuildCombinedLights()
{
	VectorLight merged = lights.lightInfo;
	for (WorldEntry& entry : entries) {
		LightComponent* light = entry.object->GetComponent<LightComponent>();
		if (!light)
			continue;

		glm::mat4 world = entry.object->WorldMatrix();
		glm::vec3 position = glm::vec3(world[3]);
		glm::vec3 direction = glm::vec3(world * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
		float length = glm::length(direction);
		direction = length > 1e-6f ? direction / length : glm::vec3(0.0f, 0.0f, 1.0f);

		switch (light->kind) {
		case LightComponent::Kind::Directional: {
			DirectionalLight l;
			l.switchL = light->on;
			l.direction = direction;
			l.ambient = light->ambient;
			l.diffuse = light->diffuse;
			l.specular = light->specular;
			merged.directionalLight.push_back(l);
			break;
		}
		case LightComponent::Kind::Point: {
			PointLight l;
			l.switchL = light->on;
			l.position = position;
			l.ambient = light->ambient;
			l.diffuse = light->diffuse;
			l.specular = light->specular;
			l.constant = light->constant;
			l.linear = light->linear;
			l.quadratic = light->quadratic;
			merged.pointLight.push_back(l);
			break;
		}
		case LightComponent::Kind::Spot: {
			SpotLight l;
			l.switchL = light->on;
			l.position = position;
			l.direction = direction;
			l.cutOff = light->cutOff;
			l.ambient = light->ambient;
			l.diffuse = light->diffuse;
			l.specular = light->specular;
			l.constant = light->constant;
			l.linear = light->linear;
			l.quadratic = light->quadratic;
			merged.spotLight.push_back(l);
			break;
		}
		}
	}
	return merged;
}

namespace {

bool LightVectorsEqual(const VectorLight& a, const VectorLight& b)
{
	if (a.ambientLight.size() != b.ambientLight.size()
	    || a.directionalLight.size() != b.directionalLight.size()
	    || a.pointLight.size() != b.pointLight.size()
	    || a.spotLight.size() != b.spotLight.size())
		return false;
	for (size_t i = 0; i < a.ambientLight.size(); i++) {
		const AmbientLight& x = a.ambientLight[i]; const AmbientLight& y = b.ambientLight[i];
		if (x.switchL != y.switchL || x.ambient != y.ambient)
			return false;
	}
	for (size_t i = 0; i < a.directionalLight.size(); i++) {
		const DirectionalLight& x = a.directionalLight[i]; const DirectionalLight& y = b.directionalLight[i];
		if (x.switchL != y.switchL || x.direction != y.direction || x.ambient != y.ambient
		    || x.diffuse != y.diffuse || x.specular != y.specular)
			return false;
	}
	for (size_t i = 0; i < a.pointLight.size(); i++) {
		const PointLight& x = a.pointLight[i]; const PointLight& y = b.pointLight[i];
		if (x.switchL != y.switchL || x.position != y.position || x.ambient != y.ambient
		    || x.diffuse != y.diffuse || x.specular != y.specular
		    || x.constant != y.constant || x.linear != y.linear || x.quadratic != y.quadratic)
			return false;
	}
	for (size_t i = 0; i < a.spotLight.size(); i++) {
		const SpotLight& x = a.spotLight[i]; const SpotLight& y = b.spotLight[i];
		if (x.switchL != y.switchL || x.position != y.position || x.direction != y.direction
		    || x.cutOff != y.cutOff || x.ambient != y.ambient || x.diffuse != y.diffuse
		    || x.specular != y.specular || x.constant != y.constant || x.linear != y.linear
		    || x.quadratic != y.quadratic)
			return false;
	}
	return true;
}

} // namespace

void World::UploadLights()
{
	combinedLights.lightInfo = BuildCombinedLights();
	if (lightsProgram == 0)
		return;
	if (!combinedLights.lightInfo.ambientLight.empty())
		combinedLights.StoreAmbientLights(lightsProgram);
	if (!combinedLights.lightInfo.directionalLight.empty())
		combinedLights.StoreDirectionalLights(lightsProgram, (int)combinedLights.lightInfo.directionalLight.size());
	combinedLights.StorePointLights(lightsProgram, (int)combinedLights.lightInfo.pointLight.size());
	combinedLights.StoreSpotLights(lightsProgram, (int)combinedLights.lightInfo.spotLight.size());
}

void World::SyncComponentLights()
{
	VectorLight merged = BuildCombinedLights();
	if (LightVectorsEqual(merged, combinedLights.lightInfo))
		return;
	UploadLights();
}

void World::Tick(float deltaTime, bool simulating)
{
	for (WorldEntry& entry : entries) {
		entry.object->Update(deltaTime);
		if (simulating)
			entry.object->Simulate(deltaTime);
	}
}

void World::NotifyPlayBegin()
{
	for (WorldEntry& entry : entries)
		entry.object->PlayBegin();
}

void World::NotifyPlayEnd()
{
	for (WorldEntry& entry : entries)
		entry.object->PlayEnd();
}

Camera* World::ActiveCamera()
{
	if (activeCameraId == 0)
		return &camera;
	GameObject* object = FindById(activeCameraId);
	CameraComponent* component = object ? object->GetComponent<CameraComponent>() : nullptr;
	if (!component)
		return &camera;

	// Pose from the owner's world transform; Camera::UpdateCam rebuilds the
	// view from position + Euler rotation each frame, so feed it those.
	glm::mat4 world = object->WorldMatrix();
	gameCamera.transform.position = glm::vec3(world[3]);
	gameCamera.transform.rotation = EulerYXZFromMatrix(world);
	gameCamera.SetPerspective(component->fovDegrees, camera.Aspect(),
	                          component->nearPlane, component->farPlane);
	return &gameCamera;
}

std::string World::UniqueName(const std::string& base)
{
	int& counter = nameCounters[base];
	std::string name;
	bool taken = true;
	while (taken) {
		name = base + "_" + std::to_string(counter++);
		taken = false;
		for (const WorldEntry& entry : entries) {
			if (entry.object->name == name) { taken = true; break; }
		}
	}
	return name;
}
