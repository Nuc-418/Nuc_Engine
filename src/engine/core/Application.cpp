// Application: window/input/GLEW bootstrap and the main loop that drives a Scene.

#include "engine/core/Application.h"
#include "engine/core/Time.h"

bool Application::Init(const Config& appConfig)
{
	config = appConfig;

	if (!glfwInit()) return false;

	if (!window.NewWindow(config.width, config.height, config.title, NULL, NULL))
		return false;

	// Place the window flush to the top-left. glfwSetWindowPos positions the
	// content area, so offset by the frame's top/left edges (the title bar and
	// border) — otherwise the caption is pushed off-screen and the window can't
	// be grabbed to move or maximize it.
	int frameLeft = 0, frameTop = 0, frameRight = 0, frameBottom = 0;
	glfwGetWindowFrameSize(window.windowPtr, &frameLeft, &frameTop, &frameRight, &frameBottom);
	window.SetWindowPos(frameLeft, frameTop);
	window.MakeContextCurrent();

	inputs.AssociateWindow(window.windowPtr, config.width, config.height);
	controller.AssociateUserInput(&inputs, &actions);

	// Engine-default bindings; scenes bind their own actions in Scene::Load.
	actions.BindAxis("MoveForward", GLFW_KEY_W, GLFW_KEY_S);
	actions.BindAxis("MoveRight", GLFW_KEY_D, GLFW_KEY_A);
	actions.BindAxis("MoveUp", GLFW_KEY_SPACE, GLFW_KEY_LEFT_CONTROL);
	actions.BindAction("Sprint", GLFW_KEY_LEFT_SHIFT);
	actions.BindAction("Exit", GLFW_KEY_ESCAPE);

	glewInit();

	// Publish the core engine services so plugins can fetch them by interface.
	services.Provide<AssetManager>(&assets);

	return true;
}

void Application::Run(Scene& scene)
{
	if (!scene.Load(*this)) return;

	// Load plugins the scene registered during Scene::Load.
	plugins.LoadAll(*this);

	while (!glfwWindowShouldClose(window.windowPtr)) {

		double begin = glfwGetTime();

		// Snapshot this frame's key state for the action queries (flipping
		// toggle latches on press edges), then clear the edges so the next
		// glfwPollEvents accumulates fresh ones.
		actions.BeginFrame(inputs.keyDown, inputs.keyPressed, UserInputs::KeyCount);
		inputs.ClearPressed();

		// Advance plugins before scene logic so this frame's scene update and
		// draw see the results (e.g. physics-driven transforms).
		plugins.UpdateAll(*this, (float)Time::deltaTime);
		scene.Update(*this);
		scene.Draw(*this);

		glfwSwapBuffers(window.windowPtr);
		glfwPollEvents();
		double end = glfwGetTime();

		// Wall-clock frame time (glfwGetTime), not CPU time (clock()), so
		// deltaTime reflects real elapsed time.
		double elapsed_secs = end - begin;
		Time::Update(elapsed_secs);
	}

	scene.Unload(*this);
	plugins.UnloadAll(*this);
	assets.UnloadAll(); // last: components/scenes may reference assets while unloading
}

void Application::Shutdown()
{
	glfwDestroyWindow(window.windowPtr);
	glfwTerminate();
}
