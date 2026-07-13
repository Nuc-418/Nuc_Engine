// CameraComponent: a scene camera attached to a GameObject.
//
// Pose comes from the owner's world transform (position + forward/up axes);
// the component holds only the lens (fov/near/far). One camera can be the
// world's ACTIVE camera (World::activeCameraId): while the simulation runs
// (Play in the editor, or a standalone game build), rendering goes through it
// via World::ActiveCamera(). With no active camera set, everything renders
// through World::camera exactly as before. The editor viewport always uses
// the editor camera in Edit mode.

#pragma once

#include "engine/scene/Component.h"

class CameraComponent : public Component
{
public:
	float fovDegrees = 60.0f;
	float nearPlane = 0.1f;
	float farPlane = 1000.0f;

	static const char* StaticTypeId() { return "Camera"; }
	const char* TypeId() const override { return StaticTypeId(); }
	const char* DisplayName() const override { return "Camera"; }

	void Serialize(ISerializer& out) const override;
	void Deserialize(const IDeserializer& in) override;
};
