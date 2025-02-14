#ifndef MOD_LIB_GRAPH_MULTISET_HPP
#define MOD_LIB_GRAPH_MULTISET_HPP

#include <boost/functional/hash.hpp>

#include <algorithm>
#include <vector>

namespace mod::lib::graph {

template<typename GraphType>
struct Multiset {
	Multiset() = default;

	explicit Multiset(GraphType *g) : graphs(1, g) {}

	explicit Multiset(std::vector<GraphType *> graphs) : graphs(std::move(graphs)) {
		this->graphs.shrink_to_fit();
		std::sort(this->graphs.begin(), this->graphs.end(), typename std::remove_cv<GraphType>::type::IdLess());
	}

	auto begin() const {
		return graphs.begin();
	}

	auto end() const {
		return graphs.end();
	}

	bool empty() const {
		return graphs.empty();
	}

	std::size_t size() const {
		return graphs.size();
	}

	friend bool operator==(const Multiset &l, const Multiset &r) {
		return l.graphs == r.graphs;
	}

	friend bool operator<(const Multiset &l, const Multiset &r) {
		return l.graphs < r.graphs;
	}

private:
	std::vector<GraphType *> graphs;
};

} // namespace mod::lib::graph

template<typename GraphType>
struct std::hash<mod::lib::graph::Multiset<GraphType>> {
	std::size_t operator()(const mod::lib::graph::Multiset<GraphType> &gs) const {
		return boost::hash_range(gs.begin(), gs.end());
	}
};

#endif // MOD_LIB_GRAPH_MULTISET_HPP