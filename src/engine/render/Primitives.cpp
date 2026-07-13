// Built-in primitive geometry generators (moved out of the demo scene so
// primitives are an engine facility, not sample code).

#include "engine/render/Primitives.h"

#include <cmath>

namespace {

const float kPi = 3.14159265358979323846f;
const glm::vec3 kGrey(0.80f);

void PushTri(PrimitiveMesh& m,
             glm::vec3 a, glm::vec3 na, glm::vec3 b, glm::vec3 nb, glm::vec3 c, glm::vec3 nc,
             glm::vec3 col)
{
	m.positions.push_back(a); m.normals.push_back(na); m.colors.push_back(col);
	m.positions.push_back(b); m.normals.push_back(nb); m.colors.push_back(col);
	m.positions.push_back(c); m.normals.push_back(nc); m.colors.push_back(col);
}

void PushFlatQuad(PrimitiveMesh& m, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d,
                  glm::vec3 n, glm::vec3 col)
{
	PushTri(m, a, n, b, n, c, n, col);
	PushTri(m, a, n, c, n, d, n, col);
}

} // namespace

const PrimitiveMesh& CubeMesh()
{
	static PrimitiveMesh m = [] {
		PrimitiveMesh mesh;
		const float h = 0.5f;
		PushFlatQuad(mesh, { h,-h,-h }, { h,-h, h }, { h, h, h }, { h, h,-h }, { 1, 0, 0 }, kGrey); // +X
		PushFlatQuad(mesh, { -h,-h, h }, { -h,-h,-h }, { -h, h,-h }, { -h, h, h }, { -1, 0, 0 }, kGrey); // -X
		PushFlatQuad(mesh, { -h, h,-h }, { h, h,-h }, { h, h, h }, { -h, h, h }, { 0, 1, 0 }, kGrey); // +Y
		PushFlatQuad(mesh, { -h,-h, h }, { h,-h, h }, { h,-h,-h }, { -h,-h,-h }, { 0,-1, 0 }, kGrey); // -Y
		PushFlatQuad(mesh, { -h,-h, h }, { -h, h, h }, { h, h, h }, { h,-h, h }, { 0, 0, 1 }, kGrey); // +Z
		PushFlatQuad(mesh, { h,-h,-h }, { h, h,-h }, { -h, h,-h }, { -h,-h,-h }, { 0, 0,-1 }, kGrey); // -Z
		return mesh;
	}();
	return m;
}

const PrimitiveMesh& PlaneMesh()
{
	static PrimitiveMesh m = [] {
		PrimitiveMesh mesh;
		const float h = 0.5f;
		PushFlatQuad(mesh, { -h, 0,-h }, { h, 0,-h }, { h, 0, h }, { -h, 0, h }, { 0, 1, 0 }, kGrey);
		return mesh;
	}();
	return m;
}

const PrimitiveMesh& SphereMesh()
{
	static PrimitiveMesh m = [] {
		PrimitiveMesh mesh;
		const int stacks = 16, sectors = 24;
		const float R = 0.5f;
		auto dir = [](float phi, float th) {
			return glm::vec3(sinf(phi) * cosf(th), cosf(phi), sinf(phi) * sinf(th));
		};
		for (int i = 0; i < stacks; i++) {
			float p1 = kPi * i / stacks, p2 = kPi * (i + 1) / stacks;
			for (int j = 0; j < sectors; j++) {
				float t1 = 2 * kPi * j / sectors, t2 = 2 * kPi * (j + 1) / sectors;
				glm::vec3 n00 = dir(p1, t1), n01 = dir(p1, t2), n10 = dir(p2, t1), n11 = dir(p2, t2);
				PushTri(mesh, R * n00, n00, R * n10, n10, R * n11, n11, kGrey);
				PushTri(mesh, R * n00, n00, R * n11, n11, R * n01, n01, kGrey);
			}
		}
		return mesh;
	}();
	return m;
}

const PrimitiveMesh& CylinderMesh()
{
	static PrimitiveMesh m = [] {
		PrimitiveMesh mesh;
		const int sectors = 24;
		const float R = 0.4f, hy = 0.5f;
		glm::vec3 topC(0, hy, 0), botC(0, -hy, 0), up(0, 1, 0), dn(0, -1, 0);
		for (int j = 0; j < sectors; j++) {
			float t1 = 2 * kPi * j / sectors, t2 = 2 * kPi * (j + 1) / sectors;
			glm::vec3 d1(cosf(t1), 0, sinf(t1)), d2(cosf(t2), 0, sinf(t2));
			glm::vec3 b1 = R * d1 + botC, b2 = R * d2 + botC, t1p = R * d1 + topC, t2p = R * d2 + topC;
			PushTri(mesh, b1, d1, b2, d2, t2p, d2, kGrey);
			PushTri(mesh, b1, d1, t2p, d2, t1p, d1, kGrey);
			PushTri(mesh, topC, up, t1p, up, t2p, up, kGrey);
			PushTri(mesh, botC, dn, b2, dn, b1, dn, kGrey);
		}
		return mesh;
	}();
	return m;
}

const PrimitiveMesh& ConeMesh()
{
	static PrimitiveMesh m = [] {
		PrimitiveMesh mesh;
		const int sectors = 24;
		const float R = 0.5f, baseY = -0.5f, apexY = 0.5f, H = apexY - baseY;
		glm::vec3 apex(0, apexY, 0), botC(0, baseY, 0), dn(0, -1, 0);
		for (int j = 0; j < sectors; j++) {
			float t1 = 2 * kPi * j / sectors, t2 = 2 * kPi * (j + 1) / sectors;
			glm::vec3 d1(cosf(t1), 0, sinf(t1)), d2(cosf(t2), 0, sinf(t2));
			glm::vec3 b1 = R * d1 + botC, b2 = R * d2 + botC;
			glm::vec3 n1 = glm::normalize(glm::vec3(d1.x * H, R, d1.z * H));
			glm::vec3 n2 = glm::normalize(glm::vec3(d2.x * H, R, d2.z * H));
			glm::vec3 nApex = glm::normalize(n1 + n2);
			PushTri(mesh, apex, nApex, b1, n1, b2, n2, kGrey);
			PushTri(mesh, botC, dn, b2, dn, b1, dn, kGrey);
		}
		return mesh;
	}();
	return m;
}

const PrimitiveMesh& GroundMesh()
{
	static PrimitiveMesh m = [] {
		PrimitiveMesh mesh;
		const int cells = 24;
		const float cell = 2.0f, half = cells * cell * 0.5f;
		glm::vec3 up(0, 1, 0);
		for (int i = 0; i < cells; i++)
			for (int j = 0; j < cells; j++) {
				float x0 = -half + i * cell, x1 = x0 + cell;
				float z0 = -half + j * cell, z1 = z0 + cell;
				glm::vec3 shade = ((i + j) & 1) ? glm::vec3(0.30f) : glm::vec3(0.38f);
				PushFlatQuad(mesh, { x0,0,z0 }, { x1,0,z0 }, { x1,0,z1 }, { x0,0,z1 }, up, shade);
			}
		return mesh;
	}();
	return m;
}
