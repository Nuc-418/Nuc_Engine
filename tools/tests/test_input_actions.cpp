// InputActions: bindings, edges, axes and toggle latches over fake key state.

#include "doctest/doctest.h"

#include "engine/input/InputActions.h"

namespace
{
	struct Keys
	{
		bool down[16] = {};
		bool pressed[16] = {};
		void Frame(InputActions& actions) { actions.BeginFrame(down, pressed, 16); }
	};
}

TEST_CASE("actions report held keys; unbound names are quiet")
{
	InputActions actions;
	actions.BindAction("Jump", 4);

	Keys keys;
	keys.down[4] = true;
	keys.Frame(actions);
	CHECK(actions.IsDown("Jump"));
	CHECK(!actions.IsDown("Fire"));

	keys.down[4] = false;
	keys.Frame(actions);
	CHECK(!actions.IsDown("Jump"));
}

TEST_CASE("WasPressed is an edge, not a level")
{
	InputActions actions;
	actions.BindAction("Jump", 4);

	Keys keys;
	keys.down[4] = true; keys.pressed[4] = true;
	keys.Frame(actions);
	CHECK(actions.WasPressed("Jump"));

	keys.pressed[4] = false; // still held, no new edge
	keys.Frame(actions);
	CHECK(actions.IsDown("Jump"));
	CHECK(!actions.WasPressed("Jump"));
}

TEST_CASE("axes combine positive and negative keys")
{
	InputActions actions;
	actions.BindAxis("Move", 1, 2);

	Keys keys;
	keys.down[1] = true;
	keys.Frame(actions);
	CHECK(actions.Axis("Move") == doctest::Approx(1.0f));

	keys.down[2] = true; // both held cancel out
	keys.Frame(actions);
	CHECK(actions.Axis("Move") == doctest::Approx(0.0f));

	keys.down[1] = false;
	keys.Frame(actions);
	CHECK(actions.Axis("Move") == doctest::Approx(-1.0f));
}

TEST_CASE("toggles flip on each press edge and can be forced")
{
	InputActions actions;
	actions.BindToggle("Deform", 5);

	Keys keys;
	CHECK(!actions.Toggle("Deform"));

	keys.pressed[5] = true;
	keys.Frame(actions);
	CHECK(actions.Toggle("Deform"));

	keys.pressed[5] = false; // holding does not re-flip
	keys.Frame(actions);
	CHECK(actions.Toggle("Deform"));

	keys.pressed[5] = true;
	keys.Frame(actions);
	CHECK(!actions.Toggle("Deform"));

	actions.SetToggle("Deform", true); // editor sync path
	CHECK(actions.Toggle("Deform"));
}

namespace
{
	struct Pad
	{
		bool buttons[15] = {};
		float axes[6] = {};
		void Frame(InputActions& actions, bool connected = true)
		{
			actions.SetGamepadState(buttons, 15, axes, 6, connected);
		}
	};
}

TEST_CASE("a gamepad button drives the same action name as a key")
{
	InputActions actions;
	actions.BindAction("Jump", 4);        // keyboard
	actions.BindGamepadButton("Jump", 0); // gamepad A

	Keys keys;
	Pad pad;
	keys.Frame(actions);
	CHECK(!actions.GamepadConnected());

	pad.buttons[0] = true;
	pad.Frame(actions);
	CHECK(actions.GamepadConnected());
	CHECK(actions.IsDown("Jump"));
	CHECK(actions.WasPressed("Jump")); // rising edge derived here

	pad.Frame(actions); // still held, no new edge
	CHECK(actions.IsDown("Jump"));
	CHECK(!actions.WasPressed("Jump"));
}

TEST_CASE("gamepad axis applies scale and deadzone")
{
	InputActions actions;
	actions.BindGamepadAxis("MoveForward", 1, -1.0f); // stick Y, inverted
	actions.SetAxisDeadzone(0.15f);

	Pad pad;
	pad.axes[1] = -0.8f;   // stick up
	pad.Frame(actions);
	CHECK(actions.Axis("MoveForward") == doctest::Approx(0.8f)); // inverted

	pad.axes[1] = 0.1f;    // inside deadzone
	pad.Frame(actions);
	CHECK(actions.Axis("MoveForward") == doctest::Approx(0.0f));
}

TEST_CASE("keyboard and stick combine by larger magnitude")
{
	InputActions actions;
	actions.BindAxis("MoveRight", 1, 2);       // keys
	actions.BindGamepadAxis("MoveRight", 0);   // stick X

	Keys keys;
	Pad pad;

	// Stick half-deflected, no keys: stick wins.
	pad.axes[0] = 0.5f;
	pad.Frame(actions);
	keys.Frame(actions);
	CHECK(actions.Axis("MoveRight") == doctest::Approx(0.5f));

	// Key fully pressed beats a half stick.
	keys.down[1] = true;
	keys.Frame(actions);
	CHECK(actions.Axis("MoveRight") == doctest::Approx(1.0f));
}

TEST_CASE("gamepad toggle flips on button edges")
{
	InputActions actions;
	actions.BindGamepadToggle("Deform", 3);

	Pad pad;
	pad.Frame(actions);
	CHECK(!actions.Toggle("Deform"));

	pad.buttons[3] = true;
	pad.Frame(actions);
	CHECK(actions.Toggle("Deform"));

	pad.Frame(actions); // held, no re-flip
	CHECK(actions.Toggle("Deform"));

	pad.buttons[3] = false;
	pad.Frame(actions);
	pad.buttons[3] = true;
	pad.Frame(actions);
	CHECK(!actions.Toggle("Deform"));
}

TEST_CASE("disconnecting the gamepad clears its held state")
{
	InputActions actions;
	actions.BindGamepadButton("Jump", 0);
	actions.BindGamepadAxis("MoveRight", 0);

	Pad pad;
	pad.buttons[0] = true;
	pad.axes[0] = 0.9f;
	pad.Frame(actions);
	CHECK(actions.IsDown("Jump"));

	pad.Frame(actions, /*connected=*/false);
	CHECK(!actions.GamepadConnected());
	CHECK(!actions.IsDown("Jump"));
	CHECK(actions.Axis("MoveRight") == doctest::Approx(0.0f));
}
