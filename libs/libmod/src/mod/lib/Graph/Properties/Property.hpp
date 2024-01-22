#ifndef MOD_LIB_GRAPH_PROPERTY_HPP
#define MOD_LIB_GRAPH_PROPERTY_HPP

#include <mod/Error.hpp>
#include <mod/lib/Graph/GraphDecl.hpp>
#include <mod/lib/LabelledGraph.hpp>
#include <mod/lib/IO/IO.hpp>

#include <vector>

namespace mod::lib::Graph {

template<typename Derived, typename VertexType, typename EdgeType>
struct Prop {
	using Vertex = typename boost::graph_traits<GraphType>::vertex_descriptor;
	using Edge = typename boost::graph_traits<GraphType>::edge_descriptor;
public:
	Prop(Prop &&) = default;
	Prop &operator=(Prop &&) = default;
	Prop(const Prop &) = delete;
	Prop &operator=(const Prop &) = delete;
public:
	void verify(const GraphType *g) const;
	explicit Prop(const GraphType &g);
	Prop(const Prop &other, const GraphType &g);
	void addVertex(Vertex v, const VertexType &label);
	void addEdge(Edge e, const EdgeType &label);
	const VertexType &operator[](Vertex v) const;
	const EdgeType &operator[](Edge e) const;
	Derived &getDerived();
	const Derived &getDerived() const;
protected:
	const GraphType *g;
	std::vector<VertexType> vertexState;
	std::vector<EdgeType> edgeState;
public:
	using Handler = IdentityPropertyHandler;
};

template<typename Derived, typename VertexType, typename EdgeType, typename VertexOrEdge>
auto get(const Prop<Derived, VertexType, EdgeType> &p, VertexOrEdge ve) -> decltype(p[ve]) {
	return p[ve];
}

//------------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------------

namespace detail {
void PropVerify(const GraphType *g, const GraphType *gOther,
                std::size_t nGraph, std::size_t nOther,
                std::size_t mGraph, std::size_t mOther);
} // namespace detail

template<typename Derived, typename VertexType, typename EdgeType>
void Prop<Derived, VertexType, EdgeType>::verify(const GraphType *g) const {
	detail::PropVerify(g, this->g, num_vertices(*g), vertexState.size(), num_edges(*g), edgeState.size());
}

template<typename Derived, typename VertexType, typename EdgeType>
Prop<Derived, VertexType, EdgeType>::Prop(const GraphType &g) : g(&g) {}

template<typename Derived, typename VertexType, typename EdgeType>
Prop<Derived, VertexType, EdgeType>::Prop(const Prop &other, const GraphType &g)
		: g(&g), vertexState(other.vertexState), edgeState(other.edgeState) {}

template<typename Derived, typename VertexType, typename EdgeType>
void Prop<Derived, VertexType, EdgeType>::addVertex([[maybe_unused]] Vertex v, const VertexType &value) {
	assert(num_vertices(*g) == vertexState.size() + 1);
	assert(get(boost::vertex_index_t(), *g, v) == vertexState.size());
	vertexState.push_back(value);
	verify(g);
}

template<typename Derived, typename VertexType, typename EdgeType>
void Prop<Derived, VertexType, EdgeType>::addEdge([[maybe_unused]] Edge e, const EdgeType &value) {
	assert(num_edges(*g) == edgeState.size() + 1);
	assert(get(boost::edge_index_t(), *g, e) == edgeState.size());
	edgeState.push_back(value);
	verify(g);
}

template<typename Derived, typename VertexType, typename EdgeType>
const VertexType &Prop<Derived, VertexType, EdgeType>::operator[](Vertex v) const {
	return vertexState[get(boost::vertex_index_t(), *g, v)];
}

template<typename Derived, typename VertexType, typename EdgeType>
const EdgeType &Prop<Derived, VertexType, EdgeType>::operator[](Edge e) const {
	assert(get(boost::edge_index_t(), *g, e) < edgeState.size());
	return edgeState[get(boost::edge_index_t(), *g, e)];
}

template<typename Derived, typename VertexType, typename EdgeType>
Derived &Prop<Derived, VertexType, EdgeType>::getDerived() {
	return static_cast<Derived &> (*this);
}

template<typename Derived, typename VertexType, typename EdgeType>
const Derived &Prop<Derived, VertexType, EdgeType>::getDerived() const {
	return static_cast<const Derived &> (*this);
}

} // namespace mod::lib::Graph

#endif // MOD_LIB_GRAPH_PROPERTY_HPP