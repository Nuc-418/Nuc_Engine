// EditorFileSystem: minimal directory listing for the Content Browser.

#include "engine/editor/EditorFileSystem.h"

#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

std::vector<DirectoryEntry> ListDirectory(const std::string& path)
{
	std::vector<DirectoryEntry> result;

#ifdef _WIN32
	WIN32_FIND_DATAA findData;
	HANDLE handle = FindFirstFileA((path + "/*").c_str(), &findData);
	if (handle == INVALID_HANDLE_VALUE)
		return result;
	do {
		std::string name = findData.cFileName;
		if (name == "." || name == "..")
			continue;
		DirectoryEntry entry;
		entry.name = name;
		entry.isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		result.push_back(entry);
	} while (FindNextFileA(handle, &findData));
	FindClose(handle);
#else
	DIR* dir = opendir(path.c_str());
	if (!dir)
		return result;
	while (struct dirent* item = readdir(dir)) {
		std::string name = item->d_name;
		if (name == "." || name == "..")
			continue;
		DirectoryEntry entry;
		entry.name = name;
		entry.isDirectory = (item->d_type == DT_DIR);
		result.push_back(entry);
	}
	closedir(dir);
#endif

	std::sort(result.begin(), result.end(), [](const DirectoryEntry& a, const DirectoryEntry& b) {
		if (a.isDirectory != b.isDirectory)
			return a.isDirectory;
		return a.name < b.name;
	});
	return result;
}

bool EnsureDirectory(const std::string& path)
{
#ifdef _WIN32
	CreateDirectoryA(path.c_str(), NULL);
	DWORD attributes = GetFileAttributesA(path.c_str());
	return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY);
#else
	mkdir(path.c_str(), 0755);
	struct stat info;
	return stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR);
#endif
}
