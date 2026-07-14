// Maps panel: list the maps in assets/scenes — create, switch, delete.

#include "engine/editor/panels/MapsPanel.h"
#include "engine/editor/Editor.h"
#include "engine/editor/EditorFileSystem.h"

static bool IsJson(const std::string& name)
{
	return name.size() >= 5 && name.compare(name.size() - 5, 5, ".json") == 0;
}

void DrawMapsPanel(Editor& editor)
{
	ImGui::Begin("Maps");

	if (ImGui::Button("+ New Map"))
		editor.OpenNewMapDialog();
	ImGui::SameLine();
	ImGui::TextDisabled("double-click to switch");
	ImGui::Separator();

	bool any = false;
	for (const DirectoryEntry& entry : ListDirectory("assets/scenes")) {
		if (entry.isDirectory || !IsJson(entry.name))
			continue;
		any = true;

		std::string path = "assets/scenes/" + entry.name;
		bool isCurrent = (path == editor.savePath);
		std::string label = entry.name + (isCurrent ? "  (open)" : "");

		ImGui::PushID(path.c_str());
		ImGui::Selectable(label.c_str(), isCurrent, ImGuiSelectableFlags_AllowDoubleClick);
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && !isCurrent)
			editor.commands.Push(EditorCommandType::LoadScene, path); // unsaved changes discarded (Ctrl+S first)

		if (ImGui::BeginPopupContextItem("map_context")) {
			if (ImGui::MenuItem("Load", NULL, false, !isCurrent))
				editor.commands.Push(EditorCommandType::LoadScene, path);
			if (ImGui::MenuItem("Delete..."))
				editor.RequestDeleteMap(path);
			ImGui::EndPopup();
		}
		ImGui::PopID();
	}
	if (!any)
		ImGui::TextDisabled("No maps yet. Create one, or Ctrl+S saves the current world.");

	ImGui::End();
}
