#include "engine/scene/RotatorComponent.h"
#include "engine/scene/BehaviorContext.h"
#include "engine/scene/ComponentRegistry.h"
#include "engine/scene/GameObject.h"
#include "engine/scene/Serialization.h"
#include "engine/input/InputActions.h"

#include <memory>

void RotatorComponent::OnSimulate(float deltaTime, const BehaviorContext& ctx)
{
	// Gate on an input action when one is named and input is available.
	if (!activeWhile.empty()) {
		if (!ctx.input || !ctx.input->IsDown(activeWhile))
			return;
	}
	owner->transform.Rotate(radiansPerSecond * deltaTime);
}

void RotatorComponent::Serialize(ISerializer& out) const
{
	out.Write("speed", radiansPerSecond);
	out.Write("activeWhile", activeWhile);
}

void RotatorComponent::Deserialize(const IDeserializer& in)
{
	radiansPerSecond = in.ReadVec3("speed", radiansPerSecond);
	activeWhile = in.ReadString("activeWhile", activeWhile);
}

namespace {
const bool kRotatorRegistered = [] {
	ComponentRegistry::Register("Rotator", "Rotator",
		[] { return std::unique_ptr<Component>(new RotatorComponent()); });
	return true;
}();
}
