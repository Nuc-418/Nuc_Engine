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

Lights are **component-only**: directional/point/spot lights are always
LightComponents on actors. The only world-level light is the ambient
environment term (edited in the **Environment** panel). Each frame
`World::SyncComponentLights` merges the ambient term with every
LightComponent into `combinedLights` and re-uploads to every lit program
**only when something changed**. Because the primitive and model shaders
share the same light uniforms, a Light actor lights everything — primitives
included. A meshless `"Light"` spawn type is registered, and meshless
actors are pickable in the viewport (a small default box), so a light can be
selected, moved, aimed and deleted like any object.

See also [[Rendering]] and the Environment panel in [[Editor Overview]].
