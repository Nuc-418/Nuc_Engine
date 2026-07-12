// DemoScene: the sample scene — a 10x10 cube grid, an indexed cube,
// two Iron Man models and four toggleable light sources.

#include "game/DemoScene.h"
#include "game/AssetPaths.h"
#include "engine/core/Time.h"
#include "engine/core/EngineMath.h"
#include "LoadShaders/LoadShaders.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <memory>
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

	return true;
}

void DemoScene::LoadObjects(Application& app)
{
	glClearColor(0.11f, 0.11f, 0.11f, 0.0f);

	/* Spawn factories: how each object type is created, for both this scene
	   and later editor spawns. */
	GLuint cubeProgram = cubeProgramShader;
	GLuint ironManProgram = ironManProgramShader;

	world.RegisterType(ObjectType::Cube, [cubeProgram] {
		std::unique_ptr<GameObject> object(new GameObject());
		object->CreateObjPosColor(cubeProgram, &CubeVertexPositions(), &CubeVertexColors());
		return object;
	});
	world.RegisterType(ObjectType::IndexedCube, [cubeProgram] {
		std::unique_ptr<GameObject> object(new GameObject());
		object->CreateObjPosColor(cubeProgram, &IndexedCubeVertices(), &IndexedCubeColors());
		object->meshRenderer.mesh.AssignElementArray(&IndexedCubeElements());
		return object;
	});
	world.RegisterType(ObjectType::IronMan, [ironManProgram] {
		std::unique_ptr<GameObject> object(new GameObject());
		object->LoadObjFile(ironManProgram, AssetPaths::IronManFolder, AssetPaths::IronManObjFile);
		return object;
	});

	/* Null the demo animation handles if the editor destroys their objects. */
	world.onDestroyed = [this](GameObject* object) {
		if (object == ironMan1) ironMan1 = nullptr;
		if (object == ironMan2) ironMan2 = nullptr;
		if (object == indexedCube) indexedCube = nullptr;
		gridCubes.erase(std::remove(gridCubes.begin(), gridCubes.end(), object), gridCubes.end());
	};

	/* Indexed cube */
	indexedCube = world.Spawn(ObjectType::IndexedCube, "IndexedCube");
	indexedCube->transform.SetPos(vec3(3, 10, 0));
	indexedCube->transform.scale = vec3(0.5f, 0.5f, 0.5f);

	/* 10x10 cube grid */
	for (int h = 0; h < 10; h++)
		for (int i = 0; i < 10; i++)
		{
			GameObject* gridCube = world.Spawn(ObjectType::Cube);
			gridCube->transform.SetPos(vec3((i * 1.5f) - 4, -1, (h * 1.5f) - 6.0f));
			gridCubes.push_back(gridCube);
		}

	/* Iron Man objects */
	ironMan1 = world.Spawn(ObjectType::IronMan, "IronMan");
	ironMan2 = world.Spawn(ObjectType::IronMan, "IronMan_2");
	ironMan2->transform.SetPos(vec3(6, 0, 0));

	ironManTexture.TextureToProgram(ironManProgramShader, AssetPaths::IronManTexture);

	/* Light sources for the Iron Man shader program */
	world.lightsProgram = ironManProgramShader;
	world.lights.AddAmbientLight(ironManProgramShader, vec3(0.1f, 0.1f, 0.1f));
	world.lights.AddDirectionalLight(ironManProgramShader, vec3(1, 0, 0), vec3(0.2, 0.2, 0.2), vec3(0, 0.5, 1)*2.0f, vec3(0, 0.5, 1)*100.0f);
	world.lights.AddPointLight(ironManProgramShader, vec3(1.0, 2.0, 0.0), vec3(0.1, 0.1, 0.1), vec3(1.0, 0, 0.1)*2.0f, vec3(1.0, 0, 0.1)*10.0f, 1, 0.06f, 0.002f);
	world.lights.AddSpotLight(ironManProgramShader, vec3(0, 3, 2), vec3(0, 0, -2), vec3(0.1, 0.1, 0.1), vec3(1, 1, 1)*2.0f, vec3(1.0, 1.0, 1.0), 1, 0.006f, 0.002f, (SMALL_PI / 12));

	glViewport(0, 0, app.config.width, app.config.height);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE); // Disabled by default
	glCullFace(GL_BACK); // GL_FRONT, [GL_BACK], GL_FRONT_AND_BACK
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

	/* Keys 1-4 toggle the corresponding lights */
	(app.inputs.onceKey1) ? world.lights.ToggleAmbientLight(ironManProgramShader, true) : world.lights.ToggleAmbientLight(ironManProgramShader, false);
	(app.inputs.onceKey2) ? world.lights.ToggleDirectionalLight(ironManProgramShader, true) : world.lights.ToggleDirectionalLight(ironManProgramShader, false);
	(app.inputs.onceKey3) ? world.lights.TogglePointLight(ironManProgramShader, 0, true) : world.lights.TogglePointLight(ironManProgramShader, 0, false);
	(app.inputs.onceKey4) ? world.lights.ToggleSpotLight(ironManProgramShader, 0, true) : world.lights.ToggleSpotLight(ironManProgramShader, 0, false);

#ifdef NUC_GAME_BUILD
	/* Standalone game: ESC closes the window (in the editor, Esc stops Play). */
	app.controller.RequestExit(app.window.windowPtr);
#endif
}

void DemoScene::Draw(Application& app)
{
	//Clear the buffers before drawing
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Push the time values to the shader programs */
	Time::TimeToProgram(ironManProgramShader);
	Time::TimeToProgram(cubeProgramShader);

	/* Render every world object */
	for (WorldEntry& entry : world.entries)
		entry.object->meshRenderer.Draw(world.renderMode, &world.camera);
}

void DemoScene::Unload(Application& app)
{
	world.Clear();

	ironManTexture.Unload();

	glDeleteProgram(cubeProgramShader);
	glDeleteProgram(ironManProgramShader);
	cubeProgramShader = 0;
	ironManProgramShader = 0;
}
