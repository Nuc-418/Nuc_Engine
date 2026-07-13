#include "engine/plugin/PluginSort.h"

PluginSortResult SortByDependencies(const std::vector<std::string>& names,
                                    const std::vector<std::vector<std::string>>& dependencies)
{
	PluginSortResult result;
	const size_t count = names.size();

	// Edges: dependency index -> dependents; count unmet deps per plugin.
	std::vector<std::vector<size_t>> dependents(count);
	std::vector<size_t> unmet(count, 0);
	for (size_t i = 0; i < count; ++i) {
		for (const std::string& dependency : dependencies[i]) {
			size_t at = count;
			for (size_t j = 0; j < count; ++j)
				if (names[j] == dependency) { at = j; break; }
			if (at == count) {
				result.errors.push_back("plugin '" + names[i] + "' depends on missing plugin '"
				                        + dependency + "'");
				continue; // ignore the edge; the plugin still loads
			}
			dependents[at].push_back(i);
			unmet[i]++;
		}
	}

	// Kahn's algorithm, stable: always pick the lowest-index ready plugin.
	std::vector<bool> emitted(count, false);
	while (result.order.size() < count) {
		size_t pick = count;
		for (size_t i = 0; i < count; ++i)
			if (!emitted[i] && unmet[i] == 0) { pick = i; break; }

		if (pick == count) {
			// Everything left is part of (or blocked by) a cycle. Emit the
			// remainder in registration order and report it.
			std::string members;
			for (size_t i = 0; i < count; ++i) {
				if (!emitted[i]) {
					if (!members.empty()) members += ", ";
					members += names[i];
					emitted[i] = true;
					result.order.push_back(i);
				}
			}
			result.errors.push_back("plugin dependency cycle involving: " + members);
			break;
		}

		emitted[pick] = true;
		result.order.push_back(pick);
		for (size_t dependent : dependents[pick])
			if (unmet[dependent] > 0)
				unmet[dependent]--;
	}
	return result;
}
