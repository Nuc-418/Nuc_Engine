---
tags: [architecture, core]
---

# Application & Main Loop

`src/engine/core/Application.{h,cpp}`

## Members

| Member | Purpose |
|---|---|
| `window` | GLFW window wrapper (`engine/core/Window`) |
| `inputs` | Raw key/mouse state — see [[Input]] |
| `actions` | Named action bindings — see [[Input]] |
| `controller` | WASD/mouse-look helper consuming actions |
| `plugins` | [[Plugin System|PluginManager]] |
| `assets` | [[Asset System|AssetManager]] |
| `simulating` | True while gameplay advances (game always; editor only in Play) |

## Init

`Init(config)` brings up GLFW, creates the window (positioned flush to the
screen's top-left, accounting for the frame), installs the input callback,
runs GLEW, and installs the **default action bindings**: `MoveForward`
(W/S), `MoveRight` (D/A), `MoveUp` (Space/LCtrl), `Sprint` (LShift),
`Exit` (Esc).

## The frame

`Run(scene)` calls `scene.Load`, then `plugins.LoadAll`, then loops:

1. `actions.BeginFrame(...)` — snapshot key state, flip toggle latches
2. `inputs.ClearPressed()` — ready for the next poll's edges
3. `plugins.UpdateAll(app, dt)` — physics etc. **before** the scene, so the
   frame sees this frame's simulation results
4. `scene.Update(app)` → `scene.Draw(app)`
5. Swap buffers, `glfwPollEvents`
6. `Time::Update` with **wall-clock** `glfwGetTime()` deltas

## Shutdown order (important)

```
scene.Unload → plugins.UnloadAll → assets.UnloadAll → Application::Shutdown
```

Assets are freed last because scenes and components may still reference
them while unloading; everything happens before `glfwTerminate`, while the
GL context is alive.
