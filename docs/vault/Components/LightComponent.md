---
tags: [component, rendering]
---

# LightComponent

`src/engine/render/LightComponent.{h,cpp}` · TypeId `"Light"`

Turns an actor into a light source. **Placement comes from the owner's
transform**: world position for point/spot, world +Z forward for aim —
move/rotate the object to place the light (parenting works).

| Field | Notes |
|---|---|
| `kind` | Directional / Point / Spot (enum combo in Details) |
| `on` | switch |
| `ambient/diffuse/specular` | colors (color pickers via `WriteColor`) |
| `constant/linear/quadratic` | point/spot attenuation |
| `cutOff` | spot cone half-angle, radians |

Each frame `World::SyncComponentLights` merges every LightComponent after
the world-level authored lights into `combinedLights` and re-uploads
through the existing uniform path **only when something changed** — the
shaders are unchanged. The demo registers a meshless `"Light"` spawn type.

See also [[Rendering]] and the Lights panel in [[Editor Overview]].
