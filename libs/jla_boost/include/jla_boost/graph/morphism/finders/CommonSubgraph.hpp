#ifndef JLA_BOOST_GRAPH_MORPHOSM_FINDERS_COMMON_SUBGRAPH_HPP
#define JLA_BOOST_GRAPH_MORPHOSM_FINDERS_COMMON_SUBGRAPH_HPP

// Adapted from Boost.Graph mcgregor_common_subgraph

#include <jla_boost/graph/morphism/finders/InjectiveEnumeration.hpp>

#include <jla_boost/Functional.hpp>
#include <jla_boost/graph/PairToRangeAdaptor.hpp>
#include <jla_boost/graph/morphism/AsPropertyMap.hpp>
#include <jla_boost/graph/morphism/models/InvertibleAdaptor.hpp>
#include <jla_boost/graph/morphism/models/InvertibleVector.hpp>
#include <jla_boost/graph/morphism/models/PropertyVertexMap.hpp>

#include <boost/make_shared.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/properties.hpp>
#include <boost/property_map/shared_array_property_map.hpp>

#include <algorithm>
#include <vector>
#include <stack>

namespace jla_boost::GraphMorphism {
using namespace boost; // TODO: remove

template<bool OnlyConnected, typename GraphLeft, typename GraphRight, typename EdgePred, typename VertexPred>
struct CommonSubgraphEnumerator : InjectiveEnumerationState<
		CommonSubgraphEnumerator<OnlyConnected, GraphLeft, GraphRight, EdgePred, VertexPred>,
		GraphLeft, GraphRight, EdgePred, VertexPred> {
	using Self = CommonSubgraphEnumerator;
	using Base = InjectiveEnumerationState<Self, GraphLeft, GraphRight, EdgePred, VertexPred>;
public:
	using VertexLeft = typename boost::graph_traits<GraphLeft>::vertex_descriptor;
	using VertexRight = typename boost::graph_traits<GraphRight>::vertex_descriptor;
	using EdgeLeft = typename boost::graph_traits<GraphLeft>::edge_descriptor;
	using EdgeRight = typename boost::graph_traits<GraphRight>::edge_descriptor;
private:
	static VertexLeft vNullLeft() {
		return boost::graph_traits<GraphLeft>::null_vertex();
	}

	static VertexRight vNullRight() {
		return boost::graph_traits<GraphRight>::null_vertex();
	}
public:
	CommonSubgraphEnumerator(const GraphLeft &gLeft, const GraphRight &gRight,
	                         EdgePred edgePred, VertexPred vertexPred)
			: Base(gLeft, gRight, edgePred, vertexPred) {}
public:
	bool visit_tryPush(VertexLeft vLeft, VertexRight vRight) {
		if(this->getTotalStackSize() == 0) return true;
		bool has_one_edge = false;

		// Verify edges with existing sub-graph
		for(auto vOtherLeft : asRange(vertices(this->gLeft))) {
			auto vOtherRight = this->rightFromLeft(vOtherLeft);
#ifdef MORPHISM_INJECTIVE_ENUMERATION_DEBUG
			std::cout << this->indent(2) << "visit_tryPush, checking " << vOtherLeft << " <-> " << vOtherRight
			          << std::endl;
#endif
			// Skip unassociated vertices
			if(vOtherRight == vNullRight()) continue;

			// NOTE: This will not work with parallel edges, since the
			// first matching edge is always chosen.
			EdgeLeft edgeToNewLeft;
			bool edgeToNewExistsLeft = false;
			EdgeRight edgeToNewRight;
			bool edgeToNewExistsRight = false;

			// Search for edge from existing to new vertex (gLeft)
			for(auto eOutLeft : asRange(out_edges(vOtherLeft, this->gLeft))) {
				if(target(eOutLeft, this->gLeft) == vLeft) {
					edgeToNewLeft = eOutLeft;
					edgeToNewExistsLeft = true;
					break;
				}
			}

			// Search for edge from existing to new vertex (gRight)
			for(auto eOutRight : asRange(out_edges(vOtherRight, this->gRight))) {
				if(target(eOutRight, this->gRight) == vRight) {
					edgeToNewRight = eOutRight;
					edgeToNewExistsRight = true;
					break;
				}
			}

			const bool is_undirected1 = is_undirected(this->gLeft);
			const bool is_undirected2 = is_undirected(this->gRight);
#ifdef MORPHISM_INJECTIVE_ENUMERATION_DEBUG
			std::cout << this->indent(3) << "undirected? "
			          << std::boolalpha << is_undirected1 << ", " << is_undirected2 << std::endl;
			std::cout << this->indent(3) << "edgeToNew_exists? "
			          << std::boolalpha << edgeToNewExistsLeft << ", " << edgeToNewExistsLeft << std::endl;
#endif

			if(edgeToNewExistsLeft && edgeToNewExistsRight) {
#ifdef MORPHISM_INJECTIVE_ENUMERATION_DEBUG
				std::cout << this->indent(3) << "edgeToNewLeft: " << edgeToNewLeft << std::endl;
				std::cout << this->indent(3) << "edgeToNewRight: " << edgeToNewRight << std::endl;
#endif
				if(this->edgePred(edgeToNewLeft, edgeToNewRight)) {
					has_one_edge = true;
				} else {
#ifdef MORPHISM_INJECTIVE_ENUMERATION_DEBUG
					std::cout << this->indent(3) << "failed: edgePred on edge from previous to new mapped pair" << std::endl;
#endif
					return false;
				}
			}

			if(is_undirected1 && is_undirected2) {
				// nothing more to do
			} else { // one of them is directed
				EdgeLeft edge_from_new1;
				bool edge_from_new_exists1 = false;
				EdgeRight edge_from_new2;
				bool edge_from_new_exists2 = false;

				// TODO: should this really work with mixed directed vs. undirected?
				if(!is_undirected1) {
					// Search for edge from new to existing vertex (gLeft)
					for(auto eOutLeft : asRange(out_edges(vLeft, this->gLeft))) {
						if(target(eOutLeft, this->gLeft) == vOtherLeft) {
							edge_from_new1 = eOutLeft;
							edge_from_new_exists1 = true;
							break;
						}
					}
				}

				if(!is_undirected2) {
					// Search for edge from new to existing vertex (gRight)
					for(auto eOutRight : asRange(out_edges(vRight, this->gRight))) {
						if(target(eOutRight, this->gRight) == vOtherRight) {
							edge_from_new2 = eOutRight;
							edge_from_new_exists2 = true;
							break;
						}
					}
				}

#ifdef MORPHISM_INJECTIVE_ENUMERATION_DEBUG
				std::cout << this->indent(3) << "edge_from_new_exists? "
				          << std::boolalpha << edge_from_new_exists1 << ", " << edge_from_new_exists2 << std::endl;
#endif

				if(edge_from_new_exists1 && edge_from_new_exists2) {
					if(!this->edgePred(edge_from_new1, edge_from_new2)) {
#ifdef MORPHISM_INJECTIVE_ENUMERATION_DEBUG
						std::cout << this->indent(3) << "failed: edgePred on edge to previous from new mapped pair"
						          << std::endl;
#endif
						return false;
					}
				}

				if(edge_from_new_exists1 && edge_from_new_exists2)
					has_one_edge = true;
			}
		} // foreach vOtherLeft

		// Make sure new vertices are connected to the existing subgraph
		if(OnlyConnected && !has_one_edge) {
#ifdef MORPHISM_INJECTIVE_ENUMERATION_DEBUG
			std::cout << this->indent(3) << "failed: onlyConnected" << std::endl;
#endif
			return false;
		}
		return true;
	}
public:
	template<typename Callback>
	void operator()(Callback &&callback) {
		extendMatch(callback, vertices(this->gLeft).first);
	}
private:
	template<typename Callback>
	bool extendMatch(Callback &&callback, typename boost::graph_traits<GraphLeft>::vertex_iterator iterLeft) {
		// If we are searching for connected graphs, then the default vertex iteration order
		// is probably not correlated with connectedness, so just try them all at each level.
		// But if we search for not-necessarily connected subgraphs, then we only give each vLeft
		// a single chance to get into the subgraph, so recurse starting with the next vLeft.
		if(OnlyConnected) iterLeft = vertices(this->gLeft).first;

		for(; iterLeft != vertices(this->gLeft).second; ++iterLeft) {
			const auto vLeft = *iterLeft;
			// Skip already matched vertices in first graph
			if(this->rightFromLeft(vLeft) != vNullRight()) continue;

			for(const auto vRight : asRange(vertices(this->gRight))) {
				// Skip already matched vertices in second graph
				if(this->leftFromRight(vRight) != vNullLeft()) continue;

				// Check if current sub-graph can be extended with the matched vertex pair
				const bool wasPushed = this->tryPush(vLeft, vRight);
				if(!wasPushed) continue;

#ifdef MORPHISM_INJECTIVE_ENUMERATION_DEBUG
				std::cout << this->indent() << "map!" << std::endl;
#endif
				// Returning false from the callback will cancel iteration
				if(!callback(this->getSizedVertexMap(), this->gLeft, this->gRight))
					return false;

				// Depth-first search into the state space of possible sub-graphs
#ifdef MORPHISM_INJECTIVE_ENUMERATION_DEBUG
				++this->debug_indent;
#endif
				const bool continueSearch = extendMatch(callback, std::next(iterLeft));
#ifdef MORPHISM_INJECTIVE_ENUMERATION_DEBUG
				--this->debug_indent;
#endif
				if(!continueSearch) return false;

				VertexLeft vStackLeft;
				VertexRight vStackRight;
				std::tie(vStackLeft, vStackRight) = this->pop();
				assert(vStackLeft == vLeft);
				assert(vStackRight == vRight);
			} // for all vertices(gRight)
		} // for all vertices(gLeft)
		return true;
	}
};

template<bool OnlyConnected, typename GraphLeft, typename GraphRight, typename EdgePred, typename VertexPred>
auto makeCommonSubgraphEnumerator(const GraphLeft &gLeft, const GraphRight &gRight,
                                  EdgePred edgePred, VertexPred vertexPred) {
	return CommonSubgraphEnumerator<OnlyConnected, GraphLeft, GraphRight, EdgePred, VertexPred>(
			gLeft, gRight, edgePred, vertexPred);
}

// ==========================================================================

// Enumerates all common subgraphs present in gLeft and gRight.
// Continues until the search space has been fully explored or false
// is returned from callback.
template<bool OnlyConnected, typename GraphLeft, typename GraphRight, typename EdgePred, typename VertexPred, typename Callback>
void commonSubgraphs(const GraphLeft &gLeft, const GraphRight &gRight,
                     EdgePred edgePred, VertexPred vertexPred, Callback callback) {
	CommonSubgraphEnumerator<OnlyConnected, GraphLeft, GraphRight, EdgePred, VertexPred>(
			gLeft, gRight, edgePred, vertexPred)(callback);
}

// Variant of commonSubgraphs with all default parameters
template<bool OnlyConnected, typename GraphLeft, typename GraphRight, typename Callback>
void commonSubgraphs(const GraphLeft &gLeft, const GraphRight &gRight, Callback callback) {
	commonSubgraphs<OnlyConnected>(gLeft, gRight, get(boost::vertex_index, gLeft), get(boost::vertex_index, gRight),
	                               jla_boost::AlwaysTrue(), jla_boost::AlwaysTrue(), callback);
}

// Named parameter variant of commonSubgraphs
template<bool OnlyConnected, typename GraphLeft, typename GraphRight, typename Callback, typename Param, typename Tag, typename Rest>
void commonSubgraphs(const GraphLeft &gLeft, const GraphRight &gRight, Callback callback,
                     const boost::bgl_named_params<Param, Tag, Rest> &params) {
	commonSubgraphs<OnlyConnected>(
			gLeft, gRight,
			choose_param(get_param(params, boost::edges_equivalent_t()), jla_boost::AlwaysTrue()),
			choose_param(get_param(params, boost::vertices_equivalent_t()), jla_boost::AlwaysTrue()),
			callback);
}

// ==========================================================================

namespace detail {

// Helper class for caching maximum and/or unique subgraphs.
template<typename GraphLeft, typename GraphRight, typename Next, bool Unique, bool Maximum>
struct CommonSubgraphCallbackHelper {
	static_assert(Unique || Maximum, "Not intended for just caching.");
	using CachedVertexMap = InvertibleVectorVertexMap<GraphLeft, GraphRight>;

	struct CachedSubgraph {
		template<typename VertexMap>
		CachedSubgraph(VertexMap &&m, const GraphLeft &gLeft, const GraphRight &gRight, std::size_t size)
				: m(std::forward<VertexMap>(m), gLeft, gRight), size(size) {}
	public:
		CachedVertexMap m;
		std::size_t size;
	};
private:
	CommonSubgraphCallbackHelper(const CommonSubgraphCallbackHelper &) = delete;
	CommonSubgraphCallbackHelper &operator=(const CommonSubgraphCallbackHelper &) = delete;
public:
	CommonSubgraphCallbackHelper(const GraphLeft &gLeft, const GraphRight &gRight, Next next)
			: gLeft(gLeft), gRight(gRight), next(next) {}

	template<typename VertexMap>
	bool operator()(VertexMap &&m, const GraphLeft &gLeft, const GraphRight &gRight) {
		const auto subgraphSize = get_prop(PreImageSizeT(), m);
		if(Maximum) {
			if(!cache.empty()) {
				if(subgraphSize > cache.front().size) cache.clear();
				else if(subgraphSize < cache.front().size) return true;
			}
		}
		if(Unique) {
			auto mAsProperty = makeAsPropertyMap(std::ref(m), gLeft, gRight);
			for(const auto &c : cache) {
				if(subgraphSize != c.size) continue;
				auto cAsProperty = makeAsPropertyMap(std::ref(c.m), gLeft, gRight);
				if(!are_property_maps_different(mAsProperty, cAsProperty, gLeft)) {
					return true;
				}
			}
		}
		cache.emplace_back(m, gLeft, gRight, subgraphSize);
		if(Maximum) return true;
		else return next(std::forward<VertexMap>(m), gLeft, gRight);
	}

	void outputMatches() {
		static_assert(Maximum, "Otherwise they are reported online.");
		for(auto &subgraph : cache)
			next(addProp(std::move(subgraph.m), PreImageSizeT(), subgraph.size), gLeft, gRight);
	}
private:
	const GraphLeft &gLeft;
	const GraphRight &gRight;
	Next next;
	std::vector<CachedSubgraph> cache;
};

} // namespace detail

// TODO: the 'unique' callbacks should be deleted once the core algorithm has been fixed to not enumerate duplicates
template<typename GraphLeft, typename GraphRight, typename Next>
using UniqueVertexMapCallback = detail::CommonSubgraphCallbackHelper<GraphLeft, GraphRight, Next, true, false>;
template<typename GraphLeft, typename GraphRight, typename Next>
using MaximumSubgraphCallback = detail::CommonSubgraphCallbackHelper<GraphLeft, GraphRight, Next, false, true>;
template<typename GraphLeft, typename GraphRight, typename Next>
using UniqueMaximumSubgraphCallback = detail::CommonSubgraphCallbackHelper<GraphLeft, GraphRight, Next, true, true>;

template<typename GraphLeft, typename GraphRight, typename Next>
auto makeMaxmimumSubgraphCallback(const GraphLeft &gLeft, const GraphRight &gRight, Next next) {
	return MaximumSubgraphCallback<GraphLeft, GraphRight, Next>(gLeft, gRight, next);
}

// Enumerates all unique common subgraphs between gLeft and gRight.
// The user callback is invoked for each unique subgraph as they are
// discovered.
template<bool OnlyConnected, typename GraphLeft, typename GraphRight, typename EdgePred, typename VertexPred, typename Callback>
void commonSubgraphs_unique(const GraphLeft &gLeft, const GraphRight &gRight,
                            EdgePred edgePred, VertexPred vertexPred, Callback callback) {
	UniqueVertexMapCallback<GraphLeft, GraphRight, Callback> unique_callback(gLeft, gRight, callback);
	commonSubgraphs<OnlyConnected>(gLeft, gRight, edgePred, vertexPred, std::ref(unique_callback));
}

// Variant of commonSubgraphs_unique with all default parameters.
template<bool OnlyConnected, typename GraphLeft, typename GraphRight, typename Callback>
void commonSubgraphs_unique(const GraphLeft &gLeft, const GraphRight &gRight, Callback callback) {
	commonSubgraphs_unique<OnlyConnected>(gLeft, gRight, jla_boost::AlwaysTrue(), jla_boost::AlwaysTrue(), callback);
}

// Named parameter variant of commonSubgraphs_unique
template<bool OnlyConnected, typename GraphLeft, typename GraphRight, typename Callback, typename Param, typename Tag, typename Rest>
void commonSubgraphs_unique(const GraphLeft &gLeft, const GraphRight &gRight, Callback callback,
                            const boost::bgl_named_params<Param, Tag, Rest> &params) {
	commonSubgraphs_unique<OnlyConnected>(
			gLeft, gRight,
			choose_param(get_param(params, boost::edges_equivalent_t()), jla_boost::AlwaysTrue()),
			choose_param(get_param(params, boost::vertices_equivalent_t()), jla_boost::AlwaysTrue()),
			callback);
}

// ==========================================================================

// Enumerates the largest common subgraphs found between gLeft and gRight
// Note that the ENTIRE search space is explored before callback is actually invoked.
template<bool OnlyConnected, typename GraphLeft, typename GraphRight, typename EdgePred, typename VertexPred, typename Callback>
void commonSubgraphs_maximum(const GraphLeft &gLeft, const GraphRight &gRight,
                             EdgePred edgePred, VertexPred vertexPred, Callback callback) {
	MaximumSubgraphCallback<GraphLeft, GraphRight, Callback> max_interceptor(gLeft, gRight, callback);
	commonSubgraphs<OnlyConnected>(gLeft, gRight, edgePred, vertexPred, std::ref(max_interceptor));
	// Only output the largest subgraphs
	max_interceptor.outputMatches();
}

// Variant of commonSubgraphs_maximum with all default
// parameters.
template<bool OnlyConnected, typename GraphLeft, typename GraphRight, typename Callback>
void commonSubgraphs_maximum(const GraphLeft &gLeft, const GraphRight &gRight, Callback callback) {
	commonSubgraphs_maximum<OnlyConnected>(gLeft, gRight, jla_boost::AlwaysTrue(), jla_boost::AlwaysTrue(), callback);
}

// Named parameter variant of commonSubgraphs_maximum
template<bool OnlyConnected, typename GraphLeft, typename GraphRight, typename Callback, typename Param, typename Tag, typename Rest>
void commonSubgraphs_maximum(const GraphLeft &gLeft, const GraphRight &gRight, Callback callback,
                             const boost::bgl_named_params<Param, Tag, Rest> &params) {
	commonSubgraphs_maximum<OnlyConnected>(
			gLeft, gRight,
			choose_param(get_param(params, boost::edges_equivalent_t()), jla_boost::AlwaysTrue()),
			choose_param(get_param(params, boost::vertices_equivalent_t()), jla_boost::AlwaysTrue()),
			callback);
}

// ==========================================================================

// Enumerates the largest, unique common subgraphs found between
// gLeft and gRight.  Note that the ENTIRE search space is explored
// before callback is actually invoked.
template<bool OnlyConnected, typename GraphLeft, typename GraphRight, typename EdgePred, typename VertexPred, typename Callback>
void commonSubgraphs_maximum_unique(const GraphLeft &gLeft, const GraphRight &gRight,
                                    EdgePred edgePred, VertexPred vertexPred, Callback callback) {
	UniqueMaximumSubgraphCallback<GraphLeft, GraphRight, Callback> unique_max_interceptor(
			gLeft, gRight, callback);
	commonSubgraphs<OnlyConnected>(gLeft, gRight, edgePred, vertexPred, std::ref(unique_max_interceptor));
	// Only output the largest, unique subgraphs
	unique_max_interceptor.outputMatches();
}

// Variant of commonSubgraphs_maximum_unique with all default parameters
template<bool OnlyConnected, typename GraphLeft, typename GraphRight, typename Callback>
void commonSubgraphs_maximum_unique(const GraphLeft &gLeft, const GraphRight &gRight, Callback callback) {
	commonSubgraphs_maximum_unique<OnlyConnected>(
			gLeft, gRight, jla_boost::AlwaysTrue(), jla_boost::AlwaysTrue(), callback);
}

// Named parameter variant of commonSubgraphs_maximum_unique
template<bool OnlyConnected, typename GraphLeft, typename GraphRight, typename Callback, typename Param, typename Tag, typename Rest>
void commonSubgraphs_maximum_unique(const GraphLeft &gLeft, const GraphRight &gRight, Callback callback,
                                    const boost::bgl_named_params<Param, Tag, Rest> &params) {
	commonSubgraphs_maximum_unique<OnlyConnected>(
			gLeft, gRight,
			choose_param(get_param(params, boost::edges_equivalent_t()), jla_boost::AlwaysTrue()),
			choose_param(get_param(params, boost::vertices_equivalent_t()), jla_boost::AlwaysTrue()),
			callback);
}

} // namespace jla_boost::GraphMorphism

#endif // JLA_BOOST_GRAPH_MORPHOSM_FINDERS_COMMON_SUBGRAPH_HPP