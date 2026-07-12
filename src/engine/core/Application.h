// Application: window/input/GLEW bootstrap and the main loop that drives a Scene.

#pragma once

#include <GL/glew.h>
#include "engine/core/Window.h"
#include "engine/core/Scene.h"
#include "engine/input/UserInputs.h"
#include "engine/input/Controller.h"

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
	Controller controller;
	Config config;
};
