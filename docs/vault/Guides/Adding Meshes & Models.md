---
tags: [guide, assets]
---

# Adding Meshes & Models

## Drop-in models (no code)

Put a folder containing an `.obj` (+ `.mtl`, + a texture image) anywhere
under `assets/models/`. On the next run it is discovered
([[Asset System|ModelDiscovery]]), registered as a spawnable type, and
appears in the Content Browser with a thumbnail. The first image file in
the folder binds as its texture.

## New built-in primitive (one row + a generator)

1. Write a generator in `engine/render/Primitives.cpp` returning a
   `PrimitiveMesh` (positions + outward normals + colors; winding doesn't
   matter, back-face culling is off for primitives).
2. Declare it in `Primitives.h`.
3. Add one row to the `primitives[]` table in `DemoScene::LoadObjects`.

## Custom geometry in code

```cpp
GameObject* obj = world.Spawn("Cube");                 // or a fresh factory
obj->EnsureMesh().CreatePosNormColor(program, &pos, &norm, &col);
```

See [[MeshComponent]] for the authoring API and [[Prefabs]] for turning a
configured object into a reusable asset.
