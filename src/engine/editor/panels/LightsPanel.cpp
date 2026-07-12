// Lights panel: live editing of the scene's light sources.

#include "engine/editor/panels/LightsPanel.h"
#include "engine/editor/Editor.h"

// The ironMan shaders declare pointLight[50] / spotLight[50].
static const int kMaxLightsPerType = 50;

void DrawLightsPanel(Editor& editor)
{
	ImGui::Begin("Lights");

	World& world = *editor.world;
	Lights& lights = world.lights;
	VectorLight& info = lights.lightInfo;
	GLuint program = world.lightsProgram;

	if (program == 0) {
		ImGui::TextDisabled("No lit shader program registered.");
		ImGui::End();
		return;
	}

	if (!info.ambientLight.empty() && ImGui::CollapsingHeader("Ambient", ImGuiTreeNodeFlags_DefaultOpen)) {
		AmbientLight& light = info.ambientLight[0];
		bool on = light.switchL != 0;
		if (ImGui::Checkbox("On##ambient", &on))
			lights.ToggleAmbientLight(program, on);
		if (ImGui::ColorEdit3("Color##ambient", &light.ambient.x))
			lights.StoreAmbientLights(program);
	}

	if (!info.directionalLight.empty() && ImGui::CollapsingHeader("Directional", ImGuiTreeNodeFlags_DefaultOpen)) {
		DirectionalLight& light = info.directionalLight[0];
		bool on = light.switchL != 0;
		bool changed = false;
		if (ImGui::Checkbox("On##dir", &on))
			lights.ToggleDirectionalLight(program, on);
		changed |= ImGui::DragFloat3("Direction##dir", &light.direction.x, 0.05f);
		changed |= ImGui::ColorEdit3("Ambient##dir", &light.ambient.x);
		changed |= ImGui::DragFloat3("Diffuse##dir", &light.diffuse.x, 0.05f);
		changed |= ImGui::DragFloat3("Specular##dir", &light.specular.x, 0.5f);
		if (changed)
			lights.StoreDirectionalLights(program, (int)info.directionalLight.size());
	}

	if (ImGui::CollapsingHeader("Point Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
		int removeIndex = -1;
		for (int i = 0; i < (int)info.pointLight.size(); i++) {
			ImGui::PushID(i);
			if (ImGui::TreeNode("point", "Point %d", i)) {
				PointLight& light = info.pointLight[i];
				bool on = light.switchL != 0;
				bool changed = false;
				if (ImGui::Checkbox("On", &on))
					lights.TogglePointLight(program, i, on);
				changed |= ImGui::DragFloat3("Position", &light.position.x, 0.1f);
				changed |= ImGui::ColorEdit3("Ambient", &light.ambient.x);
				changed |= ImGui::DragFloat3("Diffuse", &light.diffuse.x, 0.05f);
				changed |= ImGui::DragFloat3("Specular", &light.specular.x, 0.1f);
				changed |= ImGui::DragFloat("Constant", &light.constant, 0.01f);
				changed |= ImGui::DragFloat("Linear", &light.linear, 0.005f);
				changed |= ImGui::DragFloat("Quadratic", &light.quadratic, 0.0005f);
				if (changed)
					lights.StorePointLights(program, (int)info.pointLight.size());
				if (ImGui::SmallButton("Remove"))
					removeIndex = i;
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		if (removeIndex >= 0) {
			info.pointLight.erase(info.pointLight.begin() + removeIndex);
			lights.StorePointLights(program, (int)info.pointLight.size());
		}
		if ((int)info.pointLight.size() < kMaxLightsPerType && ImGui::Button("Add Point Light"))
			lights.AddPointLight(program, world.camera.transform.position, glm::vec3(0.05f),
			                     glm::vec3(1.0f), glm::vec3(1.0f), 1.0f, 0.06f, 0.002f);
	}

	if (ImGui::CollapsingHeader("Spot Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
		int removeIndex = -1;
		for (int i = 0; i < (int)info.spotLight.size(); i++) {
			ImGui::PushID(1000 + i);
			if (ImGui::TreeNode("spot", "Spot %d", i)) {
				SpotLight& light = info.spotLight[i];
				bool on = light.switchL != 0;
				bool changed = false;
				if (ImGui::Checkbox("On", &on))
					lights.ToggleSpotLight(program, i, on);
				changed |= ImGui::DragFloat3("Position", &light.position.x, 0.1f);
				changed |= ImGui::DragFloat3("Direction", &light.direction.x, 0.05f);
				changed |= ImGui::SliderAngle("Cut-off", &light.cutOff, 1.0f, 89.0f);
				changed |= ImGui::ColorEdit3("Ambient", &light.ambient.x);
				changed |= ImGui::DragFloat3("Diffuse", &light.diffuse.x, 0.05f);
				changed |= ImGui::DragFloat3("Specular", &light.specular.x, 0.1f);
				changed |= ImGui::DragFloat("Constant", &light.constant, 0.01f);
				changed |= ImGui::DragFloat("Linear", &light.linear, 0.005f);
				changed |= ImGui::DragFloat("Quadratic", &light.quadratic, 0.0005f);
				if (changed)
					lights.StoreSpotLights(program, (int)info.spotLight.size());
				if (ImGui::SmallButton("Remove"))
					removeIndex = i;
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		if (removeIndex >= 0) {
			info.spotLight.erase(info.spotLight.begin() + removeIndex);
			lights.StoreSpotLights(program, (int)info.spotLight.size());
		}
		if ((int)info.spotLight.size() < kMaxLightsPerType && ImGui::Button("Add Spot Light"))
			lights.AddSpotLight(program, world.camera.transform.position, glm::vec3(0, 0, -1),
			                    glm::vec3(0.05f), glm::vec3(1.0f), glm::vec3(1.0f),
			                    1.0f, 0.006f, 0.002f, 0.26f);
	}

	ImGui::End();
}
