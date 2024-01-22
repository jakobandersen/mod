#ifndef MOD_GRAPH_INTERNAL_GRAPH_HPP
#define MOD_GRAPH_INTERNAL_GRAPH_HPP

#include <mod/BuildConfig.hpp>
#include <mod/graph/internal/ForwardDecl.hpp>
#include <mod/lib/LabelledUnionGraph.hpp>
#include <mod/lib/Graph/LabelledGraph.hpp>
#include <mod/lib/Graph/Properties/Term.hpp>

namespace mod::graph::internal {

// Graph
MOD_DECL const lib::Graph::GraphType &getGraph(const lib::Graph::Single &g);
MOD_DECL const lib::Graph::LabelledGraph &getLabelledGraph(const lib::Graph::Single &g);
MOD_DECL std::unique_ptr<lib::Graph::PropString> makePropString(const lib::Graph::GraphType &g);
MOD_DECL void add(lib::Graph::PropString &pString, boost::graph_traits<lib::Graph::GraphType>::vertex_descriptor v,
                  const std::string &value);
MOD_DECL void add(lib::Graph::PropString &pString, boost::graph_traits<lib::Graph::GraphType>::edge_descriptor e,
                  const std::string &value);
MOD_DECL std::shared_ptr<graph::Graph> makeGraph(
		std::unique_ptr<lib::Graph::GraphType> g,
		std::unique_ptr<lib::Graph::PropString> pString,
		std::unique_ptr<lib::Graph::PropStereo> pStereo);

MOD_DECL std::string writePDF(const lib::Graph::Single &g, const mod::lib::IO::Graph::Write::Options &options);

// LabelledGraph
MOD_DECL const lib::Graph::GraphType &getGraph(const lib::Graph::LabelledGraph &g);
MOD_DECL const std::string &getString(
		boost::graph_traits<lib::Graph::LabelledGraph::GraphType>::vertex_descriptor v,
		const lib::Graph::LabelledGraph &g);
MOD_DECL const std::string &getString(
		boost::graph_traits<lib::Graph::LabelledGraph::GraphType>::edge_descriptor e,
		const lib::Graph::LabelledGraph &g);

// LabelledUnionGraph<LabelledGraph>
MOD_DECL void push_back(lib::LabelledUnionGraph<lib::Graph::LabelledGraph> &ug, const lib::Graph::LabelledGraph *g);

MOD_DECL const lib::LabelledUnionGraph<lib::Graph::LabelledGraph>::GraphType &
getGraph(const lib::LabelledUnionGraph<lib::Graph::LabelledGraph> &g);

MOD_DECL const std::string &getString(
		boost::graph_traits<lib::LabelledUnionGraph<lib::Graph::LabelledGraph>::GraphType>::vertex_descriptor v,
		const lib::LabelledUnionGraph<lib::Graph::LabelledGraph> &g);
MOD_DECL const std::string &getString(
		boost::graph_traits<lib::LabelledUnionGraph<lib::Graph::LabelledGraph>::GraphType>::edge_descriptor e,
		const lib::LabelledUnionGraph<lib::Graph::LabelledGraph> &g);

MOD_DECL const AtomData &getMolecule(
		boost::graph_traits<lib::LabelledUnionGraph<lib::Graph::LabelledGraph>::GraphType>::vertex_descriptor v,
		const lib::LabelledUnionGraph<lib::Graph::LabelledGraph> &g);
MOD_DECL BondType getMolecule(
		boost::graph_traits<lib::LabelledUnionGraph<lib::Graph::LabelledGraph>::GraphType>::edge_descriptor e,
		const lib::LabelledUnionGraph<lib::Graph::LabelledGraph> &g);

} // namespace mod::graph::internal

#endif // MOD_GRAPH_INTERNAL_GRAPH_HPP