#include "Sequence.hpp"

#include <mod/Config.hpp>

namespace mod::lib::DG::Strategies {

Sequence::Sequence(std::vector<std::unique_ptr<Strategy>> strats)
		: Strategy(calcMaxNumComponents(strats)), strats(std::move(strats)) {
	assert(!this->strats.empty());
}

Sequence::~Sequence() = default;

std::unique_ptr<Strategy> Sequence::clone() const {
	std::vector<std::unique_ptr<Strategy>> subStrats;
	for(const auto &s : strats) subStrats.push_back(s->clone());
	return std::make_unique<Sequence>(std::move(subStrats));
}

void Sequence::preAddGraphs(std::function<void(std::shared_ptr<mod::graph::Graph>, IsomorphismPolicy)> add) const {
	for(const auto &s : strats) s->preAddGraphs(add);
}

void Sequence::forEachRule(std::function<void(const lib::rule::Rule &)> f) const {
	for(const auto &s : strats) s->forEachRule(f);
}

void Sequence::printInfo(PrintSettings settings) const {
	settings.indent() << "Sequence: " << strats.size() << " substrategies\n";
	++settings.indentLevel;
	for(int i = 0; i != strats.size(); i++) {
		settings.indent() << "Substrategy " << (i + 1) << ":\n";
		++settings.indentLevel;
		strats[i]->printInfo(settings);
		--settings.indentLevel;
	}
	printBaseInfo(settings);
}

const GraphState &Sequence::getOutput() const {
	return strats.back()->getOutput();
}

bool Sequence::isConsumed(const lib::graph::Graph *g) const {
	for(const auto &s : strats)
		if(s->isConsumed(g)) return true;
	return false;
}

void Sequence::setExecutionEnvImpl() {
	for(const auto &s : strats) s->setExecutionEnv(getExecutionEnv());
}

void Sequence::executeImpl(PrintSettings settings, const GraphState &input) {
	if(settings.verbosity >= PrintSettings::V_Sequence) {
		settings.indent() << "Sequence: " << strats.size() << " substrategies" << std::endl;
		++settings.indentLevel;
	}
	for(int i = 0; i != strats.size(); ++i) {
		auto &s = strats[i];
		if(settings.verbosity >= PrintSettings::V_Sequence) {
			settings.indent() << "Sequence, substrategy " << (i + 1) << ":" << std::endl;
			++settings.indentLevel;
		}
		if(i == 0)
			s->execute(settings, input);
		else
			s->execute(settings, strats[i - 1]->getOutput());
		if(settings.verbosity >= PrintSettings::V_Sequence)
			--settings.indentLevel;
	}
}

} // namespace mob::lib::DG::Strategies