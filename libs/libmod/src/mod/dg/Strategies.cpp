#include "Strategies.hpp"

#include <mod/Error.hpp>
#include <mod/rule/Rule.hpp>
#include <mod/lib/DG/Strategies/Strategy.hpp>
#include <mod/lib/DG/Strategies/Add.hpp>
#include <mod/lib/DG/Strategies/DerivationPredicates.hpp>
#include <mod/lib/DG/Strategies/Execute.hpp>
#include <mod/lib/DG/Strategies/Filter.hpp>
#include <mod/lib/DG/Strategies/Parallel.hpp>
#include <mod/lib/DG/Strategies/Repeat.hpp>
#include <mod/lib/DG/Strategies/Revive.hpp>
#include <mod/lib/DG/Strategies/Rule.hpp>
#include <mod/lib/DG/Strategies/Sequence.hpp>

#include <ostream>

namespace mod::dg {

//------------------------------------------------------------------------------
// GraphState
//------------------------------------------------------------------------------

Strategy::GraphState::GraphState(std::function<void(std::vector<std::shared_ptr<graph::Graph>> &)> fSubset,
                                 std::function<void(std::vector<std::shared_ptr<graph::Graph>> &)> fUniverse)
		: fSubset(fSubset), fUniverse(fUniverse) {}

const std::vector<std::shared_ptr<graph::Graph>> &Strategy::GraphState::getSubset() const {
	if(!subsetInit) {
		fSubset(subset);
		subsetInit = true;
	}
	return subset;
}

const std::vector<std::shared_ptr<graph::Graph>> &Strategy::GraphState::getUniverse() const {
	if(!universeInit) {
		fUniverse(universe);
		universeInit = true;
	}
	return universe;
}

//------------------------------------------------------------------------------
// Strategy
//------------------------------------------------------------------------------

struct Strategy::Pimpl {
	Pimpl(std::unique_ptr<lib::DG::Strategies::Strategy> strategy) : strategy(std::move(strategy)) {
		assert(this->strategy);
	}
public:
	const std::unique_ptr<lib::DG::Strategies::Strategy> strategy;
};

Strategy::Strategy(std::unique_ptr<lib::DG::Strategies::Strategy> strategy)
		: p(std::make_unique<Pimpl>(std::move(strategy))) {}

Strategy::~Strategy() = default;

std::unique_ptr<Strategy> Strategy::clone() const {
	return std::make_unique<Strategy>(p->strategy->clone());
}

std::ostream &operator<<(std::ostream &s, const Strategy &strat) {
	return s << "Strategy";
}

lib::DG::Strategies::Strategy &Strategy::getStrategy() {
	return *p->strategy;
}

//------------------------------------------------------------------------------
// Static
//------------------------------------------------------------------------------

std::shared_ptr<Strategy>
Strategy::makeAdd(bool onlyUniverse, const std::vector<std::shared_ptr<graph::Graph>> &graphs,
                  IsomorphismPolicy graphPolicy) {
	if(std::any_of(graphs.begin(), graphs.end(), [](const auto &g) {
		return !g;
	}))
		throw LogicError("One of the graphs is a null pointer.");
	return std::make_unique<Strategy>(std::make_unique<lib::DG::Strategies::Add>(graphs, onlyUniverse, graphPolicy));
}

std::shared_ptr<Strategy> Strategy::makeAdd(
		bool onlyUniverse,
		const std::shared_ptr<mod::Function<std::vector<std::shared_ptr<graph::Graph> >()>> generator,
		IsomorphismPolicy graphPolicy) {
	return std::make_unique<Strategy>(std::make_unique<lib::DG::Strategies::Add>(generator, onlyUniverse, graphPolicy));
}

std::shared_ptr<Strategy> Strategy::makeSequence(const std::vector<std::shared_ptr<Strategy>> &strategies) {
	if(strategies.empty())
		throw LogicError("Can not create an empty-length sequence strategy.");
	if(std::any_of(strategies.begin(), strategies.end(), [](const auto &p) {
		return !p;
	}))
		throw LogicError("One of the strategies is a null pointer.");
	std::vector<std::unique_ptr<lib::DG::Strategies::Strategy>> cloned;
	for(const auto &strat : strategies) cloned.push_back(strat->getStrategy().clone());
	return std::make_unique<Strategy>(std::make_unique<lib::DG::Strategies::Sequence>(std::move(cloned)));
}

std::shared_ptr<Strategy> Strategy::makeParallel(const std::vector<std::shared_ptr<Strategy>> &strategies) {
	if(strategies.empty())
		throw LogicError("Can not create parallel strategy without any substrategies.");
	if(std::any_of(strategies.begin(), strategies.end(), [](const auto &p) {
		return !p;
	}))
		throw LogicError("One of the strategies is a null pointer.");
	std::vector<std::unique_ptr<lib::DG::Strategies::Strategy>> cloned;
	for(const auto &strat : strategies) cloned.push_back(strat->getStrategy().clone());
	return std::make_unique<Strategy>(std::make_unique<lib::DG::Strategies::Parallel>(std::move(cloned)));
}

std::shared_ptr<Strategy> Strategy::makeFilter(
		bool alsoUniverse, std::shared_ptr<mod::Function<bool(std::shared_ptr<graph::Graph>,
		                                                      const Strategy::GraphState &,
		                                                      bool)>> filterFunc) {
	if(!filterFunc) throw LogicError("The predicate is a null pointer.");
	return std::make_unique<Strategy>(std::make_unique<lib::DG::Strategies::Filter>(filterFunc, alsoUniverse));
}

std::shared_ptr<Strategy>
Strategy::makeExecute(std::shared_ptr<mod::Function<void(const Strategy::GraphState &)>> func) {
	if(!func) throw LogicError("The callback is a null pointer.");
	return std::make_unique<Strategy>(std::make_unique<lib::DG::Strategies::Execute>(func));
}

std::shared_ptr<Strategy> Strategy::makeRule(std::shared_ptr<rule::Rule> rule) {
	if(!rule) throw LogicError("The rule is a null pointer.");
	return std::make_unique<Strategy>(std::make_unique<lib::DG::Strategies::Rule>(rule));
}

std::shared_ptr<Strategy>
Strategy::makeLeftPredicate(std::shared_ptr<mod::Function<bool(const mod::Derivation &)>> predicate,
                            std::shared_ptr<Strategy> strategy) {
	if(!predicate) throw LogicError("The predicate is a null pointer.");
	if(!strategy) throw LogicError("The substrategy is a null pointer.");
	return std::make_unique<Strategy>(
			std::make_unique<lib::DG::Strategies::LeftPredicate>(predicate, strategy->getStrategy().clone()));
}

std::shared_ptr<Strategy>
Strategy::makeRightPredicate(std::shared_ptr<mod::Function<bool(const mod::Derivation &)>> predicate,
                             std::shared_ptr<Strategy> strategy) {
	if(!predicate) throw LogicError("The predicate is a null pointer.");
	if(!strategy) throw LogicError("The substrategy is a null pointer.");
	return std::make_unique<Strategy>(
			std::make_unique<lib::DG::Strategies::RightPredicate>(predicate, strategy->getStrategy().clone()));
}

std::shared_ptr<Strategy> Strategy::makeRevive(std::shared_ptr<Strategy> strategy) {
	if(!strategy) throw LogicError("The substrategy is a null pointer.");
	return std::make_unique<Strategy>(std::make_unique<lib::DG::Strategies::Revive>(strategy->getStrategy().clone()));
}

std::shared_ptr<Strategy> Strategy::makeRepeat(int limit, std::shared_ptr<Strategy> strategy) {
	if(limit < 0) throw LogicError("Limit must be non-negative.");
	if(!strategy) throw LogicError("The substrategy is a null pointer.");
	return std::make_unique<Strategy>(
			std::make_unique<lib::DG::Strategies::Repeat>(strategy->getStrategy().clone(), limit));
}

} // namespace mod::dg