#include "Union.hpp"

#include <mod/Chem.hpp>
#include <mod/Error.hpp>
#include <mod/graph/Graph.hpp>
#include <mod/graph/GraphInterface.hpp>
#include <mod/graph/Printer.hpp>
#include <mod/lib/GraphPimpl.hpp>
#include <mod/lib/LabelledUnionGraph.hpp>
#include <mod/lib/Graph/LabelledGraph.hpp>
#include <mod/lib/Graph/Single.hpp>
#include <mod/lib/Graph/IO/Write.hpp>
#include <mod/lib/Graph/Properties/Molecule.hpp>
#include <mod/lib/Graph/Properties/Stereo.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/Graph/Properties/Term.hpp>

#include <boost/concept/assert.hpp>
#include <boost/lexical_cast.hpp>

namespace mod::graph {

struct Union::Pimpl {
	friend std::ostream &operator<<(std::ostream &s, const Pimpl &p) {
		s << "UnionGraph{";
		bool first = true;
		for(const auto &g: p.graphs) {
			if(first) first = false;
			else s << ", ";
			s << g->getName();
		}
		return s << "}";
	}

public:
	std::vector<std::shared_ptr<Graph>> graphs;
	lib::LabelledUnionGraph<lib::Graph::LabelledGraph> g;
};

Union::Union() : p(std::make_unique<Pimpl>()) {}

Union::Union(std::vector<std::shared_ptr<Graph>> graphs) {
	for(const auto &g: graphs)
		if(!g) throw LogicError("A graph is null.");

	auto p = std::unique_ptr<Pimpl>(new Pimpl{std::move(graphs), {}});
	for(const auto &gOther: p->graphs)
		p->g.push_back(&gOther->getGraph().getLabelledGraph());
	this->p = std::move(p);
}

Union::Union(std::shared_ptr<const Pimpl> p) : p(p) {}

Union::Union(const Union &other) = default;
Union &Union::operator=(const Union &other) = default;
Union::Union(Union &&other) = default;
Union &Union::operator=(Union &&other) = default;

const lib::LabelledUnionGraph<lib::Graph::LabelledGraph> &Union::getInner() const {
	return p->g;
}

bool operator==(const Union &a, const Union &b) {
	return a.p->graphs == b.p->graphs;
}

bool operator!=(const Union &a, const Union &b) {
	return a.p->graphs != b.p->graphs;
}

bool operator<(const Union &a, const Union &b) {
	return a.p->graphs < b.p->graphs;
}

std::ostream &operator<<(std::ostream &s, const Union &ug) {
	return s << *ug.p;
}

std::size_t Union::size() const {
	return get_graph(p->g).size();
}

Union::const_iterator Union::begin() const {
	return p->graphs.begin();
}

Union::const_iterator Union::end() const {
	return p->graphs.end();
}

std::shared_ptr<Graph> Union::operator[](std::size_t i) const {
	if(i >= size()) throw LogicError("Index is out of range.");
	return begin()[i];
}

std::size_t Union::numVertices() const {
	return num_vertices(get_graph(p->g));
}

Union::VertexRange Union::vertices() const {
	return VertexRange(p);
}

std::size_t Union::numEdges() const {
	return num_edges(get_graph(p->g));
}

Union::EdgeRange Union::edges() const {
	return EdgeRange(p);
}

void Union::printTermState() const {
	lib::Graph::Write::termState(p->g, boost::lexical_cast<std::string>(*this));
}

//------------------------------------------------------------------------------
// Vertex
//------------------------------------------------------------------------------

MOD_GRAPHPIMPL_Define_Vertex(Union, UnionGraph, get_graph(this->g->p->g), g, /* VertexPrint */)

MOD_GRAPHPIMPL_Define_Vertex_Undirected(Union, get_graph(this->g->p->g), g)

const std::string &Union::Vertex::getStringLabel() const {
	if(!*this) throw LogicError("Can not get string label on a null vertex.");
	const auto &graph = get_graph(g->p->g);
	using boost::vertices;
	const auto v = vertices(graph).first[vId];
	return get_string(g->p->g)[v];
}

AtomId Union::Vertex::getAtomId() const {
	if(!*this) throw LogicError("Can not get atom id on a null vertex.");
	const auto &graph = get_graph(g->p->g);
	using boost::vertices;
	const auto v = vertices(graph).first[vId];
	return get_molecule(g->p->g)[v].getAtomId();
}

Isotope Union::Vertex::getIsotope() const {
	if(!*this) throw LogicError("Can not get isotope on a null vertex.");
	const auto &graph = get_graph(g->p->g);
	using boost::vertices;
	const auto v = vertices(graph).first[vId];
	return get_molecule(g->p->g)[v].getIsotope();
}

Charge Union::Vertex::getCharge() const {
	if(!*this) throw LogicError("Can not get charge on a null vertex.");
	const auto &graph = get_graph(g->p->g);
	using boost::vertices;
	const auto v = vertices(graph).first[vId];
	return get_molecule(g->p->g)[v].getCharge();
}

bool Union::Vertex::getRadical() const {
	if(!*this) throw LogicError("Can not get radical status on a null vertex.");
	const auto &graph = get_graph(g->p->g);
	using boost::vertices;
	const auto v = vertices(graph).first[vId];
	return get_molecule(g->p->g)[v].getRadical();
}

//------------------------------------------------------------------------------

std::string Union::Vertex::printStereo() const {
	Printer p;
	p.setMolDefault();
	p.setWithIndex(true);
	p.setCollapseHydrogens(false);
	p.setSimpleCarbons(false);
	return printStereo(p);
}

std::string Union::Vertex::printStereo(const Printer &p) const {
	if(!*this) throw LogicError("Can not print stereo on a null vertex.");
	const auto &gLabelled = g->p->g;
	const auto &graph = get_graph(gLabelled);
	using boost::vertices;
	// note, the Boost.Iterator library implements [] through returning proxy objects
	const auto v = *std::next(vertices(graph).first, vId);
	const auto &conf = get_stereo(gLabelled)[v];
	const auto vInner = v.v;
	const auto gIdx = v.gIdx;
	const auto &gInner = g->p->graphs[gIdx]->getGraph();
	return lib::Graph::Write::stereoSummary(
			gInner, vInner, *conf, p.getOptions(), graph.get_vertex_idx_offset(gIdx),
			" (" + boost::lexical_cast<std::string>(*this)) + ")";
}

int Union::Vertex::getGraphIndex() const {
	if(!*this) throw LogicError("Can not get graph index of a null vertex.");
	const auto &graph = get_graph(g->p->g);
	using boost::vertices;
	// note, the Boost.Iterator library implements [] through returning proxy objects
	const auto v = *std::next(vertices(graph).first, vId);
	return v.gIdx;
}

Graph::Vertex Union::Vertex::getVertex() const {
	if(!*this) throw LogicError("Can not get graph vertex of a null vertex.");
	const auto &graph = get_graph(g->p->g);
	using boost::vertices;
	// note, the Boost.Iterator library implements [] through returning proxy objects
	const auto v = *std::next(vertices(graph).first, vId);
	const auto gExtInner = g->p->graphs[v.gIdx];
	const auto &gInner = gExtInner->getGraph().getGraph();
	return Graph::Vertex(gExtInner, get(boost::vertex_index_t(), gInner, v.v));
}


//------------------------------------------------------------------------------
// Edge
//------------------------------------------------------------------------------

MOD_GRAPHPIMPL_Define_Indices(Union, UnionGraph, get_graph(this->g->p->g), g, /* EdgePrint */)

BOOST_CONCEPT_ASSERT((boost::ForwardIterator<Union::VertexIterator>));
BOOST_CONCEPT_ASSERT((boost::ForwardIterator<Union::EdgeIterator>));
BOOST_CONCEPT_ASSERT((boost::ForwardIterator<Union::IncidentEdgeIterator>));

const std::string &Union::Edge::getStringLabel() const {
	if(!*this) throw LogicError("Can not get string label on a null edge.");
	const auto &gLabelled = g->p->g;
	const auto &graph = get_graph(gLabelled);
	using boost::vertices;
	const auto v = vertices(graph).first[vId];
	const auto e = out_edges(v, graph).first[eId];
	return get_string(g->p->g)[e];
}

BondType Union::Edge::getBondType() const {
	if(!*this) throw LogicError("Can not get bond type on a null edge.");
	const auto &gLabelled = g->p->g;
	const auto &graph = get_graph(gLabelled);
	using boost::vertices;
	const auto v = vertices(graph).first[vId];
	const auto e = out_edges(v, graph).first[eId];
	return get_molecule(g->p->g)[e];
}

int Union::Edge::getGraphIndex() const {
	if(!*this) throw LogicError("Can not get graph index of a null edge.");
	const auto &graph = get_graph(g->p->g);
	using boost::vertices;
	// note, the Boost.Iterator library implements [] through returning proxy objects
	const auto v = *std::next(vertices(graph).first, vId);
	return v.gIdx;
}

Graph::Edge Union::Edge::getEdge() const {
	if(!*this) throw LogicError("Can not get graph vertex of a null edge.");
	const auto &graph = get_graph(g->p->g);
	using boost::vertices;
	// note, the Boost.Iterator library implements [] through returning proxy objects
	const auto v = *std::next(vertices(graph).first, vId);
	const auto gExtInner = g->p->graphs[v.gIdx];
	const auto &gInner = gExtInner->getGraph().getGraph();
	return Graph::Edge(gExtInner, get(boost::vertex_index_t(), gInner, v.v), eId);
}

} // namespace mod::graph