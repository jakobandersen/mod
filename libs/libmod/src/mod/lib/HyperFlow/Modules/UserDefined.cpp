#include "UserDefined.hpp"

#include <mod/lib/CombiOpt/LoadedSolution.hpp>
#include <mod/lib/CombiOpt/Model.hpp>
#include <mod/lib/CombiOpt/Solver.hpp>
#include <mod/lib/HyperFlow/ModuleRegistry.hpp>
#include <mod/lib/HyperFlow/Specification.hpp>
#include <mod/lib/HyperFlow/Model.hpp>
#include <mod/lib/HyperFlow/Modules/Utils.hpp>
#include <mod/lib/IO/Json.hpp>

#include <iostream>

namespace mod::lib::HyperFlow {

UserDefinedSpecification::UserDefinedSpecification(Specification &owner, bool setDefaults)
		: SpecificationModule(owner) {}

std::string UserDefinedSpecification::getName() const {
	return "UserDefined";
}

void UserDefinedSpecification::addDefaultObjective(hyperflow::LinExp &exp) const {}

bool UserDefinedSpecification::hasVar(const std::string &name) const {
	return
			boolVars.find(name) != end(boolVars) ||
			intVars.find(name) != end(intVars) ||
			floatVars.find(name) != end(floatVars);
}

hyperflow::VarCustom UserDefinedSpecification::addBoolVar(std::string name) {
	if(hasVar(name))
		throw LogicError("Name for custom variable '" + name + "' already in use.");
	boolVars.emplace(name);
	return hyperflow::VarCustom(getVariableSetName(), std::move(name));
}

hyperflow::VarCustom UserDefinedSpecification::addIntVar(std::string name) {
	if(hasVar(name))
		throw LogicError("Name for custom variable '" + name + "' already in use.");
	intVars.emplace(name);
	return hyperflow::VarCustom(getVariableSetName(), std::move(name));
}

hyperflow::VarCustom UserDefinedSpecification::addFloatVar(std::string name) {
	if(hasVar(name))
		throw LogicError("Name for custom variable '" + name + "' already in use.");
	floatVars.emplace(name);
	return hyperflow::VarCustom(getVariableSetName(), std::move(name));
}

const std::set<std::string> &UserDefinedSpecification::getBoolVars() const {
	return boolVars;
}

const std::set<std::string> &UserDefinedSpecification::getIntVars() const {
	return intVars;
}

const std::set<std::string> &UserDefinedSpecification::getFloatVars() const {
	return floatVars;
}

void UserDefinedSpecification::addConstraint(hyperflow::LinConstraint cons) {
	owner.addModulesFromLinExp(cons.exp);
	constraints.push_back(std::move(cons));
}

const std::vector<hyperflow::LinConstraint> &UserDefinedSpecification::getConstraints() const {
	return constraints;
}

void UserDefinedSpecification::listImpl(std::ostream &s) const {
	s << "Bool variables:\n";
	for(const auto &v: boolVars)
		s << v << '\n';
	s << "Int variables:\n";
	for(const auto &v: intVars)
		s << v << '\n';
	s << "Float variables:\n";
	for(const auto &v: floatVars)
		s << v << '\n';
	s << "Extra constraints:\n";
	for(auto &p: constraints)
		s << p << '\n';
}

nlohmann::json UserDefinedSpecification::dumpImpl() const {
	nlohmann::json j;
	j["version"] = 1;
	j["boolVars"] = boolVars;
	j["intVars"] = intVars;
	j["floatVars"] = floatVars;
	auto cs = nlohmann::json::array();
	for(const auto &c: constraints)
		cs.push_back(HyperFlow::dump(owner, c));
	j["constraints"] = std::move(cs);
	return j;
}

bool UserDefinedSpecification::loadImpl(const nlohmann::json &j, std::ostream &err) {
	static const nlohmann::json schema = R"({
		"$schema": "http://json-schema.org/draft-07/schema#",
		"type": "object",
		"properties": {
			"version": {"type": "integer", "minimum": 1, "maximum": 1},
			"boolVars": {"type": "array", "items": {"type": "string"}, "uniqueItems": true},
			"intVars": {"type": "array", "items": {"type": "string"}, "uniqueItems": true},
			"floatVars": {"type": "array", "items": {"type": "string"}, "uniqueItems": true},
			"constraints": {"type": "array"}
		},
		"required": ["intVars"],
		"dependentRequired": {
			"version": ["boolVars", "floatVars"]
		}
	})"_json;
	static const nlohmann::json_schema::json_validator validator(schema);
	if(!IO::validateJson(j, validator, err, "UserDefined specification data does not conform to schema:"))
		return false;
	const int version = j.contains("version") ? j["version"].get<int>() : 0;
	if(version >= 1)
		for(const auto &s: j["boolVars"])
			addBoolVar(s);
	for(const auto &s: j["intVars"])
		addIntVar(s);
	if(version >= 1)
		for(const auto &s: j["floatVars"])
			addFloatVar(s);
	for(const auto &c: j["constraints"]) {
		auto res = loadConstraint(owner, c, err);
		if(!res) return false;
		addConstraint(*res);
	}
	return true;
}

Transits UserDefinedSpecification::getTransits() const {
	return Transits();
}

std::unique_ptr<ModelModule> UserDefinedSpecification::createModel(Model &owner) const {
	return std::make_unique<UserDefinedModel>(owner, *this);
}

std::string UserDefinedSpecification::getVariableSetName() {
	return "userDefined";
}

//------------------------------------------------------------------------------
// Model
//------------------------------------------------------------------------------

struct UserDefinedModel::StaticInit {
	StaticInit() {
		auto &r = ModuleRegistry::get();
		std::type_index id = typeid(UserDefinedSpecification);
		r.addModule(id, "UserDefined", [](Specification &spec, bool setDefaults) {
			spec.addModule<UserDefinedSpecification>(setDefaults);
		});
		r.addVariableSet(UserDefinedSpecification::getVariableSetName(), makeCustomVariableSet(
				id, [](const Model &m) {
					const auto &mm = m.getModule<UserDefinedModel>();
					CombiOpt::LinExpAny res;
					for(const auto &p: mm.boolVars)
						res += p.second;
					for(const auto &p: mm.intVars)
						res += p.second;
					return res;
				}, [](const Model &m, const std::string &v) {
					const auto &mm = m.getModule<UserDefinedModel>();
					if(const auto iter = mm.boolVars.find(v); iter != end(mm.boolVars)) {
						return CombiOpt::LinExpAny() += iter->second;
					} else if(const auto iter = mm.intVars.find(v); iter != end(mm.intVars)) {
						return CombiOpt::LinExpAny() += iter->second;
					} else if(const auto iter = mm.floatVars.find(v); iter != end(mm.floatVars)) {
						return CombiOpt::LinExpAny() += iter->second;
					} else {
						throw LogicError("Can not compile user-defined variable '" + v + "'. It does not exist.");
					}
				}));
	}
};

namespace {
UserDefinedModel::StaticInit staticInit;
} // namespace

UserDefinedModel::UserDefinedModel(
		Model &owner, const UserDefinedSpecification &specification)
		: ModelModule(owner), specification(specification) {}

const UserDefinedSpecification &UserDefinedModel::getSpec() const {
	return specification;
}

void UserDefinedModel::createVariablesImpl(CombiOpt::Model &model) {
	for(const auto &name: specification.getBoolVars()) {
		const auto v = model.addBoolVar(name);
		boolVars.emplace(name, v);
	}
	for(const auto &name: specification.getIntVars()) {
		const auto v = model.addIntVariable(name);
		model.setLB(v, CombiOpt::Int::MIN);
		intVars.emplace(name, v);
	}
	for(const auto &name: specification.getFloatVars()) {
		const auto v = model.addFloatVariable(name);
		model.setLB(v, CombiOpt::Float::MIN);
		floatVars.emplace(name, v);
	}
}

void UserDefinedModel::createConstraintsImpl(CombiOpt::Model &model) {
	for(const auto &c: specification.getConstraints())
		model.addConstraint(owner.compileConstraint(c));
}

nlohmann::json UserDefinedModel::dumpImpl(const CombiOpt::Result &sol) const {
	nlohmann::json j;
	j["version"] = 1;

	auto bVars = nlohmann::json::array(),
			iVars = nlohmann::json::array(),
			fVars = nlohmann::json::array();
	for(const auto &[name, var]: boolVars) {
		const auto val = sol.getValue(var).getValue();
		if(val != 0)
			bVars.push_back({name, val});
	}
	for(const auto &[name, var]: intVars) {
		const auto val = sol.getValue(var).getValue();
		if(val != 0)
			iVars.push_back({name, val});
	}
	for(const auto &[name, var]: floatVars) {
		const auto val = sol.getValue(var).getValue();
		if(val != 0.0)
			fVars.push_back({name, val});
	}
	j["boolVars"] = std::move(bVars);
	j["intVars"] = std::move(iVars);
	j["floatVars"] = std::move(fVars);
	return j;
}

bool UserDefinedModel::loadImpl(const nlohmann::json &j, CombiOpt::LoadedSolution &s, std::ostream &err) const {
	static const nlohmann::json schema = R"({
		"$schema": "http://json-schema.org/draft-07/schema#",
		"type": "object",
		"properties": {
			"version": {"type": "integer", "minimum": 1, "maximum": 1},
			"boolVars":
				{"type": "array", "items":
					{"type": "array", "additionalItems": false, "items": [
						{"type": "string"},
						{"type": "integer"}
					]}
				},
			"intVars":
				{"type": "array", "items":
					{"type": "array", "additionalItems": false, "items": [
						{"type": "string"},
						{"type": "integer"}
					]}
				},
			"floatVars":
				{"type": "array", "items":
					{"type": "array", "additionalItems": false, "items": [
						{"type": "string"},
						{"type": "number"}
					]}
				}
		},
		"required": ["boolVars", "intVars", "floatVars"]
	})"_json;
	static const nlohmann::json_schema::json_validator validator(schema);
	if(j.is_null()) {
		if(!getSpec().getIntVars().empty()) {
			std::cout
					<< "WARNING: loading a flow dump with old format for solutions with user-defined variables."
					   " The value of the variables are not loaded, so the values will all be 0." << std::endl;
		}
		return true;
	}
	if(!IO::validateJson(j, validator, err, "UserDefined solution data does not conform to schema:"))
		return false;

	for(const auto &jp: j["boolVars"]) {
		const std::string &name = jp[0];
		const auto iter = boolVars.find(name);
		if(iter == boolVars.end()) {
			err << "Custom boolean variable '" << name << "' is in dump, but not in model.";
			return false;
		}
		s.integralValues.emplace(iter->second, jp[1].get<int>());
	}
	for(const auto &jp: j["intVars"]) {
		const std::string &name = jp[0];
		const auto iter = intVars.find(name);
		if(iter == intVars.end()) {
			err << "Custom integer variable '" << name << "' is in dump, but not in model.";
			return false;
		}
		s.integralValues.emplace(iter->second, jp[1].get<int>());
	}
	for(const auto &jp: j["floatVars"]) {
		const std::string &name = jp[0];
		const auto iter = floatVars.find(name);
		if(iter == floatVars.end()) {
			err << "Custom floating-point variable '" << name << "' is in dump, but not in model.";
			return false;
		}
		s.floatValues.emplace(iter->second, jp[1].get<double>());
	}
	return true;
}

void UserDefinedModel::loadSolutionSetDependentVarsImpl(CombiOpt::LoadedSolution &s) const {}

bool UserDefinedModel::hasListEntry(lib::DG::HyperVertex vHyper, const CombiOpt::Result &sol) const {
	return false;
}

std::vector<std::string> UserDefinedModel::listHeaderEntries() const {
	return {};
}

std::vector<std::string> UserDefinedModel::listEntries(lib::DG::HyperVertex vHyper, const CombiOpt::Result &sol) const {
	return {};
}

} // namespace mod::lib::HyperFlow