// JoltPhysicsPlugin: lifecycle + body-to-Transform synchronization.

#include "JoltPhysics/JoltPhysicsPlugin.h"

#include "engine/core/Application.h"
#include "engine/scene/Transform.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

JoltPhysicsPlugin::JoltPhysicsPlugin()
{
	// The world needs no GL context, so bring it up now — scenes create bodies
	// during Scene::Load, before the plugin's OnLoad runs.
	world.Init();
}

bool JoltPhysicsPlugin::OnLoad(Application& app)
{
	return world.IsInitialized();
}

void JoltPhysicsPlugin::OnUpdate(Application& app, float deltaTime)
{
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
