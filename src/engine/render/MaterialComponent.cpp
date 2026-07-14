// MaterialComponent: serialization + registry registration.

#include "engine/render/MaterialComponent.h"
#include "engine/scene/ComponentRegistry.h"
#include "engine/scene/Serialization.h"

#include <memory>

void MaterialComponent::Serialize(ISerializer& out) const
{
	out.WriteColor("baseColor", baseColor);
	out.Write("metallic", metallic);
	out.Write("roughness", roughness);
	out.WriteColor("emissive", emissive);
	out.Write("ao", ao);
}

void MaterialComponent::Deserialize(const IDeserializer& in)
{
	baseColor = in.ReadVec3("baseColor", baseColor);
	metallic = in.ReadFloat("metallic", metallic);
	roughness = in.ReadFloat("roughness", roughness);
	emissive = in.ReadVec3("emissive", emissive);
	ao = in.ReadFloat("ao", ao);
}

namespace {
const bool kMaterialRegistered = [] {
	ComponentRegistry::Register("Material", "Material",
		[] { return std::unique_ptr<Component>(new MaterialComponent()); });
	return true;
}();
}
