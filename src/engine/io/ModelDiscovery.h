// Model discovery: filesystem scans that turn dropped-in content into
// spawnable types (every .obj under assets/models becomes one).

#pragma once

#include <string>
#include <vector>

// Recursively collect every .obj file under `root` (paths use '/').
void DiscoverObjFiles(const std::string& root, std::vector<std::string>& out);

// First image file (.tga/.png/.jpg/.jpeg/.bmp) in a model's folder
// (folder must end with '/'), or "" when none exists.
std::string FindTextureInFolder(const std::string& folder);
