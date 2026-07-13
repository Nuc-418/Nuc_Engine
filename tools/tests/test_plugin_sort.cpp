// Plugin dependency ordering: topological, stable, loud on bad graphs.

#include "doctest/doctest.h"

#include "engine/plugin/PluginSort.h"

TEST_CASE("independent plugins keep registration order")
{
	PluginSortResult r = SortByDependencies({ "A", "B", "C" }, { {}, {}, {} });
	CHECK(r.order == std::vector<size_t>{ 0, 1, 2 });
	CHECK(r.errors.empty());
}

TEST_CASE("dependencies come first")
{
	// C depends on A and B; B depends on A. Expect A, B, C regardless of
	// registration order.
	PluginSortResult r = SortByDependencies({ "C", "B", "A" },
	                                        { { "A", "B" }, { "A" }, {} });
	CHECK(r.order == std::vector<size_t>{ 2, 1, 0 });
	CHECK(r.errors.empty());
}

TEST_CASE("a missing dependency is reported and ignored")
{
	PluginSortResult r = SortByDependencies({ "A" }, { { "Ghost" } });
	CHECK(r.order == std::vector<size_t>{ 0 }); // still loads
	REQUIRE(r.errors.size() == 1);
	CHECK(r.errors[0].find("Ghost") != std::string::npos);
}

TEST_CASE("a cycle is reported and falls back to registration order")
{
	PluginSortResult r = SortByDependencies({ "A", "B" }, { { "B" }, { "A" } });
	CHECK(r.order == std::vector<size_t>{ 0, 1 });
	REQUIRE(r.errors.size() == 1);
	CHECK(r.errors[0].find("cycle") != std::string::npos);
}
