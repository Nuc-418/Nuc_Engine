---
tags: [component, rendering]
---

# MaterialComponent

`src/engine/render/MaterialComponent.{h,cpp}` · TypeId `"Material"`

PBR surface parameters (metallic/roughness workflow) for an actor. Attach it
alongside a [[MeshComponent]]; `MeshComponent::OnRender` reads the sibling
component and uploads its values as the `pbrMaterial.*` uniforms each frame.
No MaterialComponent = sensible defaults (white dielectric, roughness 0.6).

| Field | Notes |
|---|---|
| `baseColor` | albedo tint (color picker); multiplies vertex colour / albedo texture |
| `metallic` | 0 = dielectric, 1 = metal |
| `roughness` | 0 = mirror, 1 = fully rough (clamped ≥ 0.04) |
| `emissive` | self-illumination color, added after lighting |
| `ao` | ambient-occlusion multiplier on the IBL/ambient term |

Serialize/Deserialize drive the auto-generated Details editor and undo, like
any component ([[Serialization & Reflection]]). See [[Rendering]] for the
Cook-Torrance shader, HDR tonemapping and IBL that consume these.

> To see the material model: drop a **Sphere**, add a Material component, and
> set metallic 1 / low roughness for a chrome look that reflects the IBL sky.
