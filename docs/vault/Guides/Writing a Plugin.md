---
tags: [guide, plugins]
---

# Writing a Plugin

1. Create `Plugins/<Name>/` (plus `vendor/` for any third-party library).
   Keep public headers free of the vendored library's headers — hide it
   behind a pimpl like `PhysicsWorld` does.

2. Implement `EnginePlugin`:

```cpp
class AudioPlugin : public EnginePlugin {
public:
    const char* Name() const override { return "Audio"; }
    const char* Version() const override { return "0.1.0"; }
    std::vector<std::string> Dependencies() const override { return {}; }

    void RegisterTypes() override {
        // component/spawn types — explicit, not static initializers
        ComponentRegistry::Register("AudioSource", "Audio Source", ...);
    }

    bool OnLoad(Application& app) override { return true; }
    void OnUpdate(Application& app, float dt) override {
        if (!app.simulating) return;   // honor the editor freeze
        ...
    }
    void OnUnload(Application& app) override {}
};
```

3. Add the plugin's `.cpp` files to `NucEngine.vcxproj` and
   `CMakeLists.txt` (`PLUGIN_SOURCES`). Scope per-file compiler settings if
   the vendored library needs them (Jolt does).

4. A scene registers it: `app.plugins.GetOrAdd<AudioPlugin>()` in
   `Scene::Load`. Dependencies-first ordering, loud failure on cycles —
   see [[Plugin System]].

`Plugins/JoltPhysics/` is the reference implementation, including a
plugin-defined component ([[PhysicsBodyComponent]]).
