// DemoScene: the sample scene — a 10x10 cube grid, an indexed cube, a physics
// demo and a component-driven light rig (a Sun directional + a fill point light,
// both Light actors). Imported models are discovered from assets/models.

#pragma once

#include <vector>

#include "engine/core/Application.h"
#include "engine/scene/World.h"
#include "engine/render/Shader.h"
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
	bool LoadProgramShaders(Application& app);
	void LoadObjects(Application& app);

	World world;

	// Shader assets, owned by app.assets (freed by Application::Run after
	// Unload). The GLuint mirrors feed the spawn factories and per-frame
	// uniform calls; Shader::Reload keeps program ids stable across hot
	// reloads. Model textures are also owned by app.assets. ironManShader is
	// the textured-model shader (named for its asset folder).
	Shader* ironManShader = nullptr;
	Shader* cubeShader = nullptr;
	Shader* primitiveShader = nullptr; // lit shader for the built-in primitives
	GLuint ironManProgramShader = 0;
	GLuint cubeProgramShader = 0;
	GLuint primitiveProgramShader = 0;

	// Raw handles into the world for the demo animations. Nulled through
	// World::onDestroyed if the editor deletes the objects.
	GameObject* indexedCube = nullptr;
	std::vector<GameObject*> gridCubes;

	// --- Physics demo (JoltPhysics plugin) ---------------------------------
	// A dynamic cube dropped onto a static floor, both plain cube objects
	// carrying a PhysicsBodyComponent (the component owns its body's
	// lifetime; the plugin syncs poses while the app is simulating).
	void SetupPhysicsDemo(Application& app);
	JoltPhysicsPlugin* physics = nullptr;
	GameObject* physicsFloor = nullptr;
	GameObject* physicsCube = nullptr;

	int offsetToggle = 0;
};
