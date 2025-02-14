#include "Graph.hpp"

#include <mod/graph/Graph.hpp>
#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/Graph/IO/Write.hpp>
#include <mod/lib/Graph/Properties/Molecule.hpp>
#include <mod/lib/Graph/Properties/Stereo.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/Graph/Properties/Term.hpp>

namespace mod::graph::internal {

// Graph
// ----------------------------------------------------------------------------

const lib::graph::GraphType &getGraph(const lib::graph::Graph &g) {
	return g.getGraph();
}

const lib::graph::LabelledGraph &getLabelledGraph(const lib::graph::Graph &g) {
	return g.getLabelledGraph();
}

std::unique_ptr<lib::graph::PropString> makePropString(const lib::graph::GraphType &g) {
	return std::make_unique<lib::graph::PropString>(g);
}

void add(lib::graph::PropString &pString, boost::graph_traits<lib::graph::GraphType>::vertex_descriptor v,
         const std::string &value) {
	pString.addVertex(v, value);
}

void add(lib::graph::PropString &pString, boost::graph_traits<lib::graph::GraphType>::edge_descriptor e,
         const std::string &value) {
	pString.addEdge(e, value);
}

std::shared_ptr<graph::Graph> makeGraph(
		std::unique_ptr<lib::graph::GraphType> g,
		std::unique_ptr<lib::graph::PropString> pString,
		std::unique_ptr<lib::graph::PropStereo> pStereo) {
	auto gLib = std::make_unique<lib::graph::Graph>(std::move(g), std::move(pString), std::move(pStereo));
	return graph::Graph::create(std::move(gLib));
}

std::string writePDF(const lib::graph::Graph &g, const mod::lib::IO::Graph::Write::Options &options) {
	return lib::graph::Write::pdf(g, options);
}

// LabelledGraph
// ----------------------------------------------------------------------------

const lib::graph::GraphType &getGraph(const lib::graph::LabelledGraph &g) {
	return get_graph(g);
}

const std::string &getString(
		boost::graph_traits<lib::graph::LabelledGraph::GraphType>::vertex_descriptor v,
		const lib::graph::LabelledGraph &g) {
	return get_string(g)[v];
}

const std::string &getString(
		boost::graph_traits<lib::graph::LabelledGraph::GraphType>::edge_descriptor e,
		const lib::graph::LabelledGraph &g) {
	return get_string(g)[e];
}

// LabelledUnionGraph<LabelledGraph>
// ----------------------------------------------------------------------------

void push_back(lib::LabelledUnionGraph<lib::graph::LabelledGraph> &ug, const lib::graph::LabelledGraph *g) {
	ug.push_back(g);
}

const lib::LabelledUnionGraph<lib::graph::LabelledGraph>::GraphType &
getGraph(const lib::LabelledUnionGraph<lib::graph::LabelledGraph> &g) {
	return get_graph(g);
}

const std::string &getString(
		boost::graph_traits<lib::LabelledUnionGraph<lib::graph::LabelledGraph>::GraphType>::vertex_descriptor v,
		const lib::LabelledUnionGraph<lib::graph::LabelledGraph> &g) {
	return get_string(g)[v];
}

const std::string &getString(
		boost::graph_traits<lib::LabelledUnionGraph<lib::graph::LabelledGraph>::GraphType>::edge_descriptor e,
		const lib::LabelledUnionGraph<lib::graph::LabelledGraph> &g) {
	return get_string(g)[e];
}

const AtomData &getMolecule(
		boost::graph_traits<lib::LabelledUnionGraph<lib::graph::LabelledGraph>::GraphType>::vertex_descriptor v,
		const lib::LabelledUnionGraph<lib::graph::LabelledGraph> &g) {
	return get_molecule(g)[v];
}

BondType getMolecule(
		boost::graph_traits<lib::LabelledUnionGraph<lib::graph::LabelledGraph>::GraphType>::edge_descriptor e,
		const lib::LabelledUnionGraph<lib::graph::LabelledGraph> &g) {
	return get_molecule(g)[e];
}

} // namespace mod::graph::internal