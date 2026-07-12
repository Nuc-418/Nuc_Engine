// Outliner panel: scene object list — select, spawn, delete.

#include "engine/editor/panels/OutlinerPanel.h"
#include "engine/editor/Editor.h"

static void DestroyObject(Editor& editor, GameObject* object)
{
	if (editor.selected == object)
		editor.selected = nullptr;
	editor.world->Destroy(object);
}

void DrawOutlinerPanel(Editor& editor)
{
	ImGui::Begin("Outliner");
	World& world = *editor.world;

	if (ImGui::Button("+ Add"))
		ImGui::OpenPopup("outliner_add");
	if (ImGui::BeginPopup("outliner_add")) {
		const ObjectType types[] = { ObjectType::Cube, ObjectType::IndexedCube, ObjectType::IronMan };
		for (ObjectType type : types) {
			if (ImGui::MenuItem(ToString(type), NULL, false, world.CanSpawn(type))) {
				GameObject* spawned = world.Spawn(type);
				if (spawned) {
					// Drop new objects in front of the camera.
					Transform& cam = world.camera.transform;
					spawned->transform.SetPos(cam.position + cam.forward * 6.0f);
					editor.selected = spawned;
				}
			}
		}
		ImGui::EndPopup();
	}
	ImGui::SameLine();
	ImGui::TextDisabled("%d objects", (int)world.entries.size());
	ImGui::Separator();

	GameObject* toDestroy = nullptr;

	for (WorldEntry& entry : world.entries) {
		GameObject* object = entry.object.get();
		ImGui::PushID(object);

		if (ImGui::Selectable(object->name.c_str(), editor.selected == object))
			editor.selected = object;

		if (ImGui::BeginPopupContextItem("outliner_item")) {
			editor.selected = object;
			ImGui::TextDisabled("%s", ToString(entry.type));
			ImGui::Separator();
			if (ImGui::MenuItem("Delete", "Del"))
				toDestroy = object;
			ImGui::EndPopup();
		}

		ImGui::PopID();
	}

	if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Delete) && editor.selected)
		toDestroy = editor.selected;

	if (toDestroy)
		DestroyObject(editor, toDestroy);

	ImGui::End();
}
