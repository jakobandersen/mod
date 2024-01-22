#ifndef JLA_BOOST_GRAPH_FILTEREDWRAPPER_HPP
#define JLA_BOOST_GRAPH_FILTEREDWRAPPER_HPP

#include <jla_boost/graph/AdaptorTraits.hpp>
#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <utility>
#include <vector>

namespace jla_boost {

template<typename Graph>
struct FilteredWrapper {
	using base_traits = boost::graph_traits<Graph>;
	// Graph
	using vertex_descriptor = typename base_traits::vertex_descriptor;
	using edge_descriptor = typename base_traits::edge_descriptor;
	using directed_category = typename base_traits::directed_category;
	using edge_parallel_category = typename base_traits::edge_parallel_category;
	using traversal_category = typename base_traits::traversal_category;

	// IncidenceGraph
	using out_edge_iterator = typename boost::graph_traits<Graph>::out_edge_iterator;
	using degree_size_type = typename boost::graph_traits<Graph>::degree_size_type;

	// BidirectionalGraph
	using in_edge_iterator = typename boost::graph_traits<Graph>::in_edge_iterator;

	// AdjacencyGraph
	// adjacency_iterator

	// VertexListGraph
	// vertex_iterator
	// vertices_size_type

	// EdgeListGraph
	using edge_iterator = typename boost::graph_traits<Graph>::edge_iterator;
	using edges_size_type = typename boost::graph_traits<Graph>::edges_size_type;
	using vertex_iterator = typename boost::graph_traits<Graph>::vertex_iterator;

	//graph_traits<filtered_graph>::edge_iterator 
	//
	//The type for the iterators returned by edges(), which is:
	//filter_iterator<EdgePredicate, graph_traits<Graph>::edge_iterator>
	//The iterator is a model of MultiPassInputIterator.
	//graph_traits<filtered_graph>::adjacency_iterator 
	//
	//The type for the iterators returned by adjacent_vertices(). The adjacency_iterator models the same iterator concept as out_edge_iterator.

	using vertices_size_type = typename boost::graph_traits<Graph>::vertices_size_type;

	//
	//The type used for dealing with the number of vertices in the graph.
	//graph_traits<filtered_graph>::edge_size_type 
	//
	//The type used for dealing with the number of edges in the graph.
	//graph_traits<filtered_graph>::degree_size_type 
public:
	explicit FilteredWrapper(const Graph &g)
			: g(g), map(num_vertices(g), std::numeric_limits<vertices_size_type>::max()) {
		vertices_size_type count = 0;
		//		std::cout << "FilterWrapper" << std::endl;

		for(const auto v: asRange(vertices(g))) {
			const auto vId = get(boost::vertex_index_t(), g, v);
			map[vId] = count;
			//			std::cout << "Map: " << vId << " => " << count << std::endl;
			//			std::cout << "ReverseMap: " << reverseMap.size() << " => " << vId << std::endl;
			reverseMap.push_back(vId);
			count++;
		}
	}

public:
	const Graph &g;
	std::vector<vertices_size_type> map;
	std::vector<vertices_size_type> reverseMap;
};

template<typename Graph>
FilteredWrapper<Graph> makeFilteredWrapper(const Graph &g) {
	return FilteredWrapper<Graph>(g);
}

template<typename Graph>
struct FilteredWrapperIndexMap {
	using VSizeType = typename boost::graph_traits<Graph>::vertices_size_type;
public:
	FilteredWrapperIndexMap() : g(nullptr) {}
	explicit FilteredWrapperIndexMap(const FilteredWrapper<Graph> &g) : g(&g) {}
public:
	VSizeType operator[](typename boost::graph_traits<Graph>::vertex_descriptor v) const {
		VSizeType vId = get(boost::vertex_index_t(), g->g, v);
		return g->map[vId];
	}
private:
	const FilteredWrapper<Graph> *g;
};

} // namespace jla_boost
namespace boost {

template<typename InnerGraph>
struct graph_traits<jla_boost::FilteredWrapper<InnerGraph>> : graph_traits<InnerGraph> {
};

} // namespace boost
namespace jla_boost {

// IncidenceGraph

template<typename Graph>
std::pair<typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::out_edge_iterator,
		typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::out_edge_iterator>
out_edges(typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::vertex_descriptor v,
          const jla_boost::FilteredWrapper<Graph> &g) {
	return out_edges(v, g.g);
}

template<typename Graph>
typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::vertex_descriptor
source(typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::edge_descriptor e,
       const jla_boost::FilteredWrapper<Graph> &g) {
	return source(e, g.g);
}

template<typename Graph>
typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::vertex_descriptor
target(typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::edge_descriptor e,
       const jla_boost::FilteredWrapper<Graph> &g) {
	return target(e, g.g);
}

template<typename Graph>
typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::degree_size_type
out_degree(typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::vertex_descriptor v,
           const jla_boost::FilteredWrapper<Graph> &g) {
	return out_degree(v, g.g);
}

// BidirectionalGraph

template<typename Graph>
std::pair<typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::in_edge_iterator,
		typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::in_edge_iterator>
in_edges(typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::vertex_descriptor v,
         const jla_boost::FilteredWrapper<Graph> &g) {
	return in_edges(v, g.g);
}

template<typename Graph>
typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::degree_size_type
in_degree(typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::vertex_descriptor v,
          const jla_boost::FilteredWrapper<Graph> &g) {
	return in_degree(v, g.g);
}

template<typename Graph>
typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::degree_size_type
degree(typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::vertex_descriptor v,
       const jla_boost::FilteredWrapper<Graph> &g) {
	return degree(v, g.g);
}

// AdjacencyGraph 
//adjacent_vertices(v, g)	std::pair<adjacency_iterator, adjacency_iterator>

// VertexListGraph

template<typename Graph>
std::pair<typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::vertex_iterator,
		typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::vertex_iterator>
vertices(const jla_boost::FilteredWrapper<Graph> &g) {
	return vertices(g.g);
}

template<typename Graph>
typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::vertices_size_type
num_vertices(const jla_boost::FilteredWrapper<Graph> &g) {
	return g.reverseMap.size();
}

// EdgeListGraph

template<typename Graph>
std::pair<typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::edge_iterator,
		typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::edge_iterator>
edges(const jla_boost::FilteredWrapper<Graph> &g) {
	return edges(g.g);
}

template<typename Graph>
typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::edges_size_type
num_edges(const jla_boost::FilteredWrapper<Graph> &g) {
	return std::distance(edges(g.g).first, edges(g.g).second);
}

// AdjacencyMatrix 

template<typename Graph>
std::pair<typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::edge_descriptor, bool>
edge(typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::vertex_descriptor u,
     typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::vertex_descriptor v,
     const jla_boost::FilteredWrapper<Graph> &g) {
	return edge(u, v, g.g);
}

} // namespace jla_boost
namespace boost {
// PropertyGraph

template<typename Graph>
struct property_traits<jla_boost::FilteredWrapperIndexMap<Graph>> {
	typedef typename graph_traits<jla_boost::FilteredWrapper<Graph>>::vertices_size_type value_type;
	typedef typename graph_traits<jla_boost::FilteredWrapper<Graph>>::vertices_size_type reference;
	typedef typename graph_traits<jla_boost::FilteredWrapper<Graph>>::vertex_descriptor key_type;
	typedef readable_property_map_tag category;
};

template<typename Graph>
struct vertex_property_type<jla_boost::FilteredWrapper<Graph>> : vertex_property_type<Graph> {
};

template<typename Graph>
struct edge_property_type<jla_boost::FilteredWrapper<Graph>> : edge_property_type<Graph> {
};

template<typename Graph>
struct graph_property_type<jla_boost::FilteredWrapper<Graph>> : graph_property_type<Graph> {
};

template<typename Graph, typename Property>
struct property_map<jla_boost::FilteredWrapper<Graph>, Property> {
	typedef typename property_map<Graph, Property>::type type;
	typedef typename property_map<Graph, Property>::const_type const_type;
};

template<typename Graph>
struct property_map<jla_boost::FilteredWrapper<Graph>, vertex_index_t> {
	typedef jla_boost::FilteredWrapperIndexMap<Graph> type;
	typedef type const_type;
};

} // namespace boost
namespace jla_boost {

template<typename Graph>
typename boost::property_traits<jla_boost::FilteredWrapperIndexMap<Graph>>::value_type
get(const jla_boost::FilteredWrapperIndexMap<Graph> &map,
    typename boost::property_traits<jla_boost::FilteredWrapperIndexMap<Graph>>::key_type k) {
	return map[k];
}

template<typename Graph, typename PropertyTag>
typename boost::property_map<jla_boost::FilteredWrapper<Graph>, PropertyTag>::const_type
get(PropertyTag t, const jla_boost::FilteredWrapper<Graph> &g) {
	return get(t, g.g);
}

template<typename Graph>
jla_boost::FilteredWrapperIndexMap<Graph> get(boost::vertex_index_t, const jla_boost::FilteredWrapper<Graph> &g) {
	return jla_boost::FilteredWrapperIndexMap<Graph>(g);
}

template<typename Graph, typename PropertyTag, typename VertexOrEdge>
typename boost::property_traits<typename boost::property_map<jla_boost::FilteredWrapper<Graph>, PropertyTag>::const_type>::reference
get(PropertyTag t, const jla_boost::FilteredWrapper<Graph> &g, VertexOrEdge ve) {
	return get(get(t, g), ve);
}

// Other

template<typename Graph>
inline typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::vertex_descriptor
vertex(typename boost::graph_traits<jla_boost::FilteredWrapper<Graph>>::vertices_size_type n,
       const jla_boost::FilteredWrapper<Graph> &g) {
	for(const auto v: asRange(vertices(g)))
		if(get(boost::vertex_index_t(), g, v) == n)
			return v;
	__builtin_unreachable();
}

template<typename Graph>
struct GraphAdaptorTraits<jla_boost::FilteredWrapper<Graph>> {
	using type = Graph;

	static const Graph &unwrap(const jla_boost::FilteredWrapper<Graph> &g) {
		return g.g;
	}
};

} // namespace jla_boost

#endif // JLA_BOOST_GRAPH_FILTEREDWRAPPER_HPP
