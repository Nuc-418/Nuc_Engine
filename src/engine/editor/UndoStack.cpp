// UndoStack: editor command history for transform edits, spawns and deletes.

#include "engine/editor/UndoStack.h"

static const size_t kMaxHistory = 128;

TransformState CaptureTransform(const GameObject& object)
{
	TransformState state;
	state.position = object.transform.position;
	state.rotation = object.transform.rotation;
	state.scale = object.transform.scale;
	return state;
}

void ApplyTransform(GameObject& object, const TransformState& state)
{
	object.transform.position = state.position;
	object.transform.rotation = state.rotation;
	object.transform.scale = state.scale;
}

bool TransformStatesEqual(const TransformState& a, const TransformState& b)
{
	return a.position == b.position && a.rotation == b.rotation && a.scale == b.scale;
}

void UndoStack::Push(Action action)
{
	undoActions.push_back(action);
	if (undoActions.size() > kMaxHistory)
		undoActions.erase(undoActions.begin());
	redoActions.clear(); // a fresh edit invalidates the redo branch
}

void UndoStack::RecordTransform(unsigned long long id, const TransformState& before, const TransformState& after)
{
	if (id == 0 || TransformStatesEqual(before, after))
		return;
	Action action;
	action.kind = Action::Kind::Transform;
	action.id = id;
	action.before = before;
	action.after = after;
	Push(action);
}

void UndoStack::RecordSpawn(const std::string& typeId, const std::string& name, unsigned long long id, const TransformState& state)
{
	Action action;
	action.kind = Action::Kind::Spawn;
	action.typeId = typeId;
	action.name = name;
	action.id = id;
	action.before = state;
	Push(action);
}

void UndoStack::RecordDelete(const std::string& typeId, const std::string& name, unsigned long long id, const TransformState& state)
{
	Action action;
	action.kind = Action::Kind::Delete;
	action.typeId = typeId;
	action.name = name;
	action.id = id;
	action.before = state;
	Push(action);
}

void UndoStack::RecordRename(unsigned long long id, const std::string& before, const std::string& after)
{
	if (id == 0 || before == after)
		return;
	Action action;
	action.kind = Action::Kind::Rename;
	action.id = id;
	action.name = before;
	action.nameAfter = after;
	Push(action);
}

void UndoStack::RecordLights(const VectorLight& before, const VectorLight& after)
{
	Action action;
	action.kind = Action::Kind::Lights;
	action.lightsBefore = before;
	action.lightsAfter = after;
	Push(action);
}

unsigned long long UndoStack::Undo(World& world)
{
	if (undoActions.empty())
		return 0;
	Action action = undoActions.back();
	undoActions.pop_back();
	unsigned long long affected = Apply(world, action, /*undo=*/true);
	redoActions.push_back(action);
	return affected;
}

unsigned long long UndoStack::Redo(World& world)
{
	if (redoActions.empty())
		return 0;
	Action action = redoActions.back();
	redoActions.pop_back();
	unsigned long long affected = Apply(world, action, /*undo=*/false);
	undoActions.push_back(action);
	return affected;
}

unsigned long long UndoStack::Apply(World& world, const Action& action, bool undo)
{
	switch (action.kind) {
	case Action::Kind::Transform: {
		GameObject* object = world.FindById(action.id);
		if (object)
			ApplyTransform(*object, undo ? action.before : action.after);
		return action.id;
	}
	case Action::Kind::Spawn: {
		if (undo) {
			GameObject* object = world.FindById(action.id);
			if (object)
				world.Destroy(object);
		} else {
			GameObject* object = world.Spawn(action.typeId, action.name, action.id);
			if (object)
				ApplyTransform(*object, action.before);
		}
		return action.id;
	}
	case Action::Kind::Rename: {
		GameObject* object = world.FindById(action.id);
		if (object)
			object->name = undo ? action.name : action.nameAfter;
		return action.id;
	}
	case Action::Kind::Lights: {
		world.lights.lightInfo = undo ? action.lightsBefore : action.lightsAfter;
		world.UploadLights();
		return 0;
	}
	case Action::Kind::Delete: {
		if (undo) {
			GameObject* object = world.Spawn(action.typeId, action.name, action.id);
			if (object)
				ApplyTransform(*object, action.before);
		} else {
			GameObject* object = world.FindById(action.id);
			if (object)
				world.Destroy(object);
		}
		return action.id;
	}
	}
	return 0;
}

void UndoStack::Clear()
{
	undoActions.clear();
	redoActions.clear();
}
