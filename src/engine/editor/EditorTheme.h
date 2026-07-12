// Editor theme: UE-like dark style and the default dock layout.

#pragma once

#include "imgui/imgui.h"

void ApplyEditorTheme();

// Builds the default UE-style layout (Viewport center, Outliner/Details right,
// Content Browser/Lights/Stats bottom) into the given dockspace node.
void BuildDefaultDockLayout(ImGuiID dockspaceId);
