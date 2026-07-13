// DemoScene: the sample scene — a 10x10 cube grid, an indexed cube,
// two Iron Man models and four toggleable light sources.

#pragma once

#include <vector>

#include "engine/core/Application.h"
#include "engine/scene/World.h"
#include "engine/render/Texture.h"
#include "JoltPhysics/JoltPhysicsPlugin.h"

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
	GLuint primitiveProgramShader = 0; // lit shader for the built-in primitives

	// Cached at load time; the programs are never recreated.
	GLint offsetToggleLocation = -1;

	// One texture per discovered model (bound to the shared model shader).
	std::vector<Texture> modelTextures;

	// Raw handles into the world for the demo animations. Nulled through
	// World::onDestroyed if the editor deletes the objects.
	GameObject* ironMan1 = nullptr;
	GameObject* ironMan2 = nullptr;
	GameObject* indexedCube = nullptr;
	std::vector<GameObject*> gridCubes;

	// --- Physics demo (JoltPhysics plugin) ---------------------------------
	// A dynamic cube dropped onto a static floor. Both are visualised by cube
	// GameObjects; the plugin writes the simulated pose back into their
	// transforms every frame while the app is simulating (Play mode / game).
	void SetupPhysicsDemo(Application& app);
	JoltPhysicsPlugin* physics = nullptr;
	GameObject* physicsFloor = nullptr;
	GameObject* physicsCube = nullptr;
	PhysicsWorld::BodyId floorBody = PhysicsWorld::InvalidBody;
	PhysicsWorld::BodyId cubeBody = PhysicsWorld::InvalidBody;

	int offsetToggle = 0;
};
