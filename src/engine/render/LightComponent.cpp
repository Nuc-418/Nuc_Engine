// LightComponent: serialization + registry registration.

#include "engine/render/LightComponent.h"
#include "engine/scene/ComponentRegistry.h"
#include "engine/scene/Serialization.h"

#include <memory>

void LightComponent::Serialize(ISerializer& out) const
{
	out.Write("kind", (int)kind);
	out.Write("on", on);
	out.Write("ambient", ambient);
	out.Write("diffuse", diffuse);
	out.Write("specular", specular);
	out.Write("constant", constant);
	out.Write("linear", linear);
	out.Write("quadratic", quadratic);
	out.Write("cutOff", cutOff);
}

void LightComponent::Deserialize(const IDeserializer& in)
{
	int kindValue = in.ReadInt("kind", (int)kind);
	if (kindValue >= (int)Kind::Directional && kindValue <= (int)Kind::Spot)
		kind = (Kind)kindValue;
	on = in.ReadBool("on", on);
	ambient = in.ReadVec3("ambient", ambient);
	diffuse = in.ReadVec3("diffuse", diffuse);
	specular = in.ReadVec3("specular", specular);
	constant = in.ReadFloat("constant", constant);
	linear = in.ReadFloat("linear", linear);
	quadratic = in.ReadFloat("quadratic", quadratic);
	cutOff = in.ReadFloat("cutOff", cutOff);
}

namespace {
const bool kLightRegistered = [] {
	ComponentRegistry::Register("Light", "Light",
		[] { return std::unique_ptr<Component>(new LightComponent()); });
	return true;
}();
}
