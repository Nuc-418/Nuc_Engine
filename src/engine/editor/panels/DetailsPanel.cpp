// Details panel: inspector for the selected object.

#include "engine/editor/panels/DetailsPanel.h"
#include "engine/editor/Editor.h"
#include "engine/scene/Component.h"
#include "engine/scene/ComponentRegistry.h"

#include <glm/glm.hpp>
#include <cstring>

void DrawDetailsPanel(Editor& editor)
{
	ImGui::Begin("Details");

	GameObject* object = editor.selected;
	if (!object) {
		ImGui::TextDisabled("Select an object in the Outliner or Viewport.");
		ImGui::End();
		return;
	}

	/* Type (looked up in the world entry) */
	for (const WorldEntry& entry : editor.world->entries) {
		if (entry.object.get() == object) {
			ImGui::TextDisabled("Type: %s", editor.world->TypeLabel(entry.typeId).c_str());
			break;
		}
	}

	/* Name */
	unsigned long long renameId = editor.world->IdOf(object);
	char nameBuffer[128];
	strncpy(nameBuffer, object->name.c_str(), sizeof(nameBuffer) - 1);
	nameBuffer[sizeof(nameBuffer) - 1] = '\0';
	if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
		object->name = nameBuffer;
	if (ImGui::IsItemActivated())
		editor.nameBefore = nameBuffer;
	if (ImGui::IsItemDeactivatedAfterEdit())
		editor.undoStack.RecordRename(renameId, editor.nameBefore, object->name);

	ImGui::SeparatorText("Transform");

	/* Each widget edit becomes one undo entry: snapshot on activation,
	   record when the widget is released. */
	unsigned long long objectId = editor.world->IdOf(object);
	auto trackEdit = [&]() {
		if (ImGui::IsItemActivated())
			editor.dragBefore = CaptureTransform(*object);
		if (ImGui::IsItemDeactivatedAfterEdit())
			editor.undoStack.RecordTransform(objectId, editor.dragBefore, CaptureTransform(*object));
	};

	Transform& transform = object->transform;
	ImGui::DragFloat3("Position", &transform.position.x, 0.1f);
	trackEdit();

	/* Rotation is stored in radians (engine order: yaw-Y, pitch-X, roll-Z);
	   shown in degrees like UE. */
	glm::vec3 degreesRot = glm::degrees(transform.rotation);
	if (ImGui::DragFloat3("Rotation", &degreesRot.x, 1.0f))
		transform.rotation = glm::radians(degreesRot);
	trackEdit();

	ImGui::DragFloat3("Scale", &transform.scale.x, 0.05f);
	trackEdit();

	/* Components: list what's attached, and offer the registered types to add.
	   Per-component editors arrive with the light/physics component phases. */
	ImGui::SeparatorText("Components");
	for (const std::unique_ptr<Component>& component : object->Components())
		ImGui::BulletText("%s", component->DisplayName());

	if (ImGui::Button("Add Component"))
		ImGui::OpenPopup("AddComponentPopup");
	if (ImGui::BeginPopup("AddComponentPopup")) {
		for (const std::string& typeId : ComponentRegistry::TypeIds()) {
			if (ImGui::Selectable(ComponentRegistry::Label(typeId).c_str()))
				object->AddComponentById(typeId);
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}
