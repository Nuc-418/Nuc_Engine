// Editor math helpers: Euler extraction matching Transform's rotation order.

#include "engine/editor/EditorMath.h"

#include <cmath>

glm::vec3 EulerYXZFromMatrix(const glm::mat4& matrix)
{
	// Strip scale by normalizing the rotation columns.
	glm::vec3 col0 = glm::normalize(glm::vec3(matrix[0]));
	glm::vec3 col1 = glm::normalize(glm::vec3(matrix[1]));
	glm::vec3 col2 = glm::normalize(glm::vec3(matrix[2]));

	// For R = Ry(a) * Rx(b) * Rz(c) (glm column-major, m[col][row]):
	//   m[2][1] = -sin(b)
	//   m[2][0] = sin(a)cos(b),  m[2][2] = cos(a)cos(b)
	//   m[0][1] = cos(b)sin(c),  m[1][1] = cos(b)cos(c)
	float b = std::asin(-col2.y);
	float a = std::atan2(col2.x, col2.z);
	float c = std::atan2(col0.y, col1.y);

	return glm::vec3(a, b, c);
}
