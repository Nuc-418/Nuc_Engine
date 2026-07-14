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

Uploads `Model`, `View`, `ModelView`, `NormalMatrix` (packed `mat3`), `MVP`
and `CamPos` (camera world position, for the PBR view vector) per object.
Uniform locations are cached per program and invalidated when
`Shader::GlobalGeneration()` changes (hot reload can move locations).

## PBR shading (`assets/shaders/pbr`)

Primitives and models are lit by a **physically based** shader
(Cook-Torrance, metallic/roughness) in **world space**: GGX distribution +
Smith geometry + Fresnel-Schlick, `baseColor`/`metallic`/`roughness`/
`emissive`/`ao`. One shared `pbr.frag`; `pbr_primitive.vert` (pos/normal/
colour) and `pbr_model.vert` (pos/uv/normal) differ only in attributes.
Material parameters come from a sibling [[MaterialComponent]] (defaults:
white dielectric, roughness 0.6), uploaded by `MeshComponent::OnRender`;
model albedo multiplies a bound texture (`uHasAlbedoTex`). The unlit `cube`
grid shader stays for legacy debug geometry.

## HDR + tonemapping (`engine/render/PostProcess`)

The scene renders **linear HDR** into an `RGBA16F` `Framebuffer`; a fullscreen
pass (`PostProcess`, ACES filmic + gamma, `assets/shaders/post`) tonemaps it
into whatever target was bound on entry — the editor's LDR panel FBO or the
game backbuffer — so the scene draw handles HDR for both without the caller
knowing.

## Image-based lighting (`engine/render/IblEnvironment`)

Ambient comes from IBL when built: a **procedural HDR sky** is rendered to an
environment cubemap, then convolved to a diffuse **irradiance** map,
GGX-**prefiltered** into roughness mips, and paired with a **BRDF LUT**
(precompute shaders in `assets/shaders/ibl`; standard split-sum). Bound on
texture units 1/2/3 (unit 0 = model albedo); `uHasIBL` gates it, falling back
to the flat world-ambient term when no environment is present.

## Lights (`engine/render/Lights`)

Ambient/directional/point/spot structs uploaded as uniform arrays with
counts. The scene light set is `World::combinedLights`, rebuilt each frame
from the world **ambient environment term** (`World::lights`, the only
authored light left) plus every [[LightComponent]] (position/aim from the
owner's world transform). `Lights::StoreSceneLights` uploads that whole set
to each program in `World::litPrograms`, so the **same lights illuminate the
textured models and the built-in primitives** — the primitive shader reads
the identical light uniform blocks (eye-space Phong, per-vertex colour as
albedo). Add a program with `World::AddLitProgram`.

## Shaders (`engine/render/Shader`)

A managed asset: compiles/links from source files, caches uniform
`Location(name)` lookups, and supports **hot reload** — `Reload()` relinks
the *same* program object (validated in a throwaway program first, so a
broken edit keeps the old program running). Editor: **Tools → Reload
Shaders**. Shader sources live in `assets/shaders/` (see [[Asset System]]).

## Meshes, materials, textures

- `Mesh` — VAO/VBOs/EBO owner; captures a local AABB at upload (used for
  [[Editor Overview|click-picking]] and [[PhysicsBodyComponent|physics]]).
- [[MaterialComponent]] — PBR surface parameters (base color / metallic /
  roughness / emissive / ao); edited in Details.
- `Material` — legacy `.mtl` parsing (models still load it for the albedo
  texture binding).
- `Texture` — stb_image loading bound to a program's sampler (unit 0).
- `Framebuffer` — offscreen target; RGBA8 (editor panel, thumbnails) or
  RGBA16F (`hdr = true`, the scene HDR pass).
