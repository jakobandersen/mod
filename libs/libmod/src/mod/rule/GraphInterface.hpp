#ifndef MOD_RULE_GRAPHINTERFACE_HPP
#define MOD_RULE_GRAPHINTERFACE_HPP

// rst: This header contains the definitions for the graph interface for :cpp:class:`rule::Rule`.
// rst:

#include <mod/rule/Rule.hpp>

#include <boost/iterator/iterator_facade.hpp>

#include <functional>

namespace mod {
struct AtomId;
struct Isotope;
struct Charge;
enum class BondType;
} // namespace mod
namespace mod::rule {

// rst: ========================================================================
// rst: Left
// rst: ========================================================================
// rst:
// rst-class: rule::Rule::LeftGraph
// rst:
// rst:		A proxy object representing the left graph of the rule.
// rst-class-start:
struct Rule::LeftGraph {
	class Vertex;
	class Edge;
	class VertexIterator;
	class VertexRange;
	class EdgeIterator;
	class EdgeRange;
	class IncidentEdgeIterator;
	class IncidentEdgeRange;
public:
	LeftGraph(std::shared_ptr<Rule> r);
	// rst: .. function:: friend bool operator==(const LeftGraph &a, const LeftGraph &b)
	// rst:               friend bool operator!=(const LeftGraph &a, const LeftGraph &b)
	// rst:               friend bool operator<(const LeftGraph &a, const LeftGraph &b)
	// rst:               friend std::ostream &operator<<(std::ostream &s, const LeftGraph &g)
	// rst:
	MOD_DECL friend bool operator==(const LeftGraph &a, const LeftGraph &b) { return a.r == b.r; }
	MOD_DECL friend bool operator!=(const LeftGraph &a, const LeftGraph &b) { return a.r != b.r; }
	MOD_DECL friend bool operator<(const LeftGraph &a, const LeftGraph &b) { return a.r < b.r; }
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const LeftGraph &g);
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
	// rst: .. function:: std::shared_ptr<Rule> getRule() const
	// rst:
	// rst:		:returns: the rule where the graph belongs to.
	std::shared_ptr<Rule> getRule() const;
private:
	std::shared_ptr<Rule> r;
};
// rst-class-end:



// Graph
// -----------------------------------------------------------------------------

// rst-class: rule::Rule::LeftGraph::Vertex
// rst:
// rst:		A descriptor of either a vertex in a rule, or a null vertex.
// rst-class-start:
class Rule::LeftGraph::Vertex {
	friend class Edge;
	friend class VertexIterator;
	friend class lib::rule::Rule;
	Vertex(LeftGraph g, std::size_t vId);
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
	// rst:		:returns: :cpp:texpr:`!isNull()`
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
	// rst:	.. function:: LeftGraph getGraph() const
	// rst:
	// rst:		:returns: the graph the vertex belongs to.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	LeftGraph getGraph() const;
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
	// rst:               std::string printStereo(const graph::Printer &p) const
	// rst:
	// rst:		Print the stereo configuration for the vertex.
	// rst:
	// rst:		:returns: the name of the PDF-file that will be compiled in post-processing.
	// rst: 	:throws: :cpp:class:`LogicError` if it is a null descriptor.
	std::string printStereo() const;
	std::string printStereo(const graph::Printer &p) const;
	// rst: .. function:: Rule::Vertex getCore() const
	// rst:
	// rst:		:returns: the descriptor for this vertex in the core graph.
	Rule::Vertex getCore() const;
	// rst:	.. function:: std::shared_ptr<Rule> getRule() const
	// rst:
	// rst:		:returns: the rule the vertex belongs to.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	std::shared_ptr<Rule> getRule() const;
private:
	std::optional<LeftGraph> g;
	std::size_t vId;
};
// rst-class-end:

// rst-class: rule::Rule::LeftGraph::Edge
// rst:
// rst:		A descriptor of either an edge in a rule, or a null edge.
// rst-class-start:
class Rule::LeftGraph::Edge {
	friend class EdgeIterator;
	friend class IncidentEdgeIterator;
	friend class lib::rule::Rule;
	Edge(LeftGraph g, std::size_t vId, std::size_t eId);
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
	// rst:		:returns: :cpp:texpr:`!isNull()`
	explicit operator bool() const;
	// rst:	.. function:: bool isNull() const
	// rst:
	// rst:		:returns: whether this is a null descriptor or not.
	bool isNull() const;
	// rst:	.. function:: LeftGraph getGraph() const
	// rst:
	// rst:		:returns: the graph the edge belongs to.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	LeftGraph getGraph() const;
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
	// rst: .. function:: Rule::Edge getCore() const
	// rst:
	// rst:		:returns: the descriptor for this edge in the core graph.
	Rule::Edge getCore() const;
	// rst:	.. function:: std::shared_ptr<Rule> getRule() const
	// rst:
	// rst:		:returns: the rule the edge belongs to.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	std::shared_ptr<Rule> getRule() const;
private:
	std::optional<LeftGraph> g;
	std::size_t vId, eId;
};
// rst-class-end:


// VertexList
// -----------------------------------------------------------------------------

// rst-class: rule::Rule::LeftGraph::VertexIterator
// rst:
// rst:		An iterator for traversing all vertices in a rule.
// rst:		It models a forward iterator.
// rst-class-start:
class Rule::LeftGraph::VertexIterator
		: public boost::iterator_facade<VertexIterator, Vertex, std::forward_iterator_tag, Vertex> {
	friend class Rule;
	VertexIterator(LeftGraph g);
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
	std::optional<LeftGraph> g;
	std::size_t vId;
};
// rst-class-end:

// rst-class: rule::Rule::LeftGraph::VertexRange
// rst:
// rst:		A range of all vertices in a rule.
// rst-class-start:
struct Rule::LeftGraph::VertexRange {
	using iterator = VertexIterator;
	using const_iterator = iterator;
private:
	friend class Rule;
	VertexRange(LeftGraph g);
public:
	VertexIterator begin() const;
	VertexIterator end() const;
private:
	LeftGraph g;
};
// rst-class-end:


// EdgeList
// -----------------------------------------------------------------------------

// rst-class: rule::Rule::LeftGraph::EdgeIterator
// rst:
// rst:		An iterator for traversing all edges in a rule.
// rst:		It models a forward iterator.
// rst-class-start:
class Rule::LeftGraph::EdgeIterator
		: public boost::iterator_facade<EdgeIterator, Edge, std::forward_iterator_tag, Edge> {
	friend class Rule;
	EdgeIterator(LeftGraph g);
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
	std::optional<LeftGraph> g;
	std::size_t vId, eId;
};
// rst-class-end:

// rst-class: rule::Rule::LeftGraph::EdgeRange
// rst:
// rst:		A range of all edges in a rule.
// rst-class-start:
struct Rule::LeftGraph::EdgeRange {
	using iterator = EdgeIterator;
	using const_iterator = iterator;
private:
	friend class Rule;
	EdgeRange(LeftGraph g);
public:
	EdgeIterator begin() const;
	EdgeIterator end() const;
private:
	LeftGraph g;
};
// rst-class-end:


// Incidence
// -----------------------------------------------------------------------------

// rst-class: rule::Rule::LeftGraph::IncidentEdgeIterator
// rst:
// rst:		An iterator for traversing all edges in a rule.
// rst:		It models a forward iterator.
// rst-class-start:
class Rule::LeftGraph::IncidentEdgeIterator
		: public boost::iterator_facade<IncidentEdgeIterator, Edge, std::forward_iterator_tag, Edge> {
	friend class Rule;
	IncidentEdgeIterator(LeftGraph g, std::size_t vId);
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
	std::optional<LeftGraph> g;
	std::size_t vId, eId;
};
// rst-class-end:

// rst-class: rule::Rule::LeftGraph::IncidentEdgeRange
// rst:
// rst:		A range of all incident edges to a vertex in a rule.
// rst-class-start:
struct Rule::LeftGraph::IncidentEdgeRange {
	using iterator = IncidentEdgeIterator;
	using const_iterator = iterator;
private:
	friend class Vertex;
	IncidentEdgeRange(LeftGraph g, std::size_t vId);
public:
	IncidentEdgeIterator begin() const;
	IncidentEdgeIterator end() const;
private:
	LeftGraph g;
	std::size_t vId;
};
// rst-class-end:


// rst:
// rst: ========================================================================
// rst: Context
// rst: ========================================================================
// rst:
// rst-class: rule::Rule::ContextGraph
// rst:
// rst:		A proxy object representing the context graph of the rule.
// rst-class-start:
struct Rule::ContextGraph {
	class Vertex;
	class Edge;
	class VertexIterator;
	class VertexRange;
	class EdgeIterator;
	class EdgeRange;
	class IncidentEdgeIterator;
	class IncidentEdgeRange;
public:
	ContextGraph(std::shared_ptr<Rule> r);
	// rst: .. function:: friend bool operator==(const ContextGraph &a, const ContextGraph &b)
	//	rst:               friend bool operator!=(const ContextGraph &a, const ContextGraph &b)
	//	rst:               friend bool operator<(const ContextGraph &a, const ContextGraph &b)
	//	rst:               friend std::ostream &operator<<(std::ostream &s, const ContextGraph &g)
	// rst:
	MOD_DECL friend bool operator==(const ContextGraph &a, const ContextGraph &b) { return a.r == b.r; }
	MOD_DECL friend bool operator!=(const ContextGraph &a, const ContextGraph &b) { return a.r != b.r; }
	MOD_DECL friend bool operator<(const ContextGraph &a, const ContextGraph &b) { return a.r < b.r; }
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const ContextGraph &g);
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
	// rst: .. function:: std::shared_ptr<Rule> getRule() const
	// rst:
	// rst:		:returns: the rule where the graph belongs to.
	std::shared_ptr<Rule> getRule() const;
private:
	std::shared_ptr<Rule> r;
};
// rst-class-end:


// Graph
// -----------------------------------------------------------------------------

// rst-class: rule::Rule::ContextGraph::Vertex
// rst:
// rst:		A descriptor of either a vertex in a rule, or a null vertex.
// rst-class-start:
class Rule::ContextGraph::Vertex {
	friend class Edge;
	friend class VertexIterator;
	friend class lib::rule::Rule;
	Vertex(ContextGraph g, std::size_t vId);
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
	// rst:		:returns: :cpp:texpr:`!isNull()`
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
	// rst:	.. function:: ContextGraph getGraph() const
	// rst:
	// rst:		:returns: the graph the vertex belongs to.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	ContextGraph getGraph() const;
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
	// rst: .. function:: Rule::Vertex getCore() const
	// rst:
	// rst:		:returns: the descriptor for this vertex in the core graph.
	Rule::Vertex getCore() const;
	// rst:	.. function:: std::shared_ptr<Rule> getRule() const
	// rst:
	// rst:		:returns: the rule the vertex belongs to.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	std::shared_ptr<Rule> getRule() const;
private:
	std::optional<ContextGraph> g;
	std::size_t vId;
};
// rst-class-end:

// rst-class: rule::Rule::ContextGraph::Edge
// rst:
// rst:		A descriptor of either an edge in a rule, or a null edge.
// rst-class-start:
class Rule::ContextGraph::Edge {
	friend class EdgeIterator;
	friend class IncidentEdgeIterator;
	friend class lib::rule::Rule;
	Edge(ContextGraph g, std::size_t vId, std::size_t eId);
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
	// rst:		:returns: :cpp:texpr:`!isNull()`
	explicit operator bool() const;
	// rst:	.. function:: bool isNull() const
	// rst:
	// rst:		:returns: whether this is a null descriptor or not.
	bool isNull() const;
	// rst:	.. function:: ContextGraph getGraph() const
	// rst:
	// rst:		:returns: the graph the edge belongs to.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	ContextGraph getGraph() const;
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
	// rst:	.. function:: std::shared_ptr<Rule> getRule() const
	// rst:
	// rst:		:returns: the rule the edge belongs to.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	std::shared_ptr<Rule> getRule() const;
	// rst: .. function:: Rule::Edge getCore() const
	// rst:
	// rst:		:returns: the descriptor for this edge in the core graph.
	Rule::Edge getCore() const;
private:
	std::optional<ContextGraph> g;
	std::size_t vId, eId;
};
// rst-class-end:


// VertexList
// -----------------------------------------------------------------------------

// rst-class: rule::Rule::ContextGraph::VertexIterator
// rst:
// rst:		An iterator for traversing all vertices in a rule.
// rst:		It models a forward iterator.
// rst-class-start:
class Rule::ContextGraph::VertexIterator
		: public boost::iterator_facade<VertexIterator, Vertex, std::forward_iterator_tag, Vertex> {
	friend class Rule;
	VertexIterator(ContextGraph g);
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
	std::optional<ContextGraph> g;
	std::size_t vId;
};
// rst-class-end:

// rst-class: rule::Rule::ContextGraph::VertexRange
// rst:
// rst:		A range of all vertices in a rule.
// rst-class-start:
struct Rule::ContextGraph::VertexRange {
	using iterator = VertexIterator;
	using const_iterator = iterator;
private:
	friend class Rule;
	VertexRange(ContextGraph g);
public:
	VertexIterator begin() const;
	VertexIterator end() const;
private:
	ContextGraph g;
};
// rst-class-end:


// EdgeList
// -----------------------------------------------------------------------------

// rst-class: rule::Rule::ContextGraph::EdgeIterator
// rst:
// rst:		An iterator for traversing all edges in a rule.
// rst:		It models a forward iterator.
// rst-class-start:
class Rule::ContextGraph::EdgeIterator
		: public boost::iterator_facade<EdgeIterator, Edge, std::forward_iterator_tag, Edge> {
	friend class Rule;
	EdgeIterator(ContextGraph g);
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
	std::optional<ContextGraph> g;
	std::size_t vId, eId;
};
// rst-class-end:

// rst-class: rule::Rule::ContextGraph::EdgeRange
// rst:
// rst:		A range of all edges in a rule.
// rst-class-start:
struct Rule::ContextGraph::EdgeRange {
	using iterator = EdgeIterator;
	using const_iterator = iterator;
private:
	friend class Rule;
	EdgeRange(ContextGraph g);
public:
	EdgeIterator begin() const;
	EdgeIterator end() const;
private:
	ContextGraph g;
};
// rst-class-end:


// Incidence
// -----------------------------------------------------------------------------

// rst-class: rule::Rule::ContextGraph::IncidentEdgeIterator
// rst:
// rst:		An iterator for traversing all edges in a rule.
// rst:		It models a forward iterator.
// rst-class-start:
class Rule::ContextGraph::IncidentEdgeIterator
		: public boost::iterator_facade<IncidentEdgeIterator, Edge, std::forward_iterator_tag, Edge> {
	friend class Rule;
	IncidentEdgeIterator(ContextGraph g, std::size_t vId);
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
	std::optional<ContextGraph> g;
	std::size_t vId, eId;
};
// rst-class-end:

// rst-class: rule::Rule::ContextGraph::IncidentEdgeRange
// rst:
// rst:		A range of all incident edges to a vertex in a rule.
// rst-class-start:
struct Rule::ContextGraph::IncidentEdgeRange {
	using iterator = IncidentEdgeIterator;
	using const_iterator = iterator;
private:
	friend class Vertex;
	IncidentEdgeRange(ContextGraph g, std::size_t vId);
public:
	IncidentEdgeIterator begin() const;
	IncidentEdgeIterator end() const;
private:
	ContextGraph g;
	std::size_t vId;
};
// rst-class-end:


// rst:
// rst: ========================================================================
// rst: Right
// rst: ========================================================================
// rst:
// rst-class: rule::Rule::RightGraph
// rst:
// rst:		A proxy object representing the right graph of the rule.
// rst-class-start:
struct Rule::RightGraph {
	class Vertex;
	class Edge;
	class VertexIterator;
	class VertexRange;
	class EdgeIterator;
	class EdgeRange;
	class IncidentEdgeIterator;
	class IncidentEdgeRange;
public:
	RightGraph(std::shared_ptr<Rule> r);
	// rst: .. function:: friend bool operator==(const RightGraph &a, const RightGraph &b)
	// rst:               friend bool operator!=(const RightGraph &a, const RightGraph &b)
	// rst:               friend bool operator<(const RightGraph &a, const RightGraph &b)
	// rst:               friend std::ostream &operator<<(std::ostream &s, const RightGraph &g);
	// rst:
	MOD_DECL friend bool operator==(const RightGraph &a, const RightGraph &b) { return a.r == b.r; }
	MOD_DECL friend bool operator!=(const RightGraph &a, const RightGraph &b) { return a.r != b.r; }
	MOD_DECL friend bool operator<(const RightGraph &a, const RightGraph &b) { return a.r < b.r; }
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const RightGraph &g);
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
	// rst: .. function:: std::shared_ptr<Rule> getRule() const
	// rst:
	// rst:		:returns: the rule where the graph belongs to.
	std::shared_ptr<Rule> getRule() const;
private:
	std::shared_ptr<Rule> r;
};
// rst-class-end:


// Graph
// -----------------------------------------------------------------------------

// rst-class: rule::Rule::RightGraph::Vertex
// rst:
// rst:		A descriptor of either a vertex in a rule, or a null vertex.
// rst-class-start:
class Rule::RightGraph::Vertex {
	friend class Edge;
	friend class VertexIterator;
	friend class lib::rule::Rule;
	Vertex(RightGraph g, std::size_t vId);
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
	// rst:		:returns: :cpp:texpr:`!isNull()`
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
	// rst:	.. function:: RightGraph getGraph() const
	// rst:
	// rst:		:returns: the graph the vertex belongs to.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	RightGraph getGraph() const;
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
	// rst:               std::string printStereo(const graph::Printer &p) const
	// rst:
	// rst:		Print the stereo configuration for the vertex.
	// rst:
	// rst:		:returns: the name of the PDF-file that will be compiled in post-processing.
	// rst: 	:throws: :cpp:class:`LogicError` if it is a null descriptor.
	std::string printStereo() const;
	std::string printStereo(const graph::Printer &p) const;
	// rst: .. function:: Rule::Vertex getCore() const
	// rst:
	// rst:		:returns: the descriptor for this vertex in the core graph.
	Rule::Vertex getCore() const;
	// rst:	.. function:: std::shared_ptr<Rule> getRule() const
	// rst:
	// rst:		:returns: the rule the vertex belongs to.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	std::shared_ptr<Rule> getRule() const;
private:
	std::optional<RightGraph> g;
	std::size_t vId;
};
// rst-class-end:

// rst-class: rule::Rule::RightGraph::Edge
// rst:
// rst:		A descriptor of either an edge in a rule, or a null edge.
// rst-class-start:
class Rule::RightGraph::Edge {
	friend class EdgeIterator;
	friend class IncidentEdgeIterator;
	friend class lib::rule::Rule;
	Edge(RightGraph g, std::size_t vId, std::size_t eId);
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
	// rst:		:returns: :cpp:texpr:`!isNull()`
	explicit operator bool() const;
	// rst:	.. function:: bool isNull() const
	// rst:
	// rst:		:returns: whether this is a null descriptor or not.
	bool isNull() const;
	// rst:	.. function:: RightGraph getGraph() const
	// rst:
	// rst:		:returns: the rule the edge belongs to.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	RightGraph getGraph() const;
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
	// rst:	.. function:: std::shared_ptr<Rule> getRule() const
	// rst:
	// rst:		:returns: the rule the edge belongs to.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	std::shared_ptr<Rule> getRule() const;
	// rst: .. function:: Rule::Edge getCore() const
	// rst:
	// rst:		:returns: the descriptor for this edge in the core graph.
	Rule::Edge getCore() const;
private:
	std::optional<RightGraph> g;
	std::size_t vId, eId;
};
// rst-class-end:


// VertexList
// -----------------------------------------------------------------------------

// rst-class: rule::Rule::RightGraph::VertexIterator
// rst:
// rst:		An iterator for traversing all vertices in a rule.
// rst:		It models a forward iterator.
// rst-class-start:
class Rule::RightGraph::VertexIterator
		: public boost::iterator_facade<VertexIterator, Vertex, std::forward_iterator_tag, Vertex> {
	friend class Rule;
	VertexIterator(RightGraph g);
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
	std::optional<RightGraph> g;
	std::size_t vId;
};
// rst-class-end:

// rst-class: rule::Rule::RightGraph::VertexRange
// rst:
// rst:		A range of all vertices in a rule.
// rst-class-start:
struct Rule::RightGraph::VertexRange {
	using iterator = VertexIterator;
	using const_iterator = iterator;
private:
	friend class Rule;
	VertexRange(RightGraph g);
public:
	VertexIterator begin() const;
	VertexIterator end() const;
private:
	RightGraph g;
};
// rst-class-end:


// EdgeList
// -----------------------------------------------------------------------------

// rst-class: rule::Rule::RightGraph::EdgeIterator
// rst:
// rst:		An iterator for traversing all edges in a rule.
// rst:		It models a forward iterator.
// rst-class-start:
class Rule::RightGraph::EdgeIterator
		: public boost::iterator_facade<EdgeIterator, Edge, std::forward_iterator_tag, Edge> {
	friend class Rule;
	EdgeIterator(RightGraph g);
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
	std::optional<RightGraph> g;
	std::size_t vId, eId;
};
// rst-class-end:

// rst-class: rule::Rule::RightGraph::EdgeRange
// rst:
// rst:		A range of all edges in a rule.
// rst-class-start:
struct Rule::RightGraph::EdgeRange {
	using iterator = EdgeIterator;
	using const_iterator = iterator;
private:
	friend class Rule;
	EdgeRange(RightGraph g);
public:
	EdgeIterator begin() const;
	EdgeIterator end() const;
private:
	RightGraph g;
};
// rst-class-end:


// Incidence
// -----------------------------------------------------------------------------

// rst-class: rule::Rule::RightGraph::IncidentEdgeIterator
// rst:
// rst:		An iterator for traversing all edges in a rule.
// rst:		It models a forward iterator.
// rst-class-start:
class Rule::RightGraph::IncidentEdgeIterator
		: public boost::iterator_facade<IncidentEdgeIterator, Edge, std::forward_iterator_tag, Edge> {
	friend class Rule;
	IncidentEdgeIterator(RightGraph g, std::size_t vId);
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
	std::optional<RightGraph> g;
	std::size_t vId, eId;
};
// rst-class-end:

// rst-class: rule::Rule::RightGraph::IncidentEdgeRange
// rst:
// rst:		A range of all incident edges to a vertex in a rule.
// rst-class-start:
struct Rule::RightGraph::IncidentEdgeRange {
	using iterator = IncidentEdgeIterator;
	using const_iterator = iterator;
private:
	friend class Vertex;
	IncidentEdgeRange(RightGraph g, std::size_t vId);
public:
	IncidentEdgeIterator begin() const;
	IncidentEdgeIterator end() const;
private:
	RightGraph g;
	std::size_t vId;
};
// rst-class-end:


// rst:
// rst: ========================================================================
// rst: Core
// rst: ========================================================================
// rst:

// Graph
// -----------------------------------------------------------------------------

// rst-class: rule::Rule::Vertex
// rst:
// rst:		A descriptor of either a vertex in a rule, or a null vertex.
// rst-class-start:
struct Rule::Vertex {
	friend class Edge;
	friend class VertexIterator;
	friend class lib::rule::Rule;
	Vertex(std::shared_ptr<Rule> r, std::size_t vId);
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
	// rst:		:returns: :cpp:texpr:`!isNull()`
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
	// rst:	.. function:: std::shared_ptr<Rule> getGraph() const
	// rst:	              std::shared_ptr<Rule> getRule() const
	// rst:
	// rst:		:returns: the graph the vertex belongs to, which happens to be the rule object it self.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	std::shared_ptr<Rule> getGraph() const;
	std::shared_ptr<Rule> getRule() const { return getGraph(); }
	// rst: .. function:: LeftGraph::Vertex getLeft() const
	// rst:
	// rst:		:returns: a null descriptor if this vertex is not in the left graph,
	// rst:			otherwise the descriptor of this vertex in the left graph.
	LeftGraph::Vertex getLeft() const;
	// rst: .. function:: ContextGraph::Vertex getContext() const
	// rst:
	// rst:		:returns: a null descriptor if this vertex is not in the context graph,
	// rst:			otherwise the descriptor of this vertex in the context graph.
	ContextGraph::Vertex getContext() const;
	// rst: .. function:: RightGraph::Vertex getRight() const
	// rst:
	// rst:		:returns: a null descriptor if this vertex is not in the right graph,
	// rst:			otherwise the descriptor of this vertex in the right graph.
	RightGraph::Vertex getRight() const;
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
	// rst:	.. function:: double get2DX(bool withHydrogens=true)
	// rst:
	// rst:		:returns: the x-coordinate in a 2D depiction of the rule.
	// rst:			Different sets of coordinates exists for rendering with and without certain hydrogens.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor, or
	// rst:				if `withHydrogens` is `true` and the vertex is a "clean" hydrogen.
	double get2DX(bool withHydrogens = true);
	// rst:	.. function:: double get2DY(bool withHydrogens=true)
	// rst:
	// rst:		:returns: the y-coordinate in a 2D depiction of the rule.
	// rst:			Different sets of coordinates exists for rendering with and without certain hydrogens.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor, or
	// rst:				if `withHydrogens` is `true` and the vertex is a "clean" hydrogen.
	double get2DY(bool withHydrogens = true);
private:
	std::shared_ptr<Rule> r;
	std::size_t vId;
};
// rst-class-end:

// rst-class: rule::Rule::Edge
// rst:
// rst:		A descriptor of either an edge in a rule, or a null edge.
// rst-class-start:
class Rule::Edge {
	friend class Rule::LeftGraph::Edge;
	friend class Rule::ContextGraph::Edge;
	friend class Rule::RightGraph::Edge;

	friend class EdgeIterator;
	friend class IncidentEdgeIterator;
	friend class lib::rule::Rule;
	Edge(std::shared_ptr<Rule> r, std::size_t vId, std::size_t eId);
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
	// rst:		:returns: :cpp:texpr:`!isNull()`
	explicit operator bool() const;
	// rst:	.. function:: bool isNull() const
	// rst:
	// rst:		:returns: whether this is a null descriptor or not.
	bool isNull() const;
	// rst:	.. function:: std::shared_ptr<Rule> getGraph() const
	// rst:	              std::shared_ptr<Rule> getRule() const
	// rst:
	// rst:		:returns: the rule the edge belongs to, which happens to be the rule object it self.
	// rst:		:throws: :cpp:class:`LogicError` if it is a null descriptor.
	std::shared_ptr<Rule> getGraph() const;
	std::shared_ptr<Rule> getRule() const { return getGraph(); }
	// rst: .. function:: LeftGraph::Edge getLeft() const
	// rst:
	// rst:		:returns: a null descriptor if this edge is not in the left graph,
	// rst:			otherwise the descriptor of this edge in the left graph.
	LeftGraph::Edge getLeft() const;
	// rst: .. function:: ContextGraph::Edge getContext() const
	// rst:
	// rst:		:returns: a null descriptor if this edge is not in the context graph,
	// rst:			otherwise the descriptor of this edge in the context graph.
	ContextGraph::Edge getContext() const;
	// rst: .. function:: RightGraph::Edge getRight() const
	// rst:
	// rst:		:returns: a null descriptor if this edge is not in the right graph,
	// rst:			otherwise the descriptor of this edge in the right graph.
	RightGraph::Edge getRight() const;
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
private:
	std::shared_ptr<Rule> r;
	std::size_t vId, eId;
};
// rst-class-end:


// VertexList
// -----------------------------------------------------------------------------

// rst-class: rule::Rule::VertexIterator
// rst:
// rst:		An iterator for traversing all vertices in a rule.
// rst:		It models a forward iterator.
// rst-class-start:
class Rule::VertexIterator : public boost::iterator_facade<VertexIterator, Vertex, std::forward_iterator_tag, Vertex> {
	friend class Rule;
	VertexIterator(std::shared_ptr<Rule> r);
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
	std::shared_ptr<Rule> r;
	std::size_t vId;
};
// rst-class-end:

// rst-class: rule::Rule::VertexRange
// rst:
// rst:		A range of all vertices in a rule.
// rst-class-start:
struct Rule::VertexRange {
	using iterator = VertexIterator;
	using const_iterator = iterator;
private:
	friend class Rule;
	VertexRange(std::shared_ptr<Rule> r);
public:
	VertexIterator begin() const;
	VertexIterator end() const;
	Vertex operator[](std::size_t i) const;
private:
	std::shared_ptr<Rule> r;
};
// rst-class-end:


// EdgeList
// -----------------------------------------------------------------------------

// rst-class: rule::Rule::EdgeIterator
// rst:
// rst:		An iterator for traversing all edges in a rule.
// rst:		It models a forward iterator.
// rst-class-start:
class Rule::EdgeIterator : public boost::iterator_facade<EdgeIterator, Edge, std::forward_iterator_tag, Edge> {
	friend class Rule;
	EdgeIterator(std::shared_ptr<Rule> r);
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
	std::shared_ptr<Rule> r;
	std::size_t vId, eId;
};
// rst-class-end:

// rst-class: rule::Rule::EdgeRange
// rst:
// rst:		A range of all edges in a rule.
// rst-class-start:
struct Rule::EdgeRange {
	using iterator = EdgeIterator;
	using const_iterator = iterator;
private:
	friend class Rule;
	EdgeRange(std::shared_ptr<Rule> r);
public:
	EdgeIterator begin() const;
	EdgeIterator end() const;
private:
	std::shared_ptr<Rule> r;
};
// rst-class-end:


// Incidence
// -----------------------------------------------------------------------------

// rst-class: rule::Rule::IncidentEdgeIterator
// rst:
// rst:		An iterator for traversing all edges in a rule.
// rst:		It models a forward iterator.
// rst-class-start:
class Rule::IncidentEdgeIterator
		: public boost::iterator_facade<IncidentEdgeIterator, Edge, std::forward_iterator_tag, Edge> {
	friend class Rule;
	IncidentEdgeIterator(std::shared_ptr<Rule> r, std::size_t vId);
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
	std::shared_ptr<Rule> r;
	std::size_t vId, eId;
};
// rst-class-end:

// rst-class: rule::Rule::IncidentEdgeRange
// rst:
// rst:		A range of all incident edges to a vertex in a rule.
// rst-class-start:
struct Rule::IncidentEdgeRange {
	using iterator = IncidentEdgeIterator;
	using const_iterator = iterator;
private:
	friend class Vertex;
	IncidentEdgeRange(std::shared_ptr<Rule> r, std::size_t vId);
public:
	IncidentEdgeIterator begin() const;
	IncidentEdgeIterator end() const;
private:
	std::shared_ptr<Rule> r;
	std::size_t vId;
};
// rst-class-end:


//##############################################################################
// Implementation Details
//##############################################################################

//==============================================================================
// Left
//==============================================================================

inline Rule::LeftGraph::LeftGraph(std::shared_ptr<Rule> r) : r(r) {}

inline Rule::LeftGraph::VertexRange Rule::LeftGraph::vertices() const {
	return VertexRange(r);
}

inline Rule::LeftGraph::EdgeRange Rule::LeftGraph::edges() const {
	return EdgeRange(r);
}

inline std::shared_ptr<Rule> Rule::LeftGraph::getRule() const {
	return r;
}

// VertexRange
//------------------------------------------------------------------------------

inline Rule::LeftGraph::VertexRange::VertexRange(LeftGraph g) : g(g) {}

inline Rule::LeftGraph::VertexIterator Rule::LeftGraph::VertexRange::begin() const {
	return VertexIterator(g);
}

inline Rule::LeftGraph::VertexIterator Rule::LeftGraph::VertexRange::end() const {
	return VertexIterator();
}

// EdgeRange
//------------------------------------------------------------------------------

inline Rule::LeftGraph::EdgeRange::EdgeRange(LeftGraph g) : g(g) {}

inline Rule::LeftGraph::EdgeIterator Rule::LeftGraph::EdgeRange::begin() const {
	return EdgeIterator(g);
}

inline Rule::LeftGraph::EdgeIterator Rule::LeftGraph::EdgeRange::end() const {
	return EdgeIterator();
}

// IncidentEdgeRange
//------------------------------------------------------------------------------

inline Rule::LeftGraph::IncidentEdgeRange::IncidentEdgeRange(LeftGraph g, std::size_t vId)
		: g(g), vId(vId) {}

inline Rule::LeftGraph::IncidentEdgeIterator Rule::LeftGraph::IncidentEdgeRange::begin() const {
	return IncidentEdgeIterator(g, vId);
}

inline Rule::LeftGraph::IncidentEdgeIterator Rule::LeftGraph::IncidentEdgeRange::end() const {
	return IncidentEdgeIterator();
}

//==============================================================================
// Context
//==============================================================================

inline Rule::ContextGraph::ContextGraph(std::shared_ptr<Rule> r) : r(r) {}

inline Rule::ContextGraph::VertexRange Rule::ContextGraph::vertices() const {
	return VertexRange(r);
}

inline Rule::ContextGraph::EdgeRange Rule::ContextGraph::edges() const {
	return EdgeRange(r);
}

inline std::shared_ptr<Rule> Rule::ContextGraph::getRule() const {
	return r;
}

// VertexRange
//------------------------------------------------------------------------------

inline Rule::ContextGraph::VertexRange::VertexRange(ContextGraph g) : g(g) {}

inline Rule::ContextGraph::VertexIterator Rule::ContextGraph::VertexRange::begin() const {
	return VertexIterator(g);
}

inline Rule::ContextGraph::VertexIterator Rule::ContextGraph::VertexRange::end() const {
	return VertexIterator();
}

// EdgeRange
//------------------------------------------------------------------------------

inline Rule::ContextGraph::EdgeRange::EdgeRange(ContextGraph g) : g(g) {}

inline Rule::ContextGraph::EdgeIterator Rule::ContextGraph::EdgeRange::begin() const {
	return EdgeIterator(g);
}

inline Rule::ContextGraph::EdgeIterator Rule::ContextGraph::EdgeRange::end() const {
	return EdgeIterator();
}

// IncidentEdgeRange
//------------------------------------------------------------------------------

inline Rule::ContextGraph::IncidentEdgeRange::IncidentEdgeRange(ContextGraph g, std::size_t vId)
		: g(g), vId(vId) {}

inline Rule::ContextGraph::IncidentEdgeIterator Rule::ContextGraph::IncidentEdgeRange::begin() const {
	return IncidentEdgeIterator(g, vId);
}

inline Rule::ContextGraph::IncidentEdgeIterator Rule::ContextGraph::IncidentEdgeRange::end() const {
	return IncidentEdgeIterator();
}

//==============================================================================
// Right
//==============================================================================

inline Rule::RightGraph::RightGraph(std::shared_ptr<Rule> r) : r(r) {}

inline Rule::RightGraph::VertexRange Rule::RightGraph::vertices() const {
	return VertexRange(r);
}

inline Rule::RightGraph::EdgeRange Rule::RightGraph::edges() const {
	return EdgeRange(r);
}

inline std::shared_ptr<Rule> Rule::RightGraph::getRule() const {
	return r;
}


// VertexRange
//------------------------------------------------------------------------------

inline Rule::RightGraph::VertexRange::VertexRange(RightGraph g) : g(g) {}

inline Rule::RightGraph::VertexIterator Rule::RightGraph::VertexRange::begin() const {
	return VertexIterator(g);
}

inline Rule::RightGraph::VertexIterator Rule::RightGraph::VertexRange::end() const {
	return VertexIterator();
}

// EdgeRange
//------------------------------------------------------------------------------

inline Rule::RightGraph::EdgeRange::EdgeRange(RightGraph g) : g(g) {}

inline Rule::RightGraph::EdgeIterator Rule::RightGraph::EdgeRange::begin() const {
	return EdgeIterator(g);
}

inline Rule::RightGraph::EdgeIterator Rule::RightGraph::EdgeRange::end() const {
	return EdgeIterator();
}


// IncidentEdgeRange
//------------------------------------------------------------------------------

inline Rule::RightGraph::IncidentEdgeRange::IncidentEdgeRange(RightGraph g, std::size_t vId)
		: g(g), vId(vId) {}

inline Rule::RightGraph::IncidentEdgeIterator Rule::RightGraph::IncidentEdgeRange::begin() const {
	return IncidentEdgeIterator(g, vId);
}

inline Rule::RightGraph::IncidentEdgeIterator Rule::RightGraph::IncidentEdgeRange::end() const {
	return IncidentEdgeIterator();
}

//==============================================================================
// Core
//==============================================================================

// VertexRange
//------------------------------------------------------------------------------

inline Rule::VertexRange::VertexRange(std::shared_ptr<Rule> r) : r(r) {}

inline Rule::VertexIterator Rule::VertexRange::begin() const {
	return VertexIterator(r);
}

inline Rule::VertexIterator Rule::VertexRange::end() const {
	return VertexIterator();
}

inline Rule::Vertex Rule::VertexRange::operator[](std::size_t i) const {
	return Vertex(r, i); // it will convert to a null descriptor if out of range
}

// EdgeRange
//------------------------------------------------------------------------------

inline Rule::EdgeRange::EdgeRange(std::shared_ptr<Rule> r) : r(r) {}

inline Rule::EdgeIterator Rule::EdgeRange::begin() const {
	return EdgeIterator(r);
}

inline Rule::EdgeIterator Rule::EdgeRange::end() const {
	return EdgeIterator();
}

// IncidentEdgeRange
//------------------------------------------------------------------------------

inline Rule::IncidentEdgeRange::IncidentEdgeRange(std::shared_ptr<Rule> r, std::size_t vId) : r(r), vId(vId) {}

inline Rule::IncidentEdgeIterator Rule::IncidentEdgeRange::begin() const {
	return IncidentEdgeIterator(r, vId);
}

inline Rule::IncidentEdgeIterator Rule::IncidentEdgeRange::end() const {
	return IncidentEdgeIterator();
}

} // namespace mod::rule

template<>
struct std::hash<mod::rule::Rule::LeftGraph::Vertex> {
	std::size_t operator()(const mod::rule::Rule::Vertex &v) const {
		return v.hash();
	}
};

template<>
struct std::hash<mod::rule::Rule::ContextGraph::Vertex> {
	std::size_t operator()(const mod::rule::Rule::Vertex &v) const {
		return v.hash();
	}
};

template<>
struct std::hash<mod::rule::Rule::RightGraph::Vertex> {
	std::size_t operator()(const mod::rule::Rule::Vertex &v) const {
		return v.hash();
	}
};

template<>
struct std::hash<mod::rule::Rule::Vertex> {
	std::size_t operator()(const mod::rule::Rule::Vertex &v) const {
		return v.hash();
	}
};

#endif // MOD_RULE_GRAPHINTERFACE_HPP