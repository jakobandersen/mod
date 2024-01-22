#ifndef MOD_GRAPH_GRAPHINTERFACE_HPP
#define MOD_GRAPH_GRAPHINTERFACE_HPP

// rst: This header contains the definitions for the graph interface for :cpp:class:`graph::Graph`.
// rst:

#include <mod/BuildConfig.hpp>
#include <mod/graph/Graph.hpp>

#include <boost/iterator/iterator_facade.hpp>

#include <functional>

namespace mod {
struct AtomId;
struct Isotope;
struct Charge;
enum class BondType;
} // namespace mod
namespace mod::graph {

// Graph
// -----------------------------------------------------------------------------

// rst-class: graph::Graph::Vertex
// rst:
// rst:		A descriptor of either a vertex in a graph, or a null vertex.
// rst-class-start:
struct MOD_DECL Graph::Vertex {
	Vertex(std::shared_ptr<Graph> g, std::size_t vId);
public:
	// rst:	.. function:: Vertex()
	// rst:
	// rst:		Constructs a null descriptor.
	Vertex();
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const Vertex &v);
	MOD_DECL friend bool operator==(const Vertex &v1, const Vertex &v2);
	MOD_DECL friend bool operator!=(const Vertex &v1, const Vertex &v2);
	MOD_DECL friend bool operator<(const Vertex &v1, const Vertex &v2);
	std::size_t hash() const;
	// rst: .. function:: explicit operator bool() const
	// rst:
	// rst:		:returns: :cpp:expr:`!isNull()`
	explicit operator bool() const;
	// rst:	.. function:: bool isNull() const
	// rst:
	// rst:		:returns: whether this is a null descriptor or not.
	bool isNull() const;
	// rst:	.. function:: std::size_t getId() const
	// rst:
	// rst:		:returns: the index of the vertex. It will be in the range :math:`[0, numVertices[`.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	std::size_t getId() const;
	// rst:	.. function:: std::shared_ptr<Graph> getGraph() const
	// rst:
	// rst:		:returns: the graph the vertex belongs to.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	std::shared_ptr<Graph> getGraph() const;
	// rst:	.. function:: std::size_t getDegree() const
	// rst:
	// rst:		:returns: the degree of the vertex.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	std::size_t getDegree() const;
	// rst:	.. function:: IncidentEdgeRange incidentEdges() const
	// rst:
	// rst:		:returns: a range of incident edges to this vertex.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	IncidentEdgeRange incidentEdges() const;
	// rst:	.. function:: const std::string &getStringLabel() const
	// rst:
	// rst: 	:returns: the string label of the vertex.
	// rst: 	:throws: :cpp:class:`LogicError` if it is a null descriptor.
	const std::string &getStringLabel() const;
	// rst:	.. function:: AtomId getAtomId() const
	// rst:
	// rst:		:returns: the atom id of the vertex.
	// rst: 	:throws: :cpp:class:`LogicError` if it is a null descriptor.
	AtomId getAtomId() const;
	// rst:	.. function:: Isotope getIsotope() const
	// rst:
	// rst:		:returns: the isotope of the vertex.
	// rst: 	:throws: :cpp:class:`LogicError` if it is a null descriptor.
	Isotope getIsotope() const;
	// rst:	.. function:: Charge getCharge() const
	// rst:
	// rst:		:returns: the charge of the vertex.
	// rst: 	:throws: :cpp:class:`LogicError` if it is a null descriptor.
	Charge getCharge() const;
	// rst:	.. function:: bool getRadical() const
	// rst:
	// rst:		:returns: the radical status of the vertex.
	// rst: 	:throws: :cpp:class:`LogicError` if it is a null descriptor.
	bool getRadical() const;
	// rst: .. function:: std::string printStereo() const
	// rst:               std::string printStereo(const Printer &p) const
	// rst:
	// rst:		Print the stereo configuration for the vertex.
	// rst:
	// rst:		:returns: the name of the PDF-file that will be compiled in post-processing.
	// rst: 	:throws: :cpp:class:`LogicError` if it is a null descriptor.
	std::string printStereo() const;
	std::string printStereo(const Printer &p) const;
private:
	std::shared_ptr<Graph> g;
	std::size_t vId;
};
// rst-class-end:

// rst-class: graph::Graph::Edge
// rst:
// rst:		A descriptor of either an edge in a graph, or a null edge.
// rst-class-start:
struct MOD_DECL Graph::Edge {
	Edge(std::shared_ptr<Graph> g, std::size_t vId, std::size_t eId);
public:
	// rst:	.. function:: Edge()
	// rst:
	// rst:		Constructs a null descriptor.
	Edge();
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const Edge &e);
	MOD_DECL friend bool operator==(const Edge &e1, const Edge &e2);
	MOD_DECL friend bool operator!=(const Edge &e1, const Edge &e2);
	MOD_DECL friend bool operator<(const Edge &e1, const Edge &e2);
	// rst: .. function:: explicit operator bool() const
	// rst:
	// rst:		:returns: :cpp:expr:`!isNull()`
	explicit operator bool() const;
	// rst:	.. function:: bool isNull() const
	// rst:
	// rst:		:returns: whether this is a null descriptor or not.
	bool isNull() const;
	// rst:	.. function:: std::shared_ptr<Graph> getGraph() const
	// rst:
	// rst:		:returns: the graph the edge belongs to.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	std::shared_ptr<Graph> getGraph() const;
	// rst:	.. function:: Vertex source() const
	// rst:
	// rst:		:returns: the source vertex of the edge.
	// rst: 	:throws: :cpp:class:`LogicError` if it is a null descriptor.
	Vertex source() const;
	// rst:	.. function:: Vertex target() const
	// rst:
	// rst:		:returns: the target vertex of the edge.
	// rst: 	:throws: :cpp:class:`LogicError` if it is a null descriptor.
	Vertex target() const;
	// rst:	.. function:: const std::string &getStringLabel() const
	// rst:
	// rst: 	:returns: the string label of the edge.
	// rst: 	:throws: :cpp:class:`LogicError` if it is a null descriptor.
	const std::string &getStringLabel() const;
	// rst:	.. function:: BondType getBondType() const
	// rst:
	// rst:		:returns: the bond type of the edge.
	// rst: 	:throws: :cpp:class:`LogicError` if it is a null descriptor.
	BondType getBondType() const;
private:
	std::shared_ptr<Graph> g;
	std::size_t vId, eId;
};
// rst-class-end:


// VertexList
// -----------------------------------------------------------------------------

// rst-class: graph::Graph::VertexIterator
// rst:
// rst:		An iterator for traversing all vertices in a graph.
// rst:		It models a forward iterator.
// rst-class-start:
class MOD_DECL Graph::VertexIterator
		: public boost::iterator_facade<VertexIterator, Vertex, std::forward_iterator_tag, Vertex> {
	friend class Graph;
	VertexIterator(std::shared_ptr<Graph> g);
public:
	// rst:	.. function:: VertexIterator()
	// rst:
	// rst:		Construct a past-the-end iterator.
	VertexIterator();
private:
	friend class boost::iterator_core_access;

	Vertex dereference() const;
	bool equal(const VertexIterator &iter) const;
	void increment();
private:
	std::shared_ptr<Graph> g;
	std::size_t vId;
};
// rst-class-end:

// rst-class: graph::Graph::VertexRange
// rst:
// rst:		A range of all vertices in a graph.
// rst-class-start:
struct Graph::VertexRange {
	using iterator = VertexIterator;
	using const_iterator = iterator;
private:
	friend class Graph;
	VertexRange(std::shared_ptr<Graph> g);
public:
	VertexIterator begin() const;
	VertexIterator end() const;
	Vertex operator[](std::size_t i) const;
private:
	std::shared_ptr<Graph> g;
};
// rst-class-end:


// EdgeList
// -----------------------------------------------------------------------------

// rst-class: graph::Graph::EdgeIterator
// rst:
// rst:		An iterator for traversing all edges in a graph.
// rst:		It models a forward iterator.
// rst-class-start:
class MOD_DECL Graph::EdgeIterator
		: public boost::iterator_facade<EdgeIterator, Edge, std::forward_iterator_tag, Edge> {
	friend class Graph;
	EdgeIterator(std::shared_ptr<Graph> g);
public:
	// rst:	.. function:: EdgeIterator()
	// rst:
	// rst:		Construct a past-the-end iterator.
	EdgeIterator();
private:
	friend class boost::iterator_core_access;
	Edge dereference() const;
	bool equal(const EdgeIterator &iter) const;
	void increment();
	void advanceToValid();
private:
	std::shared_ptr<Graph> g;
	std::size_t vId, eId;
};
// rst-class-end:

// rst-class: graph::Graph::EdgeRange
// rst:
// rst:		A range of all edges in a graph.
// rst-class-start:
struct Graph::EdgeRange {
	using iterator = EdgeIterator;
	using const_iterator = iterator;
private:
	friend class Graph;
	EdgeRange(std::shared_ptr<Graph> g);
public:
	EdgeIterator begin() const;
	EdgeIterator end() const;
private:
	std::shared_ptr<Graph> g;
};
// rst-class-end:


// Incidence
// -----------------------------------------------------------------------------

// rst-class: graph::Graph::IncidentEdgeIterator
// rst:
// rst:		An iterator for traversing all edges in a graph.
// rst:		It models a forward iterator.
// rst-class-start:
class MOD_DECL Graph::IncidentEdgeIterator
		: public boost::iterator_facade<IncidentEdgeIterator, Edge, std::forward_iterator_tag, Edge> {
	friend class Graph;
	IncidentEdgeIterator(std::shared_ptr<Graph> g, std::size_t vId);
public:
	// rst:	.. function:: IncidentEdgeIterator()
	// rst:
	// rst:		Construct a past-the-end iterator.
	IncidentEdgeIterator();
private:
	friend class boost::iterator_core_access;
	Edge dereference() const;
	bool equal(const IncidentEdgeIterator &iter) const;
	void increment();
private:
	std::shared_ptr<Graph> g;
	std::size_t vId, eId;
};
// rst-class-end:

// rst-class: graph::Graph::IncidentEdgeRange
// rst:
// rst:		A range of all incident edges to a vertex in a graph.
// rst-class-start:
struct Graph::IncidentEdgeRange {
	using iterator = IncidentEdgeIterator;
	using const_iterator = iterator;
private:
	friend class Vertex;
	IncidentEdgeRange(std::shared_ptr<Graph> g, std::size_t vId);
public:
	IncidentEdgeIterator begin() const;
	IncidentEdgeIterator end() const;
private:
	std::shared_ptr<Graph> g;
	std::size_t vId;
};
// rst-class-end:


//------------------------------------------------------------------------------
// Implementation Details
//------------------------------------------------------------------------------

// VertexRange
//------------------------------------------------------------------------------

inline Graph::VertexRange::VertexRange(std::shared_ptr<Graph> g) : g(g) {}

inline Graph::VertexIterator Graph::VertexRange::begin() const {
	return VertexIterator(g);
}

inline Graph::VertexIterator Graph::VertexRange::end() const {
	return VertexIterator();
}

inline Graph::Vertex Graph::VertexRange::operator[](std::size_t i) const {
	return Vertex(g, i); // it will convert to a null descriptor if out of range
}

// EdgeRange
//------------------------------------------------------------------------------

inline Graph::EdgeRange::EdgeRange(std::shared_ptr<Graph> g) : g(g) {}

inline Graph::EdgeIterator Graph::EdgeRange::begin() const {
	return EdgeIterator(g);
}

inline Graph::EdgeIterator Graph::EdgeRange::end() const {
	return EdgeIterator();
}

// IncidentEdgeRange
//------------------------------------------------------------------------------

inline Graph::IncidentEdgeRange::IncidentEdgeRange(std::shared_ptr<Graph> g, std::size_t vId) : g(g), vId(vId) {}

inline Graph::IncidentEdgeIterator Graph::IncidentEdgeRange::begin() const {
	return IncidentEdgeIterator(g, vId);
}

inline Graph::IncidentEdgeIterator Graph::IncidentEdgeRange::end() const {
	return IncidentEdgeIterator();
}

} // namespace mod::graph

template<>
struct std::hash<mod::graph::Graph::Vertex> {
	std::size_t operator()(const mod::graph::Graph::Vertex &v) const {
		return v.hash();
	}
};

#endif // MOD_GRAPH_GRAPHINTERFACE_HPP