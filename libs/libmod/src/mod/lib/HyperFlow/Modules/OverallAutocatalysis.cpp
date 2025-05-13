#include "OverallAutocatalysis.hpp"

#include <mod/Error.hpp>
#include <mod/Function.hpp>
#include <mod/lib/CombiOpt/BoolExpr.hpp>
#include <mod/lib/CombiOpt/LinConstraintInt.hpp>
#include <mod/lib/CombiOpt/LoadedSolution.hpp>
#include <mod/lib/CombiOpt/Model.hpp>
#include <mod/lib/CombiOpt/Solver.hpp>
#include <mod/lib/DG/Hyper.hpp>
#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/HyperFlow/Model.hpp>
#include <mod/lib/HyperFlow/ModuleRegistry.hpp>
#include <mod/lib/HyperFlow/Modules/Base.hpp>
#include <mod/lib/HyperFlow/Modules/Utils.hpp>
#include <mod/lib/HyperFlow/Specification.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/IO/Json.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <boost/lexical_cast.hpp>

#include <iostream>
#include <queue>

namespace mod::lib::HyperFlow {

using HyperVertex = lib::DG::HyperVertex;

OverallAutocatalysisSpecification::OverallAutocatalysisSpecification(Specification &owner, bool setDefaults)
		: SpecificationModule(owner) {
	if(setDefaults) {
		auto &base = owner.getModule<BaseSpecification>();
		base.setAllowReversal(false);
		base.setAllowIOReversal(false);
	}
}

std::string OverallAutocatalysisSpecification::getName() const {
	return "OverallAutocatalysis";
}

void OverallAutocatalysisSpecification::addDefaultObjective(hyperflow::LinExp &exp) const {
	exp += hyperflow::vars::isOverallAutocata;
}

void OverallAutocatalysisSpecification::setForceExistence(bool value) {
	forceExistence = value;
}

bool OverallAutocatalysisSpecification::getForceExistence() const {
	return forceExistence;
}

void OverallAutocatalysisSpecification::setStrictTransit(bool value) {
	strictTransit = value;
}

bool OverallAutocatalysisSpecification::getStrictTransit() const {
	return strictTransit;
}

void OverallAutocatalysisSpecification::setBFSExclusive(bool value) {
	bfsExclusive = value;
}

bool OverallAutocatalysisSpecification::getBFSExclusive() const {
	return bfsExclusive;
}

void OverallAutocatalysisSpecification::listImpl(std::ostream &s) const {
	s << "ForceExistence: " << std::boolalpha << forceExistence << "\n";
	s << "StrictTransit: " << std::boolalpha << strictTransit << "\n";
	s << "BFSExclusive: " << std::boolalpha << bfsExclusive << "\n";
}

nlohmann::json OverallAutocatalysisSpecification::dumpImpl() const {
	nlohmann::json j;
	j["version"] = 1;
	j["forceExistence"] = forceExistence;
	j["strictTransit"] = strictTransit;
	j["bfsExclusive"] = bfsExclusive;
	return j;
}

bool OverallAutocatalysisSpecification::loadImpl(const nlohmann::json &j, std::ostream &err) {
	static const auto schema = R"({
		"$schema": "http://json-schema.org/draft-07/schema#",
		"type": "object",
		"properties": {
			"version": {"type": "integer", "minimum": 1, "maximum": 1},
			"forceExistence": {"type": "boolean"},
			"strictTransit": {"type": "boolean"},
			"bfsExclusive": {"type": "boolean"}
		},
		"required": ["version"]
	})"_json;
	static const nlohmann::json_schema::json_validator validator(schema);
	if(!IO::validateJson(j, validator, err,
	                     "OverallAutocatalysis module specification data does not conform to schema:"))
		return false;
	forceExistence = j["forceExistence"];
	strictTransit = j["strictTransit"];
	bfsExclusive = j["bfsExclusive"];
	return true;
}

Transits OverallAutocatalysisSpecification::getTransits() const {
	return Transits();
}

std::unique_ptr<ModelModule> OverallAutocatalysisSpecification::createModel(Model &owner) const {
	return std::make_unique<OverallAutocatalysisModel>(owner);
}

//------------------------------------------------------------------------------
// Model
//------------------------------------------------------------------------------

struct OverallAutocatalysisModel::StaticInit {
	StaticInit() {
		auto &r = ModuleRegistry::get();
		std::type_index id = typeid(OverallAutocatalysisSpecification);
		r.addModule(id, "OverallAutocatalysis", [](Specification &spec, bool setDefaults) {
			spec.addModule<OverallAutocatalysisSpecification>(setDefaults);
		});
		r.addVariableSet("isOverallAutocata", makeVertexVariableSet(
				id, [](const Model &m) {
					if(m.specification->getModule<BaseSpecification>().getRelaxed())
						MOD_ABORT;
					const auto &mm = m.getModule<OverallAutocatalysisModel>();
					CombiOpt::LinExpAny res;
					for(const auto &p : mm.isOverallAutocatalytic)
						res += p.second;
					return res;
				}, [](const Model &m, const lib::DG::HyperVertex &v) {
					if(m.specification->getModule<BaseSpecification>().getRelaxed())
						MOD_ABORT;
					const auto &mm = m.getModule<OverallAutocatalysisModel>();
					const auto iter = mm.isOverallAutocatalytic.find(v);
					assert(iter != end(mm.isOverallAutocatalytic));
					CombiOpt::LinExpAny res;
					res += iter->second;
					return res;
				}));
	}
};

namespace {
OverallAutocatalysisModel::StaticInit staticInit;
} // namespace

OverallAutocatalysisModel::OverallAutocatalysisModel(Model &owner) : ModelModule(owner) {
	const bool relaxed = owner.specification->getModule<BaseSpecification>().getRelaxed();
	if(relaxed)
		throw LogicError("Can not create model. OverallAutocatalysis can not be enabled in relaxed mode.");
}

const OverallAutocatalysisSpecification &OverallAutocatalysisModel::getSpec() const {
	return owner.specification->getModule<OverallAutocatalysisSpecification>();
}

void OverallAutocatalysisModel::createVariablesImpl(CombiOpt::Model &model) {
	const auto &dgHyper = owner.specification->dgHyper.getGraph();
	// indicator vars
	for(const lib::DG::HyperVertex vHyper : asRange(vertices(dgHyper))) {
		if(dgHyper[vHyper].kind != lib::DG::HyperVertexKind::Vertex) continue;
		const auto *g = dgHyper[vHyper].graph;
		assert(g);
		std::string name = "isOverallAutocata(";
		name += g->getName();
		name += ")";
		auto var = model.addBoolVar(name);
		isOverallAutocatalytic.emplace(vHyper, var);
	}
}

namespace {

bool isReachable(const std::set<lib::DG::HyperVertex> &sources,
                 lib::DG::HyperVertex target,
                 const std::vector<bool> &isExcluded,
                 const lib::DG::Hyper &dgHyper) {
	const bool verbose = false;
	const auto &dg = dgHyper.getGraph();
	using Vertex = lib::DG::HyperVertex;
	std::vector<bool> available(num_vertices(dg), false);
	std::queue<Vertex> todo;
	// start with the sources (except the target)
	for(Vertex v : sources) {
		if(v == target) continue; // the target should not be immediately reachable
		todo.push(v);
	}
	if(verbose) {
		std::cout << "isReachable:" << std::endl;
		std::cout << "\tsources:";
		for(Vertex v : sources) std::cout << " " << dg[v].graph->getName();
		std::cout << std::endl;
		std::cout << "target: " << dg[target].graph->getName() << std::endl;
	}
	while(!todo.empty()) {
		const Vertex v = todo.front();
		todo.pop();
		const unsigned int vId = get(boost::vertex_index_t(), dg, v);
		if(isExcluded[vId]) continue;
		if(available[vId]) continue;
		if(dg[v].kind == lib::DG::HyperVertexKind::Vertex) {
			if(verbose) std::cout << "\tavailable: " << dg[v].graph->getName() << std::endl;
			available[vId] = true;
			for(const Vertex vAdj : asRange(adjacent_vertices(v, dg))) todo.push(vAdj);
		} else { // RuleKind
			auto invAdj = asRange(inv_adjacent_vertices(v, dg));
			const bool allAvailable = std::all_of(begin(invAdj), end(invAdj), [&dg, &available](Vertex vInvAdj) {
				return available[get(boost::vertex_index_t(), dg, vInvAdj)];
			});
			if(allAvailable) {
				available[get(boost::vertex_index_t(), dg, v)] = true;
				for(const Vertex vAdj : asRange(adjacent_vertices(v, dg))) todo.push(vAdj);
			}
		}
	}
	return available[get(boost::vertex_index_t(), dg, target)];
}

} // namespace

void OverallAutocatalysisModel::createConstraintsImpl(CombiOpt::Model &model) {
	assert(!owner.specification->getModule<BaseSpecification>().getRelaxed());
	const auto &dgHyper = owner.specification->dgHyper.getGraph();
	const auto &baseModel = owner.getModule<BaseModel>();

	for(const lib::DG::HyperVertex vHyper : asRange(vertices(dgHyper))) {
		if(dgHyper[vHyper].kind != lib::DG::HyperVertexKind::Vertex) continue;
		const auto iter = isOverallAutocatalytic.find(vHyper);
		assert(iter != end(isOverallAutocatalytic));
		const auto isAutocata = iter->second;
		const auto in = baseModel.getIn(vHyper);
		const auto out = baseModel.getOut(vHyper);
		const auto isInUsed = baseModel.getIsInUsed(vHyper);
		if(getSpec().getStrictTransit())
			addDisableInternalIfTrue(owner, model, isAutocata, vHyper);

		// isAutocata => in > 0
		model.addImplication(isAutocata, in >= 1);

		// isAutocata => in < out
		model.addImplication(isAutocata, in - out <= -1);

		// NOT isAutocata AND isInUsed => in >= out
		model.addImplication(!isAutocata && isInUsed, in - out >= 0);
	}

	if(getSpec().getForceExistence()) {
		CombiOpt::Disjunction c;
		for(const auto &p : isOverallAutocatalytic)
			c = std::move(c) || p.second;
		model.addConstraint(std::move(c));
	}

	if(getSpec().getBFSExclusive()) {
		std::cout << "Eliminating breadth-first reachable products" << std::endl;
		const auto &baseSpec = owner.specification->getModule<BaseSpecification>();
		const auto &sources = baseSpec.getSources();
		const auto &sinks = baseSpec.getSinks();
		const auto &dgHyper = owner.specification->dgHyper;
		const auto &dg = dgHyper.getGraph();
		for(const auto vSink : sinks) {
			if(isReachable(sources, vSink, baseModel.getIsExcluded(), dgHyper)) {
				const auto iter = isOverallAutocatalytic.find(vSink);
				assert(iter != end(isOverallAutocatalytic));
				const auto isAutocata = iter->second;
				model.addConstraint(!isAutocata);
			} else {
				std::cout << "\t" << dg[vSink].graph->getName() << " not eliminated" << std::endl;
			}
		}
	}
}

nlohmann::json OverallAutocatalysisModel::dumpImpl(const CombiOpt::Result &sol) const {
	return {};
}

bool
OverallAutocatalysisModel::loadImpl(const nlohmann::json &j, CombiOpt::LoadedSolution &s, std::ostream &err) const {
	static const nlohmann::json schema = R"({
		"$schema": "http://json-schema.org/draft-07/schema#",
		"type": "null"
	})"_json;
	static const nlohmann::json_schema::json_validator validator(schema);
	if(!IO::validateJson(j, validator, err, "OverallAutocatalysis solution data does not conform to schema:"))
		return false;
	return true;
}

void OverallAutocatalysisModel::loadSolutionSetDependentVarsImpl(CombiOpt::LoadedSolution &s) const {
	const auto &baseSpec = owner.specification->getModule<BaseSpecification>();
	const auto &baseModel = owner.getModule<BaseModel>();
	const auto &dg = owner.specification->dgHyper;
	const auto &dgHyper = dg.getGraph();
	if(baseSpec.getRelaxed()) return;
	bool isSolutionAutocata = false;
	for(const auto v : asRange(vertices(dgHyper))) {
		if(dgHyper[v].kind != DG::HyperVertexKind::Vertex) continue;
		const auto in = s.getVal(baseModel.getIn(v));
		const auto out = s.getVal(baseModel.getOut(v));
		if(0 < in && in < out) {
			const auto varIter = isOverallAutocatalytic.find(v);
			assert(varIter != end(isOverallAutocatalytic));
			const auto var = varIter->second;
			s.integralValues.emplace(var, 1);
			isSolutionAutocata = true;
			if(getSpec().getStrictTransit()) {
//				bool feasible = dumpCheckDisableInternalIfTrue(
//						dgGraph, base.DumpLoading().Transit().getTransitVars(v), sol);
//				if(!feasible) {
//					std::cout
//							<< "Error while loading dumped Flow solution. Overall autocatalysis is enabled, but the solution is infeasible (conflict with 'strictTransit')."
//							<< std::endl;
//					std::exit(1);
//				}
			}
			if(getSpec().getBFSExclusive()) {
//				const auto &baseSpec = base.flowNew.readSpec().getModule<HyperFlow::BaseSpecification>();
//				bool reachable = Flow::Ext::isReachable(
//						baseSpec.getSources(), v,
//						baseSpec.getVertexFilter(), base.dgHyper);
//				if(reachable) {
//					std::cout
//							<< "WARNING while loading dumped Flow solution. Overall autocatalysis is enabled with 'bfsExclusive == true', but vertex '"
//							<< base.dgGraph[v].graph->getName() << "' is reachable while being autocatalytic."
//							<< std::endl;
//				}
			}
		}
	}
	if(getSpec().getForceExistence()) {
		if(!isSolutionAutocata) {
//			std::cout
//					<< "Error while loading dumped Flow solution. Overall autocatalysis is enabled, but the solution is infeasible (conflict with 'forceExistence')."
//					<< std::endl;
//			std::exit(1);
		}
	}
}

bool OverallAutocatalysisModel::hasListEntry(lib::DG::HyperVertex vHyper, const CombiOpt::Result &sol) const {
	const auto iter = isOverallAutocatalytic.find(vHyper);
	assert(iter != end(isOverallAutocatalytic));
	return sol.getValue(iter->second) != 0;
}

std::vector<std::string> OverallAutocatalysisModel::listHeaderEntries() const {
	return {"OA"};
}

std::vector<std::string>
OverallAutocatalysisModel::listEntries(lib::DG::HyperVertex vHyper, const CombiOpt::Result &sol) const {
	const auto iter = isOverallAutocatalytic.find(vHyper);
	assert(iter != end(isOverallAutocatalytic));
	return {boost::lexical_cast<std::string>(sol.getValue(iter->second))};
}

} // namespace mod::lib::HyperFlow