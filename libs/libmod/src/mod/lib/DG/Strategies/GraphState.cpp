#include "GraphState.hpp"

#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/IO/IO.hpp>

namespace mod::lib::DG::Strategies {

GraphState::GraphState() : subset(*this) {}

GraphState::GraphState(const GraphState &other)
		: universe(other.universe), subset(*this, other.subset) {}

GraphState::GraphState(const std::vector<const lib::graph::Graph *> &universe)
		: universe(universe), subset(*this) {}

GraphState::GraphState(const std::vector<const GraphState *> &resultSets) : subset(*this) {
	// collect universe
	for(const GraphState *rs : resultSets) {
		GraphState::GraphList other(rs->getUniverse());
		std::sort(other.begin(), other.end(), [](const lib::graph::Graph *g1, const lib::graph::Graph *g2) {
			return g1->getId() < g2->getId();
		});

		GraphState::GraphList result;
		std::set_union(universe.begin(), universe.end(), other.begin(), other.end(),
		               std::back_inserter(result),
		               lib::graph::Graph::IdLess());
		std::swap(result, universe);
	}
	// collect the subset
	std::set<const lib::graph::Graph *, lib::graph::Graph::IdLess> newSubset;
	for(const GraphState *rs : resultSets)
		for(const lib::graph::Graph *g : rs->subset)
			newSubset.insert(g);

	using GraphToIndex = std::map<const lib::graph::Graph *, int>;
	GraphToIndex graphToIndex;
	for(int i = 0; i != universe.size(); i++)
		graphToIndex[universe[i]] = i;

	for(const lib::graph::Graph *g : newSubset)
		subset.indices.push_back(graphToIndex[g]);
}

GraphState::~GraphState() {}

void GraphState::addToSubset(const lib::graph::Graph *g) {
	const int gIndex = addUniverseGetIndex(g);
	for(int index : subset.indices)
		if(index == gIndex) return;
	subset.indices.push_back(gIndex);
}

void GraphState::addToUniverse(const lib::graph::Graph *g) {
	addUniverseGetIndex(g);
}

const GraphState::Subset &GraphState::getSubset() const {
	return subset;
}

const GraphState::GraphList &GraphState::getUniverse() const {
	return universe;
}

bool GraphState::isInUniverse(const lib::graph::Graph *g) const {
	for(int i = 0; i != universe.size(); i++)
		if(universe[i] == g)
			return true;
	return false;
}

bool operator==(const GraphState &a, const GraphState &b) {
	if(a.universe.size() != b.universe.size()) return false;
	if(a.subset.size() != b.subset.size()) return false;
	GraphState aCopy(a), bCopy(b);
	std::less<const lib::graph::Graph *> comp;
	aCopy.sortUniverse(comp);
	bCopy.sortUniverse(comp);
	for(int i = 0; i != a.universe.size(); i++)
		if(aCopy.universe[i] != bCopy.universe[i])
			return false;
	aCopy.sortSubset(comp);
	bCopy.sortSubset(comp);
	for(int i = 0; i != aCopy.subset.size(); i++)
		if(*(aCopy.subset.begin() + i) != *(bCopy.subset.begin() + i))
			return false;
	return true;
}

int GraphState::addUniverseGetIndex(const lib::graph::Graph *g) {
	for(int i = 0; i != universe.size(); i++)
		if(universe[i] == g)
			return i;
	universe.push_back(g);
	return universe.size() - 1;
}

} // namespace mod::lib::DG::Strategies