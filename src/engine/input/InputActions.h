// InputActions: named action/axis/toggle bindings over raw key state.
//
// Gameplay and engine code query actions by name ("MoveForward", "Exit")
// instead of hardcoded keys. Bindings map names to key codes (GLFW_KEY_*
// values, but this class only sees ints — it is GLFW-free and unit-tested).
// Application::Run snapshots the raw state once per frame via BeginFrame,
// which also flips toggle latches on press edges (the old onceKey*
// semantics: each press inverts the latch).
//
// Gamepad bindings coexist with keyboard ones on the same action/axis/toggle
// names: an action is down if a bound key OR a bound gamepad button is down,
// and a named axis takes whichever of its keyboard/stick contributions has the
// larger magnitude. Application::Run polls the gamepad (glfwGetGamepadState)
// and feeds it via SetGamepadState — GLFW stays out of this class, which sees
// only button/axis indices and raw values.
//
// Default bindings are installed by Application::Init (movement, Exit);
// scenes add their own in Scene::Load (see DemoScene).

#pragma once

#include <map>
#include <string>
#include <vector>

class InputActions
{
public:
	// Action: true while any bound key is held. Rebinding appends.
	void BindAction(const std::string& action, int key);
	// Axis: +1 while positiveKey is held, -1 for negativeKey, 0 otherwise
	// (both held cancel out).
	void BindAxis(const std::string& axis, int positiveKey, int negativeKey);
	// Toggle: a latch that inverts on every press of the bound key.
	void BindToggle(const std::string& toggle, int key);

	// --- Gamepad bindings (button/axis indices, e.g. GLFW_GAMEPAD_BUTTON_*/
	//     GLFW_GAMEPAD_AXIS_*; this class only sees ints) ---------------------
	// Action down while the bound gamepad button is held.
	void BindGamepadButton(const std::string& action, int button);
	// Toggle latch inverts on each press of the bound gamepad button.
	void BindGamepadToggle(const std::string& toggle, int button);
	// Named axis reads an analog gamepad axis, multiplied by `scale` (use -1 to
	// invert, e.g. stick Y where up is negative).
	void BindGamepadAxis(const std::string& axis, int gamepadAxis, float scale = 1.0f);

	// Snapshot the frame's key state (arrays indexed by key code). Flips
	// toggle latches for press edges. Call once per frame, before updates.
	void BeginFrame(const bool* down, const bool* pressed, int keyCount);

	// Feed this frame's gamepad state (button-down flags + analog axis values).
	// Press edges are derived here from the previous frame (GLFW reports levels,
	// not edges), and gamepad toggle latches flip on those edges. When
	// `connected` is false the gamepad contributes nothing. Call once per frame.
	void SetGamepadState(const bool* buttons, int buttonCount,
	                     const float* axisValues, int axisCount, bool connected);

	bool IsDown(const std::string& action) const;
	bool WasPressed(const std::string& action) const; // press edge this frame
	float Axis(const std::string& axis) const;
	bool Toggle(const std::string& toggle) const;

	bool GamepadConnected() const { return gamepadConnected; }

	// Analog stick values below this magnitude are treated as zero.
	void SetAxisDeadzone(float deadzone) { axisDeadzone = deadzone; }

	// Force a latch (e.g. the editor syncing toggles to loaded scene state).
	void SetToggle(const std::string& toggle, bool value);

private:
	bool KeyDown(int key) const;
	bool KeyPressed(int key) const;
	bool PadButtonDown(int button) const;
	bool PadButtonPressed(int button) const;
	float PadAxis(int axis) const;

	std::map<std::string, std::vector<int>> actions;
	std::map<std::string, std::vector<std::pair<int, int>>> axes; // positive, negative
	struct ToggleState { std::vector<int> keys; std::vector<int> padButtons; bool value = false; };
	std::map<std::string, ToggleState> toggles;

	std::map<std::string, std::vector<int>> padActions;                    // action -> buttons
	std::map<std::string, std::vector<std::pair<int, float>>> padAxes;     // axis -> (index, scale)

	std::vector<bool> frameDown;
	std::vector<bool> framePressed;

	std::vector<bool> padDown;
	std::vector<bool> padPressed;
	std::vector<bool> padPrev;      // previous-frame button-down, for edge detection
	std::vector<float> padAxisVals;
	bool gamepadConnected = false;
	float axisDeadzone = 0.15f;
};
