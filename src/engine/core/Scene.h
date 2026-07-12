// Scene: interface implemented by game scenes and driven by Application::Run.

#pragma once

class Application;

class Scene
{
public:
	virtual ~Scene() = default;

	// Create shader programs, objects and GL state. Return false to abort startup.
	virtual bool Load(Application& app) = 0;

	// Per-frame logic and rendering.
	virtual void Update(Application& app) = 0;
	virtual void Draw(Application& app) = 0;

	// Delete GL resources; runs after the main loop, while the GL context is still alive.
	virtual void Unload(Application& app) {}
};
