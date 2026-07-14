// DemoScene: the sample scene — a 10x10 cube grid, an indexed cube,
// two Iron Man models and four toggleable light sources.

#include "game/DemoScene.h"
#include "game/AssetPaths.h"
#include "engine/core/Time.h"
#include "engine/core/EngineMath.h"
#include "engine/io/ModelDiscovery.h"
#include "engine/io/PrefabLibrary.h"
#include "engine/render/Primitives.h"
#include "engine/render/LightComponent.h"
#include "engine/render/CameraComponent.h"
#include "engine/scene/RotatorComponent.h"
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

	/* HDR scene target + tonemap pass (PBR renders linear HDR). */
	hdrFramebuffer.hdr = true;
	postProcess.Init();

	/* Image-based lighting from a procedural sky: ambient diffuse + specular for
	   the PBR shader. On failure the shader falls back to flat ambient. Units
	   1/2/3 = irradiance/prefilter/BRDF (unit 0 stays the model albedo). */
	if (ibl.Build()) {
		ibl.ConfigureProgram(ironManProgramShader, 1, 2, 3);
		ibl.ConfigureProgram(primitiveProgramShader, 1, 2, 3);
	}

	/* Demo bindings: light toggles, deformation, render-mode keys. */
	app.actions.BindToggle("ToggleAmbientLight", GLFW_KEY_1);
	app.actions.BindToggle("ToggleDirectionalLight", GLFW_KEY_2);
	app.actions.BindToggle("TogglePointLight", GLFW_KEY_3);
	app.actions.BindToggle("ToggleSpotLight", GLFW_KEY_4);
	app.actions.BindToggle("ToggleDeform", GLFW_KEY_5);
	app.actions.BindAction("RenderTriangles", GLFW_KEY_6);
	app.actions.BindAction("RenderLines", GLFW_KEY_7);
	app.actions.BindAction("RenderPoints", GLFW_KEY_8);
	app.actions.BindAction("RenderFan", GLFW_KEY_9);

	LoadObjects(app);

#ifdef NUC_GAME_BUILD
	/* Packaged game builds start from the scene the editor shipped. */
	FILE* startupScene = fopen(SceneSerializer::StartupScenePath, "r");
	if (startupScene) {
		fclose(startupScene);
		SceneSerializer::Load(world, SceneSerializer::StartupScenePath);
	}
	/* Standalone: the simulation starts as soon as the scene is up. */
	world.NotifyPlayBegin();
#endif

	return true;
}

bool DemoScene::LoadProgramShaders(Application& app)
{
	// Primitives and models are lit by the shared PBR shader (metallic/roughness,
	// Cook-Torrance). The two programs differ only in their vertex stage. The
	// cube/grid shader stays unlit (legacy debug geometry).
	cubeShader = app.assets.LoadShader(AssetPaths::CubeVertexShader, AssetPaths::CubeFragmentShader);
	ironManShader = app.assets.LoadShader(AssetPaths::PbrModelVertexShader, AssetPaths::PbrFragmentShader);
	primitiveShader = app.assets.LoadShader(AssetPaths::PbrPrimitiveVertexShader, AssetPaths::PbrFragmentShader);
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

	/* Prefabs saved from the editor become spawnable types too (their base
	   types are all registered above). */
	Prefabs::RegisterAll(world);

	/* Null the demo animation handles if the editor destroys their objects. */
	world.onDestroyed = [this](GameObject* object) {
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

	/* Rotator behavior demo: the indexed cube spins while simulating (Play /
	   game build). It is a component, so it serializes and is editable in
	   Details like any other. Any model dropped in assets/models is discovered
	   and spawnable from the Content Browser — nothing is hardcoded here. */
	if (indexedCube)
		indexedCube->AddComponent<RotatorComponent>()->radiansPerSecond = vec3(0, 1, 0);

	/* Lighting is component-driven. A global ambient term is the world's
	   environment light (non-positional, edited in the Environment panel);
	   every directional/point/spot light is a Light component on an actor, so
	   it can be selected, moved and deleted like anything else. Both lit
	   programs (models + primitives) receive the combined set. */
	world.AddLitProgram(ironManProgramShader);
	world.AddLitProgram(primitiveProgramShader);
	world.lightsProgram = ironManProgramShader; // Environment panel target

	world.lights.lightInfo.ambientLight.clear();
	AmbientLight ambient;
	ambient.switchL = true;
	ambient.ambient = vec3(0.12f);
	world.lights.lightInfo.ambientLight.push_back(ambient);

	if (GameObject* sun = world.Spawn("Light", "Sun")) {
		LightComponent* light = sun->GetComponent<LightComponent>();
		light->kind = LightComponent::Kind::Directional;
		light->ambient = vec3(0.05f);
		light->diffuse = vec3(1.0f);
		light->specular = vec3(1.0f);
		sun->transform.SetPos(vec3(0.0f, 12.0f, -4.0f));
		sun->transform.rotation = vec3(0.5f, 0.9f, 0.0f); // aim the +Z light axis DOWN onto the ground
	}

	if (GameObject* fill = world.Spawn("Light", "FillLight")) {
		LightComponent* light = fill->GetComponent<LightComponent>();
		light->kind = LightComponent::Kind::Point;
		light->diffuse = vec3(0.8f, 0.85f, 1.0f);
		light->specular = vec3(0.5f);
		light->linear = 0.06f;
		light->quadratic = 0.002f;
		fill->transform.SetPos(vec3(0.0f, 6.0f, 0.0f));
	}

	world.UploadLights();

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

	/* Component dispatch: OnUpdate + behaviors (OnSimulate). Under the
	   editor this Update only runs in Play mode; standalone it runs always
	   with app.simulating true. */
	world.Tick(deltaTime, app.simulating, &app.actions);

	/* Basic camera movement */
	app.controller.BasicMovement(&world.camera.transform, 0.15f*deltaTime, 5 * deltaTime);

	/* Toggle the model deformation */
	if (app.actions.Toggle("ToggleDeform"))
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

	if (app.actions.IsDown("RenderTriangles")) world.renderMode = GL_TRIANGLES;
	if (app.actions.IsDown("RenderLines")) world.renderMode = GL_LINE_STRIP;
	if (app.actions.IsDown("RenderPoints")) world.renderMode = GL_POINTS;
	if (app.actions.IsDown("RenderFan")) world.renderMode = GL_TRIANGLE_FAN;

	/* Keys 1-4 toggle the corresponding lights. Guarded: a map need not have
	   every light type (the default map has no point or spot light), and the
	   Toggle* helpers index their vector without bounds-checking. */
	VectorLight& lightInfo = world.lights.lightInfo;
	if (!lightInfo.ambientLight.empty())
		world.lights.ToggleAmbientLight(ironManProgramShader, app.actions.Toggle("ToggleAmbientLight"));
	if (!lightInfo.directionalLight.empty())
		world.lights.ToggleDirectionalLight(ironManProgramShader, app.actions.Toggle("ToggleDirectionalLight"));
	if (!lightInfo.pointLight.empty())
		world.lights.TogglePointLight(ironManProgramShader, 0, app.actions.Toggle("TogglePointLight"));
	if (!lightInfo.spotLight.empty())
		world.lights.ToggleSpotLight(ironManProgramShader, 0, app.actions.Toggle("ToggleSpotLight"));

#ifdef NUC_GAME_BUILD
	/* Standalone game: ESC closes the window (in the editor, Esc stops Play). */
	app.controller.RequestExit(app.window.windowPtr);
#endif
}

void DemoScene::Draw(Application& app)
{
	// The scene renders linear HDR into hdrFramebuffer, then tonemaps into the
	// target that was bound on entry (the editor's LDR panel FBO, or the game's
	// backbuffer) at the same size — so this works for both without either caller
	// knowing about HDR.
	GLint prevFbo = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevFbo);
	GLint viewport[4] = { 0, 0, 1, 1 };
	glGetIntegerv(GL_VIEWPORT, viewport);
	int w = viewport[2] > 0 ? viewport[2] : 1;
	int h = viewport[3] > 0 ? viewport[3] : 1;

	hdrFramebuffer.Resize(w, h);
	hdrFramebuffer.Bind();
	glViewport(0, 0, w, h);

	// Pitch-black void behind the scene. Opaque alpha (the editor composites the
	// tonemapped result with alpha blending).
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	/* Push the time values to the shader programs */
	Time::TimeToProgram(ironManProgramShader);
	Time::TimeToProgram(cubeProgramShader);

	/* Merge LightComponent lights into the upload (no-op unless changed). This
	   pushes the combined light set to every lit program — models and
	   primitives alike. */
	world.SyncComponentLights();

	/* Bind IBL for this frame and re-assert uHasIBL on the PBR programs (the
	   thumbnail pass disables IBL on the shared program). */
	if (ibl.Ready()) {
		ibl.ConfigureProgram(ironManProgramShader, 1, 2, 3);
		ibl.ConfigureProgram(primitiveProgramShader, 1, 2, 3);
		ibl.BindForFrame(1, 2, 3);
	}

	/* While simulating (Play / game build) render through the scene's active
	   CameraComponent if one is set; otherwise — and always in Edit mode —
	   use the world camera, as before. */
	Camera* renderCamera = app.simulating ? world.ActiveCamera() : &world.camera;

	/* Render every world object (dispatches to each object's components) */
	for (WorldEntry& entry : world.entries)
		entry.object->Draw(world.renderMode, renderCamera);

	/* Tonemap the HDR scene back into the original target. */
	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)prevFbo);
	glViewport(0, 0, w, h);
	postProcess.Tonemap(hdrFramebuffer.colorTexture, 1.0f);
}

void DemoScene::Unload(Application& app)
{
	world.Clear();

	hdrFramebuffer.Unload();
	postProcess.Unload();
	ibl.Unload();

	// Shaders and textures are owned by app.assets, which Application::Run
	// frees after this unload returns.
	cubeShader = nullptr;
	ironManShader = nullptr;
	primitiveShader = nullptr;
	cubeProgramShader = 0;
	ironManProgramShader = 0;
	primitiveProgramShader = 0;
}
