#include "Repeat.hpp"

#include <mod/Config.hpp>
#include <mod/lib/DG/Strategies/GraphState.hpp>

#include <ostream>

namespace mod::lib::DG::Strategies {

Repeat::Repeat(std::unique_ptr<Strategy> strat, int limit)
		: Strategy(strat->getMaxComponents()), strat(std::move(strat)), limit(limit) {
	assert(limit >= 0);
}

Repeat::~Repeat() = default;

std::unique_ptr<Strategy> Repeat::clone() const {
	return std::make_unique<Repeat>(strat->clone(), limit);
}

void Repeat::preAddGraphs(std::function<void(std::shared_ptr<mod::graph::Graph>, IsomorphismPolicy)> add) const {
	strat->preAddGraphs(add);
}

void Repeat::forEachRule(std::function<void(const lib::rule::Rule &)> f) const {
	strat->forEachRule(f);
}

void Repeat::printInfo(PrintSettings settings) const {
	settings.indent() << "Repeat, limit = " << limit << '\n';
	++settings.indentLevel;
	for(int i = 0; i != subStrats.size(); i++) {
		settings.indent() << "Round " << (i + 1) << ":\n";
		++settings.indentLevel;
		subStrats[i]->printInfo(settings);
		--settings.indentLevel;
	}
	printBaseInfo(settings);
}

const GraphState &Repeat::getOutput() const {
	if(subStrats.empty()) return *input;
	if(subStrats.back()->getOutput().getSubset().empty()) {
		if(subStrats.size() == 1) return *input;
		else return subStrats[subStrats.size() - 2]->getOutput();
	} else return subStrats.back()->getOutput();
}

bool Repeat::isConsumed(const lib::graph::Graph *g) const {
	for(const auto &s : subStrats)
		if(s->isConsumed(g)) return true;
	return false;
}

void Repeat::setExecutionEnvImpl() {
	strat->setExecutionEnv(getExecutionEnv());
}

void Repeat::executeImpl(PrintSettings settings, const GraphState &input) {
	if(settings.verbosity >= PrintSettings::V_Repeat)
		settings.indent() << "Repeat, limit = " << limit << std::endl;
	assert(limit >= 0);
	++settings.indentLevel;
	for(int i = 0; i != limit; ++i) {
		{
			auto subStrat = strat->clone();
			subStrat->setExecutionEnv(getExecutionEnv());
			if(settings.verbosity >= PrintSettings::V_Repeat)
				settings.indent() << "Round " << (i + 1) << ":" << std::endl;
			++settings.indentLevel;
			if(i == 0)
				subStrat->execute(settings, input);
			else
				subStrat->execute(settings, subStrats[i - 1]->getOutput());
			--settings.indentLevel;

			if(settings.verbosity >= PrintSettings::V_Repeat) {
				settings.indent() << "Round " << (i + 1) << ": Result subset has "
				                  << subStrat->getOutput().getSubset().size()
				                  << " graphs." << std::endl;
			}
			subStrats.push_back(std::move(subStrat));
		}
		if(subStrats.back()->getOutput().getSubset().empty()) {
			if(settings.verbosity >= PrintSettings::V_RepeatBreak)
				settings.indent() << "Round " << (i + 1) << ": Breaking repeat due to empty subset." << std::endl;
			break;
		}
		if(!getConfig().dg.disableRepeatFixedPointCheck) {
			if(i > 0) {
				if(subStrats.back()->getOutput() == subStrats[i - 1]->getOutput()) {
					if(settings.verbosity >= PrintSettings::V_RepeatBreak)
						settings.indent() << "Round " << (i + 1) << ": Breaking repeat due to fixed point." << std::endl;
					break;
				}
			}
		}
	}
}

} // namespace mod::lib::DG::Strategies