// Content Browser panel: browse assets/; double-click a scene .json to load it.

#include "engine/editor/panels/ContentBrowserPanel.h"
#include "engine/editor/Editor.h"
#include "engine/editor/EditorFileSystem.h"

static bool HasExtension(const std::string& name, const char* extension)
{
	std::string ext = extension;
	return name.size() >= ext.size() && name.compare(name.size() - ext.size(), ext.size(), ext) == 0;
}

void DrawContentBrowserPanel(Editor& editor)
{
	ImGui::Begin("Content Browser");

	ImGui::TextDisabled("%s", editor.contentPath.c_str());
	if (editor.contentPath != "assets") {
		ImGui::SameLine();
		if (ImGui::SmallButton("..")) {
			size_t slash = editor.contentPath.find_last_of('/');
			editor.contentPath = (slash == std::string::npos) ? "assets" : editor.contentPath.substr(0, slash);
		}
	}
	ImGui::Separator();

	for (const DirectoryEntry& entry : ListDirectory(editor.contentPath)) {
		std::string label = (entry.isDirectory ? "[dir]  " : "       ") + entry.name;
		bool selected = false;
		ImGui::Selectable(label.c_str(), &selected, ImGuiSelectableFlags_AllowDoubleClick);
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			if (entry.isDirectory)
				editor.contentPath += "/" + entry.name;
			else if (HasExtension(entry.name, ".json"))
				editor.pendingSceneLoad = editor.contentPath + "/" + entry.name;
		}
	}

	ImGui::End();
}
