// InputActions: named action/axis/toggle bindings over raw key state.
//
// Gameplay and engine code query actions by name ("MoveForward", "Exit")
// instead of hardcoded keys. Bindings map names to key codes (GLFW_KEY_*
// values, but this class only sees ints — it is GLFW-free and unit-tested).
// Application::Run snapshots the raw state once per frame via BeginFrame,
// which also flips toggle latches on press edges (the old onceKey*
// semantics: each press inverts the latch).
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

	// Snapshot the frame's key state (arrays indexed by key code). Flips
	// toggle latches for press edges. Call once per frame, before updates.
	void BeginFrame(const bool* down, const bool* pressed, int keyCount);

	bool IsDown(const std::string& action) const;
	bool WasPressed(const std::string& action) const; // press edge this frame
	float Axis(const std::string& axis) const;
	bool Toggle(const std::string& toggle) const;

	// Force a latch (e.g. the editor syncing toggles to loaded scene state).
	void SetToggle(const std::string& toggle, bool value);

private:
	bool KeyDown(int key) const;
	bool KeyPressed(int key) const;

	std::map<std::string, std::vector<int>> actions;
	std::map<std::string, std::vector<std::pair<int, int>>> axes; // positive, negative
	struct ToggleState { std::vector<int> keys; bool value = false; };
	std::map<std::string, ToggleState> toggles;

	std::vector<bool> frameDown;
	std::vector<bool> framePressed;
};
