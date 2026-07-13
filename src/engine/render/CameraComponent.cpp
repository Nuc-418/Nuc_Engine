// CameraComponent: serialization + registry registration.

#include "engine/render/CameraComponent.h"
#include "engine/scene/ComponentRegistry.h"
#include "engine/scene/Serialization.h"

#include <memory>

void CameraComponent::Serialize(ISerializer& out) const
{
	out.Write("fov", fovDegrees);
	out.Write("near", nearPlane);
	out.Write("far", farPlane);
}

void CameraComponent::Deserialize(const IDeserializer& in)
{
	fovDegrees = in.ReadFloat("fov", fovDegrees);
	nearPlane = in.ReadFloat("near", nearPlane);
	farPlane = in.ReadFloat("far", farPlane);
}

namespace {
const bool kCameraRegistered = [] {
	ComponentRegistry::Register("Camera", "Camera",
		[] { return std::unique_ptr<Component>(new CameraComponent()); });
	return true;
}();
}
