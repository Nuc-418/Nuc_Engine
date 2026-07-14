// Editor theme: UE-like dark style and the default dock layout.

#include "engine/editor/EditorTheme.h"
#include "imgui/imgui_internal.h"

void ApplyEditorTheme()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::StyleColorsDark();

	style.WindowRounding = 0.0f;
	style.FrameRounding = 2.0f;
	style.GrabRounding = 2.0f;
	style.TabRounding = 2.0f;
	style.WindowBorderSize = 1.0f;
	style.FramePadding = ImVec2(6, 4);

	ImVec4* colors = style.Colors;
	const ImVec4 background(0.09f, 0.09f, 0.10f, 1.00f);
	const ImVec4 panel(0.13f, 0.13f, 0.14f, 1.00f);
	const ImVec4 field(0.06f, 0.06f, 0.07f, 1.00f);
	const ImVec4 hovered(0.22f, 0.22f, 0.24f, 1.00f);
	const ImVec4 accent(0.95f, 0.58f, 0.11f, 1.00f);       // UE orange
	const ImVec4 accentDim(0.60f, 0.37f, 0.07f, 1.00f);

	colors[ImGuiCol_WindowBg] = background;
	colors[ImGuiCol_ChildBg] = background;
	colors[ImGuiCol_PopupBg] = panel;
	colors[ImGuiCol_TitleBg] = field;
	colors[ImGuiCol_TitleBgActive] = panel;
	colors[ImGuiCol_MenuBarBg] = field;
	colors[ImGuiCol_FrameBg] = field;
	colors[ImGuiCol_FrameBgHovered] = hovered;
	colors[ImGuiCol_FrameBgActive] = hovered;
	colors[ImGuiCol_Header] = accentDim;
	colors[ImGuiCol_HeaderHovered] = accent;
	colors[ImGuiCol_HeaderActive] = accent;
	colors[ImGuiCol_Button] = panel;
	colors[ImGuiCol_ButtonHovered] = hovered;
	colors[ImGuiCol_ButtonActive] = accentDim;
	colors[ImGuiCol_Tab] = field;
	colors[ImGuiCol_TabHovered] = accent;
	colors[ImGuiCol_TabSelected] = panel;
	colors[ImGuiCol_TabDimmed] = field;
	colors[ImGuiCol_TabDimmedSelected] = panel;
	colors[ImGuiCol_CheckMark] = accent;
	colors[ImGuiCol_SliderGrab] = accent;
	colors[ImGuiCol_SliderGrabActive] = accent;
	colors[ImGuiCol_SeparatorHovered] = accent;
	colors[ImGuiCol_SeparatorActive] = accent;
	colors[ImGuiCol_ResizeGrip] = panel;
	colors[ImGuiCol_ResizeGripHovered] = accent;
	colors[ImGuiCol_ResizeGripActive] = accent;
	colors[ImGuiCol_DockingPreview] = accentDim;
	colors[ImGuiCol_TextSelectedBg] = accentDim;
}

void BuildDefaultDockLayout(ImGuiID dockspaceId)
{
	ImGui::DockBuilderRemoveNode(dockspaceId);
	ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetMainViewport()->WorkSize);

	ImGuiID center = dockspaceId;
	ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.24f, NULL, &center);
	ImGuiID rightBottom = ImGui::DockBuilderSplitNode(right, ImGuiDir_Down, 0.60f, NULL, &right);
	ImGuiID bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.28f, NULL, &center);

	ImGui::DockBuilderDockWindow("Viewport", center);
	ImGui::DockBuilderDockWindow("Outliner", right);
	ImGui::DockBuilderDockWindow("Maps", right);
	ImGui::DockBuilderDockWindow("Details", rightBottom);
	ImGui::DockBuilderDockWindow("Content Browser", bottom);
	ImGui::DockBuilderDockWindow("Environment", bottom);
	ImGui::DockBuilderDockWindow("Stats", bottom);

	ImGui::DockBuilderFinish(dockspaceId);
}
