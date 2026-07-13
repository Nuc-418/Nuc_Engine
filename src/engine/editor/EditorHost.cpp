// EditorHost: Scene wrapper that adds the editor around a game scene.

#include "engine/editor/EditorHost.h"
#include "engine/editor/EditorFileSystem.h"
#include "engine/io/SceneSerializer.h"
#include "engine/core/Application.h"
#include "engine/core/Time.h"

#include <GLFW/glfw3.h>
#include <algorithm>

bool EditorHost::Load(Application& app)
{
	if (!game.Load(app))
		return false;

	// The engine key callback is installed (Application::Init), so ImGui's
	// GLFW backend chains to it from here on.
	if (!editor.Init(app.window.windowPtr, &world))
		return false;

	// Open on a clean UE5-style default map (ground + directional light) rather
	// than the sample scene's demo clutter. The factories the demo registered in
	// game.Load stay available, so spawning still works.
	world.ResetToDefaultMap();

	app.inputs.SetCursorCaptured(false);

	// Edit mode: freeze simulation plugins (physics, ...) until Play.
	app.simulating = false;
	return true;
}

void EditorHost::Update(Application& app)
{
	if (mode == Mode::Play) {
		game.Update(app);
		if (app.actions.IsDown("Exit"))
			ExitPlay(app);
		return;
	}

	/* Edit mode: consume the UI actions recorded during last frame's Draw. */
	if (editor.playClicked) {
		editor.playClicked = false;
		EnterPlay(app);
		return;
	}
	if (editor.exitClicked) {
		editor.exitClicked = false;
		glfwSetWindowShouldClose(app.window.windowPtr, GLFW_TRUE);
	}
	if (editor.saveClicked) {
		editor.saveClicked = false;
		EnsureDirectory("assets/scenes");
		SceneSerializer::Save(world, editor.savePath);
	}
	if (!editor.pendingNewMap.empty()) {
		std::string path = editor.pendingNewMap;
		editor.pendingNewMap.clear();
		editor.selected = nullptr;
		editor.undoStack.Clear();
		world.ResetToDefaultMap();
		EnsureDirectory("assets");
		EnsureDirectory("assets/scenes");
		if (SceneSerializer::Save(world, path))
			editor.savePath = path;
	}
	if (!editor.pendingSceneLoad.empty()) {
		std::string path = editor.pendingSceneLoad;
		editor.pendingSceneLoad.clear();
		editor.selected = nullptr; // objects are about to be destroyed
		editor.undoStack.Clear();  // history ids die with the old world
		if (SceneSerializer::Load(world, path))
			editor.savePath = path;
	}

	/* Edit mode still ticks component OnUpdate (no OnSimulate). */
	world.Tick(Time::deltaTime, false);

	/* UE5-style fly: only while RMB is held over the viewport. */
	if (editor.viewportFlying) {
		float deltaTime = Time::deltaTime;
		app.controller.BasicMovement(&world.camera.transform, 0.15f * deltaTime, editor.flySpeed * deltaTime);
	}
}

void EditorHost::Draw(Application& app)
{
	/* Scene pass into the viewport framebuffer (edit and play alike:
	   play runs inside the Viewport panel, UE5 PIE-style). */
	int fbWidth = (int)editor.viewportSize.x;
	int fbHeight = (int)editor.viewportSize.y;
	editor.sceneFramebuffer.Resize(fbWidth, fbHeight);
	world.camera.SetAspect((float)std::max(1, fbWidth) / (float)std::max(1, fbHeight));

	editor.sceneFramebuffer.Bind();
	glViewport(0, 0, editor.sceneFramebuffer.width, editor.sceneFramebuffer.height);
	game.Draw(app);
	editor.sceneFramebuffer.Unbind();

	/* UI pass onto the window backbuffer. */
	int windowWidth = 0, windowHeight = 0;
	glfwGetFramebufferSize(app.window.windowPtr, &windowWidth, &windowHeight);
	glViewport(0, 0, windowWidth, windowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	editor.DrawFrame(app);
}

void EditorHost::Unload(Application& app)
{
	editor.Shutdown();
	game.Unload(app);
}

void EditorHost::EnterPlay(Application& app)
{
	savedCameraPos = world.camera.transform.position;
	savedCameraRot = world.camera.transform.rotation;

	/* Mouse-look deltas are measured from the window center, so capturing
	   the cursor is enough; the game keeps rendering in the Viewport panel. */
	app.inputs.SetCursorCaptured(true);
	app.inputs.CenterCursor();

	/* Sync the demo's once-key toggles with the current light switches so
	   entering Play does not snap lights back to their key states. */
	VectorLight& info = world.lights.lightInfo;
	if (!info.ambientLight.empty())     app.actions.SetToggle("ToggleAmbientLight", info.ambientLight[0].switchL != 0);
	if (!info.directionalLight.empty()) app.actions.SetToggle("ToggleDirectionalLight", info.directionalLight[0].switchL != 0);
	if (!info.pointLight.empty())       app.actions.SetToggle("TogglePointLight", info.pointLight[0].switchL != 0);
	if (!info.spotLight.empty())        app.actions.SetToggle("ToggleSpotLight", info.spotLight[0].switchL != 0);

	/* ImGui must not fight the recentered, hidden cursor while playing. */
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse | ImGuiConfigFlags_NoMouseCursorChange;

	app.simulating = true; // run simulation plugins while playing
	editor.playing = true;
	mode = Mode::Play;
	world.NotifyPlayBegin();
}

void EditorHost::ExitPlay(Application& app)
{
	app.inputs.SetCursorCaptured(false);

	world.camera.transform.position = savedCameraPos;
	world.camera.transform.rotation = savedCameraRot;

	ImGui::GetIO().ConfigFlags &= ~(ImGuiConfigFlags_NoMouse | ImGuiConfigFlags_NoMouseCursorChange);

	world.NotifyPlayEnd();
	app.simulating = false; // freeze simulation plugins back in Edit mode
	editor.playing = false;
	mode = Mode::Edit;
}
