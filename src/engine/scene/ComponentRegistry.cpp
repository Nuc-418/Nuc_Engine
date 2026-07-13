// ComponentRegistry: implementation over function-local statics so registration
// from static initializers (in any TU) is order-safe.

#include "engine/scene/ComponentRegistry.h"
#include "engine/scene/Component.h"

#include <map>

namespace {

struct TypeInfo { std::string label; ComponentRegistry::Factory factory; };

std::map<std::string, TypeInfo>& Types()
{
	static std::map<std::string, TypeInfo> types;
	return types;
}

std::vector<std::string>& Order()
{
	static std::vector<std::string> order;
	return order;
}

} // namespace

void ComponentRegistry::Register(const std::string& typeId, const std::string& label, Factory factory)
{
	if (Types().find(typeId) == Types().end())
		Order().push_back(typeId);
	Types()[typeId] = { label.empty() ? typeId : label, std::move(factory) };
}

std::unique_ptr<Component> ComponentRegistry::Create(const std::string& typeId)
{
	auto it = Types().find(typeId);
	return it == Types().end() ? nullptr : it->second.factory();
}

bool ComponentRegistry::IsRegistered(const std::string& typeId)
{
	return Types().count(typeId) != 0;
}

const std::vector<std::string>& ComponentRegistry::TypeIds()
{
	return Order();
}

const std::string& ComponentRegistry::Label(const std::string& typeId)
{
	auto it = Types().find(typeId);
	return it != Types().end() ? it->second.label : typeId;
}
