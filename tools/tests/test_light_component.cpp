// Light/Camera components: serialization round-trips and registry creation.

#include "doctest/doctest.h"

#include <map>
#include <string>

#include "engine/render/CameraComponent.h"
#include "engine/render/LightComponent.h"
#include "engine/scene/ComponentRegistry.h"
#include "engine/scene/Serialization.h"
#include "engine/scene/FieldStore.h"

TEST_CASE("LightComponent serialization round-trips every field")
{
	LightComponent source;
	source.kind = LightComponent::Kind::Spot;
	source.on = false;
	source.ambient = glm::vec3(0.1f, 0.2f, 0.3f);
	source.diffuse = glm::vec3(0.9f, 0.8f, 0.7f);
	source.specular = glm::vec3(0.5f);
	source.constant = 0.8f;
	source.linear = 0.05f;
	source.quadratic = 0.01f;
	source.cutOff = 0.4f;

	FieldStore store;
	source.Serialize(store);

	LightComponent restored;
	restored.Deserialize(store);

	CHECK(restored.kind == source.kind);
	CHECK(restored.on == source.on);
	CHECK(restored.ambient == source.ambient);
	CHECK(restored.diffuse == source.diffuse);
	CHECK(restored.specular == source.specular);
	CHECK(restored.constant == doctest::Approx(source.constant));
	CHECK(restored.linear == doctest::Approx(source.linear));
	CHECK(restored.quadratic == doctest::Approx(source.quadratic));
	CHECK(restored.cutOff == doctest::Approx(source.cutOff));
}

TEST_CASE("deserializing an out-of-range kind keeps the current one")
{
	FieldStore store;
	store.ints["kind"] = 99;

	LightComponent light;
	light.kind = LightComponent::Kind::Directional;
	light.Deserialize(store);
	CHECK(light.kind == LightComponent::Kind::Directional);
}

TEST_CASE("the registry creates a Light component by id")
{
	CHECK(ComponentRegistry::IsRegistered("Light"));
	std::unique_ptr<Component> created = ComponentRegistry::Create("Light");
	REQUIRE(created != nullptr);
	CHECK(std::string(created->TypeId()) == "Light");
}

TEST_CASE("CameraComponent serialization round-trips the lens")
{
	CameraComponent source;
	source.fovDegrees = 75.0f;
	source.nearPlane = 0.5f;
	source.farPlane = 2500.0f;

	FieldStore store;
	source.Serialize(store);

	CameraComponent restored;
	restored.Deserialize(store);

	CHECK(restored.fovDegrees == doctest::Approx(source.fovDegrees));
	CHECK(restored.nearPlane == doctest::Approx(source.nearPlane));
	CHECK(restored.farPlane == doctest::Approx(source.farPlane));
}

TEST_CASE("the registry creates a Camera component by id")
{
	CHECK(ComponentRegistry::IsRegistered("Camera"));
	std::unique_ptr<Component> created = ComponentRegistry::Create("Camera");
	REQUIRE(created != nullptr);
	CHECK(std::string(created->TypeId()) == "Camera");
}

#include "engine/scene/GameObject.h"
#include "engine/scene/RotatorComponent.h"

TEST_CASE("RotatorComponent spins its owner only through OnSimulate")
{
	GameObject object;
	RotatorComponent rotator;
	rotator.owner = &object;
	rotator.radiansPerSecond = glm::vec3(1.0f, 0.0f, 0.0f);

	rotator.OnUpdate(1.0f); // per-frame hook: no motion
	CHECK(object.transform.rotation == glm::vec3(0.0f));

	rotator.OnSimulate(1.0f);
	// Transform::Rotate applies its (-1,1,1) offset to the delta.
	CHECK(object.transform.rotation.x == doctest::Approx(-1.0f));

	FieldStore store;
	rotator.Serialize(store);
	RotatorComponent restored;
	restored.Deserialize(store);
	CHECK(restored.radiansPerSecond == rotator.radiansPerSecond);
}
