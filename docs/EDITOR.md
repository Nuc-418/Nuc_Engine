# NucEngine Editor

An in-app level editor modeled on the UE5 editor, built on the vendored
Dear ImGui docking branch, ImGuizmo and nlohmann/json.

## Using it

The app starts in **Edit mode** (1600x900, free cursor):

| Panel | What it does |
|---|---|
| **Viewport** (center) | The scene, rendered to a framebuffer at the panel's aspect ratio. **Click** an object to select it (empty space deselects). Hold **RMB** to fly with WASD/Space/Ctrl (Shift = boost), UE5-style. **W/E/R** switch the translate/rotate/scale gizmo, the corner button toggles Local/World, **F** focuses the selected object. |
| **Outliner** (right) | All world objects. Click to select, right-click or **Del** to delete, **+ Add** spawns a Cube / IndexedCube / IronMan in front of the camera. |
| **Details** (right, below) | Name and Position / Rotation (degrees) / Scale of the selection, live. |
| **Lights** (bottom) | All four light types with live parameter editing; add/remove point and spot lights (the shaders support 50 of each). |
| **Content Browser** (bottom) | Browse `assets/`; double-click a scene `.json` to load it. |
| **Stats** (bottom) | Wall-clock FPS, object count, camera position, render mode. |

**Edit > Undo / Redo** (Ctrl+Z / Ctrl+Y, also Ctrl+Shift+Z) covers transform
edits (gizmo drags and Details edits, one entry per drag), spawns, deletes,
renames and light edits (toggles, parameter drags, add/remove — recorded as
whole light-set snapshots). Objects are tracked by a stable world id, so a
delete can be undone even after further edits (the restored object reappears
at the end of the Outliner).

**File > Save Scene** (Ctrl+S) / **Save Scene As** / **Open Scene** persist the
world to versioned JSON under `assets/scenes/`. **[ Play ]** in the menu bar
enters **Play mode**, UE5 PIE-style: the demo runs live inside the Viewport
panel with the cursor captured (keys 1–4 lights, 5 distortion, 6–9 render
modes, WASD/mouse); the other panels stay visible but the mouse is owned by
the game. **Esc stops Play** and returns to the editor with the camera
restored — the app quits via File > Exit or the window close button, not Esc.

**File > Package Game...** creates a standalone, shippable build under
`Builds/<name>/`: the current world is saved as the startup scene
(`assets/scenes/startup.json`), the whole `assets/` tree is copied next to the
game executable, and the executable itself comes from the **Game|x64**
configuration (an editor-less build of the same project — `NUC_GAME_BUILD`
boots straight into the scene at 800x600 with the cursor captured, loads the
startup scene if present, and quits on Esc). Build Game|x64 in Visual Studio
once, then package as often as you like; run `NucEngineGame.exe` from its
build folder. `Builds/` is git-ignored.

Panel layout is dockable and persisted in `editor_layout.ini` (git-ignored);
**Window > Reset Layout** restores the default.

## Architecture

```
main() -> Application::Run(EditorHost)
             EditorHost (engine/editor) wraps the game Scene + its World
               Edit mode:  scene Update frozen; game.Draw -> Framebuffer -> Viewport panel
               Play mode:  game.Update runs; same Framebuffer path (play-in-viewport)
             Editor: ImGui lifecycle, theme, dockspace, panels, selection, gizmo state
             World (engine/scene): object registry the panels operate on
```

- **`World`** owns `unique_ptr<GameObject>` entries (GameObject self-references
  its transform, so objects never move in memory), plus the Lights, Camera and
  render mode. Scenes register **spawn factories** per `ObjectType`; both the
  scene's own loading and editor spawns go through them, and
  `SceneSerializer::Load` respawns saved objects the same way. `onDestroyed`
  lets DemoScene null its animation handles when the editor deletes objects.
- **Mode state machine** (`EditorHost`): UI actions are recorded during the
  ImGui pass (Draw) and consumed at the start of the next Update, so mode
  switches never happen mid-frame.
- **Input integration**: `UserInputs::AssociateWindow` installs the GLFW key
  callback exactly once, *before* `Editor::Init`, so the ImGui GLFW backend
  (install_callbacks=true) chains to it. The cursor is hidden/recentered only
  while flying or playing; Play mode additionally sets
  `ImGuiConfigFlags_NoMouse` so ImGui never fights the recenter.
- **Gizmo rotation**: the engine's Euler convention is unusual —
  `Transform::CalcRotationMatrix` builds `Ry(rot.x) * Rx(rot.y) * Rz(rot.z)`.
  `EulerYXZFromMatrix` inverts exactly that convention and is verified by a
  10,000-case round-trip self-test in `tools/lin_syntax_check/check.sh`.
- **GL context**: the window now requests 4.4 compatibility (shaders are
  `#version 440`; the engine needs 4.3+ program-resource queries) and falls
  back to a hint-less context if creation fails.

## Verification

`tools/lin_syntax_check/check.sh` (runs on Linux; the MSVC build on Windows is
authoritative): g++ -fsyntax-only over every vcxproj translation unit,
vcxproj/filters/filesystem consistency, hazard grep gates, and the Euler
self-test.

Manual checklist on Windows after building:
1. Editor opens with the default layout; demo scene visible in the Viewport.
2. RMB fly, W/E/R gizmos, F focus; click objects in the viewport to select
   (clicking the sky deselects); select/rename/spawn/delete in the Outliner.
3. Light edits take effect immediately; add/remove point and spot lights.
4. Ctrl+S then File > Open round-trips the scene; Content Browser double-click loads it.
   Ctrl+Z / Ctrl+Y undo and redo a gizmo drag, a Details edit, a spawn and a delete.
5. [ Play ]: the demo runs inside the Viewport panel with the cursor captured;
   keys 1–9 + distortion + WASD behave as in the pre-editor demo; Esc returns
   to the editor with the camera restored.
6. Build the Game|x64 configuration, then File > Package Game: the resulting
   Builds/<name>/NucEngineGame.exe runs standalone, starts in the scene that
   was open in the editor, and quits on Esc.
7. If linking fails with missing `glfwCreateStandardCursor` /
   `glfwSetCharModsCallback`: the committed `glfw3.lib` (and the GLFW headers
   on the global include path) are older than 3.2 — refresh
   `third_party/libs/x64/glfw3.lib` and the headers to GLFW 3.2+.

## Future work (not in scope)

- Multi-select, content thumbnails.
- Serializing material/texture assignments per object (types are respawned
  through factories, so per-type assets are already correct).
