// Details panel: inspector for the selected object.

#include "engine/editor/panels/DetailsPanel.h"
#include "engine/editor/Editor.h"
#include "engine/scene/Component.h"
#include "engine/scene/ComponentRegistry.h"
#include "engine/scene/FieldStore.h"
#include "engine/scene/Serialization.h"

#include <glm/glm.hpp>
#include <cstring>
#include <string>
#include <vector>

namespace {

/* Property reflection: a component's Serialize doubles as its property
   enumeration. PropertyCapture records name/type/value (with the color/enum
   hints); the panel draws a widget per property and, on change, feeds the
   edited set back through Deserialize. Works for ANY registered component —
   including plugin ones the editor cannot name. */
struct Property
{
	enum class Kind { Float, Int, Bool, Vec3, Color, Enum, String };
	Kind kind;
	std::string name;
	float floatValue = 0.0f;
	int intValue = 0;
	bool boolValue = false;
	glm::vec3 vec3Value = glm::vec3(0.0f);
	std::string stringValue;
	const char* const* enumLabels = nullptr;
	int enumLabelCount = 0;
};

struct PropertyCapture : ISerializer
{
	std::vector<Property> properties;

	Property& Add(Property::Kind kind, const char* key)
	{
		properties.push_back(Property());
		properties.back().kind = kind;
		properties.back().name = key;
		return properties.back();
	}
	void Write(const char* key, float value) override { Add(Property::Kind::Float, key).floatValue = value; }
	void Write(const char* key, int value) override { Add(Property::Kind::Int, key).intValue = value; }
	void Write(const char* key, bool value) override { Add(Property::Kind::Bool, key).boolValue = value; }
	void Write(const char* key, const glm::vec3& value) override { Add(Property::Kind::Vec3, key).vec3Value = value; }
	void Write(const char* key, const std::string& value) override { Add(Property::Kind::String, key).stringValue = value; }
	void WriteColor(const char* key, const glm::vec3& value) override { Add(Property::Kind::Color, key).vec3Value = value; }
	void WriteEnum(const char* key, int value, const char* const* labels, int labelCount) override
	{
		Property& p = Add(Property::Kind::Enum, key);
		p.intValue = value;
		p.enumLabels = labels;
		p.enumLabelCount = labelCount;
	}
};

/* IDeserializer over the edited property list, for Component::Deserialize. */
struct PropertyApply : IDeserializer
{
	const std::vector<Property>& properties;
	explicit PropertyApply(const std::vector<Property>& props) : properties(props) {}

	const Property* Find(const char* key) const
	{
		for (const Property& p : properties)
			if (p.name == key)
				return &p;
		return nullptr;
	}
	float ReadFloat(const char* key, float fallback) const override
	{ const Property* p = Find(key); return p && p->kind == Property::Kind::Float ? p->floatValue : fallback; }
	int ReadInt(const char* key, int fallback) const override
	{
		const Property* p = Find(key);
		return p && (p->kind == Property::Kind::Int || p->kind == Property::Kind::Enum) ? p->intValue : fallback;
	}
	bool ReadBool(const char* key, bool fallback) const override
	{ const Property* p = Find(key); return p && p->kind == Property::Kind::Bool ? p->boolValue : fallback; }
	glm::vec3 ReadVec3(const char* key, const glm::vec3& fallback) const override
	{
		const Property* p = Find(key);
		return p && (p->kind == Property::Kind::Vec3 || p->kind == Property::Kind::Color) ? p->vec3Value : fallback;
	}
	std::string ReadString(const char* key, const std::string& fallback) const override
	{ const Property* p = Find(key); return p && p->kind == Property::Kind::String ? p->stringValue : fallback; }
};

} // namespace

void DrawDetailsPanel(Editor& editor)
{
	ImGui::Begin("Details");

	GameObject* object = editor.selected;
	if (!object) {
		ImGui::TextDisabled("Select an object in the Outliner or Viewport.");
		ImGui::End();
		return;
	}

	/* Type (looked up in the world entry) */
	for (const WorldEntry& entry : editor.world->entries) {
		if (entry.object.get() == object) {
			ImGui::TextDisabled("Type: %s", editor.world->TypeLabel(entry.typeId).c_str());
			break;
		}
	}

	/* Name */
	unsigned long long renameId = editor.world->IdOf(object);
	char nameBuffer[128];
	strncpy(nameBuffer, object->name.c_str(), sizeof(nameBuffer) - 1);
	nameBuffer[sizeof(nameBuffer) - 1] = '\0';
	if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
		object->name = nameBuffer;
	if (ImGui::IsItemActivated())
		editor.nameBefore = nameBuffer;
	if (ImGui::IsItemDeactivatedAfterEdit())
		editor.undoStack.RecordRename(renameId, editor.nameBefore, object->name);

	ImGui::SeparatorText("Transform");

	/* Each widget edit becomes one undo entry: snapshot on activation,
	   record when the widget is released. */
	unsigned long long objectId = editor.world->IdOf(object);
	auto trackEdit = [&]() {
		if (ImGui::IsItemActivated())
			editor.dragBefore = CaptureTransform(*object);
		if (ImGui::IsItemDeactivatedAfterEdit())
			editor.undoStack.RecordTransform(objectId, editor.dragBefore, CaptureTransform(*object));
	};

	Transform& transform = object->transform;
	ImGui::DragFloat3("Position", &transform.position.x, 0.1f);
	trackEdit();

	/* Rotation is stored in radians (engine order: yaw-Y, pitch-X, roll-Z);
	   shown in degrees like UE. */
	glm::vec3 degreesRot = glm::degrees(transform.rotation);
	if (ImGui::DragFloat3("Rotation", &degreesRot.x, 1.0f))
		transform.rotation = glm::radians(degreesRot);
	trackEdit();

	ImGui::DragFloat3("Scale", &transform.scale.x, 0.05f);
	trackEdit();

	/* Components: an auto-generated editor per component, driven by the
	   property reflection above. Each widget edit is one undo entry
	   (component state snapshotted on activation, recorded on release). */
	ImGui::SeparatorText("Components");
	for (const std::unique_ptr<Component>& component : object->Components()) {
		ImGui::PushID(component.get());

		PropertyCapture capture;
		component->Serialize(capture);

		if (capture.properties.empty()) {
			ImGui::BulletText("%s", component->DisplayName());
		} else if (ImGui::TreeNodeEx(component->DisplayName(),
		                             ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth)) {
			bool edited = false;
			auto trackComponentEdit = [&]() {
				if (ImGui::IsItemActivated()) {
					editor.componentBefore = FieldStore();
					component->Serialize(editor.componentBefore);
				}
				if (ImGui::IsItemDeactivatedAfterEdit()) {
					FieldStore after;
					component->Serialize(after);
					editor.undoStack.RecordComponentEdit(objectId, component->TypeId(),
					                                     editor.componentBefore, after);
				}
			};

			for (Property& property : capture.properties) {
				const char* label = property.name.c_str();
				bool changed = false;
				switch (property.kind) {
				case Property::Kind::Float:
					changed = ImGui::DragFloat(label, &property.floatValue, 0.05f);
					break;
				case Property::Kind::Int:
					changed = ImGui::DragInt(label, &property.intValue);
					break;
				case Property::Kind::Bool:
					changed = ImGui::Checkbox(label, &property.boolValue);
					break;
				case Property::Kind::Vec3:
					changed = ImGui::DragFloat3(label, &property.vec3Value.x, 0.05f);
					break;
				case Property::Kind::Color:
					changed = ImGui::ColorEdit3(label, &property.vec3Value.x);
					break;
				case Property::Kind::Enum:
					changed = ImGui::Combo(label, &property.intValue,
					                       property.enumLabels, property.enumLabelCount);
					break;
				case Property::Kind::String: {
					char buffer[128];
					strncpy(buffer, property.stringValue.c_str(), sizeof(buffer) - 1);
					buffer[sizeof(buffer) - 1] = '\0';
					if (ImGui::InputText(label, buffer, sizeof(buffer))) {
						property.stringValue = buffer;
						changed = true;
					}
					break;
				}
				}
				if (changed)
					edited = true;
				trackComponentEdit();
			}

			if (edited) {
				PropertyApply apply(capture.properties);
				component->Deserialize(apply);
			}

			/* World-state extras that are not component properties. */
			if (strcmp(component->TypeId(), "Camera") == 0) {
				bool isActive = editor.world->activeCameraId == objectId;
				if (isActive) {
					ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Active camera (Play renders through this)");
					if (ImGui::Button("Clear Active Camera"))
						editor.world->activeCameraId = 0;
				} else if (ImGui::Button("Make Active Camera")) {
					editor.world->activeCameraId = objectId;
				}
			}

			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	if (ImGui::Button("Add Component"))
		ImGui::OpenPopup("AddComponentPopup");
	if (ImGui::BeginPopup("AddComponentPopup")) {
		for (const std::string& typeId : ComponentRegistry::TypeIds()) {
			if (ImGui::Selectable(ComponentRegistry::Label(typeId).c_str()))
				object->AddComponentById(typeId);
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}
