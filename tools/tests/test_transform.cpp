// Transform math: model composition, local axes, rotation wrap.
//
// These pin the behavior the Phase 1 scene-graph rework must preserve
// (see docs/ROADMAP.md).

#include "doctest/doctest.h"

#include <glm/glm.hpp>
#include "engine/scene/Transform.h"

namespace
{
	bool VecNear(glm::vec3 a, glm::vec3 b, float eps = 1e-5f)
	{
		return glm::all(glm::lessThan(glm::abs(a - b), glm::vec3(eps)));
	}
}

TEST_CASE("default transform yields the identity model")
{
	Transform t;
	t.UpdateModel();

	const glm::mat4 identity(1.0f);
	for (int col = 0; col < 4; ++col)
		for (int row = 0; row < 4; ++row)
			CHECK(t.model[col][row] == doctest::Approx(identity[col][row]));
}

TEST_CASE("initial axes match what CalcLocalAxis produces at identity")
{
	Transform computed;
	computed.UpdateModel(); // runs CalcLocalAxis with identity rotation

	Transform fresh; // never updated: in-class initializers only
	CHECK(VecNear(fresh.forward, computed.forward));
	CHECK(VecNear(fresh.up, computed.up));
	CHECK(VecNear(fresh.right, computed.right));
}

TEST_CASE("model composes translate * rotate * scale")
{
	Transform t;
	t.position = glm::vec3(1.0f, 2.0f, 3.0f);
	t.scale = glm::vec3(2.0f);
	t.UpdateModel();

	// Origin maps to the position; a unit X offset is scaled by 2.
	glm::vec3 origin = glm::vec3(t.model * glm::vec4(0, 0, 0, 1));
	glm::vec3 unitX = glm::vec3(t.model * glm::vec4(1, 0, 0, 1));
	CHECK(VecNear(origin, glm::vec3(1.0f, 2.0f, 3.0f)));
	CHECK(VecNear(unitX - origin, glm::vec3(2.0f, 0.0f, 0.0f)));
}

TEST_CASE("yaw rotates the forward axis")
{
	Transform t;
	t.rotation = glm::vec3(glm::radians(90.0f), 0.0f, 0.0f); // rotation.x = yaw (about +Y)
	t.UpdateModel();

	// defaultForward (0,0,1) yawed +90deg about +Y lands on +X.
	CHECK(VecNear(t.forward, glm::vec3(1.0f, 0.0f, 0.0f), 1e-4f));
	CHECK(VecNear(t.up, glm::vec3(0.0f, 1.0f, 0.0f), 1e-4f));
}

TEST_CASE("Translate accumulates, SetPos replaces")
{
	Transform t;
	t.Translate(glm::vec3(1.0f, 0.0f, 0.0f));
	t.Translate(glm::vec3(0.0f, 2.0f, 0.0f));
	CHECK(VecNear(t.position, glm::vec3(1.0f, 2.0f, 0.0f)));

	t.SetPos(glm::vec3(5.0f));
	CHECK(VecNear(t.position, glm::vec3(5.0f)));
}

namespace
{
	bool MatNear(const glm::mat4& a, const glm::mat4& b, float eps = 1e-4f)
	{
		for (int col = 0; col < 4; ++col)
			for (int row = 0; row < 4; ++row)
				if (std::abs(a[col][row] - b[col][row]) > eps)
					return false;
		return true;
	}
}

TEST_CASE("SetFromMatrix round-trips a TRS transform")
{
	Transform source;
	source.position = glm::vec3(3.0f, -1.0f, 7.5f);
	source.rotation = glm::vec3(0.6f, 0.3f, -0.4f); // yaw/pitch/roll, within decompose range
	source.scale = glm::vec3(2.0f, 1.0f, 0.5f);
	source.UpdateModel();

	Transform restored;
	restored.SetFromMatrix(source.model);

	CHECK(MatNear(restored.model, source.model));
	CHECK(VecNear(restored.position, source.position, 1e-4f));
	CHECK(VecNear(restored.scale, source.scale, 1e-4f));
}

TEST_CASE("reparent math preserves the world pose")
{
	// World = parentWorld * local. Rebasing a world pose into a parent's space
	// (local = inverse(parentWorld) * world) must recompose to the same world
	// matrix — this is GameObject::SetParent(keepWorldTransform=true).
	Transform parent;
	parent.position = glm::vec3(5.0f, 2.0f, -3.0f);
	parent.rotation = glm::vec3(0.8f, 0.0f, 0.0f);
	parent.scale = glm::vec3(2.0f);
	parent.UpdateModel();

	Transform object;
	object.position = glm::vec3(1.0f, 0.0f, 4.0f);
	object.rotation = glm::vec3(0.2f, 0.4f, 0.0f);
	object.UpdateModel();
	glm::mat4 worldBefore = object.model;

	Transform local;
	local.SetFromMatrix(glm::inverse(parent.model) * worldBefore);
	glm::mat4 worldAfter = parent.model * local.model;

	CHECK(MatNear(worldAfter, worldBefore));
}

TEST_CASE("CalcRotationMatrix wraps angles past a full turn to zero")
{
	Transform t;
	t.rotation = glm::vec3(2.0f * SMALL_PI + 0.1f, 0.0f, 0.0f);
	t.CalcRotationMatrix();
	CHECK(t.rotation.x == doctest::Approx(0.0f));
}
