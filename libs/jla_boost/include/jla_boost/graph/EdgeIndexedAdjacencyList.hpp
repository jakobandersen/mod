#ifndef JLA_BOOST_GRAPH_EDGEINDEXEDADJACENCYLIST_HPP
#define JLA_BOOST_GRAPH_EDGEINDEXEDADJACENCYLIST_HPP

#include <boost/graph/adjacency_list.hpp>

namespace jla_boost {

template<typename DirectedS,
		typename VertexProperty = boost::no_property,
		typename EdgeProperty = boost::no_property,
		typename GraphProperty = boost::no_property>
struct EdgeIndexedAdjacencyList {
	using Self = EdgeIndexedAdjacencyList;
	using GraphType = boost::adjacency_list<boost::vecS, boost::vecS, DirectedS,
			VertexProperty, boost::property<boost::edge_index_t, std::size_t, EdgeProperty>, GraphProperty,
			boost::listS>;
	// Do not use vecS for the edges, as it seems that the edge properties are stored by value there
	// and the edge descriptors contain a pointer to the property.
public:
	EdgeIndexedAdjacencyList() = default;
public: // Graph
	using vertex_descriptor = typename boost::graph_traits<GraphType>::vertex_descriptor;
	using edge_descriptor = typename boost::graph_traits<GraphType>::edge_descriptor;
	using directed_category = typename boost::graph_traits<GraphType>::directed_category;
	using edge_parallel_category = typename boost::graph_traits<GraphType>::edge_parallel_category;
	using traversal_category = typename boost::graph_traits<GraphType>::traversal_category;

	static vertex_descriptor null_vertex() {
		return GraphType::null_vertex();
	}
public: // IncidenceGraph
	using out_edge_iterator = typename boost::graph_traits<GraphType>::out_edge_iterator;
	using degree_size_type = typename boost::graph_traits<GraphType>::degree_size_type;

	friend std::pair<out_edge_iterator, out_edge_iterator> out_edges(vertex_descriptor v, const Self &g) {
		return out_edges(v, g.g);
	}

	friend vertex_descriptor source(edge_descriptor e, const Self &g) {
		return source(e, g.g);
	}

	friend vertex_descriptor target(edge_descriptor e, const Self &g) {
		return target(e, g.g);
	}

	friend degree_size_type out_degree(vertex_descriptor v, const Self &g) {
		return out_degree(v, g.g);
	}
public: // BidirectionalGraph
	using in_edge_iterator = typename boost::graph_traits<GraphType>::in_edge_iterator;

	friend std::pair<in_edge_iterator, in_edge_iterator> in_edges(vertex_descriptor v, const Self &g) {
		return in_edges(v, g.g);
	}

	friend degree_size_type in_degree(vertex_descriptor v, const Self &g) {
		return in_degree(v, g.g);
	}

	friend degree_size_type degree(vertex_descriptor v, const Self &g) {
		return degree(v, g.g);
	}
public:// AdjacencyGraph
	using adjacency_iterator = typename boost::graph_traits<GraphType>::adjacency_iterator;

	friend std::pair<adjacency_iterator, adjacency_iterator> adjacent_vertices(vertex_descriptor v, const Self &g) {
		return adjacent_vertices(v, g.g);
	}
public: // VertexListGraph
	using vertex_iterator = typename boost::graph_traits<GraphType>::vertex_iterator;
	using vertices_size_type = typename boost::graph_traits<GraphType>::vertices_size_type;

	friend std::pair<vertex_iterator, vertex_iterator> vertices(const Self &g) {
		return vertices(g.g);
	}

	friend vertices_size_type num_vertices(const Self &g) {
		return num_vertices(g.g);
	}
public: // EdgeListGraph
	using edge_iterator = typename boost::graph_traits<GraphType>::edge_iterator;
	using edges_size_type = typename boost::graph_traits<GraphType>::edges_size_type;

	friend std::pair<edge_iterator, edge_iterator> edges(const Self &g) {
		return edges(g.g);
	}

	friend edges_size_type num_edges(const Self &g) {
		return num_edges(g.g);
	}
public: // "AdjacencyMatrix" (it's not constant time)

	friend std::pair<edge_descriptor, bool> edge(vertex_descriptor u, vertex_descriptor v, const Self &g) {
		return edge(u, v, g.g);
	}
public: // MutableGraph

	friend vertex_descriptor add_vertex(Self &g) {
		return add_vertex(g.g);
	}
	//	clear_vertex(v, g) void
	//	remove_vertex(v, g) void

	friend std::pair<edge_descriptor, bool> add_edge(vertex_descriptor u, vertex_descriptor v, Self &g) {
		auto eId = num_edges(g.g);
		auto ePair = add_edge(u, v, {eId, {}}, g.g);
		assert(ePair.second);
		return ePair;
	}
	//	remove_edge(u, v, g) void
	//	remove_edge(e, g) void
	//	remove_edge(e_iter, g)
public: // MutablePropertyGraph
	//add_vertex(vp, g)	vertex_descriptor

	friend std::pair<edge_descriptor, bool> add_edge(vertex_descriptor u, vertex_descriptor v, EdgeProperty ep, Self &g) {
		auto eId = num_edges(g.g);
		auto ePair = add_edge(u, v, {eId, std::move(ep)}, g.g);
		assert(ePair.second);
		return ePair;
	}
public: // PropertyGraph

	template<typename PropertyTag>
	friend decltype(auto) get(PropertyTag t, const Self &g) {
		return get(t, g.g);
	}

	template<typename PropertyTag, typename VertexOrEdge>
	friend decltype(auto) get(PropertyTag t, const Self &g, VertexOrEdge ve) {
		return get(t, g.g, ve);
	}

	template<typename PropertyTag, typename VertexOrEdge, typename Value>
	friend void put(PropertyTag t, Self &g, VertexOrEdge ve, Value &&v) {
		put(t, g.g, ve, std::forward<Value>(v));
	}

	template<typename VertexOrEdge, typename Value>
	friend void put(const boost::edge_index_t &, Self &g, VertexOrEdge ve, Value &&v) = delete; // You should not mess with this.

	template<typename VertexOrEdge>
	decltype(auto) operator[](VertexOrEdge ve) {
		return g[ve];
	}

	template<typename VertexOrEdge>
	decltype(auto) operator[](VertexOrEdge ve) const {
		return g[ve];
	}
public: // Other

	friend vertex_descriptor vertex(vertices_size_type n, const Self &g) {
		return vertex(n, g.g);
	}
private:
	GraphType g;
};

} // namespace jla_boost
namespace boost {

// PropertyGraph
//------------------------------------------------------------------------------

template<typename DirectedS, typename VertexProperty, typename EdgeProperty, typename GraphProperty, typename Property>
struct property_map<jla_boost::EdgeIndexedAdjacencyList<DirectedS, VertexProperty, EdgeProperty, GraphProperty>, Property>
		: property_map<typename jla_boost::EdgeIndexedAdjacencyList<DirectedS, VertexProperty, EdgeProperty, GraphProperty>::GraphType, Property> {
};

template<typename DirectedS, typename VertexProperty, typename EdgeProperty, typename GraphProperty, typename Property>
struct property_map<const jla_boost::EdgeIndexedAdjacencyList<DirectedS, VertexProperty, EdgeProperty, GraphProperty>, Property>
		: property_map<typename jla_boost::EdgeIndexedAdjacencyList<DirectedS, VertexProperty, EdgeProperty, GraphProperty>::GraphType, Property> {
};

} // namespace boost

#endif // JLA_BOOST_GRAPH_EDGEINDEXEDADJACENCYLIST_HPP