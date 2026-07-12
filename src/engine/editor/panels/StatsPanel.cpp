// Stats panel: frame rate, camera and world counters.

#include "engine/editor/panels/StatsPanel.h"
#include "engine/editor/Editor.h"

void DrawStatsPanel(Editor& editor)
{
	ImGui::Begin("Stats");

	ImGuiIO& io = ImGui::GetIO();
	// ImGui measures wall-clock frame time (the engine's Time uses clock(),
	// i.e. CPU time, and is left untouched for gameplay parity).
	ImGui::Text("FPS: %.1f (%.2f ms)", io.Framerate, io.Framerate > 0 ? 1000.0f / io.Framerate : 0.0f);
	ImGui::Text("Objects: %d", (int)editor.world->entries.size());

	const glm::vec3& cameraPos = editor.world->camera.transform.position;
	ImGui::Text("Camera: %.1f  %.1f  %.1f", cameraPos.x, cameraPos.y, cameraPos.z);

	ImGui::SeparatorText("Render Mode");
	World& world = *editor.world;
	struct { const char* label; GLenum mode; } modes[] = {
		{ "Triangles", GL_TRIANGLES }, { "Line Strip", GL_LINE_STRIP },
		{ "Points", GL_POINTS }, { "Triangle Fan", GL_TRIANGLE_FAN },
	};
	for (auto& m : modes) {
		if (ImGui::RadioButton(m.label, world.renderMode == m.mode))
			world.renderMode = m.mode;
		ImGui::SameLine();
	}
	ImGui::NewLine();

	ImGui::End();
}
