#include "Canonicalisation.hpp"

#include <mod/lib/Graph/Properties/Molecule.hpp>
#include <mod/lib/Graph/Properties/String.hpp>

#include <graph_canon/aut/implicit_size_2.hpp>
#include <graph_canon/aut/pruner_basic.hpp>
#include <graph_canon/canonicalization.hpp>
#include <graph_canon/compare.hpp>
#include <graph_canon/edge_handler/all_equal.hpp>
#include <graph_canon/invariant/cell_split.hpp>
#include <graph_canon/invariant/partial_leaf.hpp>
#include <graph_canon/invariant/quotient.hpp>
#include <graph_canon/refine/WL_1.hpp>
#include <graph_canon/refine/degree_1.hpp>
#include <graph_canon/target_cell/flm.hpp>
#include <graph_canon/tree_traversal/dfs.hpp>
#include <graph_canon/tree_traversal/bfs-exp.hpp>
#include <graph_canon/visitor/debug.hpp>
#include <graph_canon/visitor/stats.hpp>

#include <perm_group/permutation/built_in.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <boost/graph/graph_utility.hpp> // for boost::print_graph

#include <vector>

namespace mod::lib::graph {
namespace {

template<typename LabelledGraph, typename Grpah, typename Idx>
void printGraph(std::ostream &s, const LabelledGraph &lg, const Grpah &g, const Idx &idx) {
	for(const auto v : asRange(vertices(g))) {
		s << idx[v] << "(" << get(boost::vertex_index_t(), get_graph(lg), v) << ")("
		  << get_string(lg)[v] << "):";
		for(const auto e : asRange(out_edges(v, g))) {
			const auto v = target(e, g);
			s << " " << idx[v] << "(" << get_string(lg)[e] << ")";
		}
		s << std::endl;
	}
}

//template<typename SizeType>
//struct edge_handler_string {
//
//	explicit edge_handler_string(const Single &gWrap) : gWrap(&gWrap) { }
//
//	friend bool operator<(const edge_handler_string<SizeType> &lhs, const edge_handler_string<SizeType> &rhs) {
//		auto lIter = lhs.counts.begin();
//		auto rIter = rhs.counts.begin();
//		auto lIterEnd = lhs.counts.end();
//		auto rIterEnd = rhs.counts.end();
//		// merge
//		while(lIter != lIterEnd && rIter != rIterEnd) {
//			if(lIter->first == rIter->first) {
//				if(lIter->second != rIter->second)
//					return lIter->second < rIter->second;
//				++lIter;
//				++rIter;
//			} else if(lIter->first < rIter->first) {
//				if(lIter->second != 0) return false;
//				++lIter;
//			} else {
//				if(rIter->second != 0) return true;
//				++rIter;
//			}
//		}
//		for(; lIter != lIterEnd; ++lIter) {
//			if(lIter->second != 0) return false;
//		}
//		for(; rIter != rIterEnd; ++rIter) {
//			if(rIter->second != 0) return true;
//		}
//		return false;
//	}
//
//	friend void clear(edge_handler_string<SizeType> &eInv) {
//		for(auto &p : eInv.counts) p.second = 0;
//	}
//
//	friend void add_edge(edge_handler_string<SizeType> &eInv, Edge e, const GraphType &g) {
//		const std::string &label = eInv.gWrap->getStringState()[e];
//		auto iter = eInv.counts.find(label);
//		if(iter != eInv.counts.end()) iter->second++;
//		else eInv.counts.emplace(label, 1);
//	}
//
//	friend std::ostream &operator<<(std::ostream &s, const edge_handler_string<SizeType> &eInv) {
//		assert(false);
//	}
//public:
//	const Single *gWrap;
//	std::map<std::string, SizeType> counts;
//};

template<typename SizeType>
struct edge_handler_bond_impl {
	using Counter = std::array<SizeType, 4>;

	struct CellData {

		CellData() {
			hit_count.fill(0);
			acc_count.fill(0);
		}

	public:
		// For non-singleton
		Counter hit_count;
		// For singleton
		// accumulated count from above, i.e.:
		// - [3] = #type 4 bonds
		// - [2] = [3] + #type 3 bonds
		// - [1] = [2] = #type 2 bonds
		// - [0] = [1] = #type 1 bonds
		Counter acc_count;
	};

	static constexpr SizeType Max = 256;

	explicit edge_handler_bond_impl(const Graph &gWrap)
			: lg(gWrap.getLabelledGraph()), str(get_string(lg)), mol(get_molecule(lg)) {}

	template<typename State>
	void initialize(const State &state) {
		Counter c;
		c.fill(0);
		counters.resize(state.n, c);
		cellData.resize(state.n);
	}

	//========================================================================
	// Generic Refiner
	//========================================================================

	template<typename State, typename Treenode, typename Edge>
	void add_edge(State &state, Treenode &node, const SizeType cell, const SizeType cell_end, const Edge &e_out) {
		const auto bt = mol[e_out];
		SizeType i = static_cast<SizeType> (bt);
		assert(i != 0); // invalid bond, should not happen
		--i;
		assert(i < 4);
		const auto v_out = target(e_out, state.g);
		const auto v_out_idx = state.idx[v_out];
		++counters[v_out_idx][i];
		++cellData[cell].hit_count[i];
	}

	template<bool ParallelEdges, bool Loops, typename Partition, typename Splits>
	void sort_equal_hit(Partition &pi, const SizeType cell, const SizeType cell_end,
							  const SizeType max, Splits &splits) {
		sort_range<ParallelEdges, Loops>(pi, cell, cell, cell_end, max, splits);
	}

	template<bool ParallelEdges, bool Loops, typename Partition, typename Splits>
	void sort_partitioned(Partition &pi, const SizeType cell, const SizeType cell_mid, const SizeType cell_end,
								 const SizeType max_count, Splits &splits) {
		sort_range<ParallelEdges, Loops>(pi, cell, cell_mid, cell_end, max_count, splits);
	}

	template<bool ParallelEdges, bool Loops, typename Partition, typename Splits>
	void sort_duplicate_partitioned_equal_hit(Partition &pi,
															const SizeType cell,
															const SizeType cell_mid,
															const SizeType cell_end,
															const SizeType max,
															Splits &splits) {
		sort_range<ParallelEdges, Loops>(pi, cell, cell_mid, cell_end, max, splits);
	}

	template<bool ParallelEdges, bool Loops, typename Partition, typename Counters, typename Splits>
	void sort_duplicate_partitioned(Partition &pi, const SizeType cell, const SizeType cell_mid, const SizeType cell_end,
											  const SizeType max, const SizeType max_count,
											  const Counters &counters, Splits &splits) {
		sort_range<ParallelEdges, Loops>(pi, cell, cell_mid, cell_end, max, splits);
	}

	template<bool ParallelEdges, bool Loops, typename Partition, typename Counters, typename Splits>
	void sort(Partition &pi, const SizeType cell, const SizeType cell_end,
				 const SizeType max, const SizeType max_count, SizeType &first_non_zero,
				 const Counters &counters, Splits &splits) {
		assert(max_count != cell_end - cell); // handled by refiner
		assert(max > 1); // handled by refiner
		first_non_zero = sort_range<ParallelEdges, Loops>(pi, cell, cell, cell_end, max, splits);
	}

private:
	template<typename Partition, typename Splits, typename Iter>
	void sort_level(Partition &pi,
						 const SizeType cell,
						 const SizeType idx_first,
						 Splits &splits,
						 const Iter first,
						 const Iter last,
						 const int level) {
		if(level == 4) return;
		if(cellData[cell].hit_count[level] == 0) {
			return sort_level(pi, cell, idx_first, splits, first, last, level + 1);
		}
		std::array<SizeType, Max> ends;
		sorter(first, last, [this, level](const SizeType i) {
			return counters[i][level];
		}, [&ends](const auto &end) {
			std::copy(end.begin(), end.end(), ends.begin());
		}, [&pi](auto iter, const auto value) {
			pi.put_element_on_index(value, iter - pi.begin());
		});
		const auto size = last - first;
		//		std::cout << "SortLevel " << level << ": " << idx_first << "," << (idx_first + size) << std::endl;
		//		for(const auto e : ends) {
		//			std::cout << " " << e;
		//			if(e == size) break;
		//		}
		//		std::cout << std::endl;
		auto prev = 0;
		for(auto end_idx = 0; ends[end_idx] != size; prev = ends[end_idx], ++end_idx) {
			const auto end = ends[end_idx];
			if(end == prev) continue;
			const auto sub_first = first + prev;
			const auto sub_last = first + end;
			sort_level(pi, cell, idx_first + prev, splits, sub_first, sub_last, level + 1);
			splits.push_back(end + idx_first);
		}
		// and the last bucket, where we don't append an end split
		const auto sub_first = first + prev;
		const auto sub_last = first + size;
		assert(prev != size);
		sort_level(pi, cell, idx_first + prev, splits, sub_first, sub_last, level + 1);
	}

	template<bool ParallelEdges, bool Loops, typename Partition, typename Splits>
	SizeType sort_range(Partition &pi, const SizeType cell, const SizeType idx_first, const SizeType idx_last,
							  const SizeType max, Splits &splits) {
		const auto first = pi.begin() + idx_first;
		const auto last = pi.begin() + idx_last;
		// use counting sort to sort for low max_count
		if(max < Max) {
			const auto pre_sort_num_splits = splits.size();
			sort_level(pi, cell, idx_first, splits, first, last, 0);
			//			const auto printData = [&]() {
			//				std::cout << "CountSort: " << cell << "[" << idx_first << ":" << idx_last << "[" << std::endl;
			//				for(auto c = idx_first; c != idx_last; ++c) {
			//					const auto &count = counters[pi.get(c)];
			//					std::cout << " [" << c << "]=" << pi.get(c) << "["
			//							<< count[0] << " " << count[1] << " " << count[2] << " " << count[3] << "]" << std::endl;
			//				}
			//				std::cout << "Splits:";
			//				for(auto s_idx = pre_sort_num_splits; s_idx != splits.size(); ++s_idx) {
			//					std::cout << " " << splits[s_idx];
			//				}
			//				std::cout << std::endl;
			//			};
			//			for(auto c = idx_first + 1; c < idx_last; ++c) {
			//				if(counters[pi.get(c - 1)] > counters[pi.get(c)]) {
			//					printData();
			//					std::cout << "Counter Problem: " << c << std::endl;
			//					assert(false);
			//				}
			//			}
			//			for(auto s_idx = pre_sort_num_splits + 1; s_idx < splits.size(); ++s_idx) {
			//				if(splits[s_idx - 1] >= splits[s_idx]) {
			//					printData();
			//					std::cout << "Split Problem: " << s_idx << std::endl;
			//					assert(false);
			//				}
			//			}
			//			auto s_idx = pre_sort_num_splits;
			//			for(auto c = idx_first + 1; c < idx_last; ++c) {
			//				if(counters[pi.get(c - 1)] < counters[pi.get(c)]) {
			//					if(s_idx == splits.size()) {
			//						printData();
			//						std::cout << "No more splits: " << c << std::endl;
			//						assert(false);
			//					}
			//					if(splits[s_idx] != c) {
			//						printData();
			//						std::cout << "Wrong split: " << c << ", " << splits[s_idx] << std::endl;
			//						assert(false);
			//					}
			//					++s_idx;
			//				}
			//			}
			const auto &count = counters[pi.get(idx_first)];
			const auto all_zero = std::all_of(count.begin(), count.end(), [](const auto c) {
				return c == 0;
			});
			if(all_zero) return splits[pre_sort_num_splits];
			else return idx_first;
		}
		// fallback, just sort
		std::sort(first, last, [this](const auto a, const auto b) {
			return counters[a] < counters[b];
		});
		pi.reset_inverse(idx_first, idx_last);
		// Scan the refinee invariants and split it if needed, save new subset beginnings.
		for(SizeType i_refinee = idx_first + 1; i_refinee < idx_last; i_refinee++) {
			const SizeType refinee_prev_idx = pi.get(i_refinee - 1);
			const SizeType refinee_idx = pi.get(i_refinee);
			if(counters[refinee_prev_idx] < counters[refinee_idx]) {
				assert(i_refinee != idx_first);
				assert(i_refinee != idx_last);
				if(!splits.empty())
					assert(i_refinee != splits.back());
				splits.push_back(i_refinee);
			}
		}
		return idx_first; // TODO: we could check this
	}

public:
	template<typename State, typename Treenode>
	void clear_cell(State &state, Treenode &node, const SizeType cell, const SizeType cell_end) {
		cellData[cell].hit_count.fill(0);
		for(auto i = cell; i != cell_end; ++i)
			counters[node.pi.get(i)].fill(0);
	}

	template<typename State, typename Treenode>
	void clear_cell_aborted(State &state, Treenode &node, const SizeType cell, const SizeType cell_end) {
		clear_cell(state, node, cell, cell_end);
	}

	//========================================================================
	// Singleton Refiner
	//========================================================================

	template<typename State, typename Treenode, typename Edge>
	void add_edge_singleton_refiner(State &state,
											  Treenode &node,
											  const SizeType cell,
											  const SizeType cell_end,
											  const Edge &e_out,
											  const SizeType target_pos) {
		const auto bt = mol[e_out];
		SizeType i = static_cast<SizeType> (bt);
		assert(i != 0); // invalid bond, should not happen
		--i;
		assert(i < 4);

		// we want to keep the partitioning into the four groups of bond types
		// X00...0011...1122...2233...33
		auto &acc_count = cellData[cell].acc_count;
		// first increase the span
		switch(i) {
		case 3: ++acc_count[3];
			[[fallthrough]];
		case 2: ++acc_count[2];
			[[fallthrough]];
		case 1: ++acc_count[1];
			[[fallthrough]];
		case 0: ++acc_count[0];
		}

		// and then do the swapping down
		switch(i) {
		case 1: // 000...0X11...1122...2233...33
			// but if there are no 0's, we are done
			if(acc_count[0] != acc_count[1]) {
				const auto target_0 = cell_end - acc_count[0];
				const auto target_1 = cell_end - acc_count[1];
				const auto elem_0 = node.pi.get(target_0);
				const auto elem_1 = node.pi.get(target_1);
				node.pi.put_element_on_index(elem_1, target_0);
				node.pi.put_element_on_index(elem_0, target_1);
			}
			break;
		case 2: // 000...0111...1X22...2233...33
		{
			const auto target_0 = cell_end - acc_count[0];
			const auto target_1 = cell_end - acc_count[1];
			const auto target_2 = cell_end - acc_count[2];
			const auto elem_0 = node.pi.get(target_0);
			if(acc_count[0] != acc_count[1]) {
				const auto elem_1 = node.pi.get(target_1);
				node.pi.put_element_on_index(elem_1, target_0);
			}
			if(acc_count[1] != acc_count[2]) {
				const auto elem_2 = node.pi.get(target_2);
				node.pi.put_element_on_index(elem_2, target_1);
			}
			if(acc_count[0] != acc_count[1] || acc_count[1] != acc_count[2])
				node.pi.put_element_on_index(elem_0, target_2);
		}
			break;
		case 3: // 000...0111...1222...2X33...33
		{
			const auto target_0 = cell_end - acc_count[0];
			const auto target_1 = cell_end - acc_count[1];
			const auto target_2 = cell_end - acc_count[2];
			const auto target_3 = cell_end - acc_count[3];
			const auto elem_0 = node.pi.get(target_0);
			if(acc_count[0] != acc_count[1]) {
				const auto elem_1 = node.pi.get(target_1);
				node.pi.put_element_on_index(elem_1, target_0);
			}
			if(acc_count[1] != acc_count[2]) {
				const auto elem_2 = node.pi.get(target_2);
				node.pi.put_element_on_index(elem_2, target_1);
			}
			if(acc_count[2] != acc_count[3]) {
				const auto elem_3 = node.pi.get(target_3);
				node.pi.put_element_on_index(elem_3, target_2);
			}
			if(acc_count[0] != acc_count[1] || acc_count[1] != acc_count[2] || acc_count[2] != acc_count[3])
				node.pi.put_element_on_index(elem_0, target_3);
		}
			break;
		}
	}

	template<bool ParallelEdges, bool Loops, typename Partition, typename Splits>
	void sort_singleton_refiner(Partition &pi,
										 const SizeType cell,
										 const SizeType cell_mid,
										 const SizeType cell_end,
										 Splits &splits) {
		const auto &acc_count = cellData[cell].acc_count;
		const auto target_0 = cell_end - acc_count[0];
		const auto target_1 = cell_end - acc_count[1];
		const auto target_2 = cell_end - acc_count[2];
		const auto target_3 = cell_end - acc_count[3];
		assert(target_0 == cell_mid);
		if(target_1 != target_0 && target_1 != cell_end)
			splits.push_back(target_1);
		if(target_2 != target_1 && target_2 != cell_end)
			splits.push_back(target_2);
		if(target_3 != target_2 && target_3 != cell_end)
			splits.push_back(target_3);
	}

	template<typename State, typename TreeNode>
	void clear_cell_singleton_refiner(State &state, TreeNode &node, const SizeType cell, const SizeType cell_end) {
		cellData[cell].acc_count.fill(0);
	}

	template<typename State, typename TreeNode>
	void clear_cell_singleton_refiner_aborted(State &state, TreeNode &node, const SizeType cell, const SizeType cell_end) {
		clear_cell_singleton_refiner(state, node, cell, cell_end);
	}

private:
	graph_canon::counting_sorter<SizeType, Max> sorter;
public:
	template<typename State, typename Edge>
	long long compare(State &state, const Edge e_left, const Edge e_right) const {
		return static_cast<long long> (mol[e_left]) - static_cast<long long> (mol[e_right]);
	}

public:
	const LabelledGraph &lg;
	const LabelledGraph::PropStringType &str;
	const LabelledGraph::PropMoleculeType &mol;
	std::vector<Counter> counters;
	std::vector<CellData> cellData;
};

struct edge_handler_bond {
	template<typename SizeType>
	using type = edge_handler_bond_impl<SizeType>;

	explicit edge_handler_bond(const Graph &gWrap) : gWrap(&gWrap) {}

	template<typename SizeType>
	auto make() const {
		return edge_handler_bond_impl<SizeType>(*gWrap);
	}

public:
	const Graph *gWrap;
};

template<typename LabelledGraph, typename Partition>
void printLeaf(std::ostream &s, const LabelledGraph &lg, const Partition &pi) {
	std::vector<int> perm;
	std::copy(pi.begin_inverse(), pi.end_inverse(), std::back_inserter(perm));
	const auto &str = get_string(lg);
	auto eLess = [&str](Edge lhs, Edge rhs) {
		MOD_ABORT; // should never be called, as we don't have parallel edges or loops
		return str[lhs] < str[rhs];
	};
	Graph::CanonIdxMap ordIdx(perm.begin(), get(boost::vertex_index_t(), get_graph(lg)));
	Graph::CanonForm form(get_graph(lg), ordIdx, eLess);
	printGraph(s, lg, form, ordIdx);
}

struct debug_visitor : graph_canon::debug_visitor {
	using Base = graph_canon::debug_visitor;

	debug_visitor(const LabelledGraph &lg) : Base(true, true, true, true, false), lg(lg) {}

	template<typename State, typename TreeNode>
	bool tree_create_node_begin(State &state, TreeNode &t) {
		Base::tree_create_node_begin(state, t);
		if(tree) {
			std::cout << "Degrees:";
			for(int i = 0; i < state.n; ++i)
				std::cout << " " << degree(vertex(t.pi.get(i), state.g), state.g);
			std::cout << std::endl;
		}
		return true;
	}

	template<typename State>
	void canon_new_best(const State &state) {
		Base::canon_new_best(state);
		if(canon) {
			printLeaf(std::cout, lg, state.get_canon_leaf()->pi);
		}
	}

	template<typename State, typename TreeNode>
	void canon_worse(const State &state, const TreeNode &t) {
		Base::canon_worse(state, t);
		if(canon) {
			printLeaf(std::cout, lg, state.get_canon_leaf()->pi);
		}
	}

	template<typename State, typename TreeNode, typename Perm>
	void automorphism_leaf(const State &state, const TreeNode &t, const Perm &aut) {
		Base::automorphism_leaf(state, t, aut);
		if(this->aut) {
			printLeaf(std::cout, lg, state.get_canon_leaf()->pi);
		}
	}

private:
	const LabelledGraph &lg;
};

template<typename EdgeHandler>
auto getCanonForm(const Graph &g, EdgeHandler eHandler, LabelType labelType, bool withStereo) {
	auto can = graph_canon::canonicalizer<int, EdgeHandler, false, false>(eHandler);
	const auto &graph = get_graph(g.getLabelledGraph());
	const auto idx = get(boost::vertex_index_t(), graph);
	const auto vis = graph_canon::make_visitor(
			graph_canon::traversal_bfs_exp(), graph_canon::target_cell_flm(), graph_canon::refine_WL_1(),
			graph_canon::aut_pruner_basic(), graph_canon::aut_implicit_size_2(), graph_canon::refine_degree_1(),
			graph_canon::invariant_partial_leaf(), graph_canon::invariant_cell_split(), graph_canon::invariant_quotient()
			//			, debug_visitor(g.getLabelledGraph())
			, graph_canon::stats_visitor()
	);
	const auto &str = get_string(g.getLabelledGraph());
	const auto vLess = [&str](Vertex a, Vertex b) {
		return str[a] < str[b];
	};

	auto res = can(graph, idx, vLess, vis);
	std::vector<int> perm = std::move(res.first);
	const auto &stats = get(graph_canon::stats_visitor::result_t(), res.second);
	if(getConfig().canon.printStats) {
		//		std::ofstream tree("tree.dot");
		//		graph_canonicalization::stats_visitor stats(&tree);
		//		canon_permutation = can(getGraph(), idx, vLess,
		//				graph_canonicalization::make_visitor(visitor, std::ref(stats)),
		//				eInv);
		std::cout << stats;
	}

	auto eLess = [&str](Edge lhs, Edge rhs) {
		MOD_ABORT; // should never be called, as we don't have parallel edges or loops
		return str[lhs] < str[rhs];
	};
	Graph::CanonIdxMap ordIdx(perm.begin(), idx);
	auto form = std::make_unique<Graph::CanonForm>(graph, ordIdx, eLess);
	auto autPtr = std::move(get(graph_canon::aut_pruner_basic::result_t(), res.second));
	auto autPtrRes = std::make_unique<Graph::AutGroup>(autPtr->degree());
	auto gens = autPtr->generators(); // skip the first, it should be the identity
	for(const auto &p : asRange(++begin(gens), end(gens)))
		autPtrRes->add_generator(p);
	return std::make_tuple(std::move(perm), std::move(form), std::move(autPtrRes));
}

} // namespace

std::tuple<std::vector<int>, std::unique_ptr<Graph::CanonForm>, std::unique_ptr<Graph::AutGroup> >
getCanonForm(const Graph &g, LabelType labelType, bool withStereo) {
	if(labelType != LabelType::String)
		throw LogicError("Can only canonicalise with label type string.");
	if(withStereo)
		throw LogicError("Can not canonicalise stereo.");
	const auto &mol = get_molecule(g.getLabelledGraph());
	const auto es = edges(g.getGraph());
	const bool isMol = std::all_of(es.first, es.second, [&](const auto &e) {
		return mol[e] != BondType::Invalid;
	});
	if(isMol) {
		return getCanonForm(g, edge_handler_bond(g), labelType, withStereo);
	} else {
		std::string msg = "Can not canonicalise arbitrary edge labels.\n";
		msg += "Graph is '" + g.getName() + "', with graphDFS: " + g.getGraphDFS().first;
		throw LogicError(std::move(msg));
	}
}

namespace {

template<typename LGraph, typename Graph, typename Idx>
struct PrintVisitor {
	using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
	using Edge = typename boost::graph_traits<Graph>::edge_descriptor;
public:

	PrintVisitor(const LGraph &lg1,
					 const LGraph &lg2,
					 const Graph &g1,
					 const Graph &g2,
					 const Idx &idx1,
					 const Idx &idx2)
			: lg1(lg1), lg2(lg2), g1(g1), g2(g2), idx1(idx1), idx2(idx2) {}

	void at_num_vertices() const {
		std::cout << "num_vertices: " << num_vertices(g1) << " != " << num_vertices(g2) << std::endl;
	}

	void at_vertex_compare(Vertex v1, Vertex v2) const {
		std::cout << "vertexComp(" << idx1[v1] << "(" << v1 << ")" << ", " << idx2[v2] << "(" << v2 << ")" << ") == false"
					 << std::endl;
	}

	void at_out_degree(Vertex v1, Vertex v2) const {
		std::cout << "num_out_edges: " << out_degree(v1, g1) << " != " << out_degree(v2, g2) << std::endl;
	}

	void at_out_edge(Edge e1, Edge e2) const {
		printGraph(std::cout, lg1, g1, idx1);
		std::cout << std::endl;
		printGraph(std::cout, lg2, g2, idx2);
		std::string edge = boost::is_same<typename boost::graph_traits<Graph>::directed_category, boost::undirected_tag>::value
								 ? " -- " : " -> ";
		std::cout << "out_edge: " << get(idx1, source(e1, g1)) << edge << get(idx1, target(e1, g1))
					 << " != " << get(idx2, source(e2, g2)) << edge << get(idx2, target(e2, g2)) << std::endl;
	}

	void at_edge_compare(Edge e1, Edge e2) const {
		printGraph(std::cout, lg1, g1, idx1);
		std::cout << std::endl;
		printGraph(std::cout, lg2, g2, idx2);
		std::string edge = boost::is_same<typename boost::graph_traits<Graph>::directed_category, boost::undirected_tag>::value
								 ? " -- " : " -> ";
		std::cout << "edgeComp: " << get(idx1, source(e1, g1)) << edge << get(idx1, target(e1, g1))
					 << " != " << get(idx2, source(e2, g2)) << edge << get(idx2, target(e2, g2)) << std::endl;
	}

	void at_end(bool) const {}

private:
	const LGraph &lg1;
	const LGraph &lg2;
	const Graph &g1;
	const Graph &g2;
	const Idx &idx1;
	const Idx &idx2;
};

template<typename LGraph, typename Graph, typename Idx>
PrintVisitor<LGraph, Graph, Idx> makePrintVisitor(const LGraph &lg1,
																  const LGraph &lg2,
																  const Graph &g1,
																  const Graph &g2,
																  const Idx &idx1,
																  const Idx &idx2) {
	return PrintVisitor<LGraph, Graph, Idx>(lg1, lg2, g1, g2, idx1, idx2);
}

} // namespace

bool canonicalCompare(const Graph &g1, const Graph &g2, LabelType labelType, bool withStereo) {
	const auto &ord1 = g1.getCanonForm(labelType, withStereo);
	const auto &ord2 = g2.getCanonForm(labelType, withStereo);
	if(withStereo) throw LogicError("Can not currently compare canonical forms with stereo information.");
	const auto &gl1 = g1.getLabelledGraph();
	const auto &gl2 = g2.getLabelledGraph();
	const auto visitor = graph_canon::graph_compare_null_visitor();
	//	const auto visitor = makePrintVisitor(g1.getLabelledGraph(), g2.getLabelledGraph(), ord1, ord2, ord1.get_index_map(), ord2.get_index_map());
	switch(labelType) {
	case LabelType::String:
		return graph_canon::ordered_graph_equal(ord1, ord2,
															 [&gl1, &gl2](Vertex v1, Vertex v2) -> bool {
																 return get_string(gl1)[v1] == get_string(gl2)[v2];
															 },
															 [&gl1, &gl2](Edge e1, Edge e2) -> bool {
																 return get_string(gl1)[e1] == get_string(gl2)[e2];
															 }, visitor);
	case LabelType::Term: throw LogicError("Can not currently compare canonical forms with term labels.");
	}
	MOD_ABORT;
}

} // namespace mod::lib::graph