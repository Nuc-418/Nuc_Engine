// Editor math helpers: Euler extraction matching Transform's rotation order.

#pragma once

#include <glm/glm.hpp>
#include "engine/core/EngineMath.h" // EulerYXZFromMatrix (moved to engine core)

// Slab test of a world-space ray against a mesh's local AABB transformed by
// modelMatrix. On hit returns true and the world-space distance to the entry
// point in outDistance.
bool RayIntersectsOBB(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                      const glm::vec3& aabbMin, const glm::vec3& aabbMax,
                      const glm::mat4& modelMatrix, float& outDistance);
