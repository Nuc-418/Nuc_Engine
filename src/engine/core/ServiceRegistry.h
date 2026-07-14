// ServiceRegistry: an interface-keyed locator for engine services.
//
// The engine's decoupling seam: a provider (a plugin, or the Application
// itself) publishes an implementation under an interface type, and any consumer
// with the Application in scope fetches it by that interface — without knowing
// the concrete class. This is what lets a plugin depend on `IPhysicsQuery`
// rather than `JoltPhysicsPlugin`, and keeps `PluginManager::GetOrAdd<T>` as
// typed sugar for the "I know the concrete type" case.
//
// Ownership: the registry stores only non-owning pointers. Whoever Provides a
// service owns it and must Withdraw (or outlive the registry) — plugins provide
// in OnLoad and withdraw in OnUnload.

#pragma once

#include <typeindex>
#include <unordered_map>

class ServiceRegistry
{
public:
	// Publishes `service` under interface type `Interface`. A null pointer, or
	// re-providing the same interface, replaces any previous entry (passing null
	// clears it). The stored pointer is only ever handed back as `Interface*`,
	// so the void* round-trip is well-defined.
	template <class Interface>
	void Provide(Interface* service)
	{
		std::type_index key(typeid(Interface));
		if (service)
			services[key] = static_cast<void*>(service);
		else
			services.erase(key);
	}

	// Removes the entry for `Interface`, if any.
	template <class Interface>
	void Withdraw()
	{
		services.erase(std::type_index(typeid(Interface)));
	}

	// The service registered under `Interface`, or nullptr if none.
	template <class Interface>
	Interface* Get() const
	{
		auto it = services.find(std::type_index(typeid(Interface)));
		return it == services.end() ? nullptr : static_cast<Interface*>(it->second);
	}

	// Whether a service is registered under `Interface`.
	template <class Interface>
	bool Has() const
	{
		return services.find(std::type_index(typeid(Interface))) != services.end();
	}

private:
	std::unordered_map<std::type_index, void*> services;
};
