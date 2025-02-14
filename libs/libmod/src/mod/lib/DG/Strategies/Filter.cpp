#include "Filter.hpp"

#include <mod/Config.hpp>
#include <mod/Function.hpp>
#include <mod/lib/DG/Strategies/GraphState.hpp>
#include <mod/lib/Graph/Graph.hpp>

namespace mod::lib::DG::Strategies {

Filter::Filter(std::shared_ptr<mod::Function<bool(std::shared_ptr<mod::graph::Graph>, const dg::Strategy::GraphState &,
                                                  bool)> > filterFunc, bool filterUniverse)
		: Strategy(1), filterFunc(filterFunc), filterUniverse(filterUniverse) {
	assert(filterFunc);
}

std::unique_ptr<Strategy> Filter::clone() const {
	return std::make_unique<Filter>(filterFunc->clone(), filterUniverse);
}

void Filter::printInfo(PrintSettings settings) const {
	settings.indent() << "Filter";
	std::ostream &s = settings.s;
	if(filterUniverse) s << "Universe";
	else s << "Subset";
	s << '\n';
	++settings.indentLevel;
	settings.indent() << "predicate = ";
	filterFunc->print(s);
	s << '\n';
	printBaseInfo(settings);
	--settings.indentLevel;
}

bool Filter::isConsumed(const lib::graph::Graph *g) const {
	return false;
}

void Filter::executeImpl(PrintSettings settings, const GraphState &input) {
	if(settings.verbosity >= PrintSettings::V_Filter) {
		if(filterUniverse)
			settings.indent() << "FilterUniverse: ";
		else
			settings.indent() << "FilterSubset: ";
		if(settings.verbosity >= PrintSettings::V_FilterPred)
			filterFunc->print(settings.s);
		settings.s << std::endl;
		++settings.indentLevel;
	}
	assert(!output);
	dg::Strategy::GraphState graphState(
			[&input](std::vector<std::shared_ptr<mod::graph::Graph> > &subset) {
				for(const lib::graph::Graph *g : input.getSubset())
					subset.push_back(g->getAPIReference());
			},
			[&input](std::vector<std::shared_ptr<mod::graph::Graph> > &universe) {
				for(const lib::graph::Graph *g : input.getUniverse())
					universe.push_back(g->getAPIReference());
			});
	if(!filterUniverse) {
		output = new GraphState(input.getUniverse());

		bool first = true;
		for(const lib::graph::Graph *g : input.getSubset()) {
			// TODO: the adding to the subset could be faster as the index is the same as in the input ResultSet
			if((*filterFunc)(g->getAPIReference(), graphState, first)) {
				assert(output->isInUniverse(g));
				output->addToSubset(g);
			}
			first = false;
		}
	} else {
		std::map<const lib::graph::Graph *, bool> copyToOutput;
		bool first = true;
		for(const lib::graph::Graph *g : input.getUniverse()) {
			copyToOutput[g] = (*filterFunc)(g->getAPIReference(), graphState, first);
			first = false;
		}
		std::vector<const lib::graph::Graph *> newUniverse;

		for(const lib::graph::Graph *g : input.getUniverse()) {
			if(copyToOutput[g]) newUniverse.push_back(g);
		}
		output = new GraphState(newUniverse);

		for(const lib::graph::Graph *g : input.getSubset()) {
			// TODO: this could be faster if we had access to the individual indices, as we could map old to new
			if(copyToOutput[g]) {
				assert(output->isInUniverse(g));
				output->addToSubset(g);
			}
		}
	}
}

} // namespace mod::lib::DG::Strategies