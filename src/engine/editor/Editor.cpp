// Editor: ImGui lifecycle, UE5-style dockspace/menu, panels and selection state.

#include "engine/editor/Editor.h"
#include "engine/render/Shader.h"
#include "engine/editor/EditorTheme.h"
#include "engine/editor/EditorFileSystem.h"
#include "engine/editor/GamePackager.h"
#include "engine/editor/panels/ViewportPanel.h"
#include "engine/editor/panels/OutlinerPanel.h"
#include "engine/editor/panels/DetailsPanel.h"
#include "engine/editor/panels/LightsPanel.h"
#include "engine/editor/panels/StatsPanel.h"
#include "engine/editor/panels/ContentBrowserPanel.h"
#include "engine/editor/panels/MapsPanel.h"
#include "engine/platform/WindowChrome.h"

#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstring>

void DeleteObject(Editor& editor, GameObject* object)
{
	if (!object)
		return;

	if (editor.selected == object)
		editor.selected = nullptr;

	const WorldEntry* entry = editor.world->EntryOf(object);
	if (entry)
		editor.undoStack.RecordDelete(entry->typeId, object->name, entry->id, CaptureTransform(*object));

	editor.world->Destroy(object);
}

bool Editor::Init(GLFWwindow* window, World* worldPtr)
{
	world = worldPtr;
	windowHandle = window;

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

	// Drop the OS title bar and draw our own (see DrawMenuBar). Must come after
	// the ImGui backend so our window procedure wraps the backend's.
	WindowChrome::Install(window);

	if (!sceneFramebuffer.Create((int)viewportSize.x, (int)viewportSize.y))
		return false;

	return true;
}

void Editor::Shutdown()
{
	for (auto& preview : meshPreviews)
		preview.second.Unload();
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

	DrawViewportPanel(*this, app);
	DrawMapsPanel(*this);
	DrawOutlinerPanel(*this);
	DrawDetailsPanel(*this);
	DrawLightsPanel(*this);
	DrawStatsPanel(*this);
	DrawContentBrowserPanel(*this);

	// Modals draw after the menu and panels so a popup requested by either this
	// frame takes effect immediately. OpenPopup must run at the same ID-stack
	// scope as the matching BeginPopupModal, so requests from menus/panels are
	// deferred here (a menu/window pushes its own ID scope) rather than opened
	// in place.
	if (!pendingPopup.empty()) {
		ImGui::OpenPopup(pendingPopup.c_str());
		pendingPopup.clear();
	}
	DrawSaveAsModal();
	DrawPackageModal();
	DrawMapModals();

	if (!playing) {
		if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S))
			commands.Push(EditorCommandType::SaveScene);
		if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Z)) {
			unsigned long long affected = undoStack.Undo(*world);
			if (affected) selected = world->FindById(affected);
		}
		if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Y) ||
		    ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z)) {
			unsigned long long affected = undoStack.Redo(*world);
			if (affected) selected = world->FindById(affected);
		}

		/* Delete the selected object from anywhere (viewport, outliner, ...).
		   Guarded against typing in a text field so Del in a name box edits
		   text instead of destroying the object. */
		if (selected && ImGui::IsKeyPressed(ImGuiKey_Delete) && !ImGui::GetIO().WantTextInput)
			DeleteObject(*this, selected);
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Editor::DrawMenuBar()
{
	if (!ImGui::BeginMainMenuBar())
		return;

	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem("New Map..."))
			OpenNewMapDialog();
		if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
			commands.Push(EditorCommandType::SaveScene);
		if (ImGui::MenuItem("Save Scene As..."))
			pendingPopup = "Save Scene As";
		if (ImGui::BeginMenu("Open Scene")) {
			bool any = false;
			for (const DirectoryEntry& entry : ListDirectory("assets/scenes")) {
				if (entry.isDirectory || entry.name.size() < 5 || entry.name.substr(entry.name.size() - 5) != ".json")
					continue;
				any = true;
				if (ImGui::MenuItem(entry.name.c_str()))
					commands.Push(EditorCommandType::LoadScene, "assets/scenes/" + entry.name);
			}
			if (!any)
				ImGui::MenuItem("(no saved scenes)", NULL, false, false);
			ImGui::EndMenu();
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Package Game...")) {
			packageStatus.clear();
			pendingPopup = "Package Game";
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Exit"))
			commands.Push(EditorCommandType::Exit);
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Tools")) {
		// Recompiles every live Shader in place (program ids stay stable);
		// a broken edit keeps the old program running and logs the error.
		if (ImGui::MenuItem("Reload Shaders"))
			Shader::ReloadAll();
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
		commands.Push(EditorCommandType::Play);

	DrawTitleBarControls();

	ImGui::EndMainMenuBar();
}

// Right side of the main menu bar: the centered window title plus custom
// minimize / maximize / close buttons, drawn to match the editor theme. The
// empty strip between the menus and the buttons is the draggable caption
// (see WindowChrome); the layout is published there each frame.
void Editor::DrawTitleBarControls()
{
	const float barHeight = ImGui::GetFrameHeight();
	const float menuRight = ImGui::GetItemRectMax().x;  // right edge of [ Play ]
	const ImVec2 winPos = ImGui::GetWindowPos();
	const float winW = ImGui::GetWindowWidth();
	ImDrawList* draw = ImGui::GetWindowDrawList();

	// Centered title. Drawn (not a widget) so it never eats caption drags.
	const char* title = playing ? "NucEngine  -  Playing" : "NucEngine Editor";
	const ImVec2 titleSize = ImGui::CalcTextSize(title);
	draw->AddText(ImVec2(winPos.x + (winW - titleSize.x) * 0.5f,
	                     winPos.y + (barHeight - titleSize.y) * 0.5f),
	              ImGui::GetColorU32(ImGuiCol_Text, 0.55f), title);

	const float buttonW = 46.0f;
	const float startX = winW - buttonW * 3.0f;
	ImGui::SameLine(startX);
	const float buttonsLeft = ImGui::GetCursorScreenPos().x;

	auto controlButton = [&](const char* id, int glyph) -> bool {
		const ImVec2 p0 = ImGui::GetCursorScreenPos();
		const ImVec2 size(buttonW, barHeight);
		const bool clicked = ImGui::InvisibleButton(id, size);
		const bool hovered = ImGui::IsItemHovered();

		ImU32 fg = IM_COL32(224, 224, 224, 255);
		if (hovered) {
			const ImU32 bg = (glyph == 2) ? IM_COL32(232, 17, 35, 255)   // close: red
			                              : IM_COL32(255, 255, 255, 28);  // subtle
			draw->AddRectFilled(p0, ImVec2(p0.x + size.x, p0.y + size.y), bg);
			if (glyph == 2) fg = IM_COL32(255, 255, 255, 255);
		}

		const ImVec2 c(p0.x + size.x * 0.5f, p0.y + size.y * 0.5f);
		const float r = 5.0f;
		if (glyph == 0) {                                   // minimize
			draw->AddLine(ImVec2(c.x - r, c.y), ImVec2(c.x + r, c.y), fg, 1.0f);
		} else if (glyph == 1) {                            // maximize / restore
			if (glfwGetWindowAttrib(windowHandle, GLFW_MAXIMIZED)) {
				draw->AddRect(ImVec2(c.x - r + 2, c.y - r), ImVec2(c.x + r, c.y + r - 2), fg, 0, 0, 1.0f);
				draw->AddRectFilled(ImVec2(c.x - r, c.y - r + 2), ImVec2(c.x + r - 2, c.y + r), IM_COL32(0, 0, 0, 0), 0);
				draw->AddRect(ImVec2(c.x - r, c.y - r + 2), ImVec2(c.x + r - 2, c.y + r), fg, 0, 0, 1.0f);
			} else {
				draw->AddRect(ImVec2(c.x - r, c.y - r), ImVec2(c.x + r, c.y + r), fg, 0, 0, 1.0f);
			}
		} else {                                            // close
			draw->AddLine(ImVec2(c.x - r, c.y - r), ImVec2(c.x + r, c.y + r), fg, 1.2f);
			draw->AddLine(ImVec2(c.x - r, c.y + r), ImVec2(c.x + r, c.y - r), fg, 1.2f);
		}
		return clicked;
	};

	if (controlButton("##min", 0))
		glfwIconifyWindow(windowHandle);
	ImGui::SameLine(0.0f, 0.0f);
	if (controlButton("##max", 1)) {
		if (glfwGetWindowAttrib(windowHandle, GLFW_MAXIMIZED))
			glfwRestoreWindow(windowHandle);
		else
			glfwMaximizeWindow(windowHandle);
	}
	ImGui::SameLine(0.0f, 0.0f);
	if (controlButton("##close", 2))
		commands.Push(EditorCommandType::Exit);

	WindowChrome::SetTitleBar(barHeight, menuRight, buttonsLeft);
}

void Editor::DrawSaveAsModal()
{
	if (ImGui::BeginPopupModal("Save Scene As", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::TextUnformatted("assets/scenes/");
		ImGui::SameLine();
		ImGui::InputText("##saveAsName", saveAsBuffer, sizeof(saveAsBuffer));
		if (ImGui::Button("Save")) {
			std::string name = saveAsBuffer;
			if (name.size() < 5 || name.substr(name.size() - 5) != ".json")
				name += ".json";
			commands.Push(EditorCommandType::SaveScene, "assets/scenes/" + name);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

void Editor::DrawPackageModal()
{
	if (ImGui::BeginPopupModal("Package Game", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::TextUnformatted("Creates Builds/<name>/ with the game executable,");
		ImGui::TextUnformatted("the assets folder and the current scene as startup scene.");
		ImGui::TextUnformatted("Requires the Game|x64 configuration to be built first.");
		ImGui::Separator();

		ImGui::TextUnformatted("Builds/");
		ImGui::SameLine();
		ImGui::InputText("##packageName", packageNameBuffer, sizeof(packageNameBuffer));

		if (ImGui::Button("Package")) {
			PackageResult result = PackageGame(*world, packageNameBuffer);
			packageStatus = result.message;
		}
		ImGui::SameLine();
		if (ImGui::Button("Close"))
			ImGui::CloseCurrentPopup();

		if (!packageStatus.empty()) {
			ImGui::Separator();
			ImGui::PushTextWrapPos(420.0f);
			ImGui::TextWrapped("%s", packageStatus.c_str());
			ImGui::PopTextWrapPos();
		}
		ImGui::EndPopup();
	}
}

void Editor::OpenNewMapDialog()
{
	pendingPopup = "New Map";
}

void Editor::RequestDeleteMap(std::string path)
{
	mapDeleteRequest = std::move(path);
	pendingPopup = "Delete Map";
}

void Editor::DrawMapModals()
{
	/* New map */
	if (ImGui::BeginPopupModal("New Map", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::TextUnformatted("Creates an empty map (default lights) and switches to it.");
		ImGui::TextUnformatted("Unsaved changes in the current map are discarded.");
		ImGui::Separator();
		ImGui::TextUnformatted("assets/scenes/");
		ImGui::SameLine();
		ImGui::InputText("##newMapName", newMapBuffer, sizeof(newMapBuffer));
		if (ImGui::Button("Create")) {
			std::string name;
			for (char c : std::string(newMapBuffer))
				if (c != '/' && c != '\\' && c != ':')
					name += c;
			if (name.size() < 5 || name.substr(name.size() - 5) != ".json")
				name += ".json";
			if (name != ".json")
				commands.Push(EditorCommandType::NewMap, "assets/scenes/" + name);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	/* Delete map confirmation (opened via RequestDeleteMap) */
	if (ImGui::BeginPopupModal("Delete Map", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Delete %s?", mapDeleteRequest.c_str());
		ImGui::TextUnformatted("The file is removed permanently. The world you are");
		ImGui::TextUnformatted("editing stays open (save it to recreate the file).");
		ImGui::Separator();
		if (ImGui::Button("Delete")) {
			commands.Push(EditorCommandType::DeleteMap, mapDeleteRequest);
			mapDeleteRequest.clear();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			mapDeleteRequest.clear();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}
