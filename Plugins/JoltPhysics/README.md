# JoltPhysics plugin

3D rigid-body physics for NucEngine, backed by [Jolt Physics](https://github.com/jrouwe/JoltPhysics)
(v5.3.0, MIT). This is the engine's first plugin and the reference for how a
plugin is structured (see [docs/PLUGINS.md](../../docs/PLUGINS.md)).

## Layout

```
Plugins/JoltPhysics/
  PhysicsWorld.h        Engine-facing wrapper API (Jolt-free: GLM types only)
  PhysicsWorld.cpp      The only TU that includes Jolt; builds as C++17
  JoltPhysicsPlugin.h   EnginePlugin: owns a PhysicsWorld, syncs bodies->Transforms
  JoltPhysicsPlugin.cpp
  vendor/
    Jolt/               Vendored Jolt sources (compiled into the project)
    LICENSE             Jolt's MIT license
```

Headers are included as `JoltPhysics/PhysicsWorld.h` against the
`$(ProjectDir)Plugins` include root.

## Design

- **`PhysicsWorld` hides Jolt behind a pimpl.** `PhysicsWorld.h` exposes only
  GLM types, so scene/game code can create and query bodies without pulling in
  Jolt headers or needing C++17. `PhysicsWorld.cpp` and the vendored Jolt
  sources are the only translation units compiled as C++17 (with `/sdl` off);
  that scoping lives in `NucEngine.vcxproj`.
- **`JoltPhysicsPlugin` is the engine hook.** It implements `EnginePlugin`:
  its constructor initializes the world; `OnUpdate` steps the simulation (only
  while `Application::simulating` is true, so the editor freezes it in Edit
  mode) and writes each bound body's pose back into its `Transform`.
- **Bodies bind to `Transform`s, not to `GameObject`s.** Engine core stays
  decoupled from physics: nothing in `src/engine` includes this plugin.

## Using it from a scene

```cpp
#include "JoltPhysics/JoltPhysicsPlugin.h"

// In Scene::Load (bodies can be created immediately; the world needs no GL):
JoltPhysicsPlugin& physics = app.plugins.GetOrAdd<JoltPhysicsPlugin>();
PhysicsWorld& world = physics.World();

world.CreateBox({0, -0.5f, 0}, {25, 0.5f, 25}, PhysicsWorld::Motion::Static);   // floor
auto body = world.CreateBox({0, 8, 0}, {0.5f, 0.5f, 0.5f}, PhysicsWorld::Motion::Dynamic);
physics.Bind(body, &myGameObject->transform);  // pose is written here each step
```

`Application::Run` drives the plugin lifecycle (`LoadAll` / `UpdateAll` /
`UnloadAll`) automatically; the scene only creates bodies and binds transforms.
See `DemoScene::SetupPhysicsDemo` for the falling-cube-on-a-floor demo.

## Updating Jolt

Replace `vendor/Jolt/` with the `Jolt/` folder from a newer Jolt release and
`vendor/LICENSE` with its LICENSE. The `.vcxproj` compiles
`vendor/Jolt/**/*.cpp` via a wildcard, so no file list needs editing. Keep the
Jolt build defines consistent across the whole project (we use Jolt's defaults;
they are set implicitly by not defining `JPH_DOUBLE_PRECISION`,
`JPH_DEBUG_RENDERER`, etc.).
