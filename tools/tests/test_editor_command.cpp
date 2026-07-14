// EditorCommandQueue: FIFO enqueue/drain semantics used by the editor's
// UI -> EditorHost command seam.

#include "doctest/doctest.h"

#include "engine/editor/EditorCommand.h"

TEST_CASE("a new queue is empty")
{
	EditorCommandQueue queue;
	CHECK(queue.Empty());
	CHECK(queue.Drain().empty());
}

TEST_CASE("commands drain in insertion order")
{
	EditorCommandQueue queue;
	queue.Push(EditorCommandType::SaveScene);
	queue.Push(EditorCommandType::LoadScene, "assets/scenes/a.json");
	queue.Push(EditorCommandType::Play);

	CHECK_FALSE(queue.Empty());

	std::vector<EditorCommand> drained = queue.Drain();
	REQUIRE(drained.size() == 3);
	CHECK(drained[0].type == EditorCommandType::SaveScene);
	CHECK(drained[0].path.empty());
	CHECK(drained[1].type == EditorCommandType::LoadScene);
	CHECK(drained[1].path == "assets/scenes/a.json");
	CHECK(drained[2].type == EditorCommandType::Play);
}

TEST_CASE("draining leaves the queue empty")
{
	EditorCommandQueue queue;
	queue.Push(EditorCommandType::Exit);
	(void)queue.Drain();
	CHECK(queue.Empty());
	CHECK(queue.Drain().empty());
}

TEST_CASE("a handler may enqueue follow-up commands during a drain")
{
	// Drain returns by value, so the queue is independent of the drained
	// commands: re-pushing while iterating them is safe and defers to next drain.
	EditorCommandQueue queue;
	queue.Push(EditorCommandType::NewMap, "assets/scenes/new.json");

	for (const EditorCommand& command : queue.Drain()) {
		if (command.type == EditorCommandType::NewMap)
			queue.Push(EditorCommandType::SaveScene, command.path);
	}

	std::vector<EditorCommand> followUp = queue.Drain();
	REQUIRE(followUp.size() == 1);
	CHECK(followUp[0].type == EditorCommandType::SaveScene);
	CHECK(followUp[0].path == "assets/scenes/new.json");
}
