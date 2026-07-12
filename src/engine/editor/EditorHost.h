// EditorHost: Scene wrapper that adds the editor around a game scene.
// Edit mode renders the game into the editor's viewport panel with the sim
// frozen; Play mode runs the game exactly as it runs standalone.

#pragma once

#include <glm/glm.hpp>

#include "engine/core/Scene.h"
#include "engine/editor/Editor.h"

class EditorHost : public Scene
{
public:
	EditorHost(Scene& game, World& world) : game(game), world(world) {}

	bool Load(Application& app) override;
	void Update(Application& app) override;
	void Draw(Application& app) override;
	void Unload(Application& app) override;

private:
	enum class Mode { Edit, Play };

	void EnterPlay(Application& app);
	void ExitPlay(Application& app);

	Scene& game;
	World& world;
	Editor editor;
	Mode mode = Mode::Edit;

	// Camera pose snapshot taken when entering Play, restored on Stop.
	glm::vec3 savedCameraPos = glm::vec3(0.0f);
	glm::vec3 savedCameraRot = glm::vec3(0.0f);
};
