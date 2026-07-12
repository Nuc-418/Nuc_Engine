// DemoScene: the sample scene — a 10x10 cube grid, an indexed cube,
// two Iron Man models and four toggleable light sources.

#pragma once

#include <vector>

#include "engine/core/Application.h"
#include "engine/scene/World.h"
#include "engine/render/Texture.h"

class DemoScene : public Scene
{
public:
	bool Load(Application& app) override;
	void Update(Application& app) override;
	void Draw(Application& app) override;
	void Unload(Application& app) override;

	World& GetWorld() { return world; }

private:
	bool LoadProgramShaders();
	void LoadObjects(Application& app);

	World world;

	// Shader programs
	GLuint ironManProgramShader = 0;
	GLuint cubeProgramShader = 0;

	// Cached at load time; the programs are never recreated.
	GLint offsetToggleLocation = -1;

	Texture ironManTexture;

	// Raw handles into the world for the demo animations. Nulled through
	// World::onDestroyed if the editor deletes the objects.
	GameObject* ironMan1 = nullptr;
	GameObject* ironMan2 = nullptr;
	GameObject* indexedCube = nullptr;
	std::vector<GameObject*> gridCubes;

	int offsetToggle = 0;
};
