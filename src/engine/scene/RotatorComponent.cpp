#include "engine/scene/RotatorComponent.h"
#include "engine/scene/ComponentRegistry.h"
#include "engine/scene/GameObject.h"
#include "engine/scene/Serialization.h"

#include <memory>

void RotatorComponent::OnSimulate(float deltaTime)
{
	owner->transform.Rotate(radiansPerSecond * deltaTime);
}

void RotatorComponent::Serialize(ISerializer& out) const
{
	out.Write("speed", radiansPerSecond);
}

void RotatorComponent::Deserialize(const IDeserializer& in)
{
	radiansPerSecond = in.ReadVec3("speed", radiansPerSecond);
}

namespace {
const bool kRotatorRegistered = [] {
	ComponentRegistry::Register("Rotator", "Rotator",
		[] { return std::unique_ptr<Component>(new RotatorComponent()); });
	return true;
}();
}
