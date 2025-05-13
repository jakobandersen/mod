#ifndef MOD_LIB_DG_GRAPHDECL_HPP
#define MOD_LIB_DG_GRAPHDECL_HPP

#include <mod/lib/Graph/Multiset.hpp>

#include <boost/graph/adjacency_list.hpp>

#include <optional>

namespace mod::lib::graph {
struct Graph;
} // namespace mod::lib::graph
namespace mod::lib::rule {
struct Rule;
} // namespace mod::lib::rule
namespace mod::lib::DG {
// NonHyper
struct NonHyperVProp;
struct NonHyperEProp;
using NonHyperGraphType = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, NonHyperVProp, NonHyperEProp>;
using NonHyperVertex = boost::graph_traits<NonHyperGraphType>::vertex_descriptor;
using NonHyperEdge = boost::graph_traits<NonHyperGraphType>::edge_descriptor;
using GraphMultiset = lib::graph::Multiset<const lib::graph::Graph>;

// Hyper
struct HyperVProp;
using HyperGraphType = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, HyperVProp>;
using HyperVertex = boost::graph_traits<HyperGraphType>::vertex_descriptor;
using HyperEdge = boost::graph_traits<HyperGraphType>::edge_descriptor;

// NonHyper
struct NonHyperVProp {
	NonHyperVProp() = default;
public:
	GraphMultiset graphs;
};

struct NonHyperEProp {
	HyperVertex hyper // the representative of this hyperedge
			= boost::graph_traits<HyperGraphType>::null_vertex(); // initialized to prevent GCC warning
	std::vector<const lib::rule::Rule *> rules;
	std::optional<NonHyperEdge> reverse;
};

struct NonHyperEdgeHash {
	NonHyperEdgeHash(const NonHyperGraphType &dg) : dg(&dg) {}

	std::size_t operator()(NonHyperEdge e) const {
		std::size_t hash = get(boost::vertex_index_t(), *dg, source(e, *dg));
		boost::hash_combine(hash, get(boost::vertex_index_t(), *dg, target(e, *dg)));
		return hash;
	}
private:
	const NonHyperGraphType *dg;
};

// Hyper
enum class HyperVertexKind {
	Vertex, Edge
};

struct HyperVProp {
	HyperVertexKind kind;
	const lib::graph::Graph *graph; // only defined for kind == Vertex
	NonHyperEdge edge; // only defined for kind == Edge
};

// Expanded

enum class ExpandedVertexKind {
	Edge, // same as in Hyper
	IOEdge, // a half-edge implementing IO (Extended DG)
	TransitEdge, // a one-to-one edge from an InVertex to an OutVertex (Expanded DG)
	InVertex, // in-edges are from Edge, out-edge are to TransitEdge
	OutVertex // in-edge are from TransitEdge, out-edges are to Edge
	// That is:
	// nothing -> IOEdge -> InVertex
	// OutVertex -> IOEdge -> nothing
	// InVertex -> Transit -> OutVertex
	// OutVertex -> Edge -> InVertex
};

inline bool isKindVertex(ExpandedVertexKind k) {
	switch(k) {
	case ExpandedVertexKind::Edge:
	case ExpandedVertexKind::IOEdge:
	case ExpandedVertexKind::TransitEdge: return false;
	case ExpandedVertexKind::InVertex:
	case ExpandedVertexKind::OutVertex: return true;
	}
	__builtin_unreachable();
}

struct ExpandedVProp;
using ExpandedGraphType = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, ExpandedVProp>;
using ExpandedVertex = boost::graph_traits<ExpandedGraphType>::vertex_descriptor;
using ExpandedEdge = boost::graph_traits<ExpandedGraphType>::edge_descriptor;

struct ExpandedVProp {
	ExpandedVProp() = default; // because Boost.Graph requires default constructability when resizing vectors

	ExpandedVProp(ExpandedVertexKind kind, HyperVertex vHyper, HyperVertex eRepr)
			: kind(kind), vHyper(vHyper), eRepr(eRepr) {}

	ExpandedVertexKind kind;
	// for all vertices with kind == Edge, the set of vHyper is an injection on the hyper edges in Hyper
	// kind != Edge => vHyper is the vertex in Hyper that it belongs to
	HyperVertex vHyper;
	// if kind is either InVertex or OutVertex, this is the in/out edge it represents
	// for the input/output edges, this means null_vertex
	// for the catch-all, it's also null_vertex (use the vertexData to detect which it is)
	// if kind is anything else, it's also null_vertex
	HyperVertex eRepr;
};

} // namespace mod::lib::DG

#endif // MOD_LIB_DG_GRAPHDECL_HPP