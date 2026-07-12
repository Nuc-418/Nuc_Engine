// GamePackager: File > Package Game — assembles a standalone game build
// (game executable + assets + the current scene) under Builds/<name>/.

#pragma once

#include <string>

class World;

struct PackageResult
{
	bool ok = false;
	std::string message;
};

// Saves the world as the startup scene, then copies the Game|x64 executable
// and the assets/ tree into Builds/<buildName>/. The executable must have
// been built in Visual Studio first (Game|x64 configuration).
PackageResult PackageGame(World& world, const std::string& buildName);
