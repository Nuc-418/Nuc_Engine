// PluginSort: dependency ordering for plugins (pure logic, unit-testable).

#pragma once

#include <string>
#include <vector>

struct PluginSortResult
{
	// Indices into the input arrays, dependencies-first. Always a complete
	// permutation: on cycles or missing dependencies the offenders are
	// appended in their original order so the engine can still run.
	std::vector<size_t> order;

	// Human-readable problems (missing dependency, cycle). Empty = clean.
	std::vector<std::string> errors;
};

// Stable topological sort: `dependencies[i]` names the plugins that must come
// before `names[i]`. Ties keep registration order.
PluginSortResult SortByDependencies(const std::vector<std::string>& names,
                                    const std::vector<std::vector<std::string>>& dependencies);
