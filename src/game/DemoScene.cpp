// DemoScene: the sample scene — a 10x10 cube grid, an indexed cube,
// two Iron Man models and four toggleable light sources.

#include "game/DemoScene.h"
#include "game/AssetPaths.h"
#include "engine/core/Time.h"
#include "engine/core/EngineMath.h"
#include "LoadShaders/LoadShaders.h"

#include <cmath>
#include <cstdlib>
#include <vector>

using namespace std;

DemoScene::DemoScene()
	: camera(glm::vec3(1.0f, 1.0f, -10.0f),  // position
	         glm::vec3(0.0f, 0.0f, 0.0f),    // look-at target
	         glm::vec3(0.0f, 1.0f, 0.0f))    // up vector
{
}

bool DemoScene::Load(Application& app)
{
	if (!LoadProgramShaders())
		return false;

	LoadObjects(app);

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

	/* Cube object data */
	vector<glm::vec3> vertexPos = {
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
	vector<glm::vec3> vertexColor;
	srand(31);//6,11,31
	for (int i = 0; i < 6; i++) {

		float a = ((float)rand()) / (float)RAND_MAX;
		float b = ((float)rand()) / (float)RAND_MAX;
		float c = ((float)rand()) / (float)RAND_MAX;

		for (int j = 0; j < 6; j++) {
			vertexColor.push_back(glm::vec3(a, b, c));
		}
	}
	//Indexed Cube Vertex

	vector<vec3> cube_vertices = {
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

	vector<vec3> cube_colors = {
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

	vector<GLuint> cube_elements = {
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

	/* Indexed cube: shader program, vertex array and color array */
	cube2.CreateObjPosColor(cubeProgramShader, &cube_vertices, &cube_colors);
	cube2.meshRenderer.mesh.AssignElementArray(&cube_elements);
	cube2.transform.SetPos(vec3(3, 10, 0));
	cube2.transform.scale = vec3(0.5f, 0.5f, 0.5f);

	int index = 0;
	for (int h = 0; h < 10; h++)
		for (int i = 0; i < 10; i++)
		{
			cubes[index].CreateObjPosColor(cubeProgramShader, &vertexPos, &vertexColor);
			cubes[index].transform.SetPos(vec3((i * 1.5f) - 4, -1, (h * 1.5f) - 6.0f));
			index++;
		}

	cube.transform.SetPos(glm::vec3(3, 2, 0));

	/* Iron Man objects */
	{
		ironMan.LoadObjFile(ironManProgramShader, AssetPaths::IronManFolder, AssetPaths::IronManObjFile);
		ironMan2.LoadObjFile(ironManProgramShader, AssetPaths::IronManFolder, AssetPaths::IronManObjFile);

		ironManTexture.TextureToProgram(ironManProgramShader, AssetPaths::IronManTexture);
		ironMan2.transform.SetPos(vec3(6, 0, 0));
	}

	/* Light sources for the Iron Man shader program */
	lights.AddAmbientLight(ironManProgramShader, vec3(0.1f, 0.1f, 0.1f));
	lights.AddDirectionalLight(ironManProgramShader, vec3(1, 0, 0), vec3(0.2, 0.2, 0.2), vec3(0, 0.5, 1)*2.0f, vec3(0, 0.5, 1)*100.0f);
	lights.AddPointLight(ironManProgramShader, vec3(1.0, 2.0, 0.0), vec3(0.1, 0.1, 0.1), vec3(1.0, 0, 0.1)*2.0f, vec3(1.0, 0, 0.1)*10.0f, 1, 0.06f, 0.002f);
	lights.AddSpotLight(ironManProgramShader, vec3(0, 3, 2), vec3(0, 0, -2), vec3(0.1, 0.1, 0.1), vec3(1, 1, 1)*2.0f, vec3(1.0, 1.0, 1.0), 1, 0.006f, 0.002f, (SMALL_PI / 12));

	glViewport(0, 0, app.config.width, app.config.height);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE); // Disabled by default
	glCullFace(GL_BACK); // GL_FRONT, [GL_BACK], GL_FRONT_AND_BACK
}

void DemoScene::Update(Application& app)
{
	float deltaTime = Time::deltaTime;

	/* Basic camera movement */
	app.controller.BasicMovement(&camera.transform, 0.15f*deltaTime, 5 * deltaTime);

	/* Object rotations */
	//cube.transform.Rotate(vec3(-1, -1, 0)*2.0f*deltaTime);
	ironMan.transform.Rotate(vec3(1, 0, 0)*deltaTime);
	ironMan2.transform.Rotate(vec3(-1, 0, 0)*deltaTime);

	/* Toggle the model deformation */
	if (app.inputs.onceKey5)
	{
		cube2.transform.Rotate(vec3(1, 1, 0)*2.0f*deltaTime);
		cube2.transform.scale = vec3(cos(Time::time*20)+1, cos(Time::time * 20)+1, cos(Time::time * 20)+1);

		for (int i = 0; i < 100; i++)
		{
			cubes[i].transform.Rotate(vec3(1 + (i*0.0005f), 0, 0)*(deltaTime));
			cubes[i].transform.scale = vec3(1,1,1)*(cos(Time::time*2) + 3)*0.3f;
		}

		offsetToggle = 1;
	}
	else
		offsetToggle = 0;

	//offsetToggle is passed to the shader program as a uniform
	glProgramUniform1i(ironManProgramShader, offsetToggleLocation, offsetToggle);

	if (app.inputs.key6) renderMode = GL_TRIANGLES;
	if (app.inputs.key7) renderMode = GL_LINE_STRIP;
	if (app.inputs.key8) renderMode = GL_POINTS;
	if (app.inputs.key9) renderMode = GL_TRIANGLE_FAN;

	/* Keys 1-4 toggle the corresponding lights */
	(app.inputs.onceKey1) ? lights.ToggleAmbientLight(ironManProgramShader, true) : lights.ToggleAmbientLight(ironManProgramShader, false);
	(app.inputs.onceKey2) ? lights.ToggleDirectionalLight(ironManProgramShader, true) : lights.ToggleDirectionalLight(ironManProgramShader, false);
	(app.inputs.onceKey3) ? lights.TogglePointLight(ironManProgramShader, 0, true) : lights.TogglePointLight(ironManProgramShader, 0, false);
	(app.inputs.onceKey4) ? lights.ToggleSpotLight(ironManProgramShader, 0, true) : lights.ToggleSpotLight(ironManProgramShader, 0, false);

	app.controller.RequestExit(app.window.windowPtr);
}

void DemoScene::Draw(Application& app)
{
	//Clear the buffers before drawing
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Push the time values to the shader programs */
	Time::TimeToProgram(ironManProgramShader);
	Time::TimeToProgram(cubeProgramShader);

	/* Render the objects */
	ironMan.meshRenderer.Draw(renderMode, &camera);
	ironMan2.meshRenderer.Draw(renderMode, &camera);
	//cube.meshRenderer.Draw(renderMode,&camera);
	cube2.meshRenderer.Draw(renderMode, &camera);

	for (int i = 0; i < 100; i++)
		cubes[i].meshRenderer.Draw(renderMode, &camera);
}

void DemoScene::Unload(Application& app)
{
	ironMan.meshRenderer.mesh.Unload();
	ironMan2.meshRenderer.mesh.Unload();
	cube.meshRenderer.mesh.Unload();
	cube2.meshRenderer.mesh.Unload();
	for (int i = 0; i < 100; i++)
		cubes[i].meshRenderer.mesh.Unload();

	ironManTexture.Unload();

	glDeleteProgram(cubeProgramShader);
	glDeleteProgram(ironManProgramShader);
	cubeProgramShader = 0;
	ironManProgramShader = 0;
}
