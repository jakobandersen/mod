#include "Graph.hpp"

#include <mod/graph/Graph.hpp>
#include <mod/lib/Graph/Single.hpp>
#include <mod/lib/Graph/IO/Write.hpp>
#include <mod/lib/Graph/Properties/Molecule.hpp>
#include <mod/lib/Graph/Properties/Stereo.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/Graph/Properties/Term.hpp>

namespace mod::graph::internal {

// Graph
// ----------------------------------------------------------------------------

const lib::Graph::GraphType &getGraph(const lib::Graph::Single &g) {
	return g.getGraph();
}

const lib::Graph::LabelledGraph &getLabelledGraph(const lib::Graph::Single &g) {
	return g.getLabelledGraph();
}

std::unique_ptr<lib::Graph::PropString> makePropString(const lib::Graph::GraphType &g) {
	return std::make_unique<lib::Graph::PropString>(g);
}

void add(lib::Graph::PropString &pString, boost::graph_traits<lib::Graph::GraphType>::vertex_descriptor v,
         const std::string &value) {
	pString.addVertex(v, value);
}

void add(lib::Graph::PropString &pString, boost::graph_traits<lib::Graph::GraphType>::edge_descriptor e,
         const std::string &value) {
	pString.addEdge(e, value);
}

std::shared_ptr<graph::Graph> makeGraph(
		std::unique_ptr<lib::Graph::GraphType> g,
		std::unique_ptr<lib::Graph::PropString> pString,
		std::unique_ptr<lib::Graph::PropStereo> pStereo) {
	auto gLib = std::make_unique<lib::Graph::Single>(std::move(g), std::move(pString), std::move(pStereo));
	return graph::Graph::create(std::move(gLib));
}

std::string writePDF(const lib::Graph::Single &g, const mod::lib::IO::Graph::Write::Options &options) {
	return lib::Graph::Write::pdf(g, options);
}

// LabelledGraph
// ----------------------------------------------------------------------------

const lib::Graph::GraphType &getGraph(const lib::Graph::LabelledGraph &g) {
	return get_graph(g);
}

const std::string &getString(
		boost::graph_traits<lib::Graph::LabelledGraph::GraphType>::vertex_descriptor v,
		const lib::Graph::LabelledGraph &g) {
	return get_string(g)[v];
}

const std::string &getString(
		boost::graph_traits<lib::Graph::LabelledGraph::GraphType>::edge_descriptor e,
		const lib::Graph::LabelledGraph &g) {
	return get_string(g)[e];
}

// LabelledUnionGraph<LabelledGraph>
// ----------------------------------------------------------------------------

void push_back(lib::LabelledUnionGraph<lib::Graph::LabelledGraph> &ug, const lib::Graph::LabelledGraph *g) {
	ug.push_back(g);
}

const lib::LabelledUnionGraph<lib::Graph::LabelledGraph>::GraphType &
getGraph(const lib::LabelledUnionGraph<lib::Graph::LabelledGraph> &g) {
	return get_graph(g);
}

const std::string &getString(
		boost::graph_traits<lib::LabelledUnionGraph<lib::Graph::LabelledGraph>::GraphType>::vertex_descriptor v,
		const lib::LabelledUnionGraph<lib::Graph::LabelledGraph> &g) {
	return get_string(g)[v];
}

const std::string &getString(
		boost::graph_traits<lib::LabelledUnionGraph<lib::Graph::LabelledGraph>::GraphType>::edge_descriptor e,
		const lib::LabelledUnionGraph<lib::Graph::LabelledGraph> &g) {
	return get_string(g)[e];
}

const AtomData &getMolecule(
		boost::graph_traits<lib::LabelledUnionGraph<lib::Graph::LabelledGraph>::GraphType>::vertex_descriptor v,
		const lib::LabelledUnionGraph<lib::Graph::LabelledGraph> &g) {
	return get_molecule(g)[v];
}

BondType getMolecule(
		boost::graph_traits<lib::LabelledUnionGraph<lib::Graph::LabelledGraph>::GraphType>::edge_descriptor e,
		const lib::LabelledUnionGraph<lib::Graph::LabelledGraph> &g) {
	return get_molecule(g)[e];
}

} // namespace mod::graph::internal