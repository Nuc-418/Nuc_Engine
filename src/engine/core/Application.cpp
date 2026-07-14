// Application: window/input/GLEW bootstrap and the main loop that drives a Scene.

#include "engine/core/Application.h"
#include "engine/core/Time.h"

// Common XInput-style layout for GLFW 3.2's raw joystick arrays (the vendored
// GLFW predates the 3.3 gamepad-mapping API). Real layouts vary by device and
// driver; these defaults target the typical controller and are rebindable.
namespace {
	constexpr int kPadAxisLeftX = 0;
	constexpr int kPadAxisLeftY = 1;
	constexpr int kPadButtonLeftThumb = 8;  // left stick click
	constexpr int kPadButtonStart = 7;
	constexpr int kMaxPadButtons = 32;
}

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

	// Gamepad defaults on the same action names, additive to the keyboard binds:
	// left stick moves (stick Y is negative-up, so invert it for MoveForward),
	// stick-click sprints, Start exits.
	actions.BindGamepadAxis("MoveForward", kPadAxisLeftY, -1.0f);
	actions.BindGamepadAxis("MoveRight", kPadAxisLeftX, 1.0f);
	actions.BindGamepadButton("Sprint", kPadButtonLeftThumb);
	actions.BindGamepadButton("Exit", kPadButtonStart);

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
		PollGamepad();

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

void Application::PollGamepad()
{
	// Raw joystick API (GLFW 3.2): level-state button bytes + analog axes, with
	// device-reported counts. SetGamepadState derives press edges and applies
	// the deadzone; an absent stick contributes nothing.
	if (glfwJoystickPresent(GLFW_JOYSTICK_1) != GLFW_TRUE) {
		actions.SetGamepadState(nullptr, 0, nullptr, 0, false);
		return;
	}

	int buttonCount = 0, axisCount = 0;
	const unsigned char* rawButtons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);
	const float* axisValues = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axisCount);
	if (!rawButtons || !axisValues) {
		actions.SetGamepadState(nullptr, 0, nullptr, 0, false);
		return;
	}

	if (buttonCount > kMaxPadButtons)
		buttonCount = kMaxPadButtons;
	bool buttons[kMaxPadButtons];
	for (int i = 0; i < buttonCount; ++i)
		buttons[i] = rawButtons[i] == GLFW_PRESS;

	actions.SetGamepadState(buttons, buttonCount, axisValues, axisCount, true);
}

void Application::Shutdown()
{
	glfwDestroyWindow(window.windowPtr);
	glfwTerminate();
}
