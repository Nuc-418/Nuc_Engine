---
tags: [architecture, scene]
---

# GameObject & Hierarchy

`src/engine/scene/GameObject.{h,cpp}` — an **actor**: a `Transform` root
plus a list of [[Components]]. Heap-owned by the [[World]], never copied.

## Transform (local!)

`Transform` stores `position`, Euler `rotation` (radians; `x` = yaw about Y,
`y` = pitch about X, `z` = roll about Z — the rotation matrix is
`Ry(x)·Rx(y)·Rz(z)`) and `scale`. `UpdateModel()` recomputes
`model = T·R·S` and the local axes. `SetFromMatrix(m)` decomposes a TRS
matrix back into the fields (used by reparenting).

With the hierarchy, the transform is **local to the parent**:

```
world = parentWorld * local        // GameObject::WorldMatrix()
```

## Parenting

- `SetParent(newParent, keepWorldTransform = true)` — nullptr = root.
  Refuses self/descendant parents (cycles). With `keepWorldTransform`, the
  local transform is rebased through the parent's inverse so the object
  does not move on screen.
- Links are non-owning; ownership stays flat in `World::entries`.
- Destroying a parent hands its children to the grandparent, world pose kept.
- Scene files store a `parent` id per object (see [[Scene File Format]]).
- The editor's Outliner reparents by drag-and-drop (see [[Editor Overview]]).

> [!note] TRS limitation
> Rebasing decomposes the local matrix to position/Euler/scale, so a
> non-uniform-scaled, rotated parent can introduce shear the TRS form
> cannot represent — standard TRS-engine behavior.

## Component access

- `AddComponent<T>(args...)` / `GetComponent<T>()` — typed; `GetComponent`
  matches on the component's stable TypeId (`T::StaticTypeId()`), no RTTI.
- `AddComponentById(id)` / `GetComponentById(id)` — string-keyed, used by
  scene load and the editor.
- `GetMesh()` / `EnsureMesh()` — [[MeshComponent]] convenience.

## Dispatch

`Update(dt)`, `Simulate(dt)`, `PlayBegin()`, `PlayEnd()`, `Draw(mode, camera)`
and `Unload()` forward to every component in order.
