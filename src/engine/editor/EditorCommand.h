// EditorCommand: deferred editor intents produced by menus/panels during Draw
// and executed by EditorHost on the next Update.
//
// UI code (the menu bar, the panels) never mutates the world or the app
// mid-frame: it enqueues a command describing *what* it wants, and EditorHost
// drains the queue and decides *how* to carry it out. This is the seam that
// replaced the Editor's public "flag soup" (playClicked, pendingSceneLoad,
// mapDeleteRequest, ...) — panels stop reaching into a god object.

#pragma once

#include <string>
#include <utility>
#include <vector>

enum class EditorCommandType
{
	Play,        // enter Play (PIE) mode
	Exit,        // quit the application
	SaveScene,   // save the world; path empty => the editor's current savePath
	NewMap,      // create an empty default map at path and switch to it
	LoadScene,   // load the scene file at path
	DeleteMap,   // delete the map file at path from disk
};

struct EditorCommand
{
	EditorCommandType type;
	std::string path;  // meaning depends on type; empty when unused
};

// FIFO queue of deferred editor actions. Producers Push during Draw; the one
// consumer (EditorHost::Update) Drains and executes them, so the queue is
// empty again before the next frame's UI runs.
class EditorCommandQueue
{
public:
	void Push(EditorCommandType type, std::string path = {})
	{
		commands.push_back({ type, std::move(path) });
	}

	bool Empty() const { return commands.empty(); }

	// Moves every pending command out in insertion order, leaving the queue
	// empty. Draining by value keeps execution independent of the queue, so a
	// command's handler may safely enqueue follow-up commands for next frame.
	std::vector<EditorCommand> Drain()
	{
		std::vector<EditorCommand> drained;
		drained.swap(commands);
		return drained;
	}

private:
	std::vector<EditorCommand> commands;
};
