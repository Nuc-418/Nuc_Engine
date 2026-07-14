---
tags: [editor]
---

# Editor Overview

The editor is a **Scene wrapper**: `EditorHost` wraps any game scene, so the
game runs standalone with zero editor code. It opens on a clean default map
(ground + directional light); the demo's spawn factories stay registered.
UE5-style docked layout built on Dear ImGui + ImGuizmo.

## Modes

- **Edit** — `app.simulating = false`: physics and behaviors frozen; the
  world still ticks `OnUpdate`. The viewport renders through the editor
  camera.
- **Play (PIE)** — the game runs inside the Viewport panel. Camera pose is
  saved on entry and restored on Stop; `World::NotifyPlayBegin/End` fire
  around it. **Esc stops Play.** With an active [[CameraComponent]], Play
  renders through it.

## Panels

| Panel | What it does |
|---|---|
| **Viewport** | Scene image (offscreen framebuffer). RMB-hold to fly (WASD, wheel = speed); W/E/R = translate/rotate/scale gizmo; F = focus selection; LMB click-picks (OBB ray test); drag types from the Content Browser to spawn at the ground hit; Local/World gizmo toggle. Gizmo edits convert through the parent's inverse for children. |
| **Outliner** | The scene hierarchy tree. Click select, **+ Add** spawns any registered type. Drag object onto object to attach (world pose kept, undoable); drop below the tree or right-click → *Detach from parent*. Right-click → **Save as Prefab** (see [[Prefabs]]). Del deletes the selection (works from any panel). |
| **Details** | Name, local transform, and an **auto-generated editor for every component** via [[Serialization & Reflection|reflection]] — plus *Make/Clear Active Camera* and *Add Component*. |
| **Content Browser** | Registered spawn types with rendered mesh thumbnails; drag into the viewport. |
| **Environment** | The world ambient term (color + on). Directional/point/spot lights are Light components — add a Light actor and edit it in Details. |
| **Maps** | Scenes in `assets/scenes`: create, switch (double-click), delete. |
| **Stats** | Frame stats and render-mode picker. |

## Menus

- **File** — New Map, Save Scene (Ctrl+S), Save Scene As, Open Scene,
  **Package Game** (copies a standalone build + `startup.json`), Exit
- **Edit** — Undo (Ctrl+Z) / Redo (Ctrl+Y) — see [[Undo System]]
- **Tools** — **Reload Shaders** (hot reload, stable program ids)
- **Window** — Reset Layout
