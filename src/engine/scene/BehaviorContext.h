// BehaviorContext: the per-frame world/input context handed to behavior hooks.
//
// Behavior components (OnSimulate) receive this so gameplay can read named
// input actions and reach the world without a global or an Application in
// scope. It is a lightweight view of borrowed pointers, rebuilt each frame by
// whoever ticks the world (the scene / the editor host); it owns nothing.
//
// Pointers may be null (e.g. a headless tick with no input), so behaviors must
// null-check before use. The struct grows as behaviors need more context.

#pragma once

class InputActions;
class World;

struct BehaviorContext
{
	const InputActions* input = nullptr;  // named actions/axes/toggles, or null
	World* world = nullptr;               // the world being ticked, or null
};
