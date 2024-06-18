#ifndef MOD_LIB_GRAPH_MORPHISM_FINDER_HPP
#define MOD_LIB_GRAPH_MORPHISM_FINDER_HPP

#include <jla_boost/graph/morphism/VertexOrderByMult.hpp>

namespace mod::lib::GraphMorphism {
namespace {

template<typename Graph>
std::vector<typename boost::graph_traits<Graph>::vertex_descriptor>
vertex_order_by_max_connectivity(const Graph &g) {
	std::vector<typename boost::graph_traits<Graph>::vertex_descriptor> vertex_order, candidates;
	vertex_order.reserve(num_vertices(g));
	candidates.reserve(num_vertices(g));
	std::vector<int> deg(num_vertices(g), 0);
	std::vector<bool> used(num_vertices(g), false), is_candidate(num_vertices(g), false);

//	std::cout << "graph:\n";
//	for(const auto v: asRange(vertices(g))) {
//		std::cout << get(boost::vertex_index_t(), g, v) << ":";
//		for(const auto e: asRange(out_edges(v, g)))
//			std::cout << " " << get(boost::vertex_index_t(), g, target(e, g));
//		std::cout << std::endl;
//	}
//	const auto print = [&](std::string header) {
//		const auto printVec = [&](std::string prefix, const auto &vec) {
//			std::cout << prefix;
//			for(const auto &e: vec) std::cout << " " << e;
//			std::cout << std::endl;
//		};
//		std::cout << header << std::endl;
//		printVec("vertex_order:", vertex_order);
//		printVec("candidates:  ", candidates);
//		printVec("deg:         ", deg);
//		printVec("used:        ", used);
//		printVec("is_candidate:", is_candidate);
//	};
//
//	print("start");
	const auto addNode = [&](auto v) {
//		print("addNode(" + std::to_string(get(boost::vertex_index_t(), g, v)) + ")");
		const auto idx = get(boost::vertex_index_t(), g, v);
		assert(!used[idx]);
		used[idx] = true;
		vertex_order.push_back(v);
		for(const auto eOut: asRange(out_edges(v, g))) {
			const auto vAdj = target(eOut, g);
			const auto idxAdj = get(boost::vertex_index_t(), g, vAdj);
			if(used[idxAdj]) continue;
			++deg[idxAdj];
			if(!is_candidate[idxAdj]) {
				candidates.push_back(vAdj);
				is_candidate[idxAdj] = true;
			}
		}
	};
	while(true) {
		const auto vs = vertices(g);
		// Note, for filtered_graphs we don't actually know how many vertices we need,
		// so we simply go until all are used.
		auto cand = std::find_if(vs.first, vs.second, [&](auto v) {
			return !used[get(boost::vertex_index_t(), g, v)];
		});
		if(cand == vs.second)
			break;
		auto idxCand = get(boost::vertex_index_t(), g, *cand);
		for(auto iter = std::next(cand); iter != vs.second; ++iter) {
			const auto idx = get(boost::vertex_index_t(), g, *iter);
			if(used[idx]) continue;
			if(deg[idx] > deg[idxCand]) {
				cand = iter;
				idxCand = idx;
			}
		}
		addNode(*cand);
		while(!candidates.empty()) {
			const auto iter = std::max_element(candidates.begin(), candidates.end(), [&](auto a, auto b) {
				const auto idx_a = get(boost::vertex_index_t(), g, a);
				const auto idx_b = get(boost::vertex_index_t(), g, b);
				return deg[idx_a] < deg[idx_b];
			});
			assert(iter != candidates.end());
			const auto v = *iter;
			candidates.erase(iter);
			addNode(v);
		}
	}
	return vertex_order;
}

} // namespace

struct DefaultFinderArgsProvider {
	template<typename Graph>
	friend std::vector<typename boost::graph_traits<Graph>::vertex_descriptor>
	get_vertex_order(const DefaultFinderArgsProvider &, const Graph &g) {
		if(getConfig().graph.vf2UseOrigVertexOrder)
			return jla_boost::GraphMorphism::vertex_order_by_mult(g);
		else
			return vertex_order_by_max_connectivity(g);
	}
};

} // namespace mod::lib::GraphMorphism

#endif // MOD_LIB_GRAPH_MORPHISM_FINDER_HPP