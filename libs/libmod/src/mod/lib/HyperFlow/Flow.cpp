#include "Flow.hpp"

#include <mod/Error.hpp>
#include <mod/lib/DG/IO/Read.hpp>
#include <mod/lib/DG/IO/Write.hpp>
#include <mod/lib/DG/Hyper.hpp>
#include <mod/lib/HyperFlow/Modules/Base.hpp>
#include <mod/lib/HyperFlow/Modules/UserDefined.hpp>
#include <mod/lib/HyperFlow/Modules/OverallCatalysis.hpp>
#include <mod/lib/HyperFlow/Modules/OverallAutocatalysis.hpp>
#include <mod/lib/IO/Json.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

namespace mod::lib::HyperFlow {

Flow::Flow(std::shared_ptr<dg::DG> dg_, const std::string &ilpSolver)
		: dg_(dg_), ilpSolver(ilpSolver),
		  spec(std::make_unique<lib::HyperFlow::Specification>(dg_->getHyper())) {
	assert(dg_->isLocked());
	assert(dg_->numVertices() != 0);
	spec->addModule<BaseSpecification>(true);
	spec->addModule<UserDefinedSpecification>(true);
}

bool Flow::isSpecWritable() const {
	return spec != nullptr;
}

Specification &Flow::writeSpec() {
	assert(isSpecWritable());
	return *spec;
}

void Flow::enableOverallAutocatalysis(bool setDefaults) {
	writeSpec().addModule<OverallAutocatalysisSpecification>(setDefaults);
}

void Flow::enableOverallCatalysis(bool setDefaults) {
	writeSpec().addModule<OverallCatalysisSpecification>(setDefaults);
}

void Flow::makeModel(int verbosity) {
	assert(isSpecWritable());
	mod.reset(new HyperFlow::Model(std::move(spec), ilpSolver, verbosity));
}

Model &Flow::getModel() {
	assert(!isSpecWritable());
	assert(mod);
	return *mod;
}

const Model &Flow::getModel() const {
	assert(!isSpecWritable());
	assert(mod);
	return *mod;
}

void Flow::listModel(std::ostream &s) const {
	s << "HyperFlow ======================================================================\n";
	s << "DG: " << *dg_ << "\n";
	readSpec().list(s);
}

const Specification &Flow::readSpec() const {
	if(spec) return *spec;
	else return *mod->specification;
}

nlohmann::json Flow::dump() const {
	nlohmann::json j;
	j["dgData"] = lib::DG::Write::dumpDigest(this->dg_->getHyper().getGraph());

	j["specification"] = readSpec().dump();
	if(isSpecWritable())
		j["solutions"] = nullptr;
	else
		j["solutions"] = getModel().dump();
	return j;
}

bool Flow::load(Flow &flow, const nlohmann::json &j, int verbosity, std::ostream &err) {
	static const nlohmann::json schema = R"({
		"$schema": "http://json-schema.org/draft-07/schema#",
		"type": "object",
		"properties": {
			"dgData": {"type": "object"},
			"specification": {"type": "object"},
			"solutions": {"type": ["array", "null"]}
		},
		"required": ["dgData", "specification", "solutions"]
	})"_json;
	static const nlohmann::json_schema::json_validator validator(schema);
	if(!IO::validateJson(j, validator, err, "Data does not conform to schema:"))
		return false;
	if(!lib::DG::Read::dumpDigest(flow.dg_->getHyper().getGraph(), j["dgData"], err, "flow model"))
		return false;
	const auto specRes = Specification::load(flow.writeSpec(), j["specification"], err);
	if(!specRes) return false;
	const auto &solutions = j["solutions"];
	if(solutions.is_null()) return true;

	flow.makeModel(verbosity);
	const auto modelRes = flow.getModel().load(solutions, verbosity, err);
	return modelRes;
}

} // namespace mod::lib::HyperFlow