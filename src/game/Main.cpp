// Entry point. The default build runs the demo scene inside the editor;
// the Game|x64 configuration (NUC_GAME_BUILD) runs the game standalone.

#ifdef _WIN32
#include <windows.h>
#endif

#include "engine/core/Application.h"
#include "game/DemoScene.h"

#ifndef NUC_GAME_BUILD
#include "engine/editor/EditorHost.h"
#endif

int main(void)
{
	Application::Config config;
#ifdef NUC_GAME_BUILD
	config.width = 800;
	config.height = 600;
	config.title = "NucEngine";
#else
	config.width = 1600;
	config.height = 900;
	config.title = "NucEngine Editor";
#endif

#ifdef _WIN32
	//Move the console window aside
	SetWindowPos(GetConsoleWindow(), 0, config.width - 7, -11, 0, 0, SWP_NOSIZE);
#endif

	Application app;
	if (!app.Init(config)) return -1;

	DemoScene scene;

#ifdef NUC_GAME_BUILD
	//Game build: captured cursor, no editor, ESC quits (see DemoScene::Update)
	app.inputs.SetCursorCaptured(true);
	app.Run(scene);
#else
	EditorHost host(scene, scene.GetWorld());
	app.Run(host);
#endif

	app.Shutdown();

	return 0;
}
