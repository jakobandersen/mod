#include "DerivationPredicates.hpp"

#include <mod/Config.hpp>
#include <mod/Function.hpp>

namespace mod::lib::DG::Strategies {

//------------------------------------------------------------------------------
// DerivationPredicate
//------------------------------------------------------------------------------

DerivationPredicate::DerivationPredicate(std::shared_ptr<mod::Function<bool(const mod::Derivation &)>> predicate,
                                         std::unique_ptr<Strategy> strat)
		: Strategy(strat->getMaxComponents()), predicate(predicate), strat(std::move(strat)) {}

DerivationPredicate::~DerivationPredicate() = default;

void DerivationPredicate::preAddGraphs(std::function<void(std::shared_ptr<mod::graph::Graph>,
                                                          IsomorphismPolicy)> add) const {
	strat->preAddGraphs(add);
}

void DerivationPredicate::forEachRule(std::function<void(const lib::rule::Rule &)> f) const {
	strat->forEachRule(f);
}

void DerivationPredicate::printInfo(PrintSettings settings) const {
	settings.indent();
	std::ostream &s = settings.s;
	printName(settings.s);
	s << ":" << std::endl;
	++settings.indentLevel;
	settings.indent() << "predicate = ";
	predicate->print(s);
	s << '\n';
	strat->printInfo(settings);
	printBaseInfo(settings);
}

const GraphState &DerivationPredicate::getOutput() const {
	return strat->getOutput();
}

bool DerivationPredicate::isConsumed(const lib::graph::Graph *g) const {
	return strat->isConsumed(g);
}

void DerivationPredicate::setExecutionEnvImpl() {
	strat->setExecutionEnv(getExecutionEnv());
}

void DerivationPredicate::executeImpl(PrintSettings settings, const GraphState &input) {
	if(settings.verbosity >= PrintSettings::V_DerivationPredicates) {
		settings.indent();
		std::ostream &s = settings.s;
		printName(s);
		s << ": ";
		if(settings.verbosity >= PrintSettings::V_DerivationPredicatesPred)
			predicate->print(s);
		s << std::endl;
		++settings.indentLevel;
	}
	pushPredicate(predicate);
	strat->execute(settings, input);
	popPredicate();
}

//------------------------------------------------------------------------------
// LeftPredicate
//------------------------------------------------------------------------------

LeftPredicate::LeftPredicate(std::shared_ptr<mod::Function<bool(const mod::Derivation &)>> predicate,
                             std::unique_ptr<Strategy> strat)
		: DerivationPredicate(predicate, std::move(strat)) {}

std::unique_ptr<Strategy> LeftPredicate::clone() const {
	return std::make_unique<LeftPredicate>(predicate->clone(), strat->clone());
}

void LeftPredicate::printName(std::ostream &s) const {
	s << "LeftPredicate";
}

void LeftPredicate::pushPredicate(std::shared_ptr<mod::Function<bool(const mod::Derivation &)> > pred) {
	getExecutionEnv().pushLeftPredicate(pred);
}

void LeftPredicate::popPredicate() {
	getExecutionEnv().popLeftPredicate();
}

//------------------------------------------------------------------------------
// RightPredicate
//------------------------------------------------------------------------------

RightPredicate::RightPredicate(std::shared_ptr<mod::Function<bool(const mod::Derivation &)>> predicate,
                               std::unique_ptr<Strategy> strat)
		: DerivationPredicate(predicate, std::move(strat)) {}

std::unique_ptr<Strategy> RightPredicate::clone() const {
	return std::make_unique<RightPredicate>(predicate->clone(), strat->clone());
}

void RightPredicate::printName(std::ostream &s) const {
	s << "RightPredicate";
}

void RightPredicate::pushPredicate(std::shared_ptr<mod::Function<bool(const mod::Derivation &)> > pred) {
	getExecutionEnv().pushRightPredicate(pred);
}

void RightPredicate::popPredicate() {
	getExecutionEnv().popRightPredicate();
}

} // namespace mod::lib::DG::Strategies