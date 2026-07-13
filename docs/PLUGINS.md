# NucEngine plugin system

NucEngine keeps optional features it does not itself depend on — physics, and
in future audio, scripting, networking — out of engine core and behind a small
plugin interface. Engine core never references a concrete plugin, so a feature
can be added or removed without touching core.

## Pieces

- **`EnginePlugin`** (`src/engine/plugin/Plugin.h`) — the base interface. Hooks:
  `OnLoad`, `OnUpdate(app, deltaTime)`, `OnUnload`, plus `Name()`.
- **`PluginManager`** (`src/engine/plugin/PluginManager.h`) — owns plugins,
  keyed by concrete type. `GetOrAdd<T>()` fetches or lazily creates one;
  `Get<T>()` retrieves it; `LoadAll`/`UpdateAll`/`UnloadAll` drive the lifecycle.
- **`Application`** owns one `PluginManager plugins` and a `bool simulating`
  flag. `Application::Run`:
  1. `scene.Load(app)` — scenes register plugins here via `app.plugins.GetOrAdd<T>()`.
  2. `plugins.LoadAll(app)` — `OnLoad` for everything the scene registered.
  3. each frame: `plugins.UpdateAll(app, dt)` **before** `scene.Update`, so the
     scene and draw see this frame's plugin results.
  4. `scene.Unload(app)` then `plugins.UnloadAll(app)`.

## The `simulating` flag

`Application::simulating` marks whether gameplay/simulation is advancing this
frame. A standalone game leaves it `true`; the editor (`EditorHost`) sets it
`false` in Edit mode and `true` in Play mode. Simulation plugins must check it
and skip their step when it is `false` — that is how the editor freezes physics
while you are editing. It is a generic flag: engine core never names a plugin.

## Layering

```
game / scene   -> may include engine, third_party, and Plugins
Plugins/*      -> may include engine and third_party
engine (src)   -> may include third_party only — never Plugins
```

Because core cannot include a plugin, a plugin's public header must not force
core to depend on the plugin's third-party library. The JoltPhysics plugin does
this by hiding Jolt behind a pimpl (`PhysicsWorld`), so only the plugin's own
`.cpp` files see Jolt.

## Writing a plugin

1. Create `Plugins/<Name>/` with a `src/` (and `vendor/` for any third-party
   library). Keep public headers free of the vendored library's headers.
2. Implement `EnginePlugin`.
3. Add the plugin's `.cpp` files to `NucEngine.vcxproj` (include roots
   `$(ProjectDir)Plugins` and any `vendor` path are already configured; add
   more as needed). Scope compiler settings (e.g. `LanguageStandard`) per file
   if the library needs them, as the Jolt sources do.
4. A scene registers it with `app.plugins.GetOrAdd<YourPlugin>()`.

See `Plugins/JoltPhysics/` for the reference implementation.
