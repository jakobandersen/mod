#include "GraphInterface.hpp"

#include <mod/graph/Printer.hpp>
#include <mod/lib/GraphPimpl.hpp>
#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/Graph/IO/Write.hpp>
#include <mod/lib/Graph/Properties/Molecule.hpp>
#include <mod/lib/Graph/Properties/Stereo.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/Graph/Properties/Term.hpp>

namespace mod::graph {

//------------------------------------------------------------------------------
// Vertex
//------------------------------------------------------------------------------

MOD_GRAPHPIMPL_Define_Vertex(Graph, Graph, g->getGraph().getGraph(), g, /* VertexPrint */)
MOD_GRAPHPIMPL_Define_Vertex_Undirected(Graph, g->getGraph().getGraph(), g)

const std::string &Graph::Vertex::getStringLabel() const {
	if(!g) throw LogicError("Can not get string label on a null vertex.");
	const auto &graph = g->getGraph().getGraph();
	using boost::vertices;
	auto v = *(vertices(graph).first + vId);
	return g->getGraph().getStringState()[v];
}

AtomId Graph::Vertex::getAtomId() const {
	if(!g) throw LogicError("Can not get atom id on a null vertex.");
	const auto &graph = g->getGraph().getGraph();
	using boost::vertices;
	auto v = *(vertices(graph).first + vId);
	return g->getGraph().getMoleculeState()[v].getAtomId();
}

Isotope Graph::Vertex::getIsotope() const {
	if(!g) throw LogicError("Can not get isotope on a null vertex.");
	const auto &graph = g->getGraph().getGraph();
	using boost::vertices;
	auto v = *(vertices(graph).first + vId);
	return g->getGraph().getMoleculeState()[v].getIsotope();
}

Charge Graph::Vertex::getCharge() const {
	if(!g) throw LogicError("Can not get charge on a null vertex.");
	const auto &graph = g->getGraph().getGraph();
	using boost::vertices;
	auto v = *(vertices(graph).first + vId);
	return g->getGraph().getMoleculeState()[v].getCharge();
}

bool Graph::Vertex::getRadical() const {
	if(!g) throw LogicError("Can not get radical status on a null vertex.");
	const auto &graph = g->getGraph().getGraph();
	using boost::vertices;
	auto v = *(vertices(graph).first + vId);
	return g->getGraph().getMoleculeState()[v].getRadical();
}

//------------------------------------------------------------------------------

std::string Graph::Vertex::printStereo() const {
	Printer p;
	p.setMolDefault();
	p.setWithIndex(true);
	p.setCollapseHydrogens(false);
	p.setSimpleCarbons(false);
	return printStereo(p);
}

std::string Graph::Vertex::printStereo(const Printer &p) const {
	if(!g) throw LogicError("Can not print stereo on a null vertex.");
	const auto &gLabelled = g->getGraph().getLabelledGraph();
	const auto &graph = get_graph(gLabelled);
	using boost::vertices;
	auto v = *(vertices(graph).first + vId);
	const auto &conf = get_stereo(gLabelled)[v];
	return lib::graph::Write::stereoSummary(g->getGraph(), v, *conf, p.getOptions(), 0, "");
}

//------------------------------------------------------------------------------
// Edge
//------------------------------------------------------------------------------

MOD_GRAPHPIMPL_Define_Indices(Graph, Graph, g->getGraph().getGraph(), g, /* EdgePrint */)

const std::string &Graph::Edge::getStringLabel() const {
	if(!g) throw LogicError("Can not get string label on a null edge.");
	const auto &graph = g->getGraph().getGraph();
	using boost::vertices;
	auto v = *(vertices(graph).first + vId);
	auto e = *(out_edges(v, graph).first + eId);
	return g->getGraph().getStringState()[e];
}

BondType Graph::Edge::getBondType() const {
	if(!g) throw LogicError("Can not get bond type on a null edge.");
	const auto &graph = g->getGraph().getGraph();
	using boost::vertices;
	auto v = *(vertices(graph).first + vId);
	auto e = *(out_edges(v, graph).first + eId);
	return g->getGraph().getMoleculeState()[e];
}

} // namespace mod::graph