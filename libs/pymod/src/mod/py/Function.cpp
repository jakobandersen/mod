#include <mod/py/Function.hpp>

#include <mod/Derivation.hpp>
#include <mod/VertexMap.hpp>
#include <mod/graph/Graph.hpp>
#include <mod/graph/GraphInterface.hpp>
#include <mod/dg/DG.hpp>
#include <mod/dg/GraphInterface.hpp>
#include <mod/dg/Strategies.hpp>

namespace mod::Py {

void Function_doExport() {
	// () -> X
	exportFunc<std::vector<std::shared_ptr<mod::graph::Graph> >()>("_Func_VecGraph");
	exportFunc<std::string()>("_Func_String");
	// Derivation -> X
	exportFunc<bool(const Derivation &)>("_Func_BoolDerivation");
	exportFunc<std::string(const Derivation &)>("_Func_StringDerivation");
	// DG -> X
	exportFunc<std::string(std::shared_ptr<dg::DG>)>("_Func_StringDG");
	// DG::Vertex -> X
	exportFunc<void(dg::DG::Vertex)>("_Func_VoidDGVertex");
	exportFunc<bool(dg::DG::Vertex)>("_Func_BoolDGVertex");
	exportFunc<std::string(dg::DG::Vertex)>("_Func_StringDGVertex");
	exportFunc<std::pair<double, bool>(dg::DG::Vertex)>("_Func_PairDoubleBoolDGVertex");
	// DG::Vertex x int -> X
	exportFunc<std::pair<std::string, std::string>(dg::DG::Vertex, int)>(
			"_Func_PairStringStringDGVertexInt");
	// DG::HyperEdge -> X
	exportFunc<void(dg::DG::HyperEdge)>("_Func_VoidDGHyperEdge");
	exportFunc<bool(dg::DG::HyperEdge)>("_Func_BoolDGHyperEdge");
	exportFunc<std::string(dg::DG::HyperEdge)>("_Func_StringDGHyperEdge");
	exportFunc<std::pair<double, bool>(dg::DG::HyperEdge)>("_Func_PairDoubleBoolDGHyperEdge");
	// Graph -> X
	exportFunc<bool(std::shared_ptr<mod::graph::Graph>)>("_Func_BoolGraph");
	exportFunc<int(std::shared_ptr<mod::graph::Graph>)>("_Func_IntGraph");
	exportFunc<std::string(std::shared_ptr<mod::graph::Graph>)>("_Func_StringGraph");
	// Graph x Strategy::GraphState -> X
	exportFunc<bool(std::shared_ptr<mod::graph::Graph>, const dg::Strategy::GraphState &)>(
			"_Func_BoolGraphDGStratGraphState");
	// Graph x Strategy::GraphState x bool -> X
	exportFunc<bool(std::shared_ptr<mod::graph::Graph>, const dg::Strategy::GraphState &, bool)>(
			"_Func_BoolGraphDGStratGraphStateBool");
	// Graph x Graph x Strategy::GraphState -> X
	exportFunc<bool(std::shared_ptr<mod::graph::Graph>, std::shared_ptr<mod::graph::Graph>, const dg::Strategy::GraphState &)>(
			"_Func_BoolGraphGraphDGStratGraphState");
	// Strategy::GraphState -> X
	exportFunc<void(const dg::Strategy::GraphState &)>("_Func_VoidDGStratGraphState");

	exportFunc<bool(VertexMap<graph::Graph, graph::Graph>)>("_Func_BoolVertexMapGraphGraph");
}

} // namespace mod::Py
