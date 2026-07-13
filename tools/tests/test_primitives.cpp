// Primitive mesh generators: shape invariants the renderer relies on.

#include "doctest/doctest.h"

#include <glm/glm.hpp>
#include "engine/render/Primitives.h"

static void CheckMesh(const PrimitiveMesh& mesh)
{
	CHECK(!mesh.positions.empty());
	CHECK(mesh.positions.size() % 3 == 0); // whole triangles
	CHECK(mesh.positions.size() == mesh.normals.size());
	CHECK(mesh.positions.size() == mesh.colors.size());
	for (const glm::vec3& n : mesh.normals)
		CHECK(glm::length(n) == doctest::Approx(1.0f).epsilon(0.01));
}

TEST_CASE("every primitive generator produces a consistent lit mesh")
{
	CheckMesh(PlaneMesh());
	CheckMesh(CubeMesh());
	CheckMesh(SphereMesh());
	CheckMesh(CylinderMesh());
	CheckMesh(ConeMesh());
	CheckMesh(GroundMesh());
}

TEST_CASE("generators return the same shared mesh on every call")
{
	CHECK(&CubeMesh() == &CubeMesh());
	CHECK(&GroundMesh() == &GroundMesh());
}
