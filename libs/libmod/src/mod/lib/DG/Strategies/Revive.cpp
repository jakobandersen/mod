#include "Revive.hpp"

#include <mod/Config.hpp>
#include <mod/lib/DG/Strategies/GraphState.hpp>
#include <mod/lib/Graph/Graph.hpp>

#include <ostream>

namespace mod::lib::DG::Strategies {

Revive::Revive(std::unique_ptr<Strategy> strat) : Strategy(strat->getMaxComponents()), strat(std::move(strat)) {}

Revive::~Revive() = default;

std::unique_ptr<Strategy> Revive::clone() const {
	return std::make_unique<Revive>(strat->clone());
}

void Revive::preAddGraphs(std::function<void(std::shared_ptr<mod::graph::Graph>, IsomorphismPolicy)> add) const {
	strat->preAddGraphs(add);
}

void Revive::forEachRule(std::function<void(const lib::rule::Rule &)> f) const {
	strat->forEachRule(f);
}

void Revive::printInfo(PrintSettings settings) const {
	settings.indent() << "Revive:\n";
	++settings.indentLevel;
	strat->printInfo(settings);
	printBaseInfo(settings);
	settings.indent() << "revived =";
	for(const auto *g : revivedGraphs)
		settings.s << " " << g->getName();
	settings.s << '\n';
}

bool Revive::isConsumed(const lib::graph::Graph *g) const {
	return strat->isConsumed(g);
}

void Revive::setExecutionEnvImpl() {
	strat->setExecutionEnv(getExecutionEnv());
}

void Revive::executeImpl(PrintSettings settings, const GraphState &input) {
	if(settings.verbosity >= PrintSettings::V_Revive) {
		settings.indent() << "Revive:" << std::endl;
		++settings.indentLevel;
	}
	strat->execute(settings, input);
	output = new GraphState(strat->getOutput());

	for(const auto *g : input.getSubset()) {
		if(!strat->isConsumed(g) && output->isInUniverse(g)) {
			output->addToSubset(g);
			revivedGraphs.push_back(g);
		}
	}
}

} // namespace mod::lib::DG::Strategies