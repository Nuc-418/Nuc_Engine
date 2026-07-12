// Viewport panel: scene image, RMB fly, W/E/R gizmo, F-focus.

#include "engine/editor/panels/ViewportPanel.h"
#include "engine/editor/Editor.h"
#include "engine/editor/EditorMath.h"
#include "engine/core/Application.h"

#include <glm/gtc/type_ptr.hpp>

void DrawViewportPanel(Editor& editor, Application& app)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("Viewport");
	ImGui::PopStyleVar();

	ImVec2 available = ImGui::GetContentRegionAvail();
	if (available.x >= 1.0f && available.y >= 1.0f)
		editor.viewportSize = available;

	ImVec2 imagePos = ImGui::GetCursorScreenPos();
	// The FBO is rendered in GL orientation; flip V so it displays upright.
	ImGui::Image((ImTextureID)(intptr_t)editor.sceneFramebuffer.colorTexture,
	             editor.viewportSize, ImVec2(0, 1), ImVec2(1, 0));
	bool hovered = ImGui::IsItemHovered();

	/* UE5-style RMB fly: capture the cursor while the right button is held
	   over the viewport; EditorHost runs BasicMovement while flying. */
	bool rmbDown = ImGui::IsMouseDown(ImGuiMouseButton_Right);
	bool wantFly = rmbDown && (editor.viewportFlying || hovered);
	if (wantFly && !editor.viewportFlying) {
		app.inputs.SetCursorCaptured(true);
		app.inputs.CenterCursor(); // first-frame delta must be zero (no view snap)
	}
	if (!wantFly && editor.viewportFlying)
		app.inputs.SetCursorCaptured(false);
	editor.viewportFlying = wantFly;

	if (hovered && !editor.viewportFlying) {
		/* UE5 gizmo keys */
		if (ImGui::IsKeyPressed(ImGuiKey_W)) editor.gizmoOperation = ImGuizmo::TRANSLATE;
		if (ImGui::IsKeyPressed(ImGuiKey_E)) editor.gizmoOperation = ImGuizmo::ROTATE;
		if (ImGui::IsKeyPressed(ImGuiKey_R)) editor.gizmoOperation = ImGuizmo::SCALE;

		/* F = focus the selected object */
		if (ImGui::IsKeyPressed(ImGuiKey_F) && editor.selected) {
			Transform& cameraTransform = editor.world->camera.transform;
			cameraTransform.position = editor.selected->transform.position - cameraTransform.forward * 8.0f;
		}
	}

	/* Transform gizmo on the selected object */
	if (editor.selected && !editor.viewportFlying) {
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(imagePos.x, imagePos.y, editor.viewportSize.x, editor.viewportSize.y);

		Transform& transform = editor.selected->transform;
		transform.UpdateModel();
		glm::mat4 model = transform.model;
		glm::mat4 view = editor.world->camera.GetView();
		glm::mat4 projection = editor.world->camera.GetProjection();

		if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
		                         editor.gizmoOperation, editor.gizmoMode, glm::value_ptr(model))
		    && ImGuizmo::IsUsing()) {
			transform.position = glm::vec3(model[3]);
			if (editor.gizmoOperation == ImGuizmo::SCALE) {
				transform.scale = glm::vec3(glm::length(glm::vec3(model[0])),
				                            glm::length(glm::vec3(model[1])),
				                            glm::length(glm::vec3(model[2])));
			}
			if (editor.gizmoOperation == ImGuizmo::ROTATE)
				transform.rotation = EulerYXZFromMatrix(model);
		}
	}

	/* Local/world toggle, top-left corner of the viewport */
	ImGui::SetCursorScreenPos(ImVec2(imagePos.x + 8, imagePos.y + 8));
	const char* modeLabel = (editor.gizmoMode == ImGuizmo::WORLD) ? "World" : "Local";
	if (ImGui::SmallButton(modeLabel))
		editor.gizmoMode = (editor.gizmoMode == ImGuizmo::WORLD) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;

	ImGui::End();
}
