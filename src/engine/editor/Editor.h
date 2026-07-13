// Editor: ImGui lifecycle, UE5-style dockspace/menu, panels and selection state.

#pragma once

#include <map>
#include <string>

#include "imgui/imgui.h"
#include "ImGuizmo/ImGuizmo.h"
#include "engine/render/Framebuffer.h"
#include "engine/scene/World.h"
#include "engine/editor/UndoStack.h"

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

	// -- cross-frame state read/consumed by EditorHost --
	bool playing = false;          // set by EditorHost while in play mode
	bool viewportFlying = false;   // RMB held over the viewport
	float flySpeed = 5.0f;         // WASD fly speed; mouse wheel adjusts it while flying
	bool playClicked = false;
	bool exitClicked = false;
	bool saveClicked = false;
	std::string savePath = "assets/scenes/demo_scene.json";
	std::string pendingSceneLoad;  // set by Content Browser / File > Open / Maps panel
	std::string pendingNewMap;     // path of a map to create-and-switch-to

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
	bool openSaveAs = false;
	char saveAsBuffer[128] = "demo_scene.json";
	bool openPackage = false;
	char packageNameBuffer[128] = "GameBuild";
	std::string packageStatus;
public:
	bool openNewMap = false;       // set by the Maps panel / File menu
	std::string mapDeleteRequest;  // map path awaiting delete confirmation
private:
	void DrawMapModals();
	char newMapBuffer[128] = "NewMap";
};
