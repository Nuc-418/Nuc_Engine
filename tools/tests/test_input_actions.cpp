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
