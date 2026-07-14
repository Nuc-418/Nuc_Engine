#include "engine/input/InputActions.h"

void InputActions::BindAction(const std::string& action, int key)
{
	actions[action].push_back(key);
}

void InputActions::BindAxis(const std::string& axis, int positiveKey, int negativeKey)
{
	axes[axis].push_back({ positiveKey, negativeKey });
}

void InputActions::BindToggle(const std::string& toggle, int key)
{
	toggles[toggle].keys.push_back(key);
}

void InputActions::BindGamepadButton(const std::string& action, int button)
{
	padActions[action].push_back(button);
}

void InputActions::BindGamepadToggle(const std::string& toggle, int button)
{
	toggles[toggle].padButtons.push_back(button);
}

void InputActions::BindGamepadAxis(const std::string& axis, int gamepadAxis, float scale)
{
	padAxes[axis].push_back({ gamepadAxis, scale });
}

void InputActions::BeginFrame(const bool* down, const bool* pressed, int keyCount)
{
	frameDown.assign(down, down + keyCount);
	framePressed.assign(pressed, pressed + keyCount);

	for (auto& entry : toggles)
		for (int key : entry.second.keys)
			if (KeyPressed(key))
				entry.second.value = !entry.second.value;
}

void InputActions::SetGamepadState(const bool* buttons, int buttonCount,
                                   const float* axisValues, int axisCount, bool connected)
{
	gamepadConnected = connected;

	if (!connected) {
		// Drop all held/edge state so a disconnect can't leave a button "stuck".
		padDown.assign(padDown.size(), false);
		padPressed.assign(padPressed.size(), false);
		padPrev.assign(padPrev.size(), false);
		padAxisVals.assign(padAxisVals.size(), 0.0f);
		return;
	}

	padPrev.resize(buttonCount, false);
	padDown.assign(buttons, buttons + buttonCount);
	padPressed.assign(buttonCount, false);
	for (int i = 0; i < buttonCount; ++i) {
		padPressed[i] = padDown[i] && !padPrev[i]; // rising edge
		padPrev[i] = padDown[i];
	}

	padAxisVals.assign(axisValues, axisValues + axisCount);
	for (float& v : padAxisVals)
		if (v > -axisDeadzone && v < axisDeadzone)
			v = 0.0f;

	// Gamepad toggle latches flip on button press edges (keyboard latches
	// already flipped in BeginFrame; the two binding sets never overlap).
	for (auto& entry : toggles)
		for (int button : entry.second.padButtons)
			if (PadButtonPressed(button))
				entry.second.value = !entry.second.value;
}

bool InputActions::KeyDown(int key) const
{
	return key >= 0 && key < (int)frameDown.size() && frameDown[key];
}

bool InputActions::KeyPressed(int key) const
{
	return key >= 0 && key < (int)framePressed.size() && framePressed[key];
}

bool InputActions::PadButtonDown(int button) const
{
	return gamepadConnected && button >= 0 && button < (int)padDown.size() && padDown[button];
}

bool InputActions::PadButtonPressed(int button) const
{
	return gamepadConnected && button >= 0 && button < (int)padPressed.size() && padPressed[button];
}

float InputActions::PadAxis(int axis) const
{
	return (gamepadConnected && axis >= 0 && axis < (int)padAxisVals.size()) ? padAxisVals[axis] : 0.0f;
}

bool InputActions::IsDown(const std::string& action) const
{
	auto it = actions.find(action);
	if (it != actions.end())
		for (int key : it->second)
			if (KeyDown(key))
				return true;
	auto pit = padActions.find(action);
	if (pit != padActions.end())
		for (int button : pit->second)
			if (PadButtonDown(button))
				return true;
	return false;
}

bool InputActions::WasPressed(const std::string& action) const
{
	auto it = actions.find(action);
	if (it != actions.end())
		for (int key : it->second)
			if (KeyPressed(key))
				return true;
	auto pit = padActions.find(action);
	if (pit != padActions.end())
		for (int button : pit->second)
			if (PadButtonPressed(button))
				return true;
	return false;
}

float InputActions::Axis(const std::string& axis) const
{
	float keyValue = 0.0f;
	auto it = axes.find(axis);
	if (it != axes.end())
		for (const std::pair<int, int>& pair : it->second) {
			if (KeyDown(pair.first)) keyValue += 1.0f;
			if (KeyDown(pair.second)) keyValue -= 1.0f;
		}
	keyValue = keyValue < -1.0f ? -1.0f : (keyValue > 1.0f ? 1.0f : keyValue);

	// Analog contribution: whichever gamepad-axis term has the largest
	// magnitude (multiple binds to one name are alternatives, not additive).
	float padValue = 0.0f;
	auto pit = padAxes.find(axis);
	if (pit != padAxes.end())
		for (const std::pair<int, float>& bind : pit->second) {
			float v = PadAxis(bind.first) * bind.second;
			if (v < -1.0f) v = -1.0f; else if (v > 1.0f) v = 1.0f;
			if (v < 0.0f ? -v > (padValue < 0.0f ? -padValue : padValue)
			             :  v > (padValue < 0.0f ? -padValue : padValue))
				padValue = v;
		}

	// Keyboard vs stick: the larger magnitude wins, so either input works and
	// an idle stick never cancels a key press.
	float keyMag = keyValue < 0.0f ? -keyValue : keyValue;
	float padMag = padValue < 0.0f ? -padValue : padValue;
	return padMag > keyMag ? padValue : keyValue;
}

bool InputActions::Toggle(const std::string& toggle) const
{
	auto it = toggles.find(toggle);
	return it != toggles.end() && it->second.value;
}

void InputActions::SetToggle(const std::string& toggle, bool value)
{
	toggles[toggle].value = value;
}
