// Editor: ImGui lifecycle, UE5-style dockspace/menu, panels and selection state.

#pragma once

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

	// Full edit-mode UI: dockspace, menu bar, all panels, ImGuizmo.
	void DrawFrame(Application& app);

	// Minimal play-mode frame (keeps the backend event queue drained).
	void DrawPlayOverlay();

	// -- cross-frame state read/consumed by EditorHost --
	bool viewportFlying = false;   // RMB held over the viewport
	bool playClicked = false;
	bool exitClicked = false;
	bool saveClicked = false;
	std::string savePath = "assets/scenes/demo_scene.json";
	std::string pendingSceneLoad;  // set by Content Browser / File > Open

	// -- shared editor state --
	World* world = nullptr;
	GameObject* selected = nullptr;
	UndoStack undoStack;

	// In-flight edit capture (gizmo drag / Details widget drag).
	bool gizmoDragging = false;
	unsigned long long dragTargetId = 0;
	TransformState dragBefore = {};
	ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE gizmoMode = ImGuizmo::WORLD;
	Framebuffer sceneFramebuffer;
	ImVec2 viewportSize = ImVec2(1280, 720);
	std::string contentPath = "assets";

private:
	void DrawMenuBar();
	void DrawSaveAsModal();

	bool layoutNeedsBuild = false;
	bool openSaveAs = false;
	char saveAsBuffer[128] = "demo_scene.json";
};
