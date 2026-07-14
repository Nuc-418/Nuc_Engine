// ServiceRegistry: interface-keyed provide/get/withdraw, including polymorphic
// retrieval through a base interface.

#include "doctest/doctest.h"

#include "engine/core/ServiceRegistry.h"

namespace {

struct IGreeter
{
	virtual ~IGreeter() = default;
	virtual int Value() const = 0;
};

struct Greeter : IGreeter
{
	int v = 0;
	int Value() const override { return v; }
};

struct IOtherService
{
	virtual ~IOtherService() = default;
};

} // namespace

TEST_CASE("an empty registry has and gets nothing")
{
	ServiceRegistry registry;
	CHECK_FALSE(registry.Has<IGreeter>());
	CHECK(registry.Get<IGreeter>() == nullptr);
}

TEST_CASE("a provided service is retrievable through its interface")
{
	ServiceRegistry registry;
	Greeter greeter;
	greeter.v = 42;

	registry.Provide<IGreeter>(&greeter);

	CHECK(registry.Has<IGreeter>());
	IGreeter* found = registry.Get<IGreeter>();
	REQUIRE(found != nullptr);
	CHECK(found->Value() == 42);  // dispatches to the concrete Greeter
}

TEST_CASE("interfaces are independent keys")
{
	ServiceRegistry registry;
	Greeter greeter;
	registry.Provide<IGreeter>(&greeter);

	CHECK(registry.Has<IGreeter>());
	CHECK_FALSE(registry.Has<IOtherService>());
	CHECK(registry.Get<IOtherService>() == nullptr);
}

TEST_CASE("providing again replaces the entry")
{
	ServiceRegistry registry;
	Greeter first, second;
	first.v = 1;
	second.v = 2;

	registry.Provide<IGreeter>(&first);
	registry.Provide<IGreeter>(&second);

	CHECK(registry.Get<IGreeter>()->Value() == 2);
}

TEST_CASE("withdraw removes a service")
{
	ServiceRegistry registry;
	Greeter greeter;
	registry.Provide<IGreeter>(&greeter);
	registry.Withdraw<IGreeter>();

	CHECK_FALSE(registry.Has<IGreeter>());
	CHECK(registry.Get<IGreeter>() == nullptr);
}

TEST_CASE("providing null clears the entry")
{
	ServiceRegistry registry;
	Greeter greeter;
	registry.Provide<IGreeter>(&greeter);
	registry.Provide<IGreeter>(nullptr);

	CHECK_FALSE(registry.Has<IGreeter>());
}
