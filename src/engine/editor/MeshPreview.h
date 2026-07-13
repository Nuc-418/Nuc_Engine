// MeshPreview: renders small thumbnails of mesh types for the Content Browser.

#pragma once

#include <map>
#include <string>
#include <vector>

#include "engine/render/Framebuffer.h"
#include "engine/scene/World.h"

namespace MeshPreview
{
	// Renders a one-off thumbnail of each given type id into `out` (one
	// framebuffer per type, kept alive so the colour texture can be shown by
	// ImGui). Each mesh is drawn with its own shader program under a fixed
	// studio light and framed from its bounds. Saves and restores GL state.
	void Generate(std::map<std::string, Framebuffer>& out, const World& world,
	              const std::vector<std::string>& typeIds, int size);
}
