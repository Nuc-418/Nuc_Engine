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

void InputActions::BeginFrame(const bool* down, const bool* pressed, int keyCount)
{
	frameDown.assign(down, down + keyCount);
	framePressed.assign(pressed, pressed + keyCount);

	for (auto& entry : toggles)
		for (int key : entry.second.keys)
			if (KeyPressed(key))
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

bool InputActions::IsDown(const std::string& action) const
{
	auto it = actions.find(action);
	if (it == actions.end())
		return false;
	for (int key : it->second)
		if (KeyDown(key))
			return true;
	return false;
}

bool InputActions::WasPressed(const std::string& action) const
{
	auto it = actions.find(action);
	if (it == actions.end())
		return false;
	for (int key : it->second)
		if (KeyPressed(key))
			return true;
	return false;
}

float InputActions::Axis(const std::string& axis) const
{
	auto it = axes.find(axis);
	if (it == axes.end())
		return 0.0f;
	float value = 0.0f;
	for (const std::pair<int, int>& pair : it->second) {
		if (KeyDown(pair.first)) value += 1.0f;
		if (KeyDown(pair.second)) value -= 1.0f;
	}
	return value < -1.0f ? -1.0f : (value > 1.0f ? 1.0f : value);
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
