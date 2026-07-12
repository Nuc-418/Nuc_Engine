// UndoStack: editor command history for transform edits, spawns and deletes.
// Objects are referenced by their stable World id, so history stays valid
// across delete/respawn cycles.

#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "engine/scene/World.h"

struct TransformState
{
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};

TransformState CaptureTransform(const GameObject& object);
void ApplyTransform(GameObject& object, const TransformState& state);
bool TransformStatesEqual(const TransformState& a, const TransformState& b);

class UndoStack
{
public:
	void RecordTransform(unsigned long long id, const TransformState& before, const TransformState& after);
	void RecordSpawn(ObjectType type, const std::string& name, unsigned long long id, const TransformState& state);
	void RecordDelete(ObjectType type, const std::string& name, unsigned long long id, const TransformState& state);
	void RecordRename(unsigned long long id, const std::string& before, const std::string& after);
	void RecordLights(const VectorLight& before, const VectorLight& after);

	// Returns the id of the affected object (0 if nothing to undo/redo).
	unsigned long long Undo(World& world);
	unsigned long long Redo(World& world);

	bool CanUndo() const { return !undoActions.empty(); }
	bool CanRedo() const { return !redoActions.empty(); }

	// History references world ids; call after anything that rebuilds the
	// world wholesale (scene load).
	void Clear();

private:
	struct Action
	{
		enum class Kind { Transform, Spawn, Delete, Rename, Lights };
		Kind kind;
		unsigned long long id = 0;
		ObjectType type = ObjectType::Cube;
		std::string name;           // Spawn/Delete: object name; Rename: name before the edit
		std::string nameAfter;      // Rename only
		TransformState before = {}; // Transform: pre-edit; Spawn/Delete: state at record time
		TransformState after = {};  // Transform only: post-edit
		VectorLight lightsBefore;   // Lights only
		VectorLight lightsAfter;    // Lights only
	};

	// Applies one action in the given direction and returns the affected id.
	unsigned long long Apply(World& world, const Action& action, bool undo);
	void Push(Action action);

	std::vector<Action> undoActions;
	std::vector<Action> redoActions;
};
