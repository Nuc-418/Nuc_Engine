// Editor: ImGui lifecycle, UE5-style dockspace/menu, panels and selection state.

#include "engine/editor/Editor.h"
#include "engine/editor/EditorTheme.h"
#include "engine/editor/EditorFileSystem.h"
#include "engine/editor/panels/ViewportPanel.h"
#include "engine/editor/panels/OutlinerPanel.h"
#include "engine/editor/panels/DetailsPanel.h"
#include "engine/editor/panels/LightsPanel.h"
#include "engine/editor/panels/StatsPanel.h"
#include "engine/editor/panels/ContentBrowserPanel.h"

#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include <cstdio>
#include <cstring>

bool Editor::Init(GLFWwindow* window, World* worldPtr)
{
	world = worldPtr;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.IniFilename = "editor_layout.ini";

	// Build the default layout only when no saved layout exists yet.
	FILE* iniFile = fopen(io.IniFilename, "r");
	if (iniFile)
		fclose(iniFile);
	else
		layoutNeedsBuild = true;

	ApplyEditorTheme();

	// install_callbacks=true: the backend saves the engine's key callback
	// (installed by UserInputs::AssociateWindow) and chains to it.
	if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
		return false;
	if (!ImGui_ImplOpenGL3_Init("#version 130")) {
		ImGui_ImplGlfw_Shutdown();
		return false;
	}

	if (!sceneFramebuffer.Create((int)viewportSize.x, (int)viewportSize.y))
		return false;

	return true;
}

void Editor::Shutdown()
{
	sceneFramebuffer.Unload();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Editor::DrawFrame(Application& app)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	// The selection may have been destroyed outside the panels (undo, scene
	// changes); never keep a dangling pointer into the world.
	if (selected && !world->EntryOf(selected))
		selected = nullptr;

	ImGuiID dockspaceId = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
	if (layoutNeedsBuild) {
		BuildDefaultDockLayout(dockspaceId);
		layoutNeedsBuild = false;
	}

	DrawMenuBar();
	DrawSaveAsModal();

	DrawViewportPanel(*this, app);
	DrawOutlinerPanel(*this);
	DrawDetailsPanel(*this);
	DrawLightsPanel(*this);
	DrawStatsPanel(*this);
	DrawContentBrowserPanel(*this);

	if (!playing) {
		if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S))
			saveClicked = true;
		if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Z)) {
			unsigned long long affected = undoStack.Undo(*world);
			if (affected) selected = world->FindById(affected);
		}
		if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Y) ||
		    ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z)) {
			unsigned long long affected = undoStack.Redo(*world);
			if (affected) selected = world->FindById(affected);
		}
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Editor::DrawMenuBar()
{
	if (!ImGui::BeginMainMenuBar())
		return;

	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
			saveClicked = true;
		if (ImGui::MenuItem("Save Scene As..."))
			openSaveAs = true;
		if (ImGui::BeginMenu("Open Scene")) {
			bool any = false;
			for (const DirectoryEntry& entry : ListDirectory("assets/scenes")) {
				if (entry.isDirectory || entry.name.size() < 5 || entry.name.substr(entry.name.size() - 5) != ".json")
					continue;
				any = true;
				if (ImGui::MenuItem(entry.name.c_str()))
					pendingSceneLoad = "assets/scenes/" + entry.name;
			}
			if (!any)
				ImGui::MenuItem("(no saved scenes)", NULL, false, false);
			ImGui::EndMenu();
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Exit"))
			exitClicked = true;
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Edit")) {
		if (ImGui::MenuItem("Undo", "Ctrl+Z", false, undoStack.CanUndo())) {
			unsigned long long affected = undoStack.Undo(*world);
			if (affected) selected = world->FindById(affected);
		}
		if (ImGui::MenuItem("Redo", "Ctrl+Y", false, undoStack.CanRedo())) {
			unsigned long long affected = undoStack.Redo(*world);
			if (affected) selected = world->FindById(affected);
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Window")) {
		if (ImGui::MenuItem("Reset Layout"))
			layoutNeedsBuild = true;
		ImGui::EndMenu();
	}

	ImGui::Separator();
	if (playing)
		ImGui::MenuItem("[ Playing - Esc stops ]", NULL, false, false);
	else if (ImGui::MenuItem("[ Play ]", "toolbar"))
		playClicked = true;

	ImGui::EndMainMenuBar();
}

void Editor::DrawSaveAsModal()
{
	if (openSaveAs) {
		ImGui::OpenPopup("Save Scene As");
		openSaveAs = false;
	}

	if (ImGui::BeginPopupModal("Save Scene As", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::TextUnformatted("assets/scenes/");
		ImGui::SameLine();
		ImGui::InputText("##saveAsName", saveAsBuffer, sizeof(saveAsBuffer));
		if (ImGui::Button("Save")) {
			std::string name = saveAsBuffer;
			if (name.size() < 5 || name.substr(name.size() - 5) != ".json")
				name += ".json";
			savePath = "assets/scenes/" + name;
			saveClicked = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}
