// Entry point: boots the Application and runs the demo scene.

#ifdef _WIN32
#include <windows.h>
#endif

#include "engine/core/Application.h"
#include "game/DemoScene.h"

int main(void)
{
	Application::Config config;
	config.width = 800;
	config.height = 600;
	config.title = "NucEngine";

#ifdef _WIN32
	//Move the console window aside
	SetWindowPos(GetConsoleWindow(), 0, config.width - 7, -11, 0, 0, SWP_NOSIZE);
#endif

	Application app;
	if (!app.Init(config)) return -1;

	//Hide and capture the cursor for mouse-look (play behavior)
	app.inputs.SetCursorCaptured(true);

	DemoScene scene;
	app.Run(scene);

	app.Shutdown();

	return 0;
}
