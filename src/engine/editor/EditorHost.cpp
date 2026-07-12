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

	app.inputs.SetCursorCaptured(false);
	return true;
}

void EditorHost::Update(Application& app)
{
	if (mode == Mode::Play) {
		game.Update(app);
		if (app.inputs.keyEsc)
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
	if (!editor.pendingSceneLoad.empty()) {
		std::string path = editor.pendingSceneLoad;
		editor.pendingSceneLoad.clear();
		editor.selected = nullptr; // objects are about to be destroyed
		editor.undoStack.Clear();  // history ids die with the old world
		if (SceneSerializer::Load(world, path))
			editor.savePath = path;
	}

	/* UE5-style fly: only while RMB is held over the viewport. */
	if (editor.viewportFlying) {
		float deltaTime = Time::deltaTime;
		app.controller.BasicMovement(&world.camera.transform, 0.15f * deltaTime, 5 * deltaTime);
	}
}

void EditorHost::Draw(Application& app)
{
	if (mode == Mode::Play) {
		glViewport(0, 0, playWidth, playHeight);
		game.Draw(app);
		editor.DrawPlayOverlay();
		return;
	}

	/* Scene pass into the viewport framebuffer. */
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

	app.window.SetWindowSize(playWidth, playHeight);
	app.inputs.SetWindowSize(playWidth, playHeight);
	world.camera.SetAspect((float)playWidth / (float)playHeight);

	app.inputs.SetCursorCaptured(true);
	app.inputs.CenterCursor();

	/* Sync the demo's once-key toggles with the current light switches so
	   entering Play does not snap lights back to their key states. */
	VectorLight& info = world.lights.lightInfo;
	if (!info.ambientLight.empty())     app.inputs.onceKey1 = info.ambientLight[0].switchL != 0;
	if (!info.directionalLight.empty()) app.inputs.onceKey2 = info.directionalLight[0].switchL != 0;
	if (!info.pointLight.empty())       app.inputs.onceKey3 = info.pointLight[0].switchL != 0;
	if (!info.spotLight.empty())        app.inputs.onceKey4 = info.spotLight[0].switchL != 0;

	/* ImGui must not fight the recentered, hidden cursor while playing. */
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse | ImGuiConfigFlags_NoMouseCursorChange;

	mode = Mode::Play;
}

void EditorHost::ExitPlay(Application& app)
{
	app.window.SetWindowSize(app.config.width, app.config.height);
	app.inputs.SetWindowSize(app.config.width, app.config.height);
	app.inputs.SetCursorCaptured(false);

	world.camera.transform.position = savedCameraPos;
	world.camera.transform.rotation = savedCameraRot;

	ImGui::GetIO().ConfigFlags &= ~(ImGuiConfigFlags_NoMouse | ImGuiConfigFlags_NoMouseCursorChange);

	mode = Mode::Edit;
}
