// Outliner panel: scene hierarchy tree — select, spawn, delete, reparent.

#include "engine/editor/panels/OutlinerPanel.h"
#include "engine/editor/Editor.h"

static void DestroyObject(Editor& editor, GameObject* object)
{
	if (editor.selected == object)
		editor.selected = nullptr;

	const WorldEntry* entry = editor.world->EntryOf(object);
	if (entry)
		editor.undoStack.RecordDelete(entry->typeId, object->name, entry->id, CaptureTransform(*object));

	editor.world->Destroy(object);
}

// Reparents `child` under `newParent` (nullptr = root), keeping its world
// pose, and records the change for undo.
static void ReparentObject(Editor& editor, GameObject* child, GameObject* newParent)
{
	if (child->Parent() == newParent)
		return;
	World& world = *editor.world;
	unsigned long long parentBefore = child->Parent() ? world.IdOf(child->Parent()) : 0;
	TransformState before = CaptureTransform(*child);
	if (!child->SetParent(newParent, /*keepWorldTransform=*/true))
		return; // cycle refused
	unsigned long long parentAfter = newParent ? world.IdOf(newParent) : 0;
	editor.undoStack.RecordReparent(world.IdOf(child), parentBefore, parentAfter,
	                                before, CaptureTransform(*child));
}

// Drag source + drop target on the last drawn item. Payload is the dragged
// object's stable World id.
static void HierarchyDragDrop(Editor& editor, GameObject* object)
{
	if (ImGui::BeginDragDropSource()) {
		unsigned long long id = editor.world->IdOf(object);
		ImGui::SetDragDropPayload("NUC_OUTLINER_OBJECT", &id, sizeof(id));
		ImGui::TextUnformatted(object->name.c_str());
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("NUC_OUTLINER_OBJECT")) {
			unsigned long long id = *(const unsigned long long*)payload->Data;
			GameObject* dragged = editor.world->FindById(id);
			if (dragged && dragged != object)
				ReparentObject(editor, dragged, object);
		}
		ImGui::EndDragDropTarget();
	}
}

static void DrawObjectNode(Editor& editor, GameObject* object, GameObject*& toDestroy)
{
	World& world = *editor.world;
	ImGui::PushID(object);

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
	                         | ImGuiTreeNodeFlags_OpenOnDoubleClick
	                         | ImGuiTreeNodeFlags_SpanAvailWidth
	                         | ImGuiTreeNodeFlags_DefaultOpen;
	if (object->Children().empty())
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	if (editor.selected == object)
		flags |= ImGuiTreeNodeFlags_Selected;

	bool open = ImGui::TreeNodeEx(object->name.c_str(), flags);
	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
		editor.selected = object;

	HierarchyDragDrop(editor, object);

	if (ImGui::BeginPopupContextItem("outliner_item")) {
		editor.selected = object;
		const WorldEntry* entry = world.EntryOf(object);
		ImGui::TextDisabled("%s", world.TypeLabel(entry ? entry->typeId : "").c_str());
		ImGui::Separator();
		if (object->Parent() && ImGui::MenuItem("Detach from parent"))
			ReparentObject(editor, object, nullptr);
		if (ImGui::MenuItem("Delete", "Del"))
			toDestroy = object;
		ImGui::EndPopup();
	}

	if (open && !object->Children().empty()) {
		for (GameObject* child : object->Children())
			DrawObjectNode(editor, child, toDestroy);
		ImGui::TreePop();
	}

	ImGui::PopID();
}

void DrawOutlinerPanel(Editor& editor)
{
	ImGui::Begin("Outliner");
	World& world = *editor.world;

	if (ImGui::Button("+ Add"))
		ImGui::OpenPopup("outliner_add");
	if (ImGui::BeginPopup("outliner_add")) {
		// Every registered type, straight from the World's dynamic registry.
		for (const std::string& type : world.TypeIds()) {
			if (ImGui::MenuItem(world.TypeLabel(type).c_str())) {
				GameObject* spawned = world.Spawn(type);
				if (spawned) {
					// Drop new objects in front of the camera.
					Transform& cam = world.camera.transform;
					spawned->transform.SetPos(cam.position + cam.forward * 6.0f);
					editor.selected = spawned;
					editor.undoStack.RecordSpawn(type, spawned->name, world.IdOf(spawned), CaptureTransform(*spawned));
				}
			}
		}
		ImGui::EndPopup();
	}
	ImGui::SameLine();
	ImGui::TextDisabled("%d objects", (int)world.entries.size());
	ImGui::Separator();

	GameObject* toDestroy = nullptr;

	// Roots only; children render nested under their parents.
	for (WorldEntry& entry : world.entries)
		if (!entry.object->Parent())
			DrawObjectNode(editor, entry.object.get(), toDestroy);

	// Dropping onto the empty area below the tree detaches to the root.
	ImGui::InvisibleButton("outliner_root_drop",
	                       ImVec2(ImGui::GetContentRegionAvail().x > 1.0f ? ImGui::GetContentRegionAvail().x : 1.0f,
	                              ImGui::GetContentRegionAvail().y > 24.0f ? ImGui::GetContentRegionAvail().y : 24.0f));
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("NUC_OUTLINER_OBJECT")) {
			unsigned long long id = *(const unsigned long long*)payload->Data;
			if (GameObject* dragged = editor.world->FindById(id))
				ReparentObject(editor, dragged, nullptr);
		}
		ImGui::EndDragDropTarget();
	}

	if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Delete) && editor.selected)
		toDestroy = editor.selected;

	if (toDestroy)
		DestroyObject(editor, toDestroy);

	ImGui::End();
}
