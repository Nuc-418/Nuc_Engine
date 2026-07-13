---
tags: [architecture, plugins]
---

# Plugin System

Optional features the engine does not itself depend on (physics — later
audio, scripting, networking) live under `Plugins/` behind a small
interface. Engine core never references a concrete plugin.

## EnginePlugin (`engine/plugin/Plugin.h`)

| Member | Purpose |
|---|---|
| `Name()` / `Version()` | Stable identity for logs and dependencies |
| `Dependencies()` | Names of plugins that must load/update first |
| `RegisterTypes()` | Called once at registration, **before** any `OnLoad`: register component and spawn types here (not in static initializers, which a static-lib link can strip) |
| `OnLoad(app)` | Once after the scene loads, GL context current |
| `OnUpdate(app, dt)` | Every frame, before the scene updates |
| `OnUnload(app)` | After the main loop, GL context still alive |

Simulation plugins must honor `app.simulating` (skip stepping when false)
so the editor can freeze them in Edit mode.

## PluginManager (`engine/plugin/PluginManager.h`)

- `GetOrAdd<T>()` — fetch or lazily construct (calls `RegisterTypes`);
  `Get<T>()` retrieves. Scenes register plugins in `Scene::Load`.
- `LoadAll` / `UpdateAll` run **dependencies-first** (stable topological
  sort, `engine/plugin/PluginSort` — unit-tested); `UnloadAll` runs the
  reverse. Missing dependencies and cycles are logged loudly; offenders
  fall back to registration order.

## The reference plugin: JoltPhysics

`Plugins/JoltPhysics/` — three layers:

- **PhysicsWorld** — Jolt behind a pimpl (only `PhysicsWorld.cpp` sees Jolt
  headers). `Init/Step/CreateBox/CreateSphere/RemoveBody/GetPosition/
  GetRotation/SetPose`.
- **JoltPhysicsPlugin** — owns a PhysicsWorld; steps it while simulating;
  writes dynamic body poses back into bound `Transform`s; realizes and
  syncs [[PhysicsBodyComponent]]s each update. Exposes a static
  `Instance()` (interim seam until a service registry exists).
- **PhysicsBodyComponent** — registered in `RegisterTypes()`.

See [[Writing a Plugin]] for the how-to.
