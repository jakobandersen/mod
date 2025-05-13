#include "Specification.hpp"

#include <mod/Function.hpp>
#include <mod/lib/DG/Hyper.hpp>
#include <mod/lib/DG/IO/Read.hpp>
#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/HyperFlow/ModuleRegistry.hpp>
#include <mod/lib/HyperFlow/Modules/Module.hpp>
#include <mod/lib/HyperFlow/Modules/Utils.hpp>
#include <mod/lib/Rule/Rule.hpp>
#include <mod/lib/IO/Json.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <iostream>

namespace mod::lib::HyperFlow {
namespace {

struct AddModuleVisitor {
	AddModuleVisitor(std::ostream &s, Specification &spec) : s(s), spec(spec) {}

	template<typename T>
	void operator()(const T &var) {
		handleVarSet(var.id);
	}
private:
	void handleVarSet(const std::string &id) {
		const auto *vs = lib::HyperFlow::ModuleRegistry::get().getVariableSet(id);
		if(!vs)
			throw LogicError("Can not use variable '" + id + "'. Variable set not registered.");
		if(spec.hasModule(vs->owner))
			return;
		const auto *module = lib::HyperFlow::ModuleRegistry::get().getModule(vs->owner);
		s << "Enabling extension " << module->name << ", used in flow specificiation.\n";
		module->enable(spec, true);
	}
private:
	std::ostream &s;
	HyperFlow::Specification &spec;
};

int getNextId() {
	static int next = 0;
	return next++;
}

} // namespace

Specification::Specification(const lib::DG::Hyper &dgHyper)
		: dgHyper(dgHyper), id(getNextId()) {}

void Specification::addModulesFromLinExp(const hyperflow::LinExp &exp) {
	for(const auto &e: exp.getIntElements())
		e.second.applyVisitor(AddModuleVisitor(std::cout, *this));
	for(const auto &e: exp.getFloatElements())
		e.second.applyVisitor(AddModuleVisitor(std::cout, *this));
}

bool Specification::hasModule(std::type_index id) const {
	return modules.find(id) != modules.end();
}

SpecificationModule &Specification::getModule(std::type_index id) {
	const auto iter = modules.find(id);
	assert(iter != modules.end());
	return *iter->second;
}

//------------------------------------------------------------------------------

void Specification::setObjectiveFunction(hyperflow::LinExp exp) {
	addModulesFromLinExp(exp);
	objectiveFunction = std::move(exp);
}

hyperflow::LinExp Specification::getObjectiveFunction() const {
	if(objectiveFunction) return *objectiveFunction;
	hyperflow::LinExp exp;
	for(const auto &mp: modules)
		mp.second->addDefaultObjective(exp);
	return exp;
}

void Specification::addEnumerationVar(hyperflow::Var v) {
	addModulesFromLinExp(v);
	if(!enumerationVars) enumerationVars = std::vector<EnumerationVar>();
	enumerationVars->push_back(EnumerationVar{v});
}

bool Specification::hasExplicitEnumerationVars() const {
	return enumerationVars.has_value();
}

std::vector<Specification::EnumerationVar> Specification::getEnumerationVars() const {
	if(enumerationVars) return *enumerationVars;
	return {
			EnumerationVar{hyperflow::vars::edgeFlow},
			EnumerationVar{hyperflow::vars::inFlow},
			EnumerationVar{hyperflow::vars::outFlow}
	};
}

void Specification::addTransitEnumeration(lib::DG::HyperVertex v) {
	assert(dgHyper.getGraph()[v].kind == lib::DG::HyperVertexKind::Vertex);
	transitEnumeration.push_back(TransitEnumeration{v});
}

const std::vector<Specification::TransitEnumeration> &Specification::getTransitEnumeration() const {
	return transitEnumeration;
}

std::optional<int> Specification::getAbsGap() const {
	return absGap;
}

void Specification::setAbsGap(int absGap) {
	if(absGap < 0) this->absGap.reset();
	else this->absGap = absGap;
}

void Specification::list(std::ostream &s) const {
	s << "Solution Control " << std::string(80 - 16 - 1, '-') << "\n";
	if(objectiveFunction)
		s << "ObjectiveFunction (user defined): " << *objectiveFunction << "\n";
	else
		s << "ObjectiveFunction (current default): " << getObjectiveFunction() << "\n";
	s << "enumerationVars:";
	if(!hasExplicitEnumerationVars())
		s << " (default)";
	for(const auto &v: getEnumerationVars())
		s << " " << v.v;
	s << "\n";
	s << "transitEnumeration:";
	for(const auto e: getTransitEnumeration()) {
		const lib::graph::Graph *g = dgHyper.getGraph()[e.v].graph;
		s << "'" << g->getName() << "' ";
	}
	s << "\n";
	s << "absGap: ";
	if(absGap) s << *absGap;
	else s << "none";
	s << '\n';

	for(const auto &mp: modules)
		mp.second->list(s);
}

nlohmann::json Specification::dump() const {
	nlohmann::json j;
	j["version"] = 9;

	if(absGap) j["absGap"] = *absGap;
	else j["absGap"] = nullptr;
	if(enumerationVars) {
		auto vars = nlohmann::json::array();
		for(const auto &v: *enumerationVars)
			vars.push_back(HyperFlow::dump(*this, v.v));
		j["enumerationVars"] = std::move(vars);
	} else {
		j["enumerationVars"] = nullptr;
	}
	{ // transit enumeration
		std::vector<TransitEnumeration> vs;
		vs.assign(begin(getTransitEnumeration()), end(getTransitEnumeration()));
		/*std::sort(begin(vs), end(vs), [](const TransitEnumeration &a, const TransitEnumeration &b) {
			return a.v < b.v;
		});*/
		auto &s = j["transitEnumeration"] = nlohmann::json::array();
		const auto dgIdx = get(boost::vertex_index_t(), dgHyper.getGraph());
		for(const auto v: vs) s.push_back(dgIdx[v.v]);
	}
	if(objectiveFunction)
		j["objectiveFunction"] = HyperFlow::dump(*this, *objectiveFunction);
	else
		j["objectiveFunction"] = nullptr;

	nlohmann::json ms;
	for(const auto &mp: modules)
		ms[mp.second->getName()] = mp.second->dump();
	j["modules"] = std::move(ms);
	return j;
}

bool Specification::load(Specification &spec, const nlohmann::json &j, std::ostream &err) {
	static const auto schema = R"({
		"$schema": "http://json-schema.org/draft-07/schema#",
		"type": "object",
		"properties": {
			"version": {"type": "integer", "minimum": 8, "maximum": 9},
			"absGap": {"type": ["integer", "null"], "minimum": 0},
			"enumerationVars": {"type": ["array", "null"]},
			"objectiveFunction": {},
			"modules": {"type": "object"}
		},
		"required": ["version", "absGap", "enumerationVars", "objectiveFunction", "modules"]
	})"_json;
	static const nlohmann::json_schema::json_validator validator(schema);
	if(!IO::validateJson(j, validator, err, "Specification data does not conform to schema:"))
		return false;

	const int version = j["version"];

	const auto &jAbsGap = j["absGap"];
	if(jAbsGap.is_null()) spec.absGap.reset();
	else spec.absGap = jAbsGap.get<int>();

	const auto &jEnumVars = j["enumerationVars"];
	if(!jEnumVars.is_null()) {
		for(const auto &jVar: jEnumVars) {
			auto res = HyperFlow::loadVar(spec, jVar, err);
			if(!res) return false;
			spec.addEnumerationVar(*res);
		}
	}

	if(version >= 9) {
		const auto &dg = spec.dgHyper.getGraph();
		for(const int id: j["transitEnumeration"]) {
			const auto vOpt = lib::DG::Read::vertex(dg, id, err, "Transit enumeration error.");
			if(!vOpt) return false;
			spec.addTransitEnumeration(*vOpt);
		}
	}

	const auto &jObjFunction = j["objectiveFunction"];
	if(!jObjFunction.is_null()) {
		auto res = HyperFlow::loadExp(spec, jObjFunction, err);
		if(!res) return false;
		spec.setObjectiveFunction(*res);
	}

	for(const auto &mp: j["modules"].items()) {
		const auto &name = mp.key();
		const auto *mod = ModuleRegistry::get().getModule(name);
		if(!mod) {
			err << "module in specification, '" << name << "', has not been registered.";
			err << "\nRegistered modules:";
			for(const auto &m: ModuleRegistry::get().getAllModules())
				err << "\n   " << m.second.name;
			return false;
		}
		// TODO: once the outer Flow is not pre-defined, as the modules are then not existing
		if(name == "Base" || name == "UserDefined") {
			assert(spec.hasModule(mod->id));
		} else {
			assert(!spec.hasModule(mod->id));
			mod->enable(spec, false);
		}
		const auto res = spec.getModule(mod->id).load(mp.value(), err);
		if(!res) return false;
	}
	return true;
}

//------------------------------------------------------------------------------

Transits ModelCreator::getTransits(const Specification &spec) {
	Transits rs;
	for(const auto &mp: spec.modules) {
		auto rsm = mp.second->getTransits();
		for(auto &&e: rsm.expressed) rs.expressed.insert(e);
		for(auto &&e: rsm.deleted) rs.deleted.insert(e);
	}
	return rs;
}

std::map<std::type_index, std::unique_ptr<ModelModule> >
ModelCreator::createModules(const Specification &spec, Model &owner) {
	std::map<std::type_index, std::unique_ptr<ModelModule> > modules;
	for(const auto &mp: spec.modules)
		modules.emplace(mp.first, mp.second->createModel(owner));
	return modules;
}

} // namespace mod::lib::HyperFlow