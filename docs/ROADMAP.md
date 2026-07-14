# NucEngine Modular-Engine Roadmap

A deep review of the current engine structure and a phased plan of reworks
and features toward a properly modular engine. Each phase is sequenced so the
demo keeps running unchanged at every step, and each item names the code it
touches.

## Where the engine stands

The bones are genuinely good for an engine at this stage:

- **Clean layering** (`game -> Plugins -> engine -> third_party`) that is
  actually enforced by the Linux gate, not just documented.
- **Actor + Component model** with a string-keyed `ComponentRegistry`, so
  plugins can add component types without core knowing them.
- **Plugin system** with a lifecycle (`OnLoad`/`OnUpdate`/`OnUnload`), a
  pimpl'd reference plugin (JoltPhysics), and the `simulating` flag as a
  clean editor/game seam.
- **Serialization behind engine-owned interfaces** (`ISerializer`), so
  components never see nlohmann directly.
- **Editor as a Scene wrapper** (`EditorHost`), so the game runs standalone
  with zero editor code.

The weaknesses cluster in five places, and they gate almost every future
feature:

1. **Rendering is immediate-mode and self-service.** Every `MeshComponent`
   issues its own GL in `OnRender`; raw `GLuint` program ids thread through
   `GameObject::LoadObjFile`, `MeshRenderer::program`, `World::lightsProgram`
   and the `Lights` uniform helpers. There is no frame graph, no sorting, no
   batching, and uniform-location caches that "must never be invalidated"
   are load-bearing. Nothing renderer-shaped can be added (shadows,
   post-processing, a second view) without first separating *what to draw*
   from *how to draw it*.

2. **There is no asset system.** Meshes, textures and shader programs are
   loaded ad hoc; `Mesh` is copied by value with a manual `Unload` foot-gun
   (documented in RESTRUCTURE.md); primitive geometry lives in function-local
   statics inside `DemoScene.cpp`; `vector<glm::vec3>*` pointers with unclear
   ownership cross the `Mesh::Assign*` API. Prefabs, content hot-reload and
   editor thumbnails all want a real asset layer.

3. **Transform has no hierarchy.** `Transform` is Euler-only, public-field,
   recompute-on-demand, and `GameObject`s cannot parent each other. The
   default axes are also inconsistent (`forward = {1,0,0}` vs
   `defaultForward = {0,0,1}`). Attachment, skeletal anything, and sane
   editor gizmo behavior on children all need a proper hierarchy.

4. **World mixes concerns.** It is the object registry *and* the owner of
   lights, camera and `renderMode`. Lights are engine-global structs uploaded
   per program rather than components, which is why the editor needs a
   dedicated Lights panel instead of the Details panel.

5. **Modularity is link-time only.** Plugins are compiled in and keyed by
   `std::type_index`; registration order defines update order; component
   registration rides on static initializers (which MSVC will dead-strip in
   a static-lib future); there are no plugin dependencies, versions, or
   dynamic loading.

Cross-cutting hygiene: `using namespace std/glm` in public headers
(`GameObject.h`, `Mesh.h`, `Lights.h`), the C++14/v141 ceiling (no
`std::filesystem` — already worked around in `EditorFileSystem`), static
singletons (`Time`, the `UserInputs` callback pointer), hardcoded key fields
(`keyW`..`key9`, `onceKey1`..`onceKey9`), and a Linux gate whose consistency
lists have drifted from the Plugins restructure.

## Phase 0 — Foundations (hygiene, unblockers) — DONE

Small, low-risk, and everything later leans on it.

- **Repair the verification gate**: add `-IPlugins` and the Jolt include
  path to `check.sh`, replace the literal `vendor\Jolt\**\*.cpp` vcxproj glob
  with real entries (or teach the consistency check to expand it), and add
  the new engine/Plugins files to the filters check. The gate is the safety
  net for every phase after this one.
- **Bump to C++17** (v141 supports it; the CMake build already exists).
  Unlocks `std::filesystem` (replacing the Win32 code in
  `EditorFileSystem`), `optional`, `variant`, `string_view`. Update the gate
  to `-std=c++17`.
- **De-pollute public headers**: remove `using namespace std/glm` from
  `GameObject.h`, `Mesh.h`, `Lights.h`, `Material.h`; qualify names.
  Mechanical but touches many files — do it before the big reworks, not
  during.
- **Fix `Transform` axis defaults** (`forward`/`right` initializers disagree
  with `defaultForward`/`defaultRight`) and make `UpdateModel` the only
  public recompute path.
- **Normalize source encoding to UTF-8** (`.gitattributes` +
  `/utf-8` MSVC flag) so tooling stops corrupting latin1 comments.
- **Add a unit-test target** (doctest vendored, one header) wired into
  `check.sh` next to the euler self-test: Transform math, World
  spawn/destroy/undo identity, serializer round-trip. These are the systems
  the reworks below rewrite; tests come first.

## Phase 1 — Scene graph and transforms — DONE (quaternions deferred)

Goal: hierarchy and rotation done right, because retrofitting it later
touches everything.

*Status: shipped — hierarchy with keep-world reparenting (Outliner
drag-and-drop, undoable), LightComponent and CameraComponent with an active
scene camera, RTTI-free GetComponent. The quaternion storage rework is
deferred: the tested matrix/Euler decompose path covers everything the
hierarchy needs, and quaternions pay off more alongside the Phase 3 renderer
work.*

- **Quaternion-backed `Transform`** with cached local/world matrices and
  dirty flags. Keep Euler get/set for the editor UI (it already has
  decompose helpers in `EditorMath`).
- **Parent/child hierarchy on `GameObject`**: `SetParent`, ordered children,
  world = parentWorld * local. `World::entries` stays the flat owning store;
  hierarchy is pointers over it. Serializer writes a `parent` id; Outliner
  becomes a tree; ImGuizmo edits convert through the parent's inverse.
- **Camera and lights become components.** `CameraComponent` (World keeps an
  "active camera" id) and `LightComponent` (one type, a kind enum:
  directional/point/spot). `World::lights`, `lightsProgram` and the Lights
  panel migrate to components + Details panel; a scene-lighting collector
  gathers `LightComponent`s each frame (interim: it uploads through the
  existing `Lights` uniform code, so shaders don't change yet).
- **`GetComponent` without `dynamic_cast`**: give `Component` a static
  type-id per class (the registry id it already has) and match on it;
  keeps the API, drops RTTI dependence.

Exit criteria: demo scene identical; an object parented under another moves
with it; lights editable through Details like any component.

## Phase 2 — Asset system — IN PROGRESS

Goal: named, shared, ref-counted resources; no raw `GLuint`s crossing module
boundaries.

- **`AssetManager` engine service** (owned by `Application`) with typed
  handles: `MeshHandle`, `TextureHandle`, `ShaderHandle`, `MaterialHandle`.
  Path-keyed cache, explicit `Load`/`Get`, refcount or generation-checked
  handles. GPU objects are freed by the manager at shutdown — the
  `Mesh::Unload`-by-hand era ends. *(First cut done: app.assets owns
  path-keyed Shader and Texture caches, freed by Application::Run at
  shutdown; mesh handles arrive with the Phase 3 render-queue work.)*
- **`ShaderAsset`**: wraps program creation (absorbing `LoadShaders`),
  exposes uniform setting; owns the location caching that today lives in
  `Camera`, `Time` and `Lights`, and invalidates it on reload. Add
  file-watch hot-reload in the editor (cheap once locations are owned here).
  *(Done as `engine/render/Shader`: LoadShaders removed, reload keeps
  program ids stable, Camera/Time/Lights caches are generation-invalidated,
  and Tools > Reload Shaders hot-reloads in the editor.)*
- **Primitive mesh library moves into the engine** (`engine/render/
  Primitives`): the cube/plane/cone/... generators currently in
  `DemoScene.cpp` become engine mesh assets; `DemoScene::LoadObjects`
  shrinks to registrations. Model discovery (`DiscoverObjFiles`) moves to
  the asset manager as a content scan. *(Done: Primitives and ModelDiscovery
  are engine modules; discovery runs on std::filesystem.)*
- **`Mesh` API rework**: `Assign*(vector<...>*)` becomes value/`span`-style
  upload with clear CPU-side ownership; `MaterialInfo materialInfo = {NULL}`
  and `loadMaterial(char*)` get typed, path-based replacements.
- **`GameObject` sheds mesh authoring**: `LoadObjFile`/`CreateObj*` move to
  a `MeshComponent` builder or free functions in the asset layer; the actor
  stops knowing how meshes are made (`GetMesh` convenience stays). *(Done:
  authoring lives on MeshComponent as LoadObj/Create*.)*

Exit criteria: no `GLuint program` parameter anywhere outside
`engine/render`; deleting the demo still leaves a working engine + editor.

## Phase 3 — Renderer rework

Goal: retained submission, one place that talks to GL. This is the largest
phase; it depends on Phases 1–2.

- **Split visit from draw.** `Component::OnRender(GLenum, Camera*)` is
  replaced by `Collect(RenderQueue&)`: mesh components submit
  `RenderItem { meshHandle, materialHandle, worldMatrix }`. A `Renderer`
  sorts items (by shader, then material), binds once per group, and draws.
  `World::renderMode` becomes a renderer debug setting.
- **Per-frame uniform blocks.** Camera matrices, time and the collected
  lights go into UBOs bound once per frame, replacing per-object
  `CamToProgram`/`TimeToProgram`/`Toggle*Light` calls and their caches.
  Shaders update to a shared `#include`-style prelude (assets already load
  from disk, so this is a content change).
- **Render targets as first-class**: `Framebuffer` grows into a render-pass
  concept (the editor viewport and `MeshPreview` already render offscreen —
  they become the first two passes). This is the hook for shadow maps and
  post-processing later, without building a full frame graph now.
- **Thin GL wrapper** (`engine/render/rhi`): buffer/texture/program/vao
  creation and state changes behind small functions. Not a multi-backend
  RHI yet — just a single choke point so one can grow later.

Exit criteria: identical visuals; a `StatsPanel` draw-call counter shows
sorting works; adding "render the scene again from a second camera" is a
ten-line change.

## Phase 4 — Plugin system v2 — CORE DONE

Goal: from "compiled-in modules" to a real module system.

- **Explicit registration over static initializers**: each plugin exports a
  `RegisterPlugin(PluginRegistrar&)` function that declares its name,
  version, dependencies, components (into `ComponentRegistry`) and spawn
  types. Immune to static-init order and dead-stripping; one visible list
  per plugin. *(Done as EnginePlugin::RegisterTypes + Version +
  Dependencies; JoltPhysics registers PhysicsBody there.)*
- **Dependency-ordered lifecycle**: `PluginManager` topo-sorts by declared
  dependencies for `LoadAll`/`UpdateAll` (today: registration order) and
  fails loudly on cycles/missing deps. *(Done: stable Kahn sort in
  PluginSort, unit-tested; unload runs the reverse order.)*
- **Engine services registry**: `Application` exposes named services
  (assets, renderer, physics query interface) so plugins talk to interfaces
  rather than `Get<ConcreteType>()`. Keep `GetOrAdd<T>` as typed sugar.
  *(Done: `ServiceRegistry` — an interface-keyed, non-owning locator on
  `Application::services`. Init publishes the `AssetManager`; the JoltPhysics
  plugin implements the engine-owned `IPhysicsService` and Provides it in
  OnLoad / Withdraws in OnUnload, so engine/editor code adjusts physics
  through the interface without knowing Jolt. `GetOrAdd<T>` stays as typed
  sugar for concrete-type access. Unit-tested, incl. polymorphic retrieval.)*
- **`PhysicsBodyComponent`** in the JoltPhysics plugin (the docs already
  promise it): body kind + shape from the mesh AABB, `Serialize`d, editable
  in Details via the registry — replacing `DemoScene::SetupPhysicsDemo`'s
  hand-wiring and `Bind/UnbindTransform` bookkeeping. *(Done: the demo's
  floor/cube now carry the component; bodies follow objects in Edit mode
  and round-trip through scene files. Details-panel parameter editing
  waits on Phase 6 reflection.)*
- **(Stretch) DLL plugins**: the registrar function is `extern "C"`-able, so
  dynamic loading becomes a loader detail. Only worth it once a second real
  plugin exists; static linking remains the default.

Exit criteria: JoltPhysics registers itself through the new path; a physics
body added in the editor round-trips through save/load with zero
scene-code involvement.

## Phase 5 — Gameplay and input — IN PROGRESS

- **Input action mapping**: replace the `keyW`..`key9`/`onceKey*` fields
  with named actions and axis bindings (`"MoveForward" -> W/S`), polled or
  event-subscribed; per-window instance instead of the file-static callback
  pointer; gamepad via GLFW joystick API. `Controller` becomes a consumer
  of actions. *(Done: InputActions with actions/axes/toggle latches,
  per-window callback via the GLFW user pointer, engine-default + scene
  bindings, unit-tested. Gamepad done via the GLFW 3.2 joystick API:
  gamepad buttons/toggles and analog axes bind to the same action names as
  the keyboard (Application polls joystick 1 each frame; press edges +
  deadzone in SetGamepadState; keyboard/stick combine by larger magnitude),
  with engine-default stick/button binds. Unit-tested.)*
- **Behavior components**: a `ScriptableComponent` base with
  `OnPlayBegin`/`OnUpdate`/`OnPlayEnd` (only active while
  `app.simulating`), so gameplay is written as components — the demo's
  wave animation in `DemoScene::Update` becomes the first behavior.
  *(Done as base-Component hooks: OnPlayBegin/OnSimulate/OnPlayEnd
  dispatched by World::Tick and NotifyPlayBegin/End (editor Play/Stop and
  game boot). RotatorComponent is the first behavior — the Iron Man spins
  are components now, serializable and editable. Behaviors read input via a
  `BehaviorContext` (input actions + world) threaded through
  Tick -> Simulate -> OnSimulate; RotatorComponent's optional `activeWhile`
  action gate is the first input-reading behavior, unit-tested.)*
- **(Later, as a plugin)** a Lua scripting plugin exposing the component
  and input APIs — the real proof that Phase 4's seams are sufficient.

## Phase 6 — Editor and content workflow — DONE

- **Property reflection for components**: extend the `ISerializer` visitor
  so a component's `Serialize` doubles as a property enumeration
  (name/type/get/set). DetailsPanel then auto-generates UI for *any*
  registered component — including plugin ones — and per-component editor
  code stops accumulating. *(Done: WriteColor/WriteEnum hints on
  ISerializer, PropertyCapture/PropertyApply in the Details panel; the
  hand-written Light/Camera editors are gone and PhysicsBody is editable.)*
- **Generalized undo**: `UndoStack` currently special-cases transforms,
  names and lights. With reflection, an edit becomes "component state
  before/after" (two serialized blobs) and one undo entry type covers every
  component forever. *(Done for component properties: ComponentEdit actions
  snapshot before/after FieldStores and re-apply via Deserialize.)*
- **Prefabs**: save a `GameObject` (its component array) as an asset;
  spawning a prefab instance goes through the same reconciliation path the
  scene loader already has. `World::RegisterType`'s hand-written factories
  gradually become prefab references. *(Done: Outliner right-click > Save
  as Prefab writes assets/prefabs/<name>.prefab.json — base type +
  components + rotation/scale — and every prefab registers as a
  "prefab:<name>" spawn type, so Add menu, Content Browser, drag-drop,
  undo and scene files all work unchanged. Prefabs can be based on
  prefabs.)*
- **Editor state cleanup**: `Editor`'s public flag-soup (`playClicked`,
  `pendingSceneLoad`, `mapDeleteRequest`, ...) becomes a small
  command/event queue consumed by `EditorHost` — panels stop reaching into
  a god object. *(Done: `EditorCommand`/`EditorCommandQueue` — menus and
  panels `Push` deferred intents (Play/Exit/SaveScene/NewMap/LoadScene/
  DeleteMap) during Draw; `EditorHost::Update` drains and executes them, so
  UI code never mutates the world mid-frame. Modal-open requests defer
  through one `pendingPopup` buffer opened at the correct ImGui ID scope.
  Unit-tested queue semantics.)*

## Phase 7 — Platform and build convergence

- **CMake as the source of truth** (it already builds everything);
  the vcxproj is generated or retired. The gate then checks CMake targets
  instead of parsing the vcxproj.
- **Platform seams**: `WindowChrome` (Win32 custom title bar) behind an
  interface with a no-op fallback; `Main.cpp`'s console positioning stays
  `#ifdef`'d. With Phase 0's `std::filesystem` and the already-GLFW input,
  a Linux build of engine + demo becomes realistic; the editor follows.
- **Packaged-build hardening**: `GamePackager` consumes the asset manager's
  manifest instead of copying directory trees.

## Phase 8 — Render Hardware Interface (RHI) + Vulkan backend

Goal: a backend abstraction so the renderer stops calling `glXxx` directly, then
a **Vulkan** backend behind it. This is the largest single initiative here. It
extends Phase 1 (the render seam) and Phase 3 (the shader/UBO change) — which are
its real prerequisites — and is sequenced so the OpenGL path keeps rendering the
demo unchanged until Vulkan reaches parity. Context: the metallic/roughness PBR
pipeline (Cook-Torrance + HDR tonemapping + IBL) currently lives directly on raw
GL, which is exactly what this phase abstracts.

**Why the uniform model is the crux.** The renderer leans on things Vulkan does
not have: runtime-compiled GLSL, name-based reflection
(`glGetProgramResourceLocation`), and thousands of loose `glProgramUniform*`
calls (`Camera::CamToProgram`, `Lights::StoreSceneLights`, the MeshComponent
material upload, `IblEnvironment`). Vulkan takes **SPIR-V** only and has **no
loose uniforms** — everything is UBOs / push constants / descriptor sets.
Restructuring that path is most of the work and is backend-agnostic, so it is
done on the known-good GL backend first and pixel-compared before Vulkan begins.

### 8.1 — RHI seam (GL backend only)

- `engine/rhi/`: a `Device` (creates resources, submits work) + a
  `CommandContext`.
- Opaque resource handles: `Buffer`, `Texture`, `Sampler`, `ShaderModule`,
  `Pipeline` (immutable state — blend/depth/raster/vertex layout), `RenderTarget`.
- Backend-neutral enums/formats, so no `GLenum` leaks above the backend.
- `rhi/gl/`: wrap the current GL calls, then port `Mesh`, `MeshRenderer`,
  `Framebuffer`, `Texture`, `Shader`, `PostProcess`, `IblEnvironment` and every
  `OnRender` onto the RHI. No visible change.

### 8.2 — Uniforms → UBO / descriptor model + SPIR-V (still GL)

- A per-frame **scene UBO** (view/projection, `CamPos`, the combined light set)
  and a per-object **push-constant/UBO** (model matrix + `pbrMaterial.*`);
  textures (albedo, IBL cubemaps, BRDF LUT) as **descriptor sets**. Replaces the
  loose `glProgramUniform*` uploads.
- Shader pipeline: author GLSL, compile to **SPIR-V** in the build
  (glslang/shaderc), reflect descriptor layouts (spirv-reflect). GL 4.6 consumes
  SPIR-V (`GL_ARB_gl_spirv`), so both backends share one shader artifact.
- Keep the old uniform path compiled until the UBO path is pixel-compared, then
  delete it (the Phase 3 mitigation).

### 8.3 — Vulkan backend (`rhi/vk/`)

Built incrementally, each step a runnable checkpoint:

- Instance (+ validation layers), physical/logical device, queues; **volk** loader.
- **Swapchain** + resize recreation; per-frame command buffers; frames-in-flight
  (image-available / render-finished semaphores + in-flight fences).
- **Dynamic rendering** (KHR) in preference to render passes + framebuffers.
- Graphics **pipelines** created from the RHI `Pipeline` (state baked at creation).
- **Descriptor** set layouts / pools / sets for the UBOs + samplers from 8.2.
- Memory via **VMA** (Vulkan Memory Allocator).
- Milestones: clear screen → triangle → textured mesh → full scene parity
  (PBR + HDR + IBL) → render the viewport to an offscreen image.

### 8.4 — Editor + backend selection

- Swap `imgui_impl_opengl3` → `imgui_impl_vulkan`; the FBO-in-a-panel trick
  becomes an offscreen image sampled as a descriptor.
- Backend chosen at launch (e.g. `--backend=gl|vk`); the GL path stays as the
  reference / fallback.

**New dependencies:** Vulkan SDK (LunarG: headers, validation layers,
glslang/shaderc, SPIRV-Tools), VMA, volk, spirv-reflect. Build wiring in the
vcxproj and `CMakeLists.txt` (Phase 7's "CMake as source of truth" helps here).

**Scope decision — settle this first:**

- **Proof-of-concept** — Vulkan renders the 3D scene into the viewport; the
  editor stays on GL; no live switching. Much smaller; recommended first target.
- **Full parity** — editor and everything on Vulkan, backend-switchable at
  launch. The largest effort in the engine (weeks).

## Sequencing and risk

```
P0 ──> P1 ──> P2 ──> P3 ──> (P5, P6 in parallel)
        │      │      └────> P4
        │      └────> P8.1 ─> P8.2 ─> P8.3 ─> P8.4   (RHI + Vulkan)
        └── P7 anytime after P0, cheapest after P3
```

- Phases 0–1 are small (days each); 2 and 4 medium; 3 and 6 are the big
  ones. Every phase ends with the demo visually identical and the gate
  green — the same behavior-preserving discipline RESTRUCTURE.md
  established.
- **Phase 8 (RHI + Vulkan)** is the largest and sits on P1 + P3: do 8.1–8.2
  first (they harden the GL engine on their own and are the real prerequisite);
  8.2 is the riskiest (touches every shader + upload, de-risked by staying on GL
  and pixel-comparing); 8.3 is the biggest volume (~a few thousand lines) but
  additive, so low-risk to the existing GL path.
- The riskiest step is Phase 3's shader/UBO change (it *can* alter
  visuals). Mitigation: keep the old per-uniform path compiled until the
  UBO path is pixel-compared on hardware, then delete it.
- Biggest payoff-to-effort: Phase 0's gate repair (protects everything),
  Phase 2 (unlocks 3, 6, and 7's packaging), and Phase 6's reflection
  (deletes editor code instead of adding it).

## Backlog / future exploration

Ideas parked outside the phased plan — not scheduled, recorded so they are
not lost.

### 3D Gaussian Splatting (3DGS)

A radiance-field scene representation (Kerbl et al., SIGGRAPH 2023): the scene
is millions of oriented, translucent 3D Gaussians (position + scale + rotation
quaternion + opacity + spherical-harmonic, view-dependent color) rather than
triangles. It renders by projecting each Gaussian to a 2D "splat", depth-sorting
them, and alpha-compositing — so it is *a rasterizer*, not ray tracing, and runs
in real time. Data is reconstructed from photos (SfM init + gradient-descent
optimization against posed images), not modeled by hand.

Possible fit here: a self-contained `GaussianSplatComponent` that loads a `.ply`
of trained splats and renders on its **own** path (instanced quads/point sprites,
a fragment shader that evaluates the 2D Gaussian falloff + SH color, depth-read /
no-write, blending on, a per-frame GPU depth sort). It would bypass the lighting
system entirely — splats carry baked radiance. Depends on nothing in the phased
plan, but a real renderer seam (Phase 1/3) makes adding a second primitive path
much cleaner.

**Recent variants (each targets one weakness):**

- **2D Gaussian Splatting** and **SuGaR** — surface-aligned Gaussians that allow
  extracting a real **mesh** (recovers collision/physics geometry).
- **Mip-Splatting** — anti-aliasing across scales.
- **Compression** methods — cut the (hundreds-of-MB) storage/memory cost.
- **4D / dynamic Gaussian Splatting** — animated scenes / video.
- **Relightable Gaussians** — early work to make splats respond to lights.
- **Engine/web integrations** — Unreal/Unity plugins and three.js / WebGPU viewers.

**Strengths:**

- Photorealistic novel-view synthesis; real-time (100+ FPS) rasterization.
- Fast to train (minutes) vs NeRF (hours); explicit primitives you can inspect
  and move (unlike NeRF's implicit MLP).
- GPU-friendly; the compositing is the same *over* operator as ordinary alpha
  blending.

**Weaknesses (engine-relevant):**

- **No geometry** — no surface/normals/UVs/connectivity, so no collision or
  physics without a mesh-extraction step (2DGS/SuGaR).
- **Baked lighting** — color is a captured radiance field, so splats ignore the
  scene lights; can't relight or drop into a dynamic lit scene naturally.
- **Ordering/compositing** — must re-sort every frame; mixing splats with opaque
  triangle geometry through one depth buffer is fiddly.
- **Large storage/memory**, and aliasing artifacts up close.
