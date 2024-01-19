#ifndef MOD_GRAPH_UNION_HPP
#define MOD_GRAPH_UNION_HPP

#include <mod/BuildConfig.hpp>
#include <mod/graph/ForwardDecl.hpp>
#include <mod/graph/Graph.hpp>

#include <boost/iterator/iterator_facade.hpp>

#include <memory>
#include <optional>
#include <vector>

namespace mod {
struct AtomId;
struct Isotope;
struct Charge;
enum class BondType;
} // namespace mod
namespace mod::graph {

// rst-class: graph::Union
// rst:
// rst:		An adaptor for a multiset of :class:`Graph`\ s to present them
// rst:		as their disjoint union.
// rst-class-start:
struct MOD_DECL Union {
	class Vertex;
	class Edge;
	class VertexIterator;
	class VertexRange;
	class EdgeIterator;
	class EdgeRange;
	class IncidentEdgeIterator;
	class IncidentEdgeRange;
private:
	struct Pimpl;
	Union(std::shared_ptr<const Pimpl> p);
public:
	// rst: .. type:: const_iterator
	// rst:           iterator = const_iterator
	// rst:
	// rst:		A random-access iterator over the :class:`Graph`\ s adapted by this object.
	using const_iterator = std::vector<std::shared_ptr<Graph>>::const_iterator;
	using iterator = const_iterator;
public:
	// rst: .. function:: Union()
	// rst:
	// rst:		Construct an empty graph.
	Union();
	// rst: .. function:: explicit Union(std::vector<std::shared_ptr<Graph>> graphs)
	// rst:
	// rst:		Construct a graph representing the disjoint union of `graphs`.
	// rst:
	// rst:		:throws LogicError: if a given graph is null.
	explicit Union(std::vector<std::shared_ptr<Graph>> graphs);
	Union(const Union &other);
	Union &operator=(const Union &other);
	Union(Union &&other);
	Union &operator=(Union &&other);
	const lib::LabelledUnionGraph<lib::Graph::LabelledGraph> &getInner() const;
	MOD_DECL friend bool operator==(const Union &a, const Union &b);
	MOD_DECL friend bool operator!=(const Union &a, const Union &b);
	MOD_DECL friend bool operator<(const Union &a, const Union &b);
	// rst: .. function:: friend std::ostream &operator<<(std::ostream &s, const Union &ug)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const Union &ug);
	// rst: .. function:: std::size_t size() const
	// rst:
	// rst:		:returns: the number of adapted graphs.
	std::size_t size() const;
	// rst: .. function:: const_iterator begin() const
	// rst:               const_iterator end() const
	// rst:
	// rst:		:returns: the range of graphs adapted by this object.
	const_iterator begin() const;
	const_iterator end() const;
	// rst: .. function:: std::shared_ptr<Graph> operator[](std::size_t i) const
	// rst:
	// rst:		:returns: the `i`\ th adapted graph.
	// rst:		:throws: :class:`LogicError` if `i` is out of range.
	std::shared_ptr<Graph> operator[](std::size_t i) const;
public:
	// rst: .. function:: std::size_t numVertices() const
	// rst:
	// rst:		:returns: the number of vertices in the graph.
	std::size_t numVertices() const;
	// rst: .. function:: VertexRange vertices() const
	// rst:
	// rst:		:returns: a range of all vertices in the graph.
	VertexRange vertices() const;
	// rst: .. function:: std::size_t numEdges() const
	// rst:
	// rst:		:returns: the number of edges in the graph.
	std::size_t numEdges() const;
	// rst: .. function:: EdgeRange edges() const
	// rst:
	// rst:		:returns: a range of all edges in the graph.
	EdgeRange edges() const;
public:
	// rst: .. function:: void printTermState() const
	// rst:
	// rst:		Print the term state for the graph.
	void printTermState() const;
private:
	std::shared_ptr<const Pimpl> p;
};
// rst-class-end:


// Graph
// -----------------------------------------------------------------------------

// rst-class: graph::Union::Vertex
// rst:
// rst:		A descriptor of either a vertex in a union graph, or a null vertex.
// rst-class-start:
struct MOD_DECL Union::Vertex {
	Vertex(Union g, std::size_t vId);
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
	// rst:	.. function:: Union getGraph() const
	// rst:
	// rst:		:returns: the union graph the vertex belongs to.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	Union getGraph() const;
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
	// rst:		:returns: the string label of the vertex.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	const std::string &getStringLabel() const;
	// rst:	.. function:: AtomId getAtomId() const
	// rst:
	// rst:		:returns: the atom id of the vertex.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	AtomId getAtomId() const;
	// rst:	.. function:: Isotope getIsotope() const
	// rst:
	// rst:		:returns: the isotope of the vertex.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	Isotope getIsotope() const;
	// rst:	.. function:: Charge getCharge() const
	// rst:
	// rst:		:returns: the charge of the vertex.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	Charge getCharge() const;
	// rst:	.. function:: bool getRadical() const
	// rst:
	// rst:		:returns: the radical status of the vertex.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	bool getRadical() const;
	// rst: .. function:: std::string printStereo() const
	// rst:               std::string printStereo(const Printer &p) const
	// rst:
	// rst:		Print the stereo configuration for the vertex.
	// rst:
	// rst:		:returns: the name of the PDF-file that will be compiled in post-processing.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	std::string printStereo() const;
	std::string printStereo(const Printer &p) const;
	// rst: .. function:: int getGraphIndex() const
	// rst:
	// rst:		:returns: The index of the graph in the owning :class:`Union` graph this vertex
	// rst:			is from. The index is thus in the range :math:`[0, len(getGraph())[`.
	// rst:
	// rst:			We can obtain the underlying :class:`Graph` this vertex
	// rst:			is from both directly via `getVertex()` as `getVertex().getGraph()`,
	// rst:			or via the graph index as ``getGraph()[getGraphIndex()]``.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	int getGraphIndex() const;
	// rst: .. function:: int getVertex() const
	// rst:
	// rst:		:returns: The underlying :class:`Graph::Vertex` this vertex represents.
	// rst:
	// rst:			We can obtain the underlying :class:`Graph` this vertex
	// rst:			is from both directly via `getVertex()` as `getVertex().getGraph()`,
	// rst:			or via the graph index as ``getGraph()[getGraphIndex()]``.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	Graph::Vertex getVertex() const;
private:
	std::optional<Union> g;
	std::size_t vId;
};
// rst-class-end:

// rst-class: graph::Union::Edge
// rst:
// rst:		A descriptor of either an edge in a union graph, or a null edge.
// rst-class-start:
class MOD_DECL Union::Edge {
	friend class EdgeIterator;
	friend class IncidentEdgeIterator;
	Edge(Union g, std::size_t vId, std::size_t eId);
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
	// rst:	.. function:: Union getGraph() const
	// rst:
	// rst:		:returns: the graph the edge belongs to.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	Union getGraph() const;
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
	// rst: .. function:: int getGraphIndex() const
	// rst:
	// rst:		:returns: The index of the graph in the owning :class:`Union` graph this e
	// rst:			is from. The index is thus in the range :math:`[0, len(getGraph())[`.
	// rst:
	// rst:			We can obtain the underlying :class:`Graph` this edge
	// rst:			is from both directly via `getEdge()` as `getEdge().getGraph()`,
	// rst:			or via the graph index as ``getGraph()[getGraphIndex()]``.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	int getGraphIndex() const;
	// rst: .. function:: int getEdge() const
	// rst:
	// rst:		:returns: The underlying :class:`Graph::Vertex` this vertex represents.
	// rst:
	// rst:			We can obtain the underlying :class:`Graph` this edge
	// rst:			is from both directly via `getEdge()` as `getEdge().getGraph()`,
	// rst:			or via the graph index as ``getGraph()[getGraphIndex()]``.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	Graph::Edge getEdge() const;
private:
	std::optional<Union> g;
	std::size_t vId, eId;
};
// rst-class-end:


// VertexList
// -----------------------------------------------------------------------------

// rst-class: graph::Union::VertexIterator
// rst:
// rst:		An iterator for traversing all vertices in a union graph.
// rst:		It models a forward iterator.
// rst-class-start:
class MOD_DECL Union::VertexIterator
		: public boost::iterator_facade<VertexIterator, Vertex, std::forward_iterator_tag, Vertex> {
	friend class Union;
	VertexIterator(Union g);
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
	std::optional<Union> g;
	std::size_t vId;
};
// rst-class-end:

// rst-class: graph::Union::VertexRange
// rst:
// rst:		A range of all vertices in a union graph.
// rst-class-start:
struct Union::VertexRange {
	using iterator = VertexIterator;
	using const_iterator = iterator;
private:
	friend class Union;
	VertexRange(Union g);
public:
	VertexIterator begin() const;
	VertexIterator end() const;
	Vertex operator[](std::size_t i) const;
private:
	Union g;
};
// rst-class-end:


// EdgeList
// -----------------------------------------------------------------------------

// rst-class: graph::Union::EdgeIterator
// rst:
// rst:		An iterator for traversing all edges in a union graph.
// rst:		It models a forward iterator.
// rst-class-start:
class MOD_DECL Union::EdgeIterator
		: public boost::iterator_facade<EdgeIterator, Edge, std::forward_iterator_tag, Edge> {
	friend class Union;
	EdgeIterator(Union g);
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
	std::optional<Union> g;
	std::size_t vId, eId;
};
// rst-class-end:

// rst-class: graph::Union::EdgeRange
// rst:
// rst:		A range of all edges in a union graph.
// rst-class-start:
struct Union::EdgeRange {
	using iterator = EdgeIterator;
	using const_iterator = iterator;
private:
	friend class Union;
	EdgeRange(Union g);
public:
	EdgeIterator begin() const;
	EdgeIterator end() const;
private:
	Union g;
};
// rst-class-end:


// Incidence
// -----------------------------------------------------------------------------

// rst-class: graph::Union::IncidentEdgeIterator
// rst:
// rst:		An iterator for traversing all edges in a union graph.
// rst:		It models a forward iterator.
// rst-class-start:
class MOD_DECL Union::IncidentEdgeIterator
		: public boost::iterator_facade<IncidentEdgeIterator, Edge, std::forward_iterator_tag, Edge> {
	friend class Union;
	IncidentEdgeIterator(Union g, std::size_t vId);
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
	std::optional<Union> g;
	std::size_t vId, eId;
};
// rst-class-end:

// rst-class: graph::Union::IncidentEdgeRange
// rst:
// rst:		A range of all incident edges to a vertex in a union graph.
// rst-class-start:
struct Union::IncidentEdgeRange {
	using iterator = IncidentEdgeIterator;
	using const_iterator = iterator;
private:
	friend class Vertex;
	IncidentEdgeRange(Union g, std::size_t vId);
public:
	IncidentEdgeIterator begin() const;
	IncidentEdgeIterator end() const;
private:
	Union g;
	std::size_t vId;
};
// rst-class-end:


//------------------------------------------------------------------------------
// Implementation Details
//------------------------------------------------------------------------------

// VertexRange
//------------------------------------------------------------------------------

inline Union::VertexRange::VertexRange(Union g) : g(g) {}

inline Union::VertexIterator Union::VertexRange::begin() const {
	return VertexIterator(g);
}

inline Union::VertexIterator Union::VertexRange::end() const {
	return VertexIterator();
}

inline Union::Vertex Union::VertexRange::operator[](std::size_t i) const {
	return Vertex(g, i); // it will convert to a null descriptor if out of range
}

// EdgeRange
//------------------------------------------------------------------------------

inline Union::EdgeRange::EdgeRange(Union g) : g(g) {}

inline Union::EdgeIterator Union::EdgeRange::begin() const {
	return EdgeIterator(g);
}

inline Union::EdgeIterator Union::EdgeRange::end() const {
	return EdgeIterator();
}

// IncidentEdgeRange
//------------------------------------------------------------------------------

inline Union::IncidentEdgeRange::IncidentEdgeRange(Union g, std::size_t vId) : g(g), vId(vId) {}

inline Union::IncidentEdgeIterator Union::IncidentEdgeRange::begin() const {
	return IncidentEdgeIterator(g, vId);
}

inline Union::IncidentEdgeIterator Union::IncidentEdgeRange::end() const {
	return IncidentEdgeIterator();
}

} // namespace mod::graph

template<>
struct std::hash<mod::graph::Union::Vertex> {
	std::size_t operator()(const mod::graph::Union::Vertex &v) const {
		return v.hash();
	}
};

#endif // MOD_GRAPH_UNION_HPP