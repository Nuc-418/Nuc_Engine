// Editor math helpers: Euler extraction matching Transform's rotation order.

#include "engine/editor/EditorMath.h"

#include <cmath>

bool RayIntersectsOBB(const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
                      const glm::vec3& aabbMin, const glm::vec3& aabbMax,
                      const glm::mat4& modelMatrix, float& outDistance)
{
	// Work in the box's local space (handles rotation and non-uniform scale).
	glm::mat4 inverseModel = glm::inverse(modelMatrix);
	glm::vec3 localOrigin = glm::vec3(inverseModel * glm::vec4(rayOrigin, 1.0f));
	glm::vec3 localDirection = glm::vec3(inverseModel * glm::vec4(rayDirection, 0.0f));

	float tNear = -1e30f;
	float tFar = 1e30f;
	const float epsilon = 1e-8f;

	for (int axis = 0; axis < 3; axis++) {
		if (std::fabs(localDirection[axis]) < epsilon) {
			// Ray parallel to the slab: must already be inside it.
			if (localOrigin[axis] < aabbMin[axis] || localOrigin[axis] > aabbMax[axis])
				return false;
			continue;
		}
		float t1 = (aabbMin[axis] - localOrigin[axis]) / localDirection[axis];
		float t2 = (aabbMax[axis] - localOrigin[axis]) / localDirection[axis];
		if (t1 > t2) { float swap = t1; t1 = t2; t2 = swap; }
		if (t1 > tNear) tNear = t1;
		if (t2 < tFar) tFar = t2;
		if (tNear > tFar || tFar < 0.0f)
			return false;
	}

	// Convert the local-space hit back to a world-space distance.
	float tHit = (tNear >= 0.0f) ? tNear : tFar;
	glm::vec3 localHit = localOrigin + localDirection * tHit;
	glm::vec3 worldHit = glm::vec3(modelMatrix * glm::vec4(localHit, 1.0f));
	outDistance = glm::length(worldHit - rayOrigin);
	return true;
}
