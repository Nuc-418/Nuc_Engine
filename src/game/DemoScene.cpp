// DemoScene: the sample scene — a 10x10 cube grid, an indexed cube,
// two Iron Man models and four toggleable light sources.

#include "game/DemoScene.h"
#include "game/AssetPaths.h"
#include "engine/core/Time.h"
#include "engine/core/EngineMath.h"
#include "engine/editor/EditorFileSystem.h"
#include "LoadShaders/LoadShaders.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#ifdef NUC_GAME_BUILD
#include "engine/io/SceneSerializer.h"
#endif

using namespace std;

/* Cube geometry lives in statics so the World spawn factories can reuse it
   for editor spawns; Mesh uploads the data immediately, so sharing is safe. */
static vector<glm::vec3>& CubeVertexPositions()
{
	static vector<glm::vec3> vertexPos = {
	{0.5f, -0.5f,  0.5f},
	{0.5f, -0.5f, -0.5f},
	{0.5f,  0.5f,  0.5f},

	{0.5f,  0.5f,  0.5f},
	{0.5f, -0.5f, -0.5f},
	{0.5f,  0.5f, -0.5f},

	{-0.5f, -0.5f, -0.5f},
	{-0.5f, -0.5f,  0.5f},
	{-0.5f,  0.5f, -0.5f},

	{-0.5f,  0.5f, -0.5f},
	{-0.5f, -0.5f,  0.5f},
	{-0.5f,  0.5f,  0.5f},

	{-0.5f,  0.5f,  0.5f},
	{ 0.5f,  0.5f,  0.5f},
	{-0.5f,  0.5f, -0.5f},

	{-0.5f,  0.5f, -0.5f},
	{ 0.5f,  0.5f,  0.5f},
	{ 0.5f,  0.5f, -0.5f},

	{-0.5f, -0.5f, -0.5f},
	{ 0.5f, -0.5f, -0.5f},
	{-0.5f, -0.5f,  0.5f},

	{-0.5f, -0.5f,  0.5f},
	{ 0.5f, -0.5f, -0.5f},
	{ 0.5f, -0.5f,  0.5f},

	{-0.5f, -0.5f, 0.5f},
	{ 0.5f, -0.5f, 0.5f},
	{-0.5f,  0.5f, 0.5f},

	{-0.5f,  0.5f, 0.5f},
	{ 0.5f, -0.5f, 0.5f},
	{ 0.5f,  0.5f, 0.5f},

	{ 0.5f, -0.5f, -0.5f},
	{-0.5f, -0.5f, -0.5f},
	{ 0.5f,  0.5f, -0.5f},

	{ 0.5f,  0.5f, -0.5f},
	{-0.5f, -0.5f, -0.5f},
	{-0.5f,  0.5f, -0.5f}
	};
	return vertexPos;
}

static vector<glm::vec3>& CubeVertexColors()
{
	static vector<glm::vec3> vertexColor = [] {
		vector<glm::vec3> colors;
		srand(31);//6,11,31
		for (int i = 0; i < 6; i++) {
			float a = ((float)rand()) / (float)RAND_MAX;
			float b = ((float)rand()) / (float)RAND_MAX;
			float c = ((float)rand()) / (float)RAND_MAX;
			for (int j = 0; j < 6; j++)
				colors.push_back(glm::vec3(a, b, c));
		}
		return colors;
	}();
	return vertexColor;
}

static vector<glm::vec3>& IndexedCubeVertices()
{
	static vector<glm::vec3> cube_vertices = {
		// front
		{-1.0, -1.0,  1.0},
		{ 1.0, -1.0,  1.0},
		{ 1.0,  1.0,  1.0},
		{-1.0,  1.0,  1.0},
		// back
		{-1.0, -1.0, -1.0},
		{ 1.0, -1.0, -1.0},
		{ 1.0,  1.0, -1.0},
		{-1.0,  1.0, -1.0},
	};
	return cube_vertices;
}

static vector<glm::vec3>& IndexedCubeColors()
{
	static vector<glm::vec3> cube_colors = {
		// front colors
		{1.0, 0.0, 0.0},
		{0.0, 1.0, 0.0},
		{0.0, 0.0, 1.0},
		{1.0, 1.0, 1.0},
		// back colors
		{1.0, 0.0, 0.0},
		{0.0, 1.0, 0.0},
		{0.0, 0.0, 1.0},
		{1.0, 1.0, 1.0},
	};
	return cube_colors;
}

static vector<GLuint>& IndexedCubeElements()
{
	static vector<GLuint> cube_elements = {
		// front
		0, 1, 2,
		2, 3, 0,
		// top
		3, 2, 6,
		6, 7, 3,
		// back
		7, 6, 5,
		5, 4, 7,
		// bottom
		4, 5, 1,
		1, 0, 4,
		// left
		4, 0, 3,
		3, 7, 4,
		// right
		1, 5, 6,
		6, 2, 1,
	};
	return cube_elements;
}

/* Built-in primitive geometry. Each mesh carries positions, normals and
   per-vertex colours so the lit primitive shader can shade it. Back-face
   culling is left disabled (see LoadObjects), so winding is not significant;
   only the outward normals matter for shading. */
namespace {

struct PrimitiveMesh {
	vector<glm::vec3> positions;
	vector<glm::vec3> normals;
	vector<glm::vec3> colors;
};

const float kPi = 3.14159265358979323846f;

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

const glm::vec3 kGrey(0.80f);

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

bool EndsWithNoCase(const std::string& s, const char* suffix)
{
	size_t n = strlen(suffix);
	if (s.size() < n)
		return false;
	for (size_t i = 0; i < n; i++)
		if (std::tolower((unsigned char)s[s.size() - n + i]) != std::tolower((unsigned char)suffix[i]))
			return false;
	return true;
}

// Recursively collect every .obj file under `root` (paths use '/').
void DiscoverObjFiles(const std::string& root, std::vector<std::string>& out)
{
	for (const DirectoryEntry& entry : ListDirectory(root)) {
		std::string full = root + "/" + entry.name;
		if (entry.isDirectory)
			DiscoverObjFiles(full, out);
		else if (EndsWithNoCase(entry.name, ".obj"))
			out.push_back(full);
	}
}

// First image file in a model's folder (folder ends with '/'), or "".
std::string FindTextureInFolder(const std::string& folder)
{
	const char* exts[] = { ".tga", ".png", ".jpg", ".jpeg", ".bmp" };
	for (const DirectoryEntry& entry : ListDirectory(folder)) {
		if (entry.isDirectory)
			continue;
		for (const char* ext : exts)
			if (EndsWithNoCase(entry.name, ext))
				return folder + entry.name;
	}
	return "";
}

} // namespace

bool DemoScene::Load(Application& app)
{
	if (!LoadProgramShaders())
		return false;

	LoadObjects(app);

#ifdef NUC_GAME_BUILD
	/* Packaged game builds start from the scene the editor shipped. */
	FILE* startupScene = fopen(SceneSerializer::StartupScenePath, "r");
	if (startupScene) {
		fclose(startupScene);
		SceneSerializer::Load(world, SceneSerializer::StartupScenePath);
	}
#endif

	return true;
}

bool DemoScene::LoadProgramShaders()
{
	//Load cube shader
	ShaderInfo cubeShaders[] = {
		{ GL_VERTEX_SHADER,   AssetPaths::CubeVertexShader },
		{ GL_FRAGMENT_SHADER, AssetPaths::CubeFragmentShader },
		{ GL_NONE, NULL }
	};
	cubeProgramShader = LoadShaders(cubeShaders);
	if (!cubeProgramShader) return false;

	//Load Iron Man shader
	ShaderInfo ironManshaders[] = {
		{ GL_VERTEX_SHADER,   AssetPaths::IronManVertexShader },
		{ GL_FRAGMENT_SHADER, AssetPaths::IronManFragmentShader },
		{ GL_NONE, NULL }
	};
	ironManProgramShader = LoadShaders(ironManshaders);
	if (!ironManProgramShader) return false;

	offsetToggleLocation = glGetProgramResourceLocation(ironManProgramShader, GL_UNIFORM, "offsetToggle");

	//Load the lit primitive shader (ground, cube, sphere, ...)
	ShaderInfo primitiveShaders[] = {
		{ GL_VERTEX_SHADER,   AssetPaths::PrimitiveVertexShader },
		{ GL_FRAGMENT_SHADER, AssetPaths::PrimitiveFragmentShader },
		{ GL_NONE, NULL }
	};
	primitiveProgramShader = LoadShaders(primitiveShaders);
	if (!primitiveProgramShader) return false;

	return true;
}

void DemoScene::LoadObjects(Application& app)
{
	// Pitch-black void behind the scene. Opaque alpha: the scene renders into a
	// texture (the editor viewport) that ImGui composites with alpha blending,
	// so a 0 alpha would let the panel behind show through.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	/* Spawn factories: how each object type is created, for both this scene and
	   later editor spawns. Types are registered by string id, so adding a mesh
	   here (or dropping a model in assets/models) needs no enum or switch. */
	GLuint cubeProgram = cubeProgramShader;
	GLuint modelProgram = ironManProgramShader;
	GLuint primitiveProgram = primitiveProgramShader;

	// --- Built-in lit primitives -------------------------------------------
	// Add a mesh = add one row here plus its geometry generator above.
	struct PrimitiveDef { const char* id; const PrimitiveMesh& (*mesh)(); };
	static const PrimitiveDef primitives[] = {
		{ "Plane", &PlaneMesh }, { "Cube", &CubeMesh }, { "Sphere", &SphereMesh },
		{ "Cylinder", &CylinderMesh }, { "Cone", &ConeMesh }, { "Ground", &GroundMesh },
	};
	for (const PrimitiveDef& def : primitives) {
		const PrimitiveMesh& (*mesh)() = def.mesh;
		world.RegisterType(def.id, def.id, [primitiveProgram, mesh] {
			std::unique_ptr<GameObject> object(new GameObject());
			const PrimitiveMesh& m = mesh();
			object->CreateObjPosNormColor(primitiveProgram,
			                              const_cast<vector<glm::vec3>*>(&m.positions),
			                              const_cast<vector<glm::vec3>*>(&m.normals),
			                              const_cast<vector<glm::vec3>*>(&m.colors));
			return object;
		});
	}

	// Indexed cube: legacy demo geometry (indexed, unlit) on the cube shader.
	world.RegisterType("IndexedCube", "IndexedCube", [cubeProgram] {
		std::unique_ptr<GameObject> object(new GameObject());
		object->CreateObjPosColor(cubeProgram, &IndexedCubeVertices(), &IndexedCubeColors());
		object->meshRenderer.mesh.AssignElementArray(&IndexedCubeElements());
		return object;
	});

	// --- Dynamic model discovery -------------------------------------------
	// Every .obj under assets/models becomes a spawnable, draggable type. Drop
	// a model in and it shows up in the Content Browser automatically.
	std::vector<std::string> objFiles;
	DiscoverObjFiles("assets/models", objFiles);
	modelTextures.reserve(objFiles.size());
	for (const std::string& objPath : objFiles) {
		size_t slash = objPath.find_last_of('/');
		std::string folder = objPath.substr(0, slash + 1);
		std::string file = objPath.substr(slash + 1);
		std::string label = file.substr(0, file.size() - 4); // drop ".obj"

		world.RegisterType(objPath, label, [modelProgram, folder, file] {
			std::unique_ptr<GameObject> object(new GameObject());
			object->LoadObjFile(modelProgram, folder, file);
			return object;
		});

		// Bind the model's texture (first image beside it) to the model shader.
		std::string texture = FindTextureInFolder(folder);
		if (!texture.empty()) {
			modelTextures.emplace_back();
			modelTextures.back().TextureToProgram(modelProgram, texture);
		}
	}

	/* Null the demo animation handles if the editor destroys their objects. */
	world.onDestroyed = [this](GameObject* object) {
		if (object == ironMan1) ironMan1 = nullptr;
		if (object == ironMan2) ironMan2 = nullptr;
		if (object == indexedCube) indexedCube = nullptr;
		gridCubes.erase(std::remove(gridCubes.begin(), gridCubes.end(), object), gridCubes.end());
	};

	/* Indexed cube */
	indexedCube = world.Spawn("IndexedCube", "IndexedCube");
	indexedCube->transform.SetPos(vec3(3, 10, 0));
	indexedCube->transform.scale = vec3(0.5f, 0.5f, 0.5f);

	/* 10x10 cube grid */
	for (int h = 0; h < 10; h++)
		for (int i = 0; i < 10; i++)
		{
			GameObject* gridCube = world.Spawn("Cube");
			gridCube->transform.SetPos(vec3((i * 1.5f) - 4, -1, (h * 1.5f) - 6.0f));
			gridCubes.push_back(gridCube);
		}

	/* Iron Man objects (spawned through the dynamic model id) */
	std::string ironManId = std::string(AssetPaths::IronManFolder) + AssetPaths::IronManObjFile;
	if (world.CanSpawn(ironManId)) {
		ironMan1 = world.Spawn(ironManId, "IronMan");
		ironMan2 = world.Spawn(ironManId, "IronMan_2");
		if (ironMan2)
			ironMan2->transform.SetPos(vec3(6, 0, 0));
	}

	/* Light sources for the Iron Man shader program */
	world.lightsProgram = ironManProgramShader;
	world.lights.AddAmbientLight(ironManProgramShader, vec3(0.1f, 0.1f, 0.1f));
	world.lights.AddDirectionalLight(ironManProgramShader, vec3(1, 0, 0), vec3(0.2, 0.2, 0.2), vec3(0, 0.5, 1)*2.0f, vec3(0, 0.5, 1)*100.0f);
	world.lights.AddPointLight(ironManProgramShader, vec3(1.0, 2.0, 0.0), vec3(0.1, 0.1, 0.1), vec3(1.0, 0, 0.1)*2.0f, vec3(1.0, 0, 0.1)*10.0f, 1, 0.06f, 0.002f);
	world.lights.AddSpotLight(ironManProgramShader, vec3(0, 3, 2), vec3(0, 0, -2), vec3(0.1, 0.1, 0.1), vec3(1, 1, 1)*2.0f, vec3(1.0, 1.0, 1.0), 1, 0.006f, 0.002f, (SMALL_PI / 12));

	glViewport(0, 0, app.config.width, app.config.height);
	glEnable(GL_DEPTH_TEST);
	// Culling stays off: the built-in primitives are drawn double-sided so their
	// winding never matters, and closed shapes still sort correctly via depth.
	glDisable(GL_CULL_FACE);
}

void DemoScene::Update(Application& app)
{
	float deltaTime = Time::deltaTime;

	/* Basic camera movement */
	app.controller.BasicMovement(&world.camera.transform, 0.15f*deltaTime, 5 * deltaTime);

	/* Object rotations */
	if (ironMan1) ironMan1->transform.Rotate(vec3(1, 0, 0)*deltaTime);
	if (ironMan2) ironMan2->transform.Rotate(vec3(-1, 0, 0)*deltaTime);

	/* Toggle the model deformation */
	if (app.inputs.onceKey5)
	{
		if (indexedCube) {
			indexedCube->transform.Rotate(vec3(1, 1, 0)*2.0f*deltaTime);
			indexedCube->transform.scale = vec3(cos(Time::time*20)+1, cos(Time::time * 20)+1, cos(Time::time * 20)+1);
		}

		for (size_t i = 0; i < gridCubes.size(); i++)
		{
			gridCubes[i]->transform.Rotate(vec3(1 + (i*0.0005f), 0, 0)*(deltaTime));
			gridCubes[i]->transform.scale = vec3(1,1,1)*(cos(Time::time*2) + 3)*0.3f;
		}

		offsetToggle = 1;
	}
	else
		offsetToggle = 0;

	//offsetToggle is passed to the shader program as a uniform
	glProgramUniform1i(ironManProgramShader, offsetToggleLocation, offsetToggle);

	if (app.inputs.key6) world.renderMode = GL_TRIANGLES;
	if (app.inputs.key7) world.renderMode = GL_LINE_STRIP;
	if (app.inputs.key8) world.renderMode = GL_POINTS;
	if (app.inputs.key9) world.renderMode = GL_TRIANGLE_FAN;

	/* Keys 1-4 toggle the corresponding lights. Guarded: a map need not have
	   every light type (the default map has no point or spot light), and the
	   Toggle* helpers index their vector without bounds-checking. */
	VectorLight& lightInfo = world.lights.lightInfo;
	if (!lightInfo.ambientLight.empty())
		world.lights.ToggleAmbientLight(ironManProgramShader, app.inputs.onceKey1);
	if (!lightInfo.directionalLight.empty())
		world.lights.ToggleDirectionalLight(ironManProgramShader, app.inputs.onceKey2);
	if (!lightInfo.pointLight.empty())
		world.lights.TogglePointLight(ironManProgramShader, 0, app.inputs.onceKey3);
	if (!lightInfo.spotLight.empty())
		world.lights.ToggleSpotLight(ironManProgramShader, 0, app.inputs.onceKey4);

#ifdef NUC_GAME_BUILD
	/* Standalone game: ESC closes the window (in the editor, Esc stops Play). */
	app.controller.RequestExit(app.window.windowPtr);
#endif
}

void DemoScene::Draw(Application& app)
{
	//Clear the buffers before drawing. Set the colour every frame so the
	//pitch-black void is not affected by other passes (e.g. thumbnail rendering)
	//that may have changed the global clear colour.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Push the time values to the shader programs */
	Time::TimeToProgram(ironManProgramShader);
	Time::TimeToProgram(cubeProgramShader);

	/* Feed the primitive shader the scene's directional + ambient light. */
	world.lights.StorePrimitiveLight(primitiveProgramShader);

	/* Render every world object */
	for (WorldEntry& entry : world.entries)
		entry.object->meshRenderer.Draw(world.renderMode, &world.camera);
}

void DemoScene::Unload(Application& app)
{
	world.Clear();

	for (Texture& texture : modelTextures)
		texture.Unload();
	modelTextures.clear();

	glDeleteProgram(cubeProgramShader);
	glDeleteProgram(ironManProgramShader);
	glDeleteProgram(primitiveProgramShader);
	cubeProgramShader = 0;
	ironManProgramShader = 0;
	primitiveProgramShader = 0;
}
