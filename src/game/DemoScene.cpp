// DemoScene: the sample scene — a 10x10 cube grid, an indexed cube,
// two Iron Man models and four toggleable light sources.

#include "game/DemoScene.h"
#include "game/AssetPaths.h"
#include "engine/core/Time.h"
#include "engine/core/EngineMath.h"
#include "engine/io/ModelDiscovery.h"
#include "engine/render/Primitives.h"
#include "engine/render/LightComponent.h"
#include "engine/render/CameraComponent.h"
#include "JoltPhysics/PhysicsBodyComponent.h"

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
using namespace glm;

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

bool DemoScene::Load(Application& app)
{
	if (!LoadProgramShaders(app))
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

bool DemoScene::LoadProgramShaders(Application& app)
{
	cubeShader = app.assets.LoadShader(AssetPaths::CubeVertexShader, AssetPaths::CubeFragmentShader);
	ironManShader = app.assets.LoadShader(AssetPaths::IronManVertexShader, AssetPaths::IronManFragmentShader);
	primitiveShader = app.assets.LoadShader(AssetPaths::PrimitiveVertexShader, AssetPaths::PrimitiveFragmentShader);
	if (!cubeShader || !ironManShader || !primitiveShader)
		return false;

	cubeProgramShader = cubeShader->Program();
	ironManProgramShader = ironManShader->Program();
	primitiveProgramShader = primitiveShader->Program();
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
			object->EnsureMesh().CreatePosNormColor(primitiveProgram,
			                              const_cast<vector<glm::vec3>*>(&m.positions),
			                              const_cast<vector<glm::vec3>*>(&m.normals),
			                              const_cast<vector<glm::vec3>*>(&m.colors));
			return object;
		});
	}

	// Indexed cube: legacy demo geometry (indexed, unlit) on the cube shader.
	world.RegisterType("IndexedCube", "IndexedCube", [cubeProgram] {
		std::unique_ptr<GameObject> object(new GameObject());
		object->EnsureMesh().CreatePosColor(cubeProgram, &IndexedCubeVertices(), &IndexedCubeColors());
		object->GetMesh()->renderer.mesh.AssignElementArray(&IndexedCubeElements());
		return object;
	});

	// A meshless light actor: point light by default. Move it to place the
	// light; rotate it to aim directional/spot kinds (see LightComponent).
	world.RegisterType("Light", "Light", [] {
		std::unique_ptr<GameObject> object(new GameObject());
		object->AddComponent<LightComponent>();
		return object;
	});

	// A meshless scene camera: aim it via its transform, then Make Active in
	// the Details panel to render Play mode / game builds through it.
	world.RegisterType("Camera", "Camera", [] {
		std::unique_ptr<GameObject> object(new GameObject());
		object->AddComponent<CameraComponent>();
		return object;
	});

	// --- Dynamic model discovery -------------------------------------------
	// Every .obj under assets/models becomes a spawnable, draggable type. Drop
	// a model in and it shows up in the Content Browser automatically.
	std::vector<std::string> objFiles;
	DiscoverObjFiles("assets/models", objFiles);
	for (const std::string& objPath : objFiles) {
		size_t slash = objPath.find_last_of('/');
		std::string folder = objPath.substr(0, slash + 1);
		std::string file = objPath.substr(slash + 1);
		std::string label = file.substr(0, file.size() - 4); // drop ".obj"

		world.RegisterType(objPath, label, [modelProgram, folder, file] {
			std::unique_ptr<GameObject> object(new GameObject());
			object->EnsureMesh().LoadObj(modelProgram, folder, file);
			return object;
		});

		// Bind the model's texture (first image beside it) to the model shader.
		std::string texture = FindTextureInFolder(folder);
		if (!texture.empty())
			app.assets.LoadTexture(modelProgram, texture);
	}

	/* Null the demo animation handles if the editor destroys their objects. */
	world.onDestroyed = [this](GameObject* object) {
		if (object == ironMan1) ironMan1 = nullptr;
		if (object == ironMan2) ironMan2 = nullptr;
		if (object == indexedCube) indexedCube = nullptr;
		gridCubes.erase(std::remove(gridCubes.begin(), gridCubes.end(), object), gridCubes.end());

		/* Physics demo handles: the PhysicsBodyComponent releases its own
		   body (unbind + remove) when the object is destroyed. */
		if (object == physicsCube) physicsCube = nullptr;
		if (object == physicsFloor) physicsFloor = nullptr;
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

	/* Physics demo: a dynamic cube falling onto a static floor (JoltPhysics
	   plugin). Runs in the standalone game and in the editor's Play mode. */
	SetupPhysicsDemo(app);

	glViewport(0, 0, app.config.width, app.config.height);
	glEnable(GL_DEPTH_TEST);
	// Culling stays off: the built-in primitives are drawn double-sided so their
	// winding never matters, and closed shapes still sort correctly via depth.
	glDisable(GL_CULL_FACE);
}

void DemoScene::SetupPhysicsDemo(Application& app)
{
	// Register (or fetch) the physics plugin. GetOrAdd constructs it and brings
	// up its world; bodies are realized by the plugin's next update from each
	// object's PhysicsBodyComponent (box shape from the mesh AABB and scale).
	physics = &app.plugins.GetOrAdd<JoltPhysicsPlugin>();
	if (!physics->World().IsInitialized())
		return;

	// Static floor: a wide, thin cube whose top surface sits at y = 0.
	physicsFloor = world.Spawn("Cube", "PhysicsFloor");
	if (physicsFloor) {
		physicsFloor->transform.SetPos(glm::vec3(0.0f, -0.5f, 0.0f));
		physicsFloor->transform.scale = glm::vec3(50.0f, 1.0f, 50.0f);
		physicsFloor->AddComponent<PhysicsBodyComponent>()->dynamic = false;
	}

	// Dynamic cube dropped from above; it falls and rests on the floor at y = 0.5.
	physicsCube = world.Spawn("Cube", "PhysicsCube");
	if (physicsCube) {
		physicsCube->transform.SetPos(glm::vec3(0.0f, 8.0f, 0.0f));
		physicsCube->AddComponent<PhysicsBodyComponent>();
	}
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
	glProgramUniform1i(ironManProgramShader, ironManShader->Location("offsetToggle"), offsetToggle);

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

	/* Merge LightComponent lights into the upload (no-op unless changed). */
	world.SyncComponentLights();

	/* Feed the primitive shader the scene's directional + ambient light. */
	world.combinedLights.StorePrimitiveLight(primitiveProgramShader);

	/* While simulating (Play / game build) render through the scene's active
	   CameraComponent if one is set; otherwise — and always in Edit mode —
	   use the world camera, as before. */
	Camera* renderCamera = app.simulating ? world.ActiveCamera() : &world.camera;

	/* Render every world object (dispatches to each object's components) */
	for (WorldEntry& entry : world.entries)
		entry.object->Draw(world.renderMode, renderCamera);
}

void DemoScene::Unload(Application& app)
{
	world.Clear();

	// Shaders and textures are owned by app.assets, which Application::Run
	// frees after this unload returns.
	cubeShader = nullptr;
	ironManShader = nullptr;
	primitiveShader = nullptr;
	cubeProgramShader = 0;
	ironManProgramShader = 0;
	primitiveProgramShader = 0;
}
