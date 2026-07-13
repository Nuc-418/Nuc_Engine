// JoltPhysicsPlugin: lifecycle + body-to-Transform synchronization.

#include "JoltPhysics/JoltPhysicsPlugin.h"
#include "JoltPhysics/PhysicsBodyComponent.h"

#include "engine/core/Application.h"
#include "engine/core/EngineMath.h"
#include "engine/render/MeshComponent.h"
#include "engine/scene/GameObject.h"
#include "engine/scene/Transform.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace {
JoltPhysicsPlugin* instance = nullptr;
}

JoltPhysicsPlugin* JoltPhysicsPlugin::Instance()
{
	return instance;
}

JoltPhysicsPlugin::JoltPhysicsPlugin()
{
	// The world needs no GL context, so bring it up now — scenes create bodies
	// during Scene::Load, before the plugin's OnLoad runs.
	world.Init();
	instance = this;
}

JoltPhysicsPlugin::~JoltPhysicsPlugin()
{
	if (instance == this)
		instance = nullptr;
}

bool JoltPhysicsPlugin::OnLoad(Application& app)
{
	return world.IsInitialized();
}

void JoltPhysicsPlugin::OnUpdate(Application& app, float deltaTime)
{
	SyncBodyComponents(app.simulating);

	// Freeze the simulation when the app isn't simulating (editor Edit mode).
	if (!app.simulating)
		return;

	world.Step(deltaTime);

	// Write each dynamic body's pose back into its Transform. The renderer
	// recomputes the model matrix from position/rotation, so we only set those.
	for (const Binding& b : bindings) {
		if (!b.transform)
			continue;
		b.transform->position = world.GetPosition(b.id);

		// Transform's rotation is Euler applied as Ry(x)*Rx(y)*Rz(z); extract the
		// matching angles from the body's orientation quaternion.
		glm::mat4 rot = glm::mat4_cast(world.GetRotation(b.id));
		float yaw, pitch, roll;
		glm::extractEulerAngleYXZ(rot, yaw, pitch, roll);
		b.transform->rotation = glm::vec3(yaw, pitch, roll);
	}
}

void JoltPhysicsPlugin::SyncBodyComponents(bool simulating)
{
	if (!world.IsInitialized())
		return;

	for (PhysicsBodyComponent* component : PhysicsBodyComponent::Instances()) {
		GameObject* object = component->owner;
		if (!object)
			continue;

		glm::mat4 worldMatrix = object->WorldMatrix();
		glm::vec3 position = glm::vec3(worldMatrix[3]);

		if (component->body == PhysicsWorld::InvalidBody) {
			// Box shape from the mesh's local AABB scaled by the object's
			// world scale; a plain 0.5 half-extent box for meshless objects.
			glm::vec3 halfExtents(0.5f);
			glm::vec3 center = position;
			glm::vec3 worldScale(glm::length(glm::vec3(worldMatrix[0])),
			                     glm::length(glm::vec3(worldMatrix[1])),
			                     glm::length(glm::vec3(worldMatrix[2])));
			MeshComponent* mesh = object->GetMesh();
			if (mesh && mesh->renderer.mesh.hasAabb) {
				glm::vec3 localHalf = (mesh->renderer.mesh.aabbMax - mesh->renderer.mesh.aabbMin) * 0.5f;
				glm::vec3 localCenter = (mesh->renderer.mesh.aabbMax + mesh->renderer.mesh.aabbMin) * 0.5f;
				halfExtents = glm::max(localHalf * worldScale, glm::vec3(0.01f));
				center = glm::vec3(worldMatrix * glm::vec4(localCenter, 1.0f));
			}

			component->body = world.CreateBox(center, halfExtents,
				component->dynamic ? PhysicsWorld::Motion::Dynamic : PhysicsWorld::Motion::Static);
			if (component->body != PhysicsWorld::InvalidBody && component->dynamic)
				Bind(component->body, &object->transform);
			component->lastSyncedPosition = position;
			continue;
		}

		// Edit mode: the body follows its object, so what you place is what
		// simulates when Play starts. (While simulating, the flow reverses:
		// the binding sync below writes body poses into transforms.)
		if (!simulating) {
			glm::mat4 rotationOnly = glm::mat4(glm::mat3(
				glm::vec3(worldMatrix[0]) / glm::max(glm::length(glm::vec3(worldMatrix[0])), 1e-6f),
				glm::vec3(worldMatrix[1]) / glm::max(glm::length(glm::vec3(worldMatrix[1])), 1e-6f),
				glm::vec3(worldMatrix[2]) / glm::max(glm::length(glm::vec3(worldMatrix[2])), 1e-6f)));
			glm::quat rotation = glm::quat_cast(rotationOnly);
			if (position != component->lastSyncedPosition || rotation != component->lastSyncedRotation) {
				world.SetPose(component->body, position, rotation);
				component->lastSyncedPosition = position;
				component->lastSyncedRotation = rotation;
			}
		}
	}
}

void JoltPhysicsPlugin::OnUnload(Application& app)
{
	bindings.clear();
	world.Shutdown();
}

void JoltPhysicsPlugin::Bind(PhysicsWorld::BodyId id, Transform* transform)
{
	for (Binding& b : bindings) {
		if (b.transform == transform) {
			b.id = id;
			return;
		}
	}
	bindings.push_back({ id, transform });
}

void JoltPhysicsPlugin::UnbindTransform(Transform* transform)
{
	for (size_t i = 0; i < bindings.size(); ++i) {
		if (bindings[i].transform == transform) {
			bindings.erase(bindings.begin() + i);
			return;
		}
	}
}
