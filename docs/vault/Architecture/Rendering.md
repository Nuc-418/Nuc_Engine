---
tags: [architecture, rendering]
---

# Rendering

The renderer is currently **immediate-mode component drawing** — each
[[MeshComponent]] issues its own GL in `OnRender`. A retained render-queue
rework is the roadmap's Phase 3 (see [[Roadmap Status]]).

## The draw path

```
Scene::Draw
  └─ per object: GameObject::Draw(renderMode, camera)
       └─ MeshComponent::OnRender
            └─ MeshRenderer::Draw(mode, camera, owner->WorldMatrix())
                 ├─ glUseProgram / glBindVertexArray
                 ├─ Camera::CamToProgram(program, model)   // uniforms
                 └─ glDrawArrays / glDrawElements
```

While simulating, the scene renders through `World::ActiveCamera()`
(a placed [[CameraComponent]]) when one is set; otherwise `World::camera`.

## Camera (`engine/render/Camera`)

Uploads `Model`, `View`, `ModelView`, `NormalMatrix` (packed `mat3`) and
`MVP` per object. Uniform locations are cached per program and invalidated
when `Shader::GlobalGeneration()` changes (hot reload can move locations).

## Lights (`engine/render/Lights`)

Ambient/directional/point/spot structs uploaded as uniform arrays with
counts. Two sources merge into `World::combinedLights` each frame:
world-level authored lights and every [[LightComponent]] (position/aim from
the owner's world transform). `StorePrimitiveLight` feeds the simple
primitive shader the first directional + ambient light.

## Shaders (`engine/render/Shader`)

A managed asset: compiles/links from source files, caches uniform
`Location(name)` lookups, and supports **hot reload** — `Reload()` relinks
the *same* program object (validated in a throwaway program first, so a
broken edit keeps the old program running). Editor: **Tools → Reload
Shaders**. Shader sources live in `assets/shaders/` (see [[Asset System]]).

## Meshes, materials, textures

- `Mesh` — VAO/VBOs/EBO owner; captures a local AABB at upload (used for
  [[Editor Overview|click-picking]] and [[PhysicsBodyComponent|physics]]).
- `Material` — `.mtl` parsing and `material.*` uniform upload.
- `Texture` — stb_image loading bound to a program's sampler.
- `Framebuffer` — offscreen target for the editor viewport and thumbnails.
