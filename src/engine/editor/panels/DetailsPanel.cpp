// Details panel: inspector for the selected object.

#include "engine/editor/panels/DetailsPanel.h"
#include "engine/editor/Editor.h"
#include "engine/scene/Component.h"
#include "engine/scene/ComponentRegistry.h"
#include "engine/render/LightComponent.h"
#include "engine/render/CameraComponent.h"

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

	/* Light component editor. Placement is driven by the transform above
	   (world position for point/spot, world forward for aim), so only the
	   light's own parameters live here. Component edits are not yet undoable
	   (generalized component undo is a later roadmap phase). */
	if (LightComponent* light = object->GetComponent<LightComponent>()) {
		ImGui::SeparatorText("Light");
		const char* kinds[] = { "Directional", "Point", "Spot" };
		int kind = (int)light->kind;
		if (ImGui::Combo("Kind", &kind, kinds, 3))
			light->kind = (LightComponent::Kind)kind;
		ImGui::Checkbox("On", &light->on);
		ImGui::ColorEdit3("Ambient", &light->ambient.x);
		ImGui::ColorEdit3("Diffuse", &light->diffuse.x);
		ImGui::ColorEdit3("Specular", &light->specular.x);
		if (light->kind != LightComponent::Kind::Directional) {
			ImGui::DragFloat("Constant", &light->constant, 0.01f, 0.0f, 10.0f);
			ImGui::DragFloat("Linear", &light->linear, 0.005f, 0.0f, 2.0f);
			ImGui::DragFloat("Quadratic", &light->quadratic, 0.005f, 0.0f, 2.0f);
		}
		if (light->kind == LightComponent::Kind::Spot) {
			float cutOffDegrees = glm::degrees(light->cutOff);
			if (ImGui::DragFloat("Cone (deg)", &cutOffDegrees, 0.5f, 1.0f, 89.0f))
				light->cutOff = glm::radians(cutOffDegrees);
		}
		ImGui::TextDisabled("Position and aim follow the object's transform.");
	}

	/* Camera component editor: lens parameters plus the active-camera toggle
	   (Play mode and game builds render through the active camera). */
	if (CameraComponent* cameraComponent = object->GetComponent<CameraComponent>()) {
		ImGui::SeparatorText("Camera");
		ImGui::DragFloat("FOV (deg)", &cameraComponent->fovDegrees, 0.5f, 10.0f, 140.0f);
		ImGui::DragFloat("Near", &cameraComponent->nearPlane, 0.01f, 0.01f, 10.0f);
		ImGui::DragFloat("Far", &cameraComponent->farPlane, 1.0f, 10.0f, 10000.0f);
		bool isActive = editor.world->activeCameraId == objectId;
		if (isActive) {
			ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Active camera (Play renders through this)");
			if (ImGui::Button("Clear Active Camera"))
				editor.world->activeCameraId = 0;
		} else if (ImGui::Button("Make Active Camera")) {
			editor.world->activeCameraId = objectId;
		}
		ImGui::TextDisabled("Pose follows the object's transform.");
	}

	/* Components: list what's attached, and offer the registered types to add. */
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
