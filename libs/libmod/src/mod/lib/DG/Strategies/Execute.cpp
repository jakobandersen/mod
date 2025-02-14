#include "Execute.hpp"

#include <mod/Config.hpp>
#include <mod/Function.hpp>
#include <mod/lib/DG/Strategies/GraphState.hpp>
#include <mod/lib/Graph/Graph.hpp>

namespace mod::lib::DG::Strategies {

Execute::Execute(std::shared_ptr<mod::Function<void(const dg::Strategy::GraphState &)> > func)
		: Strategy::Strategy(0), func(func) {}

std::unique_ptr<Strategy> Execute::clone() const {
	return std::make_unique<Execute>(func->clone());
}

void Execute::preAddGraphs(std::function<void(std::shared_ptr<mod::graph::Graph>, IsomorphismPolicy)> add) const {}

void Execute::printInfo(PrintSettings settings) const {
	settings.indent() << "Execute:\n";
	++settings.indentLevel;
	settings.indent() << "function = ";
	func->print(settings.s);
	settings.s << '\n';
}

const GraphState &Execute::getOutput() const {
	return *input;
}

bool Execute::isConsumed(const lib::graph::Graph *g) const {
	return false;
}

void Execute::executeImpl(PrintSettings settings, const GraphState &input) {
	if(settings.verbosity >= PrintSettings::V_Execute)
		printInfo(settings);
	dg::Strategy::GraphState gs(
			[&input](std::vector<std::shared_ptr<mod::graph::Graph> > &subset) {
				for(const lib::graph::Graph *g : input.getSubset())
					subset.push_back(g->getAPIReference());
			},
			[&input](std::vector<std::shared_ptr<mod::graph::Graph> > &universe) {
				for(const lib::graph::Graph *g : input.getUniverse())
					universe.push_back(g->getAPIReference());
			});
	(*func)(gs);
}

} // namespace mod::lib::DG::Strategies