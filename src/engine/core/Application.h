// Application: window/input/GLEW bootstrap and the main loop that drives a Scene.

#pragma once

#include <GL/glew.h>
#include "engine/core/Window.h"
#include "engine/core/Scene.h"
#include "engine/input/UserInputs.h"
#include "engine/input/InputActions.h"
#include "engine/input/Controller.h"
#include "engine/plugin/PluginManager.h"
#include "engine/asset/AssetManager.h"
#include "engine/core/ServiceRegistry.h"

class Application
{
public:
	struct Config
	{
		int width = 800;
		int height = 600;
		const char* title = "NucEngine";
	};

	bool Init(const Config& config);

	// Loads the scene, runs the main loop until the window closes,
	// then unloads the scene while the GL context is still current.
	void Run(Scene& scene);

	void Shutdown();

	Window window;
	UserInputs inputs;
	// Named bindings over the raw key state; Init installs the defaults
	// (MoveForward/MoveRight/MoveUp axes, Sprint, Exit), scenes add theirs.
	InputActions actions;
	Controller controller;
	Config config;

	// Registered engine plugins (physics, ...). Scenes register concrete
	// plugins here; Application drives their lifecycle in Run().
	PluginManager plugins;

	// Shared shader/texture assets, freed by Run() after the scene unloads.
	AssetManager assets;

	// Interface-keyed locator for engine services. Init publishes the core
	// services (the AssetManager); plugins publish theirs (e.g. IPhysicsService)
	// in OnLoad and withdraw them in OnUnload, so code fetches a service by
	// interface without knowing the concrete provider.
	ServiceRegistry services;

	// Whether gameplay/simulation is advancing this frame. True in a standalone
	// game; the editor sets it false in Edit mode so simulation plugins freeze.
	// Simulation plugins should skip their step when this is false.
	bool simulating = true;
};
