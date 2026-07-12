// Entry point: boots the Application and runs the demo scene inside the editor.

#ifdef _WIN32
#include <windows.h>
#endif

#include "engine/core/Application.h"
#include "engine/editor/EditorHost.h"
#include "game/DemoScene.h"

int main(void)
{
	Application::Config config;
	config.width = 1600;
	config.height = 900;
	config.title = "NucEngine Editor";

#ifdef _WIN32
	//Move the console window aside
	SetWindowPos(GetConsoleWindow(), 0, config.width - 7, -11, 0, 0, SWP_NOSIZE);
#endif

	Application app;
	if (!app.Init(config)) return -1;

	DemoScene scene;
	EditorHost host(scene, scene.GetWorld());
	app.Run(host);

	app.Shutdown();

	return 0;
}
