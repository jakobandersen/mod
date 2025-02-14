#ifndef MOD_LIB_DG_STRATEGIES_GRAPHSTATE_HPP
#define MOD_LIB_DG_STRATEGIES_GRAPHSTATE_HPP

#include <boost/iterator/transform_iterator.hpp>

#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <vector>

namespace mod::lib {
namespace graph {
class Graph;
} // namespace graph
namespace DG::Strategies {

struct GraphState {
	using GraphList = std::vector<const lib::graph::Graph *>;

	struct Subset {
		struct Transformer {
			Transformer(const GraphList &graphs) : graphs(graphs) {}
			GraphList::value_type operator()(int i) const {
				assert(i < graphs.size());
				return graphs[i];
			}
		private:
			const GraphList &graphs;
		};
		using const_iterator = boost::transform_iterator<Transformer, std::vector<int>::const_iterator, const lib::graph::Graph *>;
		using size_type = std::vector<int>::size_type;
	public:
		explicit Subset(const GraphState &rs) : rs(rs) {}
		explicit Subset(const GraphState &rs, const Subset &other) : rs(rs), indices(other.indices) {}

		const_iterator begin() const {
			return const_iterator(indices.begin(), Transformer(rs.getUniverse()));
		}

		const_iterator end() const {
			return const_iterator(indices.end(), Transformer(rs.getUniverse()));
		}

		size_type size() const {
			return indices.size();
		}

		bool empty() const {
			return indices.empty();
		}

		std::vector<int> getIndices() const {
			return indices;
		}
	private:
		friend class GraphState;
		const GraphState &rs;
		std::vector<int> indices;
	};
private:
	GraphState &operator=(const GraphState &other); // disable
public:
	explicit GraphState();
	explicit GraphState(const GraphState &other);
	explicit GraphState(const std::vector<const lib::graph::Graph *> &universe);
	explicit GraphState(const std::vector<const GraphState *> &resultSets);
	~GraphState();
	void addToSubset(const lib::graph::Graph *g);
	void addToUniverse(const lib::graph::Graph *g);
	template<typename T>
	struct Compare;
	template<typename T>
	void sortUniverse(const T compare);
	template<typename T>
	void sortSubset(const T compare);
	const Subset &getSubset() const;
	const GraphList &getUniverse() const;
	bool isInUniverse(const lib::graph::Graph *g) const;
	friend bool operator==(const GraphState &a, const GraphState &b);
private:
	int addUniverseGetIndex(const lib::graph::Graph *g);
private:
	GraphList universe;
	Subset subset;
};

template<typename T>
class GraphState::Compare {
	Compare();
public:
	explicit Compare(const GraphState::GraphList &graphs, T compare)
			: graphs(graphs), compare(compare) {}

	bool operator()(int a, int b) {
		assert(a < graphs.size());
		assert(b < graphs.size());
		assert(graphs[a]);
		assert(graphs[b]);
		return compare(graphs[a], graphs[b]);
	}
	const GraphState::GraphList &graphs;
	T compare;
};

template<typename T>
void GraphState::sortUniverse(T compare) {
	std::vector<int> newToOld(universe.size());
	for(int i = 0; i != newToOld.size(); i++) newToOld[i] = i;
	Compare<T> comp(universe, compare);
	std::stable_sort(newToOld.begin(), newToOld.end(), comp);
	std::vector<int> oldToNew(universe.size());
	for(int i = 0; i != universe.size(); i++)
		oldToNew[newToOld[i]] = i;
	{
		GraphList newUniverse(universe.size());
		for(int i = 0; i != universe.size(); i++)
			newUniverse[i] = universe[newToOld[i]];
		std::swap(newUniverse, universe);
	}
	{ // TODO: remove, stupid sanity check
		for(int i = 1; i < universe.size(); i++)
			assert(!compare(universe[i], universe[i - 1]));
	}
	for(unsigned int i = 0; i != subset.indices.size(); i++)
		subset.indices[i] = oldToNew[subset.indices[i]];
}

template<typename T>
void GraphState::sortSubset(T compare) {
	Compare<T> comp(universe, compare);
	std::stable_sort(subset.indices.begin(), subset.indices.end(), comp);
}

} // namespace DG::Strategies
} // namespace mod::lib

#endif // MOD_LIB_DG_STRATEGIES_GRAPHSTATE_HPP