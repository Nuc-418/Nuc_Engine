---
tags: [architecture, assets]
---

# Asset System

## AssetManager (`engine/asset/AssetManager`)

One instance lives on [[Application & Main Loop|Application]]
(`app.assets`). Path-keyed caches — repeated `Load*` calls return the same
shared object:

- `LoadShader(vertPath, fragPath)` → [[Rendering|Shader]]* (null on compile
  failure)
- `LoadTexture(program, path)` → Texture*

`UnloadAll()` frees every GL object at shutdown (called by
`Application::Run` after the scene and plugins unload) — scenes never
delete shaders or textures by hand.

## Primitives (`engine/render/Primitives`)

Engine-owned CPU meshes for the lit primitive shader: `PlaneMesh`,
`CubeMesh`, `SphereMesh`, `CylinderMesh`, `ConeMesh`, `GroundMesh`
(checkerboard floor). Built once (function-local statics), shared by all
spawn factories. Unit-tested invariants: whole triangles, matching
attribute counts, unit normals.

## Model discovery (`engine/io/ModelDiscovery`)

`DiscoverObjFiles(root, out)` recursively collects every `.obj` under
`assets/models` — each becomes a spawnable type automatically (drop a model
folder in, it appears in the Content Browser). `FindTextureInFolder` grabs
the first image next to a model. Runs on `std::filesystem`.

## Asset paths

Runtime paths are relative to the **repo root** (run executables from
there). `game/AssetPaths.h` centralizes the demo's literals:
`assets/shaders/…`, `assets/models/…`. Scenes save to `assets/scenes/`,
[[Prefabs]] to `assets/prefabs/`.
