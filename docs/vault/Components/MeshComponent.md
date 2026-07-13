---
tags: [component, rendering]
---

# MeshComponent

`src/engine/render/MeshComponent.{h,cpp}` · TypeId `"Mesh"`

The renderable: a `MeshRenderer` (VAO + shader program + draw call) and a
`Material`. `OnRender` draws with the owner's **world** matrix, so child
meshes render attached (see [[GameObject & Hierarchy]]).

## Authoring (used by spawn factories)

- `LoadObj(program, folderPath, fileName)` — loads an `.obj` + its `.mtl`
- `CreatePosColor / CreatePosNormColor / CreatePosUvNorm(program, arrays…)`
  — upload raw vertex arrays (Mesh copies to the GPU immediately)

Factories reach these via `GameObject::EnsureMesh()`. Geometry comes from
the factory, not the scene file — a registry-created MeshComponent starts
empty, which is why scene load reconciles instead of recreating (see
[[Serialization & Reflection]]).

`OnUnload` frees the mesh's GL objects. No serialized state of its own.
