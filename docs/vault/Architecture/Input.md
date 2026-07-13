---
tags: [architecture, input]
---

# Input

Two layers: raw state and named bindings. Gameplay code never hardcodes keys.

## UserInputs (`engine/input/UserInputs`) — raw state

- Key state table: `keyDown[key]` (held) + `keyPressed[key]` (edge since the
  last clear), indexed by `GLFW_KEY_*`. The callback finds its instance via
  the **GLFW window user pointer** — no singleton, multiple windows possible.
- Mouse-look deltas by cursor recentering: `UpdateMouse(true)` measures the
  offset from the window center and warps back (`CenterCursor`,
  `SetCursorCaptured`).

## InputActions (`engine/input/InputActions`) — named bindings

GLFW-free (sees only ints) and unit-tested. Three binding kinds:

| Kind | Bind | Query |
|---|---|---|
| Action | `BindAction("Sprint", key)` | `IsDown` (held), `WasPressed` (edge) |
| Axis | `BindAxis("MoveForward", posKey, negKey)` | `Axis` → −1..+1 |
| Toggle | `BindToggle("ToggleDeform", key)` | `Toggle` (latch inverts per press), `SetToggle` (force, e.g. editor sync) |

`Application::Run` snapshots the raw state into the actions once per frame
(`BeginFrame`), which also flips toggle latches, then clears the edges.

## Default bindings

Installed by `Application::Init`: `MoveForward` W/S · `MoveRight` D/A ·
`MoveUp` Space/LCtrl · `Sprint` LShift · `Exit` Esc. Scenes add their own
in `Scene::Load` (the demo binds light toggles 1–4, deform 5, render modes
6–9). See [[Controls & Keybindings]].

## Controller (`engine/input/Controller`)

`BasicMovement(transform, rotSpeed, moveSpeed)` — mouse-look plus the
movement axes (Sprint doubles speed). Used by the demo's fly camera and the
editor's RMB viewport fly. `RequestExit(window)` closes cleanly on "Exit".
