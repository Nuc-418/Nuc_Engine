// EditorFileSystem: minimal directory listing for the Content Browser
// (v141 targets C++14, so no std::filesystem).

#pragma once

#include <string>
#include <vector>

struct DirectoryEntry
{
	std::string name;
	bool isDirectory = false;
};

// Lists a directory (non-recursive), directories first, sorted by name.
// Returns an empty list if the path cannot be read.
std::vector<DirectoryEntry> ListDirectory(const std::string& path);

// Creates a directory if missing (single level). Returns true if it exists after the call.
bool EnsureDirectory(const std::string& path);

// Deletes a file. Returns true when the file is gone.
bool RemoveFile(const std::string& path);
