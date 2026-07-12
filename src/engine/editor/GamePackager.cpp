// GamePackager: assembles a standalone game build under Builds/<name>/.

#include "engine/editor/GamePackager.h"
#include "engine/editor/EditorFileSystem.h"
#include "engine/io/SceneSerializer.h"

#include <fstream>

// Output of the Game|x64 configuration (see NucEngine.vcxproj).
static const char* kGameExecutable = "x64/Game/NucEngineGame.exe";
static const char* kGameExecutableName = "NucEngineGame.exe";

static bool CopyFileOverwrite(const std::string& source, const std::string& destination)
{
	std::ifstream in(source, std::ios::binary);
	if (!in.is_open())
		return false;
	std::ofstream out(destination, std::ios::binary | std::ios::trunc);
	if (!out.is_open())
		return false;
	out << in.rdbuf();
	return out.good();
}

static bool CopyDirectoryRecursive(const std::string& source, const std::string& destination)
{
	if (!EnsureDirectory(destination))
		return false;
	bool ok = true;
	for (const DirectoryEntry& entry : ListDirectory(source)) {
		std::string from = source + "/" + entry.name;
		std::string to = destination + "/" + entry.name;
		if (entry.isDirectory)
			ok = CopyDirectoryRecursive(from, to) && ok;
		else
			ok = CopyFileOverwrite(from, to) && ok;
	}
	return ok;
}

PackageResult PackageGame(World& world, const std::string& buildName)
{
	PackageResult result;

	/* Sanitize the folder name: no separators, no empty. */
	std::string name;
	for (char c : buildName)
		if (c != '/' && c != '\\' && c != ':' && c != '.')
			name += c;
	if (name.empty())
		name = "GameBuild";

	std::string buildDir = "Builds/" + name;
	if (!EnsureDirectory("Builds") || !EnsureDirectory(buildDir)) {
		result.message = "Could not create " + buildDir;
		return result;
	}

	/* 1. The scene being edited becomes the game's startup scene. */
	if (!EnsureDirectory("assets") || !EnsureDirectory("assets/scenes") ||
	    !SceneSerializer::Save(world, SceneSerializer::StartupScenePath)) {
		result.message = "Could not save the startup scene";
		return result;
	}

	/* 2. Assets (shaders, models, scenes) travel next to the executable;
	      all runtime paths are relative to the exe's directory. */
	if (!CopyDirectoryRecursive("assets", buildDir + "/assets")) {
		result.message = "Failed while copying assets to " + buildDir;
		return result;
	}

	/* 3. The game executable, built by the Game|x64 configuration. */
	if (!CopyFileOverwrite(kGameExecutable, buildDir + "/" + kGameExecutableName)) {
		result.message = "Assets packaged to " + buildDir +
			", but the game executable is missing. Build the Game|x64 "
			"configuration in Visual Studio, then package again.";
		return result;
	}

	result.ok = true;
	result.message = "Packaged " + buildDir + "/" + kGameExecutableName +
		" - run it from that folder.";
	return result;
}
