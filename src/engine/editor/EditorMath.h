// Editor math helpers: Euler extraction matching Transform's rotation order.

#pragma once

#include <glm/glm.hpp>

// Transform::CalcRotationMatrix builds R = Ry(rotation.x) * Rx(rotation.y) * Rz(rotation.z)
// (rotation.x is yaw about Y, rotation.y is pitch about X, rotation.z is roll about Z).
// This extracts a rotation vector in that convention from a rotation matrix
// whose columns may still carry scale (they are normalized first).
glm::vec3 EulerYXZFromMatrix(const glm::mat4& matrix);
