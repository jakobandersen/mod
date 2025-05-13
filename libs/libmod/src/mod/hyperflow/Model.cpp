#include "Model.hpp"

#include <mod/Error.hpp>
#include <mod/Post.hpp>
#include <mod/dg/Printer.hpp>
#include <mod/hyperflow/ModelImplementationView.hpp>
#include <mod/hyperflow/Printer.hpp>
#include <mod/hyperflow/Solution.hpp>
#include <mod/lib/CombiOpt/Solver.hpp>
#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/HyperFlow/Flow.hpp>
#include <mod/lib/HyperFlow/IO/Read.hpp>
#include <mod/lib/HyperFlow/IO/Write.hpp>
#include <mod/lib/HyperFlow/Modules/Base.hpp>
#include <mod/lib/HyperFlow/Modules/OverallAutocatalysis.hpp>
#include <mod/lib/HyperFlow/Modules/OverallCatalysis.hpp>
#include <mod/lib/HyperFlow/Modules/UserDefined.hpp>
#include <mod/lib/IO/IO.hpp>

#include <iostream>

namespace mod::hyperflow {
struct Model::Pimpl {
	std::unique_ptr<lib::HyperFlow::Flow> f;
};

Model::Model(std::unique_ptr<lib::HyperFlow::Flow> f)
	: overallAutocatalysis(*this),
	  overallCatalysis(*this),
	  p(new Pimpl{std::move(f)}) {}

Model::~Model() = default;

using BaseSpec = lib::HyperFlow::BaseSpecification;
using UserDefSpec = lib::HyperFlow::UserDefinedSpecification;
using OCSpec = lib::HyperFlow::OverallCatalysisSpecification;
using OASpec = lib::HyperFlow::OverallAutocatalysisSpecification;

std::ostream &operator<<(std::ostream &s, const Model &dgFlow) {
	return s << "HyperflowModel(" << dgFlow.getId() << ")";
}

int Model::getId() const {
	return p->f->readSpec().id;
}

std::shared_ptr<dg::DG> Model::getDG() const {
	return p->f->dg_;
}

lib::HyperFlow::Flow &Model::getFlow() {
	return *p->f;
}

const lib::HyperFlow::Flow &Model::getFlow() const {
	return *p->f;
}

bool Model::isSpecificationLocked() const {
	return !p->f->isSpecWritable();
}

void Model::listSpecification() const {
	p->f->listModel(std::cout);
}

void Model::addSource(dg::DG::Vertex v) {
	if(isSpecificationLocked())
		throw LogicError("Specification locked.");
	if(!v) throw LogicError("Can not add null vertex as source.");
	if(v.getDG() != getDG())
		throw LogicError("The vertex does not belong to the underlying derivation graph.");
	const auto vHyper = getDG()->getHyper().getInternalVertex(v);
	p->f->writeSpec().getModule<BaseSpec>().addSource(vHyper);
}

void Model::addSource(std::shared_ptr<graph::Graph> g) {
	auto v = getDG()->findVertex(g);
	if(!v) throw LogicError("The graph is not a vertex in the underlying derivation graph.");
	addSource(v);
}

std::vector<dg::DG::Vertex> Model::getSources() const {
	const auto &sources = p->f->readSpec().getModule<lib::HyperFlow::BaseSpecification>().getSources();
	const auto &dg = p->f->dg_->getHyper();
	std::vector<dg::DG::Vertex> res;
	res.reserve(sources.size());
	for(const auto v: sources) {
		const auto vOuter = dg.getInterfaceVertex(v);
		assert(vOuter);
		res.push_back(vOuter);
	}
	return res;
}

void Model::addSink(dg::DG::Vertex v) {
	if(isSpecificationLocked())
		throw LogicError("Specification locked.");
	if(!v) throw LogicError("Can not add null vertex as sink.");
	if(v.getDG() != getDG())
		throw LogicError("The vertex does not belong to the underlying derivation graph.");
	const auto vHyper = getDG()->getHyper().getInternalVertex(v);
	p->f->writeSpec().getModule<BaseSpec>().addSink(vHyper);
}

void Model::addSink(std::shared_ptr<graph::Graph> g) {
	auto v = getDG()->findVertex(g);
	if(!v) throw LogicError("The graph is not a vertex in the underlying derivation graph.");
	addSink(v);
}

std::vector<dg::DG::Vertex> Model::getSinks() const {
	const auto &sinks = p->f->readSpec().getModule<lib::HyperFlow::BaseSpecification>().getSinks();
	const auto &dg = p->f->dg_->getHyper();
	std::vector<dg::DG::Vertex> res;
	res.reserve(sinks.size());
	for(const auto v: sinks) {
		const auto vOuter = dg.getInterfaceVertex(v);
		assert(vOuter);
		res.push_back(vOuter);
	}
	return res;
}

void Model::exclude(dg::DG::Vertex v) {
	if(isSpecificationLocked())
		throw LogicError("Specification locked.");
	if(!v) throw LogicError("Can not exclude null vertex.");
	if(v.getDG() != getDG())
		throw LogicError("The vertex does not belong to the underlying derivation graph.");
	const auto vHyper = getDG()->getHyper().getInternalVertex(v);
	p->f->writeSpec().getModule<BaseSpec>().exclude(vHyper);
}

void Model::exclude(std::shared_ptr<graph::Graph> g) {
	exclude(getDG()->findVertex(g));
}

std::vector<dg::DG::Vertex> Model::getExcluded() const {
	const auto &vs = p->f->readSpec().getModule<lib::HyperFlow::BaseSpecification>().getExcluded();
	const auto &dg = p->f->dg_->getHyper();
	std::vector<dg::DG::Vertex> res;
	res.reserve(vs.size());
	for(const auto v: vs) {
		const auto vOuter = dg.getInterfaceVertex(v);
		assert(vOuter);
		res.push_back(vOuter);
	}
	return res;
}

void Model::separateIOInternalTransit(dg::DG::Vertex v) {
	if(isSpecificationLocked())
		throw LogicError("Specification locked.");
	if(!v) throw LogicError("Can not separate transit edges in null vertex.");
	if(v.getDG() != getDG())
		throw LogicError("The vertex does not belong to the underlying derivation graph.");
	const auto vHyper = getDG()->getHyper().getInternalVertex(v);
	p->f->writeSpec().getModule<BaseSpec>().separateIOInternalTransit(vHyper);
}

void Model::separateIOInternalTransit(std::shared_ptr<graph::Graph> g) {
	separateIOInternalTransit(getDG()->findVertex(g));
}

std::vector<dg::DG::Vertex> Model::getSeparatedIOInternalTransit() const {
	const auto &vs = p->f->readSpec().getModule<lib::HyperFlow::BaseSpecification>().getSeparatedIOInternalTransit();
	const auto &dg = p->f->dg_->getHyper();
	std::vector<dg::DG::Vertex> res;
	res.reserve(vs.size());
	for(const auto v: vs) {
		const auto vOuter = dg.getInterfaceVertex(v);
		assert(vOuter);
		res.push_back(vOuter);
	}
	return res;
}

void Model::setAllowHyperLoops(bool value) {
	if(isSpecificationLocked())
		throw LogicError("Specification locked.");
	p->f->writeSpec().getModule<BaseSpec>().setAllowHyperLoops(value);
}

bool Model::getAllowHyperLoops() const {
	return p->f->readSpec().getModule<BaseSpec>().getAllowHyperLoops();
}

void Model::setAllowReversal(bool value) {
	if(isSpecificationLocked())
		throw LogicError("Specification locked.");
	p->f->writeSpec().getModule<BaseSpec>().setAllowReversal(value);
}

bool Model::getAllowReversal() const {
	return p->f->readSpec().getModule<BaseSpec>().getAllowReversal();
}

void Model::setAllowIOReversal(bool value) {
	if(isSpecificationLocked())
		throw LogicError("Specification locked.");
	p->f->writeSpec().getModule<BaseSpec>().setAllowIOReversal(value);
}

bool Model::getAllowIOReversal() const {
	return p->f->readSpec().getModule<BaseSpec>().getAllowIOReversal();
}

void Model::setRelaxed(bool value) {
	if(isSpecificationLocked())
		throw LogicError("Specification locked.");
	p->f->writeSpec().getModule<BaseSpec>().setRelaxed(value);
}

bool Model::getRelaxed() const {
	return p->f->readSpec().getModule<BaseSpec>().getRelaxed();
}

void Model::setObjectiveFunction(LinExp func) {
	if(isSpecificationLocked())
		throw LogicError("Specification locked.");
	p->f->writeSpec().setObjectiveFunction(std::move(func));
}

//------------------------------------------------------------------------------
// UserDefined
//------------------------------------------------------------------------------

VarCustom Model::addBoolVariable(const std::string &name) {
	if(isSpecificationLocked())
		throw LogicError("Specification locked.");
	return p->f->writeSpec().getModule<UserDefSpec>().addBoolVar(name);
}

VarCustom Model::addIntVariable(const std::string &name) {
	if(isSpecificationLocked())
		throw LogicError("Specification locked.");
	return p->f->writeSpec().getModule<UserDefSpec>().addIntVar(name);
}

VarCustom Model::addFloatVariable(const std::string &name) {
	if(isSpecificationLocked())
		throw LogicError("Specification locked.");
	return p->f->writeSpec().getModule<UserDefSpec>().addFloatVar(name);
}

std::vector<VarCustom> Model::getCustomBoolVariables() const {
	std::vector<VarCustom> res;
	const auto &spec = p->f->readSpec().getModule<UserDefSpec>();
	const auto &id = spec.getVariableSetName();
	const auto &vs = spec.getBoolVars();
	res.reserve(vs.size());
	for(const std::string &s: vs)
		res.emplace_back(id, s);
	return res;
}

std::vector<VarCustom> Model::getCustomIntVariables() const {
	std::vector<VarCustom> res;
	const auto &spec = p->f->readSpec().getModule<UserDefSpec>();
	const auto &id = spec.getVariableSetName();
	const auto &vs = spec.getIntVars();
	res.reserve(vs.size());
	for(const std::string &s: vs)
		res.emplace_back(id, s);
	return res;
}

std::vector<VarCustom> Model::getCustomFloatVariables() const {
	std::vector<VarCustom> res;
	const auto &spec = p->f->readSpec().getModule<UserDefSpec>();
	const auto &id = spec.getVariableSetName();
	const auto &vs = spec.getFloatVars();
	res.reserve(vs.size());
	for(const std::string &s: vs)
		res.emplace_back(id, s);
	return res;
}

void Model::addConstraint(const LinConstraint &constraint) {
	if(isSpecificationLocked())
		throw LogicError("Specification locked.");
	p->f->writeSpec().getModule<UserDefSpec>().addConstraint(constraint);
}

//------------------------------------------------------------------------------
// OverallAutocatalysis
//------------------------------------------------------------------------------

Model::OverallAutocatalysis::OverallAutocatalysis(Model &dgFlow) : dgFlow(dgFlow) {}

void Model::OverallAutocatalysis::enable() {
	if(!dgFlow.p->f->isSpecWritable())
		throw LogicError("Specification locked.");
	if(dgFlow.p->f->readSpec().hasModule<OASpec>())
		throw LogicError("Module already enabled.");
	dgFlow.p->f->enableOverallAutocatalysis(true);
}

bool Model::OverallAutocatalysis::isEnabled() const {
	return dgFlow.p->f->readSpec().hasModule<OASpec>();
}

void Model::OverallAutocatalysis::setForceExistence(bool value) {
	if(dgFlow.isSpecificationLocked())
		throw LogicError("Specification locked.");
	if(!dgFlow.p->f->readSpec().hasModule<OASpec>())
		throw LogicError("Module not enabled.");
	dgFlow.p->f->writeSpec().getModule<OASpec>().setForceExistence(value);
}

bool Model::OverallAutocatalysis::getForceExistence() const {
	if(!dgFlow.p->f->readSpec().hasModule<OASpec>())
		throw LogicError("Module not enabled.");
	return dgFlow.p->f->readSpec().getModule<OASpec>().getForceExistence();
}

void Model::OverallAutocatalysis::setStrictTransit(bool value) {
	if(dgFlow.isSpecificationLocked())
		throw LogicError("Specification locked.");
	if(!dgFlow.p->f->readSpec().hasModule<OASpec>())
		throw LogicError("Module not enabled.");
	dgFlow.p->f->writeSpec().getModule<OASpec>().setStrictTransit(value);
}

bool Model::OverallAutocatalysis::getStrictTransit() const {
	if(!dgFlow.p->f->readSpec().hasModule<OASpec>())
		throw LogicError("Module not enabled.");
	return dgFlow.p->f->readSpec().getModule<OASpec>().getStrictTransit();
}

void Model::OverallAutocatalysis::setBFSExclusive(bool value) {
	if(dgFlow.isSpecificationLocked())
		throw LogicError("Specification locked.");
	if(!dgFlow.p->f->readSpec().hasModule<OASpec>())
		throw LogicError("Module not enabled.");
	dgFlow.p->f->writeSpec().getModule<OASpec>().setBFSExclusive(value);
}

bool Model::OverallAutocatalysis::getBFSExclusive() const {
	if(!dgFlow.p->f->readSpec().hasModule<OASpec>())
		throw LogicError("Module not enabled.");
	return dgFlow.p->f->readSpec().getModule<OASpec>().getBFSExclusive();
}

//------------------------------------------------------------------------------
// OverallCatalysis
//------------------------------------------------------------------------------

Model::OverallCatalysis::OverallCatalysis(Model &dgFlow) : dgFlow(dgFlow) {}

void Model::OverallCatalysis::enable() {
	if(dgFlow.isSpecificationLocked())
		throw LogicError("Specification locked.");
	if(dgFlow.p->f->readSpec().hasModule<OCSpec>())
		throw LogicError("Module already enabled.");
	dgFlow.p->f->enableOverallCatalysis(true);
}

bool Model::OverallCatalysis::isEnabled() const {
	return dgFlow.p->f->readSpec().hasModule<OCSpec>();
}

void Model::OverallCatalysis::setForceExistence(bool value) {
	if(dgFlow.isSpecificationLocked())
		throw LogicError("Specification locked.");
	if(!dgFlow.p->f->readSpec().hasModule<OCSpec>())
		throw LogicError("Module not enabled.");
	dgFlow.p->f->writeSpec().getModule<OCSpec>().setForceExistence(value);
}

bool Model::OverallCatalysis::getForceExistence() const {
	if(!dgFlow.p->f->readSpec().hasModule<OCSpec>())
		throw LogicError("Module not enabled.");
	return dgFlow.p->f->readSpec().getModule<OCSpec>().getForceExistence();
}

void Model::OverallCatalysis::setStrictTransit(bool value) {
	if(dgFlow.isSpecificationLocked())
		throw LogicError("Specification locked.");
	if(!dgFlow.p->f->readSpec().hasModule<OCSpec>())
		throw LogicError("Module not enabled.");
	dgFlow.p->f->writeSpec().getModule<OCSpec>().setStrictTransit(value);
}

bool Model::OverallCatalysis::getStrictTransit() const {
	if(!dgFlow.p->f->readSpec().hasModule<OCSpec>())
		throw LogicError("Module not enabled.");
	return dgFlow.p->f->readSpec().getModule<OCSpec>().getStrictTransit();
}

//------------------------------------------------------------------------------
// Solution definition
//------------------------------------------------------------------------------

void Model::addEnumerationVar(Var var) {
	if(isSpecificationLocked())
		throw LogicError("Specification locked.");
	p->f->writeSpec().addEnumerationVar(std::move(var));
}

std::vector<Var> Model::getEnumerationVars() const {
	const auto &vars = p->f->readSpec().getEnumerationVars();
	std::vector<Var> res;
	res.reserve(vars.size());
	for(const auto &e: vars)
		res.push_back(e.v);
	return res;
}

void Model::addTransitEnumeration(dg::DG::Vertex v) {
	if(isSpecificationLocked())
		throw LogicError("Specification locked.");
	if(!v) throw LogicError("Can not add null vertex for transit enumeration.");
	if(v.getDG() != getDG())
		throw LogicError("The vertex does not belong to the underlying derivation graph.");
	using boost::vertices;
	const auto &dg = getDG()->getHyper().getGraph();
	const auto vHyper = vertices(dg).first[v.getId()];
	p->f->writeSpec().addTransitEnumeration(vHyper);
}

void Model::addTransitEnumeration(std::shared_ptr<graph::Graph> g) {
	addTransitEnumeration(getDG()->findVertex(g));
}

std::vector<dg::DG::Vertex> Model::getTransitEnumeration() const {
	const auto &enums = p->f->readSpec().getTransitEnumeration();
	const auto &dg = p->f->dg_->getHyper();
	std::vector<dg::DG::Vertex> res;
	res.reserve(enums.size());
	for(const auto e: enums) {
		const auto vOuter = dg.getInterfaceVertex(e.v);
		assert(vOuter);
		res.push_back(vOuter);
	}
	return res;
}

void Model::setAbsGap(int absGap) {
	if(isSpecificationLocked())
		throw LogicError("Specification locked.");
	p->f->writeSpec().setAbsGap(absGap);
}

std::optional<int> Model::getAbsGap() {
	return p->f->readSpec().getAbsGap();
}

//------------------------------------------------------------------------------

SolutionRange Model::findSolutions(int maxNumSolutions) {
	return findSolutions(maxNumSolutions, 1);
}

SolutionRange Model::findSolutions(int maxNumSolutions, int verbosity) {
	return findSolutions(maxNumSolutions, verbosity, 1);
}

SolutionRange Model::findSolutions(int maxNumSolutions, int verbosity, int ilpVerbosity) {
	if(maxNumSolutions < 0)
		throw LogicError("Too low value for maxNumSolutions ("
						 + std::to_string(maxNumSolutions) +
						 "). Must be at least 0.");
	if(!isSpecificationLocked())
		p->f->makeModel(verbosity);
	const auto res = p->f->getModel().findSolutions(maxNumSolutions, verbosity, ilpVerbosity);
	std::cout << std::flush;
	const auto all = p->f->getModel().getSolutions();
	return SolutionRange(std::const_pointer_cast<Model>(this->shared_from_this()),
	                     res.begin() - all.begin(), res.end() - all.begin());
}

std::string Model::dump() const {
	return dump("");
}

std::string Model::dump(const std::string &filename) const {
	if(filename.empty()) {
		std::string name = lib::IO::makeUniqueFilePrefix() + "Flow.flow";
		lib::IO::writeJsonFile(name, getFlow().dump());
		return name;
	} else {
		lib::IO::writeJsonFile(filename, getFlow().dump());
		return filename;
	}
}

//------------------------------------------------------------------------------

SolutionRange Model::getSolutions() const {
	if(!isSpecificationLocked())
		throw LogicError("Can not retrieve solutions before calculation.");
	return SolutionRange(std::const_pointer_cast<Model>(this->shared_from_this()),
	                     0, p->f->getModel().getSolutions().size());
}

ModelImplementationView Model::getImplementationView() const {
	return ModelImplementationView(std::const_pointer_cast<Model>(this->shared_from_this()));
}

//------------------------------------------------------------------------------
// Static
//------------------------------------------------------------------------------

std::shared_ptr<Model> Model::make(std::shared_ptr<dg::DG> dg) {
	return make(dg, "default");
}

std::shared_ptr<Model> Model::make(std::shared_ptr<dg::DG> dg, const std::string &ilpSolver) {
	if(!dg)
		throw LogicError("Can not create model from null pointer.");
	if(!dg->isLocked())
		throw LogicError("Can not create model with unlocked DG.");
	if(dg->numVertices() == 0)
		throw LogicError("Can not create model with an empty DG.");
	auto f = std::make_unique<lib::HyperFlow::Flow>(dg, ilpSolver);
	return std::shared_ptr<Model>(new Model(std::move(f)));
}

std::shared_ptr<Model> Model::copy(std::shared_ptr<Model> other) {
	if(!other) throw LogicError("Can not create model from null pointer.");
	auto json = other->p->f->dump();
	json["solutions"] = nullptr;
	auto f = std::make_unique<lib::HyperFlow::Flow>(other->getDG(), other->p->f->ilpSolver);
	std::ostringstream err;
	const auto res = lib::HyperFlow::Flow::load(*f, std::move(json), 0, err);
	if(!res) throw FatalError("Internal error in hyperflow model copying. Details: " + err.str());
	assert(f->isSpecWritable());
	return std::shared_ptr<Model>(new Model(std::move(f)));
}

std::shared_ptr<Model> Model::load(std::shared_ptr<dg::DG> dg, const std::string &file) {
	return load(dg, "default", file, 1);
}

std::shared_ptr<Model>
Model::load(std::shared_ptr<dg::DG> dg, const std::string &file, const std::string &ilpSolver, int verbosity) {
	if(!dg)
		throw LogicError("Can not load model with null pointer DG.");
	if(!dg->isLocked())
		throw LogicError("Can not load model with unlocked DG.");
	std::ifstream fileStream(file.c_str());
	if(!fileStream.is_open()) throw InputError("Hyperflow dump file not found, '" + file + "'\n");
	std::ostringstream err;
	auto libFlow = lib::HyperFlow::Read::dump(dg, ilpSolver, fileStream, err, verbosity);
	if(!libFlow) throw InputError("Hyperflow load error: " + err.str());
	return std::shared_ptr<Model>(new Model(std::move(libFlow)));
}

std::shared_ptr<Model> Model::loadString(std::shared_ptr<dg::DG> dg, const std::string &data) {
	return loadString(dg, data, "default", 1);
}

std::shared_ptr<Model>
Model::loadString(std::shared_ptr<dg::DG> dg, const std::string &data, const std::string &ilpSolver, int verbosity) {
	if(!dg)
		throw LogicError("Can not load model with null pointer DG.");
	if(!dg->isLocked())
		throw LogicError("Can not load model with unlocked DG.");
	std::istringstream s(data);
	std::ostringstream err;
	auto libFlow = lib::HyperFlow::Read::dump(dg, ilpSolver, s, err, verbosity);
	if(!libFlow) throw InputError(err.str());
	return std::shared_ptr<Model>(new Model(std::move(libFlow)));
}

} // namespace mod::hyperflow