// Math constants and matrix helpers shared across the engine.

#pragma once
#include <glm/glm.hpp>

#define SMALL_PI           3.14  /* pi */

// Transform::CalcRotationMatrix builds R = Ry(rotation.x) * Rx(rotation.y) * Rz(rotation.z)
// (rotation.x is yaw about Y, rotation.y is pitch about X, rotation.z is roll about Z).
// This extracts a rotation vector in that convention from a rotation matrix
// whose columns may still carry scale (they are normalized first).
glm::vec3 EulerYXZFromMatrix(const glm::mat4& matrix);
