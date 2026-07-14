// Editor: ImGui lifecycle, UE5-style dockspace/menu, panels and selection state.

#pragma once

#include <map>
#include <string>

#include "imgui/imgui.h"
#include "ImGuizmo/ImGuizmo.h"
#include "engine/render/Framebuffer.h"
#include "engine/scene/World.h"
#include "engine/editor/UndoStack.h"
#include "engine/editor/EditorCommand.h"
#include "engine/scene/FieldStore.h"

class Application;
struct GLFWwindow;

class Editor
{
public:
	// Call after Application::Init (the engine key callback must already be
	// installed so ImGui's GLFW backend chains to it).
	bool Init(GLFWwindow* window, World* worldPtr);
	void Shutdown();

	// Full UI: dockspace, menu bar, all panels, ImGuizmo. In play mode the
	// viewport shows the running game and editing interactions are disabled.
	void DrawFrame(Application& app);

	// Deferred intents queued by menus/panels during Draw and executed by
	// EditorHost on the next Update. Panels enqueue here instead of poking
	// editor fields directly.
	EditorCommandQueue commands;

	// UI-intent helpers panels call to open a modal (kept off the raw ImGui
	// popup ids so callers don't duplicate the id strings).
	void OpenNewMapDialog();
	void RequestDeleteMap(std::string path);

	// -- cross-frame state read/consumed by EditorHost --
	bool playing = false;          // set by EditorHost while in play mode
	bool viewportFlying = false;   // RMB held over the viewport
	float flySpeed = 5.0f;         // WASD fly speed; mouse wheel adjusts it while flying
	std::string savePath = "assets/scenes/demo_scene.json";  // current document

	// -- shared editor state --
	World* world = nullptr;
	GameObject* selected = nullptr;
	UndoStack undoStack;

	// In-flight edit capture (gizmo drag / Details widget drag).
	bool gizmoDragging = false;
	unsigned long long dragTargetId = 0;
	TransformState dragBefore = {};
	std::string nameBefore;      // Details name field capture
	VectorLight lightsBefore;    // Lights panel drag capture
	FieldStore componentBefore;  // Details component-editor capture (one edit at a time)
	ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE gizmoMode = ImGuizmo::WORLD;
	Framebuffer sceneFramebuffer;
	ImVec2 viewportSize = ImVec2(1280, 720);
	std::string contentPath = "assets";

	// Content Browser mesh thumbnails, keyed by type id, rendered once on first use.
	std::map<std::string, Framebuffer> meshPreviews;
	bool meshPreviewsReady = false;

private:
	void DrawMenuBar();
	void DrawTitleBarControls();
	void DrawSaveAsModal();
	void DrawPackageModal();

	GLFWwindow* windowHandle = nullptr;  // for the custom title bar controls
	bool layoutNeedsBuild = false;
	char saveAsBuffer[128] = "demo_scene.json";
	char packageNameBuffer[128] = "GameBuild";
	std::string packageStatus;
	std::string mapDeleteRequest;  // map path awaiting delete confirmation
	std::string pendingPopup;      // modal id to OpenPopup at top-level scope next draw
	void DrawMapModals();
	char newMapBuffer[128] = "NewMap";
};

// Deletes an object from the editor's world: clears selection if it was
// selected, records the delete for undo, then destroys it. Shared by the
// Outliner (context menu) and the global Delete shortcut so both paths behave
// identically. Safe to pass null.
void DeleteObject(Editor& editor, GameObject* object);
