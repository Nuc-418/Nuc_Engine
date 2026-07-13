// Model discovery over std::filesystem (C++17), so it works the same in the
// editor, game builds and the Linux verification gate.

#include "engine/io/ModelDiscovery.h"

#include <cctype>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

namespace {

bool EndsWithNoCase(const std::string& s, const char* suffix)
{
	size_t n = std::strlen(suffix);
	if (s.size() < n)
		return false;
	for (size_t i = 0; i < n; i++)
		if (std::tolower((unsigned char)s[s.size() - n + i]) != std::tolower((unsigned char)suffix[i]))
			return false;
	return true;
}

} // namespace

void DiscoverObjFiles(const std::string& root, std::vector<std::string>& out)
{
	std::error_code ec;
	for (fs::directory_iterator it(root, ec), end; !ec && it != end; it.increment(ec)) {
		const fs::directory_entry& entry = *it;
		std::string name = entry.path().filename().string();
		std::string full = root + "/" + name;
		if (entry.is_directory(ec))
			DiscoverObjFiles(full, out);
		else if (EndsWithNoCase(name, ".obj"))
			out.push_back(full);
	}
}

std::string FindTextureInFolder(const std::string& folder)
{
	const char* exts[] = { ".tga", ".png", ".jpg", ".jpeg", ".bmp" };
	std::error_code ec;
	for (fs::directory_iterator it(folder, ec), end; !ec && it != end; it.increment(ec)) {
		const fs::directory_entry& entry = *it;
		if (entry.is_directory(ec))
			continue;
		std::string name = entry.path().filename().string();
		for (const char* ext : exts)
			if (EndsWithNoCase(name, ext))
				return folder + name;
	}
	return "";
}
