// DemoScene: the sample scene — a 10x10 cube grid, an indexed cube,
// two Iron Man models and four toggleable light sources.

#pragma once

#include "engine/core/Application.h"
#include "engine/scene/GameObject.h"
#include "engine/render/Texture.h"
#include "engine/render/Lights.h"
#include "engine/render/Camera.h"

class DemoScene : public Scene
{
public:
	DemoScene();

	bool Load(Application& app) override;
	void Update(Application& app) override;
	void Draw(Application& app) override;
	void Unload(Application& app) override;

private:
	bool LoadProgramShaders();
	void LoadObjects(Application& app);

	// Shader programs
	GLuint ironManProgramShader = 0;
	GLuint cubeProgramShader = 0;

	// Iron Man models and their texture
	GameObject ironMan;
	GameObject ironMan2;
	Texture ironManTexture;

	// Cubes
	GameObject cubes[100];
	GameObject cube;
	GameObject cube2;

	Lights lights;
	Camera camera;
	GLenum renderMode = GL_TRIANGLES;
	int offsetToggle = 0;
};
