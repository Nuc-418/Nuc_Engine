---
tags: [guide]
---

# Adding a Component Type

1. **Subclass `Component`** with a unique TypeId:

```cpp
class HealthComponent : public Component {
public:
    float hitPoints = 100.0f;

    static const char* StaticTypeId() { return "Health"; }   // GetComponent<T>
    const char* TypeId() const override { return StaticTypeId(); }
    const char* DisplayName() const override { return "Health"; }

    void OnSimulate(float dt) override { /* gameplay here */ }

    void Serialize(ISerializer& out) const override { out.Write("hp", hitPoints); }
    void Deserialize(const IDeserializer& in) override { hitPoints = in.ReadFloat("hp", hitPoints); }
};
```

2. **Register it** so scene load and the editor can create it by id:
   - Engine component: a static initializer in the `.cpp`
     (`ComponentRegistry::Register("Health", "Health", factory)` — see
     `RotatorComponent.cpp`).
   - Plugin component: register inside `EnginePlugin::RegisterTypes()`
     (see [[Writing a Plugin]]).

3. That's it. You automatically get:
   - **Editor UI** in the Details panel (one widget per serialized field —
     use `WriteColor`/`WriteEnum` hints for nicer widgets)
   - **Undo** for every edit ([[Undo System|ComponentEdit]])
   - **Scene file + prefab round-tripping**
   - The **Add Component** menu entry

Pick the right hooks: `OnUpdate` runs every frame; `OnSimulate` and
`OnPlayBegin/End` only while playing ([[Components#Behavior hooks]]);
`OnRender` for drawing; `OnUnload` for GL/external resources.

Add the new files to `NucEngine.vcxproj` (+ `.filters`) — the
[[Testing & Verification Gate|gate]] fails if a tracked source is missing
from the project.
