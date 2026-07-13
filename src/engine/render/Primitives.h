// Built-in primitive geometry: engine-owned CPU meshes for the lit
// primitive shader (positions + outward normals + per-vertex colors).
//
// Each accessor builds its mesh once (function-local static) and returns a
// shared reference; Mesh uploads the data to the GPU immediately, so sharing
// the CPU arrays between spawn factories is safe. Winding is not significant
// (the primitive pass leaves back-face culling off); only the outward
// normals matter for shading.

#pragma once

#include <vector>
#include <glm/glm.hpp>

struct PrimitiveMesh {
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> colors;
};

const PrimitiveMesh& PlaneMesh();
const PrimitiveMesh& CubeMesh();
const PrimitiveMesh& SphereMesh();
const PrimitiveMesh& CylinderMesh();
const PrimitiveMesh& ConeMesh();
const PrimitiveMesh& GroundMesh(); // checkerboard floor used by the default map
