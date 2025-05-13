#include "Module.hpp"

#include <mod/lib/HyperFlow/Model.hpp>
#include <mod/lib/HyperFlow/Specification.hpp>

namespace mod::lib::HyperFlow {

//------------------------------------------------------------------------------
// ModelModule
//------------------------------------------------------------------------------

SpecificationModule::SpecificationModule(Specification &owner) : owner(owner) { }

void SpecificationModule::list(std::ostream &s) const {
	s << getName() << " " << std::string(80 - getName().size()  - 1, '-') << "\n";
	listImpl(s);
}

nlohmann::json SpecificationModule::dump() const {
	return dumpImpl();
}

bool SpecificationModule::load(const nlohmann::json &j, std::ostream &err) {
	return loadImpl(j, err);
}

//------------------------------------------------------------------------------
// ModelModule
//------------------------------------------------------------------------------

ModelModule::ModelModule(Model &owner) : owner(owner) { }

nlohmann::json ModelModule::dump(const CombiOpt::Result &sol) const {
	return dumpImpl(sol);
}

bool ModelModule::load(const nlohmann::json &j, CombiOpt::LoadedSolution &s, std::ostream &err) const {
	return loadImpl(j, s, err);
}

void ModelModule::loadSolutionSetDependentVars(CombiOpt::LoadedSolution &s) const {
	loadSolutionSetDependentVarsImpl(s);
}

void ModelModule::createVariables(CombiOpt::Model &model) {
	createVariablesImpl(model);
}

void ModelModule::createConstraints(CombiOpt::Model &model) {
	createConstraintsImpl(model);
}

} // namespace mod::lib::HyperFlow