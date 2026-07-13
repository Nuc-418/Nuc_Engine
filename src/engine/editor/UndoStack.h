// UndoStack: editor command history for transform edits, spawns and deletes.
// Objects are referenced by their stable World id, so history stays valid
// across delete/respawn cycles.

#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "engine/scene/World.h"
#include "engine/scene/FieldStore.h"

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
	void RecordSpawn(const std::string& typeId, const std::string& name, unsigned long long id, const TransformState& state);
	void RecordDelete(const std::string& typeId, const std::string& name, unsigned long long id, const TransformState& state);
	void RecordRename(unsigned long long id, const std::string& before, const std::string& after);
	void RecordLights(const VectorLight& before, const VectorLight& after);
	// Reparent (Outliner drag): parent ids may be 0 (root). The local
	// transform changes when the world pose is kept, so both states ride along.
	void RecordReparent(unsigned long long id, unsigned long long parentBefore, unsigned long long parentAfter,
	                    const TransformState& before, const TransformState& after);
	// Component property edit (Details panel): before/after snapshots of the
	// component's serialized state, applied back via Deserialize.
	void RecordComponentEdit(unsigned long long id, const std::string& componentTypeId,
	                         const FieldStore& before, const FieldStore& after);

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
		enum class Kind { Transform, Spawn, Delete, Rename, Lights, Reparent, ComponentEdit };
		Kind kind;
		unsigned long long id = 0;
		std::string typeId;         // Spawn/Delete: the type to respawn
		std::string name;           // Spawn/Delete: object name; Rename: name before the edit
		std::string nameAfter;      // Rename only
		TransformState before = {}; // Transform/Reparent: pre-edit; Spawn/Delete: state at record time
		TransformState after = {};  // Transform/Reparent: post-edit
		VectorLight lightsBefore;   // Lights only
		VectorLight lightsAfter;    // Lights only
		unsigned long long parentBefore = 0; // Reparent only (0 = root)
		unsigned long long parentAfter = 0;  // Reparent only
		std::string componentTypeId;         // ComponentEdit only
		FieldStore fieldsBefore;             // ComponentEdit only
		FieldStore fieldsAfter;              // ComponentEdit only
	};

	// Applies one action in the given direction and returns the affected id.
	unsigned long long Apply(World& world, const Action& action, bool undo);
	void Push(Action action);

	std::vector<Action> undoActions;
	std::vector<Action> redoActions;
};
