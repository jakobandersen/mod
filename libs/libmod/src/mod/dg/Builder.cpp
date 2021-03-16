#include "Builder.hpp"

#include <mod/Error.hpp>
#include <mod/dg/Strategies.hpp>
#include <mod/lib/DG/Hyper.hpp>
#include <mod/lib/DG/NonHyperBuilder.hpp>
#include <mod/lib/DG/Strategies/GraphState.hpp>
#include <mod/lib/DG/Strategies/Strategy.hpp>
#include <mod/lib/Graph/Single.hpp>
#include <mod/lib/IO/DG.hpp>

#include <boost/lexical_cast.hpp>

#include <fstream>

namespace mod::dg {

struct Builder::Pimpl {
	Pimpl(std::shared_ptr<DG> dg_, lib::DG::NonHyperBuilder &dgLib) : dg_(dg_), b(dgLib.build()) {}
public:
	std::shared_ptr<DG> dg_;
	lib::DG::Builder b;
};

Builder::Builder(lib::DG::NonHyperBuilder &dg_) : p(new Pimpl(dg_.getAPIReference(), dg_)) {}

Builder::Builder(Builder &&other) = default;
Builder &Builder::operator=(Builder &&other) = default;
Builder::~Builder() = default;

bool Builder::isActive() const {
	return p != nullptr;
}

namespace {

template<typename T>
// because Pimpl is private
void check(const T &p) {
	if(!p) throw LogicError("The builder is not active.");
}

} // namespace

DG::HyperEdge Builder::addDerivation(const Derivations &d) {
	return addDerivation(d, IsomorphismPolicy::Check);
}

DG::HyperEdge Builder::addDerivation(const Derivations &d, IsomorphismPolicy graphPolicy) {
	check(p);
	if(d.left.empty()) throw LogicError("Derivation has empty left side: " + boost::lexical_cast<std::string>(d));
	if(d.right.empty()) throw LogicError("Derivation has empty right side: " + boost::lexical_cast<std::string>(d));
	const auto checkPtr = [](const auto &p) {
		return !p;
	};
	if(std::any_of(d.left.begin(), d.left.end(), checkPtr))
		throw LogicError("Derivation has a nullptr in the left side: " + boost::lexical_cast<std::string>(d));
	if(std::any_of(d.right.begin(), d.right.end(), checkPtr))
		throw LogicError("Derivation has a nullptr in the right side: " + boost::lexical_cast<std::string>(d));
	if(std::any_of(d.rules.begin(), d.rules.end(), checkPtr))
		throw LogicError("Derivation has a nullptr in the rule list: " + boost::lexical_cast<std::string>(d));
	auto innerRes = p->b.addDerivation(d, graphPolicy);
	return p->dg_->getHyper().getInterfaceEdge(p->dg_->getNonHyper().getHyperEdge(innerRes.first));
}

ExecuteResult Builder::execute(std::shared_ptr<Strategy> strategy) {
	return execute(strategy, 1);
}

ExecuteResult Builder::execute(std::shared_ptr<Strategy> strategy, int verbosity) {
	return execute(strategy, verbosity, false);
}

ExecuteResult Builder::execute(std::shared_ptr<Strategy> strategy, int verbosity, bool ignoreRuleLabelTypes) {
	check(p);
	if(!strategy) throw LogicError("The strategy may not be a null pointer.");
	auto res = p->b.execute(std::unique_ptr<lib::DG::Strategies::Strategy>(strategy->getStrategy().clone()),
	                        verbosity, ignoreRuleLabelTypes);
	return ExecuteResult(p->dg_, std::move(res));
}

std::vector<DG::HyperEdge> Builder::apply2(const std::vector<std::shared_ptr<graph::Graph> > &graphs,
                                          std::shared_ptr<rule::Rule> r) {
	auto innerRes = p->b.apply2(graphs, r, 0, IsomorphismPolicy::Check);
	std::vector<DG::HyperEdge> res;
	const auto &nonHyper = p->dg_->getNonHyper();
	const auto &hyper = p->dg_->getHyper();
	for(const auto &rp : innerRes)
		res.push_back(hyper.getInterfaceEdge(nonHyper.getHyperEdge(rp.first)));
	return res;
}

std::vector<DG::HyperEdge> Builder::apply(const std::vector<std::shared_ptr<graph::Graph> > &graphs,
                                          std::shared_ptr<rule::Rule> r) {
	return apply(graphs, r, true, 0);
}
std::vector<DG::HyperEdge> Builder::apply(const std::vector<std::shared_ptr<graph::Graph> > &graphs,
                                          std::shared_ptr<rule::Rule> r, bool onlyProper,
                                          int verbosity) {
	return apply(graphs, r, onlyProper, verbosity, IsomorphismPolicy::Check);
}

std::vector<DG::HyperEdge> Builder::apply(const std::vector<std::shared_ptr<graph::Graph> > &graphs,
                                          std::shared_ptr<rule::Rule> r, bool onlyProper,
                                          int verbosity, IsomorphismPolicy graphPolicy) {
	check(p);
	if(std::any_of(graphs.begin(), graphs.end(), [](const auto &p) {
		return !p;
	}))
		throw LogicError("One of the graphs is a null pointer.");
	if(!r) throw LogicError("The rule is a null pointer.");
	auto innerRes = onlyProper
			? p->b.apply(graphs, r, verbosity, graphPolicy)
			: p->b.applyRelaxed(graphs, r, verbosity, graphPolicy);
	std::vector<DG::HyperEdge> res;
	const auto &nonHyper = p->dg_->getNonHyper();
	const auto &hyper = p->dg_->getHyper();
	for(const auto &rp : innerRes)
		res.push_back(hyper.getInterfaceEdge(nonHyper.getHyperEdge(rp.first)));
	return res;
}

void Builder::addAbstract(const std::string &description) {
	check(p);
	p->b.addAbstract(description);
}

void Builder::load(const std::vector<std::shared_ptr<rule::Rule>> &ruleDatabase,
                   const std::string &file, int verbosity) {
	if(std::any_of(ruleDatabase.begin(), ruleDatabase.end(), [](const auto &r) {
		return !r;
	}))
		throw LogicError("Nullptr in rule database.");

	std::ifstream fileStream(file.c_str());
	if(!fileStream.is_open()) throw InputError("DG dump file not found, '" + file + "'.");
	fileStream.close();
	std::ostringstream err;
	const bool res = p->b.load(ruleDatabase, file, err, verbosity);
	if(!res) throw InputError("DG load error: " + err.str());
}

lib::DG::Builder& Builder::getLibBuilder() {
	return p->b;
}

std::shared_ptr<dg::DG> Builder::getDG() {
	return p->dg_;
}

// -----------------------------------------------------------------------------

struct ExecuteResult::Pimpl {
	Pimpl(std::shared_ptr<DG> dg_, lib::DG::ExecuteResult res) : dg_(dg_), res(std::move(res)) {}
public:
	std::shared_ptr<DG> dg_;
	lib::DG::ExecuteResult res;
	std::vector<std::shared_ptr<graph::Graph>> subset;
	std::vector<std::shared_ptr<graph::Graph>> universe;
};

ExecuteResult::ExecuteResult(std::shared_ptr<DG> dg_, lib::DG::ExecuteResult innerRes)
		: p(new Pimpl(dg_, std::move(innerRes))) {}

ExecuteResult::ExecuteResult(ExecuteResult &&other) = default;
ExecuteResult &ExecuteResult::operator=(ExecuteResult &&other) = default;
ExecuteResult::~ExecuteResult() = default;

const std::vector<std::shared_ptr<graph::Graph>> &ExecuteResult::getSubset() const {
	if(p->subset.empty()) {
		const auto &inner = p->res.getResult().getSubset();
		p->subset.reserve(inner.size());
		for(const auto *g : inner)
			p->subset.push_back(g->getAPIReference());
	}
	return p->subset;
}
const std::vector<std::shared_ptr<graph::Graph>> &ExecuteResult::getUniverse() const {
	if(p->universe.empty()) {
		const auto &inner = p->res.getResult().getUniverse();
		p->universe.reserve(inner.size());
		for(const auto *g : inner)
			p->universe.push_back(g->getAPIReference());
	}
	return p->universe;
}

void ExecuteResult::list(bool withUniverse) const {
	p->res.list(withUniverse);
}

} // namespace mod::dg
