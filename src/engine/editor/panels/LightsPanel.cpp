// Environment panel: the scene's global ambient term. Directional, point and
// spot lights are LightComponents on actors now (Add Component > Light, or spawn
// a Light from the Outliner / Content Browser), so they live in the world and
// follow their object's transform. Ambient is inherently non-positional, so it
// stays here as a world environment setting.

#include "engine/editor/panels/LightsPanel.h"
#include "engine/editor/Editor.h"

void DrawLightsPanel(Editor& editor)
{
	ImGui::Begin("Environment");

	World& world = *editor.world;
	Lights& lights = world.lights;
	VectorLight& info = lights.lightInfo;

	// Ensure there is always an ambient term to edit (the environment light).
	if (info.ambientLight.empty()) {
		AmbientLight ambient;
		ambient.switchL = true;
		ambient.ambient = glm::vec3(0.12f);
		info.ambientLight.push_back(ambient);
	}

	ImGui::TextWrapped("Directional, point and spot lights are Light components. "
	                   "Add a Light actor (Outliner > + Add > Light) or attach a "
	                   "Light component, then edit it in Details.");
	ImGui::Separator();

	AmbientLight& light = info.ambientLight[0];
	bool on = light.switchL != 0;
	if (ImGui::Checkbox("Ambient##env", &on)) {
		VectorLight before = info;
		light.switchL = on;
		world.UploadLights(); // push to every lit program
		editor.undoStack.RecordLights(before, info);
	}

	if (ImGui::IsItemActivated())
		editor.lightsBefore = info;
	if (ImGui::ColorEdit3("Color##env", &light.ambient.x))
		world.UploadLights();
	if (ImGui::IsItemDeactivatedAfterEdit())
		editor.undoStack.RecordLights(editor.lightsBefore, info);

	ImGui::End();
}
