#include "OverallCatalysis.hpp"

#include <mod/Error.hpp>
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
#include <mod/lib/IO/Json.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <boost/lexical_cast.hpp>

namespace mod::lib::HyperFlow {

using HyperVertex = lib::DG::HyperVertex;

OverallCatalysisSpecification::OverallCatalysisSpecification(Specification &owner, bool setDefaults)
		: SpecificationModule(owner) {
	if(setDefaults) {
		auto &base = owner.getModule<BaseSpecification>();
		base.setAllowReversal(false);
		base.setAllowIOReversal(false);
	}
}

std::string OverallCatalysisSpecification::getName() const {
	return "OverallCatalysis";
}

void OverallCatalysisSpecification::addDefaultObjective(hyperflow::LinExp &exp) const {
	exp += hyperflow::vars::isOverallCata;
}

void OverallCatalysisSpecification::setForceExistence(bool value) {
	forceExistence = value;
}

bool OverallCatalysisSpecification::getForceExistence() const {
	return forceExistence;
}

void OverallCatalysisSpecification::setStrictTransit(bool value) {
	strictTransit = value;
}

bool OverallCatalysisSpecification::getStrictTransit() const {
	return strictTransit;
}

void OverallCatalysisSpecification::listImpl(std::ostream &s) const {
	s << "ForceExistence: " << std::boolalpha << forceExistence << "\n";
	s << "StrictTransit: " << std::boolalpha << strictTransit << "\n";
}

nlohmann::json OverallCatalysisSpecification::dumpImpl() const {
	nlohmann::json j;
	j["version"] = 1;
	j["forceExistence"] = forceExistence;
	j["strictTransit"] = strictTransit;
	return j;
}

bool OverallCatalysisSpecification::loadImpl(const nlohmann::json &j, std::ostream &err) {
	static const auto schema = R"({
		"$schema": "http://json-schema.org/draft-07/schema#",
		"type": "object",
		"properties": {
			"version": {"type": "integer", "minimum": 1, "maximum": 1},
			"forceExistence": {"type": "boolean"},
			"strictTransit": {"type": "boolean"}
		},
		"required": ["version"]
	})"_json;
	static const nlohmann::json_schema::json_validator validator(schema);
	if(!IO::validateJson(j, validator, err, "OverallCatalysis module specification data does not conform to schema:"))
		return false;
	forceExistence = j["forceExistence"];
	strictTransit = j["strictTransit"];
	return true;
}

Transits OverallCatalysisSpecification::getTransits() const {
	return Transits();
}

std::unique_ptr<ModelModule> OverallCatalysisSpecification::createModel(Model &owner) const {
	return std::make_unique<OverallCatalysisModel>(owner);
}

//------------------------------------------------------------------------------
// Model
//------------------------------------------------------------------------------


struct OverallCatalysisModel::StaticInit {
	StaticInit() {
		auto &r = ModuleRegistry::get();
		std::type_index id = typeid(OverallCatalysisSpecification);
		r.addModule(id, "OverallCatalysis", [](Specification &spec, bool setDefaults) {
			spec.addModule<OverallCatalysisSpecification>(setDefaults);
		});
		r.addVariableSet("isOverallCata", makeVertexVariableSet(
				id, [](const Model &m) {
					if(m.specification->getModule<BaseSpecification>().getRelaxed())
						MOD_ABORT;
					const auto &mm = m.getModule<OverallCatalysisModel>();
					CombiOpt::LinExpAny res;
					for(const auto &p : mm.isOverallCatalytic)
						res += p.second;
					return res;
				}, [](const Model &m, const lib::DG::HyperVertex &v) {
					if(m.specification->getModule<BaseSpecification>().getRelaxed())
						MOD_ABORT;
					const auto &mm = m.getModule<OverallCatalysisModel>();
					const auto iter = mm.isOverallCatalytic.find(v);
					assert(iter != end(mm.isOverallCatalytic));
					CombiOpt::LinExpAny res;
					res += iter->second;
					return res;
				}));
	}
};

namespace {
OverallCatalysisModel::StaticInit staticInit;
} // namespace

OverallCatalysisModel::OverallCatalysisModel(Model &owner) : ModelModule(owner) {
	const bool relaxed = owner.specification->getModule<BaseSpecification>().getRelaxed();
	if(relaxed)
		throw LogicError("Can not create model. OverallCatalysis can not be enabled in relaxed mode.");
}

const OverallCatalysisSpecification &OverallCatalysisModel::getSpec() const {
	return owner.specification->getModule<OverallCatalysisSpecification>();
}

void OverallCatalysisModel::createVariablesImpl(CombiOpt::Model &model) {
	assert(!owner.specification->getModule<BaseSpecification>().getRelaxed());
	const auto &dgHyper = owner.specification->dgHyper.getGraph();
	// indicator vars
	for(const lib::DG::HyperVertex vHyper : asRange(vertices(dgHyper))) {
		if(dgHyper[vHyper].kind != lib::DG::HyperVertexKind::Vertex) continue;
		const auto *g = dgHyper[vHyper].graph;
		assert(g);
		std::string name = "isOverallCata(";
		name += g->getName();
		name += ")";
		auto var = model.addBoolVar(name);
		isOverallCatalytic.emplace(vHyper, var);
	}
}

void OverallCatalysisModel::createConstraintsImpl(CombiOpt::Model &model) {
	assert(!owner.specification->getModule<BaseSpecification>().getRelaxed());
	const auto &dgHyper = owner.specification->dgHyper.getGraph();
	const auto &baseModel = owner.getModule<BaseModel>();

	for(const lib::DG::HyperVertex vHyper : asRange(vertices(dgHyper))) {
		if(dgHyper[vHyper].kind != lib::DG::HyperVertexKind::Vertex) continue;
		const auto iter = isOverallCatalytic.find(vHyper);
		assert(iter != end(isOverallCatalytic));
		const auto isCata = iter->second;
		const auto isInLessOut = baseModel.getIsInLessOut(vHyper);
		const auto isInGreaterOut = baseModel.getIsInGreaterOut(vHyper);
		const auto isInOutZero = baseModel.getIsInOutZero(vHyper);
		if(getSpec().getStrictTransit())
			addDisableInternalIfTrue(owner, model, isCata, vHyper);

		// NOT inLessOut AND NOT inGreaterOut AND NOT inOutZero => isCata
		model.addImplication(!isInLessOut && !isInGreaterOut && !isInOutZero, isCata);
		// inOutZero => NOT isCata
		model.addImplication(isInOutZero, !isCata);
		// inLessOut => NOT isCata
		model.addImplication(isInLessOut, !isCata);
		// inGreaterOut => NOT isCata
		model.addImplication(isInGreaterOut, !isCata);
	}

	if(getSpec().getForceExistence()) {
		CombiOpt::Disjunction c;
		for(const auto &p : isOverallCatalytic)
			c = std::move(c) || p.second;
		model.addConstraint(std::move(c));
	}
}

nlohmann::json OverallCatalysisModel::dumpImpl(const CombiOpt::Result &sol) const {
	return {};
}

bool OverallCatalysisModel::loadImpl(const nlohmann::json &j, CombiOpt::LoadedSolution &s, std::ostream &err) const {
	static const nlohmann::json schema = R"({
		"$schema": "http://json-schema.org/draft-07/schema#",
		"type": "null"
	})"_json;
	static const nlohmann::json_schema::json_validator validator(schema);
	if(!IO::validateJson(j, validator, err, "OverallCatalysis solution data does not conform to schema:"))
		return false;
	return true;
}

void OverallCatalysisModel::loadSolutionSetDependentVarsImpl(CombiOpt::LoadedSolution &s) const {
	const auto &baseSpec = owner.specification->getModule<BaseSpecification>();
	const auto &baseModel = owner.getModule<BaseModel>();
	const auto &dg = owner.specification->dgHyper;
	const auto &dgHyper = dg.getGraph();
	if(baseSpec.getRelaxed()) return;
	bool isSolutionCata = false;
	for(const auto v : asRange(vertices(dgHyper))) {
		if(dgHyper[v].kind != DG::HyperVertexKind::Vertex) continue;
		const auto in = s.getVal(baseModel.getIn(v));
		const auto out = s.getVal(baseModel.getOut(v));
		if(0 < in && in == out) {
			const auto varIter = isOverallCatalytic.find(v);
			assert(varIter != end(isOverallCatalytic));
			const auto var = varIter->second;
			s.integralValues.emplace(var, 1);
			isSolutionCata = true;
			if(getSpec().getStrictTransit()) {
//				bool feasible = dumpCheckDisableInternalIfTrue(
//						dgGraph, base.DumpLoading().Transit().getTransitVars(v), sol);
//				if(!feasible) {
//					std::cout
//							<< "Error while loading dumped Flow solution. Overall catalysis is enabled, but the solution is infeasible (conflict with 'strictTransit')."
//							<< std::endl;
//					std::exit(1);
//				}
			}
		}
	}
	if(getSpec().getForceExistence()) {
		if(!isSolutionCata) {
//			std::cout
//					<< "Error while loading dumped Flow solution. Overall catalysis is enabled, but the solution is infeasible (conflict with 'forceExistence')."
//					<< std::endl;
//			std::exit(1);
		}
	}
}

bool OverallCatalysisModel::hasListEntry(lib::DG::HyperVertex vHyper, const CombiOpt::Result &sol) const {
	const auto iter = isOverallCatalytic.find(vHyper);
	assert(iter != end(isOverallCatalytic));
	return sol.getValue(iter->second) != 0;
}

std::vector<std::string> OverallCatalysisModel::listHeaderEntries() const {
	return {"OC"};
}

std::vector<std::string>
OverallCatalysisModel::listEntries(lib::DG::HyperVertex vHyper, const CombiOpt::Result &sol) const {
	const auto iter = isOverallCatalytic.find(vHyper);
	assert(iter != end(isOverallCatalytic));
	return {boost::lexical_cast<std::string>(sol.getValue(iter->second))};
}

} // namespace mod::lib::HyperFlow