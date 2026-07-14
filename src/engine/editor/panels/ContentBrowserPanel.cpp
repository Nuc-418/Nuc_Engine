// Content Browser panel: browse assets/; double-click a scene .json to load it.

#include "engine/editor/panels/ContentBrowserPanel.h"
#include "engine/editor/Editor.h"
#include "engine/editor/EditorFileSystem.h"
#include "engine/editor/MeshPreview.h"

#include <algorithm>
#include <vector>

static bool HasExtension(const std::string& name, const char* extension)
{
	std::string ext = extension;
	return name.size() >= ext.size() && name.compare(name.size() - ext.size(), ext.size(), ext) == 0;
}

void DrawContentBrowserPanel(Editor& editor)
{
	ImGui::Begin("Content Browser");

	/* Place Actors palette: every registered mesh/model, drawn from the World's
	   dynamic type registry — no hardcoded list. Drag a thumbnail into the
	   viewport to spawn it. */
	ImGui::TextDisabled("Place Actors  (drag a mesh into the viewport)");

	const std::vector<std::string>& palette = editor.world->TypeIds();

	// Render the thumbnails once, the first time this panel is drawn (the GL
	// context and the spawn factories are both ready by now).
	if (!editor.meshPreviewsReady) {
		MeshPreview::Generate(editor.meshPreviews, *editor.world, palette, 72);
		editor.meshPreviewsReady = true;
	}

	const float thumb = 72.0f;
	const float cellW = thumb + 14.0f;
	int perRow = std::max(1, (int)(ImGui::GetContentRegionAvail().x / cellW));
	int shown = 0;
	for (const std::string& type : palette) {
		const std::string& label = editor.world->TypeLabel(type);

		ImGui::PushID(type.c_str());
		ImGui::BeginGroup();

		GLuint tex = 0;
		auto it = editor.meshPreviews.find(type);
		if (it != editor.meshPreviews.end())
			tex = it->second.colorTexture;

		if (tex)
			ImGui::ImageButton("thumb", (ImTextureID)(intptr_t)tex, ImVec2(thumb, thumb), ImVec2(0, 1), ImVec2(1, 0));
		else
			ImGui::Button(label.c_str(), ImVec2(thumb, thumb));

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
			// Payload is the type id string (null-terminated).
			ImGui::SetDragDropPayload("NUC_SPAWN_TYPE", type.c_str(), type.size() + 1);
			ImGui::Text("Place %s", label.c_str());
			ImGui::EndDragDropSource();
		}
		ImGui::TextUnformatted(label.c_str());

		ImGui::EndGroup();
		ImGui::PopID();

		if (++shown % perRow != 0)
			ImGui::SameLine();
	}
	if (shown % perRow != 0)
		ImGui::NewLine();
	ImGui::Separator();

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
				editor.commands.Push(EditorCommandType::LoadScene, editor.contentPath + "/" + entry.name);
		}
	}

	ImGui::End();
}
