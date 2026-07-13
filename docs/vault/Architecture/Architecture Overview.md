---
tags: [architecture]
---

# Architecture Overview

NucEngine is organized as four strictly layered tiers:

```
game (src/game)        → may include engine, third_party, Plugins
Plugins/*              → may include engine, third_party
engine (src/engine)    → may include third_party only — never Plugins, never game
third_party            → vendored libraries, includes nothing of ours
```

The layering is **enforced**, not aspirational: the
[[Testing & Verification Gate|verification gate]] greps for violations
(e.g. `#include "game/` inside `src/engine` fails the build check).

## The big pieces

- [[Application & Main Loop|Application]] owns the window, [[Input|input]],
  the [[Plugin System|PluginManager]] and the [[Asset System|AssetManager]],
  and drives a `Scene` through its loop.
- A **Scene** (`engine/core/Scene.h`) is the game-facing interface:
  `Load / Update / Draw / Unload`. The demo game implements it
  (`game/DemoScene`), and the editor wraps any scene in `EditorHost`
  without the game knowing (see [[Editor Overview]]).
- The [[World]] is the registry of live [[GameObject & Hierarchy|GameObjects]]
  plus the scene-level state (lights, cameras, render mode).
- Behaviour and data live in [[Components]]; the engine core knows only the
  base class, so [[Plugin System|plugins]] can define their own component
  types (e.g. [[PhysicsBodyComponent]]).

## Design rules worth knowing

- **Explicit `Unload`, not destructors.** GL resources are freed while the
  context is still current (`Scene::Unload`, `Mesh::Unload`,
  `Shader::Unload`); destructors would run after `glfwTerminate`.
- **Behavior preservation.** Every refactor phase keeps the demo visually
  identical and the gate green (see [[Roadmap Status]]).
- **String type-ids everywhere.** Spawnable types, component types and
  prefabs are keyed by strings, so nothing needs an enum or switch to grow.
- **The `simulating` flag** (`Application::simulating`) is the editor/game
  seam: gameplay ([[Components#Behavior hooks|behaviors]], physics) only
  advances while it is true.
