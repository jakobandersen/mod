#include "Utils.hpp"

#include <mod/lib/CombiOpt/Model.hpp>
#include <mod/lib/DG/IO/Read.hpp>
#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/HyperFlow/Model.hpp>
#include <mod/lib/HyperFlow/Modules/Base.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/IO/Json.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <iostream>

namespace mod::lib::HyperFlow {

void addDisableInternalIfTrue(Model &model, CombiOpt::Model &cModel, CombiOpt::BoolVar var, lib::DG::HyperVertex v) {
	constexpr bool DEBUG = false;
	const auto &dgExpanded = model.getExpanded();
	const auto &gExpanded = dgExpanded.getGraph();
	const auto &vertexData = dgExpanded.getVertexData(v);
	const auto vInput = vertexData.inputEdge;
	const auto vOutput = vertexData.outputEdge;
	assert(out_degree(vInput, gExpanded) == 1);
	assert(in_degree(vOutput, gExpanded) == 1);
	const auto vInputTrans = *adjacent_vertices(vInput, gExpanded).first;
	const auto vOutputTrans = *inv_adjacent_vertices(vOutput, gExpanded).first;

	const auto &baseModule = model.getModule<BaseModel>();
	for(const auto vInTrans: vertexData.inVertices) {
		if(vInTrans == vInputTrans) { // skip inFlow edge
			// but only if it is not in the catch-all
			if(!vertexData.hasInCatchAll || vertexData.inVertices.front() != vInTrans)
				continue;
		}
		for(const auto vTrans: asRange(adjacent_vertices(vInTrans, gExpanded))) {
			if(DEBUG)
				std::cout << __func__ << " (new): tVar = " << cModel.getVarName(baseModule.getEdge(vTrans))
				          << std::endl;
			assert(out_degree(vTrans, gExpanded) == 1);
			const auto vOutTrans = *adjacent_vertices(vTrans, gExpanded).first;
			if(vOutTrans == vOutputTrans) { // skip outFlow edge
				// but only if it is not in the catch-all
				if(!vertexData.hasOutCatchAll || vertexData.outVertices.front() != vOutTrans) {
					if(DEBUG) std::cout << "\tskipping" << std::endl;
					continue;
				}
			}
			assert(gExpanded[vTrans].kind == lib::DG::ExpandedVertexKind::TransitEdge);
			const auto tVar = baseModule.getEdge(vTrans);
			// boolVar = 1	=>	tVar = 0
			// but tVar >= 0 so let's just do tVar <= 0 as the constraint
			cModel.addImplication(var, tVar <= 0);
		}
	}
}

namespace {

struct VarDumpVisitor {
	VarDumpVisitor(const Specification &spec) : spec(spec) {}

	nlohmann::json operator()(const hyperflow::VarSumVertex &v) const {
		auto j = common(v);
		j["type"] = "VarSumVertex";
		return j;
	}

	nlohmann::json operator()(const hyperflow::VarVertex &v) const {
		auto j = common(v);
		j["type"] = "VarVertex";
		j["vId"] = v.v.getId();
		return j;
	}

	nlohmann::json operator()(const hyperflow::VarVertexGraph &v) const {
		auto j = common(v);
		j["type"] = "VarVertexGraph";
		const auto vertex = spec.dgHyper.getVertexFromGraph(&v.g->getGraph());
		j["vId"] = get(boost::vertex_index_t(), spec.dgHyper.getGraph(), vertex);
		return j;
	}

	nlohmann::json operator()(const hyperflow::VarSumEdge &v) const {
		auto j = common(v);
		j["type"] = "VarSumEdge";
		return j;
	}

	nlohmann::json operator()(const hyperflow::VarEdge &v) const {
		auto j = common(v);
		j["type"] = "VarEdge";
		j["vId"] = v.edge.getId();
		return j;
	}

	nlohmann::json operator()(const hyperflow::VarSumCustom &v) const {
		auto j = common(v);
		j["type"] = "VarSumCustom";
		return j;
	}

	nlohmann::json operator()(const hyperflow::VarCustom &v) const {
		auto j = common(v);
		j["type"] = "VarCustom";
		j["name"] = v.name;
		return j;
	}

private:
	template<typename T>
	nlohmann::json common(const T &v) const {
		nlohmann::json j;
		j["id"] = v.id;
		return j;
	}
public:
	const Specification &spec;
};

} // namespace

nlohmann::json dump(const Specification &spec, const hyperflow::Var &var) {
	return var.applyVisitor(VarDumpVisitor(spec));
}

std::optional<hyperflow::Var>
loadVar(const Specification &spec, const nlohmann::json &j, std::ostream &err) {
	static const nlohmann::json schema = R"({
		"$schema": "http://json-schema.org/draft-07/schema#",
		"type": "object",
		"properties": {
			"type": {"type": "string",
				"enum": ["VarSumVertex", "VarVertex", "VarVertexGraph",
							"VarSumEdge","VarEdge",
							"VarSumCustom", "VarCustom"]},
			"id": {"type": "string"}
		},
		"required": ["type", "id"],
		"allOf": [
			{
				"if": {
					"properties": {"type": {"enum": ["VarSumVertex", "VarSumEdge", "VarSumCustom"]}}
				}, "then": {
				}
			},
			{
				"if": {
					"properties": {"type": {"enum": ["VarVertex", "VarEdge"]}}
				}, "then": {
					"properties": {"vId": {"type": "integer", "minimum": 0}},
					"required": ["vId"]
				}
			},
			{
				"if": {"properties": {"type": {"enum": ["VarCustom"]}}
				}, "then": {
					"properties": {"name": {"type": "string"}},
					"required": ["name"]
				}
			}
		]
	})"_json; // TODO: conditional required vId and name properties
	static const nlohmann::json_schema::json_validator validator(schema);
	if(!IO::validateJson(j, validator, err, "Variable data does not conform to schema:"))
		return {};
	const std::string type = j["type"];
	std::string id = j["id"];

	// handle old variable specifier names
	if(id == "vertex" || id == "transitInternal" || id == "edge")
		id += "Flow";

	const auto &dgHyper = spec.dgHyper;
	const auto &dg = dgHyper.getGraph();
	if(type == "VarSumVertex") return hyperflow::Var(hyperflow::VarSumVertex(std::move(id)));
	if(type == "VarVertex") {
		const auto vOpt = lib::DG::Read::vertex(dg, j["vId"], err, "VarVertex variable.");
		if(!vOpt) return {};
		return hyperflow::Var(hyperflow::VarVertex(std::move(id), dgHyper.getInterfaceVertex(*vOpt)));
	}
	if(type == "VarVertexGraph") {
		const auto vOpt = lib::DG::Read::vertex(dg, j["vId"], err, "VarVertexGraph variable.");
		if(!vOpt) return {};
		return hyperflow::Var(hyperflow::VarVertexGraph(std::move(id), dg[*vOpt].graph->getAPIReference()));
	}
	if(type == "VarSumEdge") return hyperflow::Var(hyperflow::VarSumEdge(std::move(id)));
	if(type == "VarEdge") {
		const auto vOpt = lib::DG::Read::edge(dg, j["vId"], err, "VarEdge variable.");
		if(!vOpt) return {};
		return hyperflow::Var(hyperflow::VarEdge(std::move(id), dgHyper.getInterfaceEdge(*vOpt)));
	}
	if(type == "VarSumCustom") return hyperflow::Var(hyperflow::VarSumCustom(std::move(id)));
	if(type == "VarCustom") return hyperflow::Var(hyperflow::VarCustom(std::move(id), j["name"]));
	__builtin_unreachable();
}

//------------------------------------------------------------------------------

nlohmann::json dump(const Specification &spec, const hyperflow::LinExp &exp) {
	auto j = nlohmann::json::array();
	for(const auto &e: exp.getIntElements()) {
		auto je = nlohmann::json::array();
		je.push_back(e.first);
		je.push_back(dump(spec, e.second));
		j.push_back(std::move(je));
	}
	for(const auto &e: exp.getFloatElements()) {
		auto je = nlohmann::json::array();
		je.push_back(e.first);
		je.push_back(dump(spec, e.second));
		j.push_back(std::move(je));
	}
	return j;
}

std::optional<hyperflow::LinExp>
loadExp(const Specification &spec, const nlohmann::json &j, std::ostream &err) {
	static const nlohmann::json schema = R"({
		"$schema": "http://json-schema.org/draft-07/schema#",
		"type": "array",
		"items": {"type": "array", "items": [{"type": "number"}, {}]}
	})"_json;
	static const nlohmann::json_schema::json_validator validator(schema);
	if(!IO::validateJson(j, validator, err, "Constraint data does not conform to schema:"))
		return {};
	hyperflow::LinExp exp;
	for(const auto &e: j) {
		assert(e.size() == 2);
		const auto &jNum = e[0];
		const auto &jVar = e[1];
		auto optVar = loadVar(spec, jVar, err);
		if(!optVar) return {};
		assert(jNum.is_number_integer() || jNum.is_number_float());
		if(jNum.is_number_integer())
			exp += jNum.get<int>() * *optVar;
		else
			exp += jNum.get<double>() * *optVar;
	}
	return exp;
}

//------------------------------------------------------------------------------

nlohmann::json dump(const Specification &spec, const hyperflow::LinConstraint &c) {
	nlohmann::json j;
	j["bound"] = c.bound;
	switch(c.relation) {
	case hyperflow::LinConstraint::Relation::Leq:
		j["relation"] = "leq";
		break;
	case hyperflow::LinConstraint::Relation::Eq:
		j["relation"] = "eq";
		break;
	case hyperflow::LinConstraint::Relation::Geq:
		j["relation"] = "geq";
		break;
	}
	j["exp"] = dump(spec, c.exp);
	return j;
}

std::optional<hyperflow::LinConstraint>
loadConstraint(const Specification &spec, const nlohmann::json &j, std::ostream &err) {
	static const nlohmann::json schema = R"({
		"$schema": "http://json-schema.org/draft-07/schema#",
		"type": "object",
		"properties": {
			"bound": {"type": "number"},
			"relation": {"type": "string", "enum": ["leq", "eq", "geq"]},
			"exp": {}
		},
		"required": ["bound", "relation", "exp"]
	})"_json;
	static const nlohmann::json_schema::json_validator validator(schema);
	if(!IO::validateJson(j, validator, err, "Constraint data does not conform to schema:"))
		return {};
	double bound = j["bound"];
	hyperflow::LinConstraint::Relation relation;
	const std::string r = j["relation"];
	if(r == "leq") relation = hyperflow::LinConstraint::Relation::Leq;
	else if(r == "eq") relation = hyperflow::LinConstraint::Relation::Eq;
	else if(r == "geq") relation = hyperflow::LinConstraint::Relation::Geq;
	else __builtin_unreachable();
	auto exp = loadExp(spec, j["exp"], err);
	if(!exp) return {};
	return hyperflow::LinConstraint(std::move(*exp), relation, bound);
}

} // namespace mod::lib::HyperFlow