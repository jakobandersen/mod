#ifndef MOD_GRAPH_INTERNAL_GRAPH_HPP
#define MOD_GRAPH_INTERNAL_GRAPH_HPP

#include <mod/BuildConfig.hpp>
#include <mod/graph/internal/ForwardDecl.hpp>
#include <mod/lib/LabelledUnionGraph.hpp>
#include <mod/lib/Graph/LabelledGraph.hpp>
#include <mod/lib/Graph/Properties/Term.hpp>

namespace mod::graph::internal {

// Graph
MOD_DECL const lib::graph::GraphType &getGraph(const lib::graph::Graph &g);
MOD_DECL const lib::graph::LabelledGraph &getLabelledGraph(const lib::graph::Graph &g);
MOD_DECL std::unique_ptr<lib::graph::PropString> makePropString(const lib::graph::GraphType &g);
MOD_DECL void add(lib::graph::PropString &pString, boost::graph_traits<lib::graph::GraphType>::vertex_descriptor v,
                  const std::string &value);
MOD_DECL void add(lib::graph::PropString &pString, boost::graph_traits<lib::graph::GraphType>::edge_descriptor e,
                  const std::string &value);
MOD_DECL std::shared_ptr<graph::Graph> makeGraph(
		std::unique_ptr<lib::graph::GraphType> g,
		std::unique_ptr<lib::graph::PropString> pString,
		std::unique_ptr<lib::graph::PropStereo> pStereo);

MOD_DECL std::string writePDF(const lib::graph::Graph &g, const mod::lib::IO::Graph::Write::Options &options);

// LabelledGraph
MOD_DECL const lib::graph::GraphType &getGraph(const lib::graph::LabelledGraph &g);
MOD_DECL const std::string &getString(
		boost::graph_traits<lib::graph::LabelledGraph::GraphType>::vertex_descriptor v,
		const lib::graph::LabelledGraph &g);
MOD_DECL const std::string &getString(
		boost::graph_traits<lib::graph::LabelledGraph::GraphType>::edge_descriptor e,
		const lib::graph::LabelledGraph &g);

// LabelledUnionGraph<LabelledGraph>
MOD_DECL void push_back(lib::LabelledUnionGraph<lib::graph::LabelledGraph> &ug, const lib::graph::LabelledGraph *g);

MOD_DECL const lib::LabelledUnionGraph<lib::graph::LabelledGraph>::GraphType &
getGraph(const lib::LabelledUnionGraph<lib::graph::LabelledGraph> &g);

MOD_DECL const std::string &getString(
		boost::graph_traits<lib::LabelledUnionGraph<lib::graph::LabelledGraph>::GraphType>::vertex_descriptor v,
		const lib::LabelledUnionGraph<lib::graph::LabelledGraph> &g);
MOD_DECL const std::string &getString(
		boost::graph_traits<lib::LabelledUnionGraph<lib::graph::LabelledGraph>::GraphType>::edge_descriptor e,
		const lib::LabelledUnionGraph<lib::graph::LabelledGraph> &g);

MOD_DECL const AtomData &getMolecule(
		boost::graph_traits<lib::LabelledUnionGraph<lib::graph::LabelledGraph>::GraphType>::vertex_descriptor v,
		const lib::LabelledUnionGraph<lib::graph::LabelledGraph> &g);
MOD_DECL BondType getMolecule(
		boost::graph_traits<lib::LabelledUnionGraph<lib::graph::LabelledGraph>::GraphType>::edge_descriptor e,
		const lib::LabelledUnionGraph<lib::graph::LabelledGraph> &g);

} // namespace mod::graph::internal

#endif // MOD_GRAPH_INTERNAL_GRAPH_HPP