// Application: window/input/GLEW bootstrap and the main loop that drives a Scene.

#include "engine/core/Application.h"
#include "engine/core/Time.h"
#include <time.h>

bool Application::Init(const Config& appConfig)
{
	config = appConfig;

	if (!glfwInit()) return false;

	if (!window.NewWindow(config.width, config.height, (char*)config.title, NULL, NULL))
		return false;

	window.SetWindowPos(0, 0);
	window.MakeContextCurrent();

	inputs.AssociateWindow(window.windowPtr, config.width, config.height);
	controller.AssociateUserInput(&inputs);

	glewInit();

	return true;
}

void Application::Run(Scene& scene)
{
	if (!scene.Load(*this)) return;

	while (!glfwWindowShouldClose(window.windowPtr)) {

		clock_t begin = clock();

		scene.Update(*this);
		scene.Draw(*this);

		glfwSwapBuffers(window.windowPtr);
		glfwPollEvents();
		clock_t end = clock();

		double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
		Time::Update(elapsed_secs);
	}

	scene.Unload(*this);
}

void Application::Shutdown()
{
	glfwDestroyWindow(window.windowPtr);
	glfwTerminate();
}
