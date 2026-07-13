// Viewport panel: scene image, RMB fly, W/E/R gizmo, F-focus.

#include "engine/editor/panels/ViewportPanel.h"
#include "engine/editor/Editor.h"
#include "engine/editor/EditorMath.h"
#include "engine/core/Application.h"

#include <glm/gtc/type_ptr.hpp>
#include <cmath> // powf

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

	/* Drag-and-drop spawn: a "Place Actors" item dropped onto the viewport is
	   placed where the drop ray meets the ground plane (y=0). */
	if (!editor.playing && ImGui::BeginDragDropTarget()) {
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("NUC_SPAWN_TYPE");
		if (payload && payload->Data) {
			std::string type((const char*)payload->Data); // null-terminated type id
			World& world = *editor.world;
			if (world.CanSpawn(type)) {
				ImVec2 mouse = ImGui::GetMousePos();
				float ndcX = 2.0f * (mouse.x - imagePos.x) / editor.viewportSize.x - 1.0f;
				float ndcY = 1.0f - 2.0f * (mouse.y - imagePos.y) / editor.viewportSize.y;

				Camera& camera = world.camera;
				glm::mat4 inverseViewProjection = glm::inverse(camera.GetProjection() * camera.GetView());
				glm::vec4 farPoint = inverseViewProjection * glm::vec4(ndcX, ndcY, 1.0f, 1.0f);
				farPoint /= farPoint.w;

				glm::vec3 rayOrigin = camera.transform.position;
				glm::vec3 rayDirection = glm::normalize(glm::vec3(farPoint) - rayOrigin);

				/* Intersect the ground plane y=0; fall back to a point ahead. */
				glm::vec3 dropPos;
				float denom = rayDirection.y;
				float t = (denom != 0.0f) ? (-rayOrigin.y / denom) : -1.0f;
				if (t > 0.0f)
					dropPos = rayOrigin + rayDirection * t;
				else
					dropPos = rayOrigin + rayDirection * 12.0f;

				GameObject* spawned = world.Spawn(type);
				if (spawned) {
					spawned->transform.SetPos(dropPos);
					editor.selected = spawned;
					editor.undoStack.RecordSpawn(type, spawned->name, world.IdOf(spawned),
					                             CaptureTransform(*spawned));
				}
			}
		}
		ImGui::EndDragDropTarget();
	}

	/* Play-in-viewport: the game is running; no editing interactions. */
	if (editor.playing) {
		ImGui::SetCursorScreenPos(ImVec2(imagePos.x + 8, imagePos.y + 8));
		ImGui::TextColored(ImVec4(0.95f, 0.58f, 0.11f, 1.0f), "PLAYING - press Esc to stop");
		ImGui::End();
		return;
	}

	/* UE5-style RMB fly: capture the cursor while the right button is held
	   over the viewport; EditorHost runs BasicMovement while flying. */
	bool rmbDown = ImGui::IsMouseDown(ImGuiMouseButton_Right);
	bool wantFly = rmbDown && (editor.viewportFlying || hovered);
	if (wantFly && !editor.viewportFlying) {
		app.inputs.SetCursorCaptured(true);
		app.inputs.CenterCursor(); // first-frame delta must be zero (no view snap)
		// Stop ImGui's backend from re-showing the cursor every frame while flying.
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse | ImGuiConfigFlags_NoMouseCursorChange;
	}
	if (!wantFly && editor.viewportFlying) {
		app.inputs.SetCursorCaptured(false);
		ImGui::GetIO().ConfigFlags &= ~(ImGuiConfigFlags_NoMouse | ImGuiConfigFlags_NoMouseCursorChange);
	}
	editor.viewportFlying = wantFly;

	/* Mouse wheel while flying scales the fly speed, UE5-style: up = faster,
	   down = slower. Multiplicative so each notch is a proportional step. */
	if (editor.viewportFlying) {
		float wheel = ImGui::GetIO().MouseWheel;
		if (wheel != 0.0f) {
			editor.flySpeed *= powf(1.1f, wheel);
			editor.flySpeed = glm::clamp(editor.flySpeed, 0.5f, 100.0f);
		}
	}

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

	/* Click-picking: LMB over the viewport selects the nearest object whose
	   oriented bounds the camera ray hits (empty space deselects). Skipped
	   while flying or when the click lands on the gizmo. */
	if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)
	    && !editor.viewportFlying && !ImGuizmo::IsOver() && !ImGuizmo::IsUsing()) {
		ImVec2 mouse = ImGui::GetMousePos();
		// The image is drawn V-flipped, so panel-top maps to NDC +1 directly.
		float ndcX = 2.0f * (mouse.x - imagePos.x) / editor.viewportSize.x - 1.0f;
		float ndcY = 1.0f - 2.0f * (mouse.y - imagePos.y) / editor.viewportSize.y;

		Camera& camera = editor.world->camera;
		glm::mat4 inverseViewProjection = glm::inverse(camera.GetProjection() * camera.GetView());
		glm::vec4 farPoint = inverseViewProjection * glm::vec4(ndcX, ndcY, 1.0f, 1.0f);
		farPoint /= farPoint.w;

		glm::vec3 rayOrigin = camera.transform.position;
		glm::vec3 rayDirection = glm::normalize(glm::vec3(farPoint) - rayOrigin);

		GameObject* closest = nullptr;
		float closestDistance = 0.0f;
		for (WorldEntry& entry : editor.world->entries) {
			Mesh& mesh = entry.object->meshRenderer.mesh;
			if (!mesh.hasAabb)
				continue;
			entry.object->transform.UpdateModel();
			float distance = 0.0f;
			if (RayIntersectsOBB(rayOrigin, rayDirection, mesh.aabbMin, mesh.aabbMax,
			                     entry.object->transform.model, distance)) {
				if (!closest || distance < closestDistance) {
					closest = entry.object.get();
					closestDistance = distance;
				}
			}
		}
		editor.selected = closest;
	}

	/* Transform gizmo on the selected object */
	if (editor.selected && !editor.viewportFlying) {
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(imagePos.x, imagePos.y, editor.viewportSize.x, editor.viewportSize.y);

		Transform& transform = editor.selected->transform;
		TransformState preManipulate = CaptureTransform(*editor.selected);
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

		/* One undo entry per drag: capture on grab, record on release. */
		bool usingNow = ImGuizmo::IsUsing();
		if (usingNow && !editor.gizmoDragging) {
			editor.gizmoDragging = true;
			editor.dragBefore = preManipulate;
			editor.dragTargetId = editor.world->IdOf(editor.selected);
		}
		if (!usingNow && editor.gizmoDragging) {
			editor.gizmoDragging = false;
			GameObject* target = editor.world->FindById(editor.dragTargetId);
			if (target)
				editor.undoStack.RecordTransform(editor.dragTargetId, editor.dragBefore, CaptureTransform(*target));
		}
	} else if (editor.gizmoDragging) {
		/* Selection vanished mid-drag; close out the capture. */
		editor.gizmoDragging = false;
	}

	/* Local/world toggle, top-left corner of the viewport */
	ImGui::SetCursorScreenPos(ImVec2(imagePos.x + 8, imagePos.y + 8));
	const char* modeLabel = (editor.gizmoMode == ImGuizmo::WORLD) ? "World" : "Local";
	if (ImGui::SmallButton(modeLabel))
		editor.gizmoMode = (editor.gizmoMode == ImGuizmo::WORLD) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;

	ImGui::End();
}
