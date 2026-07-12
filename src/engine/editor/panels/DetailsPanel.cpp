// Details panel: inspector for the selected object.

#include "engine/editor/panels/DetailsPanel.h"
#include "engine/editor/Editor.h"

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
			ImGui::TextDisabled("Type: %s", ToString(entry.type));
			break;
		}
	}

	/* Name */
	char nameBuffer[128];
	strncpy(nameBuffer, object->name.c_str(), sizeof(nameBuffer) - 1);
	nameBuffer[sizeof(nameBuffer) - 1] = '\0';
	if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
		object->name = nameBuffer;

	ImGui::SeparatorText("Transform");

	Transform& transform = object->transform;
	ImGui::DragFloat3("Position", &transform.position.x, 0.1f);

	/* Rotation is stored in radians (engine order: yaw-Y, pitch-X, roll-Z);
	   shown in degrees like UE. */
	glm::vec3 degreesRot = glm::degrees(transform.rotation);
	if (ImGui::DragFloat3("Rotation", &degreesRot.x, 1.0f))
		transform.rotation = glm::radians(degreesRot);

	ImGui::DragFloat3("Scale", &transform.scale.x, 0.05f);

	ImGui::End();
}
