// PhysicsBodyComponent: registry wiring + body lifetime. The body itself is
// created by JoltPhysicsPlugin::OnUpdate (the component may be attached
// before the plugin exists, e.g. during scene load).

#include "JoltPhysics/PhysicsBodyComponent.h"
#include "JoltPhysics/JoltPhysicsPlugin.h"

#include "engine/scene/ComponentRegistry.h"
#include "engine/scene/GameObject.h"
#include "engine/scene/Serialization.h"

#include <algorithm>
#include <memory>

namespace {
std::vector<PhysicsBodyComponent*>& MutableInstances()
{
	static std::vector<PhysicsBodyComponent*> instances;
	return instances;
}
}

const std::vector<PhysicsBodyComponent*>& PhysicsBodyComponent::Instances()
{
	return MutableInstances();
}

void PhysicsBodyComponent::OnAttach()
{
	MutableInstances().push_back(this);
}

PhysicsBodyComponent::~PhysicsBodyComponent()
{
	ReleaseBody();
	std::vector<PhysicsBodyComponent*>& instances = MutableInstances();
	instances.erase(std::remove(instances.begin(), instances.end(), this), instances.end());
}

void PhysicsBodyComponent::OnUnload()
{
	ReleaseBody();
}

void PhysicsBodyComponent::ReleaseBody()
{
	if (body == PhysicsWorld::InvalidBody)
		return;
	if (JoltPhysicsPlugin* plugin = JoltPhysicsPlugin::Instance()) {
		if (owner)
			plugin->UnbindTransform(&owner->transform);
		plugin->World().RemoveBody(body);
	}
	body = PhysicsWorld::InvalidBody;
}

void PhysicsBodyComponent::Serialize(ISerializer& out) const
{
	out.Write("dynamic", dynamic);
}

void PhysicsBodyComponent::Deserialize(const IDeserializer& in)
{
	dynamic = in.ReadBool("dynamic", dynamic);
}

namespace {
const bool kPhysicsBodyRegistered = [] {
	ComponentRegistry::Register("PhysicsBody", "Physics Body",
		[] { return std::unique_ptr<Component>(new PhysicsBodyComponent()); });
	return true;
}();
}
