#ifndef MOD_LIB_DG_GRAPHDECL_HPP
#define MOD_LIB_DG_GRAPHDECL_HPP

#include <mod/lib/Graph/Multiset.hpp>

#include <boost/graph/adjacency_list.hpp>

#include <optional>

namespace mod::lib::Graph {
struct Single;
} // namespace mod::lib::Graph
namespace mod::lib::Rules {
struct Real;
} // namespace mod::lib::Rules
namespace mod::lib::DG {
// NonHyper
struct NonHyperVProp;
struct NonHyperEProp;
using NonHyperGraphType = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, NonHyperVProp, NonHyperEProp>;
using NonHyperVertex = boost::graph_traits<NonHyperGraphType>::vertex_descriptor;
using NonHyperEdge = boost::graph_traits<NonHyperGraphType>::edge_descriptor;
using GraphMultiset = lib::Graph::Multiset<const lib::Graph::Single>;

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
	std::vector<const lib::Rules::Real *> rules;
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
	const lib::Graph::Single *graph; // only defined for kind == Vertex
	NonHyperEdge edge; // only defined for kind == Edge
};

} // namespace mod::lib::DG

#endif // MOD_LIB_DG_GRAPHDECL_HPP