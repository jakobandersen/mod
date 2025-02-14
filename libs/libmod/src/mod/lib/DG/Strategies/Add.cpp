#include "Add.hpp"

#include <mod/Error.hpp>
#include <mod/Function.hpp>
#include <mod/graph/Graph.hpp>
#include <mod/lib/DG/Strategies/GraphState.hpp>

#include <ostream>

namespace mod::lib::DG::Strategies {

Add::Add(const std::vector<std::shared_ptr<mod::graph::Graph> > graphs, bool onlyUniverse, IsomorphismPolicy graphPolicy)
		: Strategy::Strategy(0),
		  graphs(graphs), onlyUniverse(onlyUniverse), graphPolicy(graphPolicy) {}

Add::Add(const std::shared_ptr<mod::Function<std::vector<std::shared_ptr<mod::graph::Graph> >()> > generator,
         bool onlyUniverse, IsomorphismPolicy graphPolicy)
		: Strategy::Strategy(0),
		  generator(generator), onlyUniverse(onlyUniverse), graphPolicy(graphPolicy) {}

Add::~Add() = default;

std::unique_ptr<Strategy> Add::clone() const {
	if(!generator)
		return std::make_unique<Add>(graphs, onlyUniverse, graphPolicy);
	else
		return std::make_unique<Add>(generator, onlyUniverse, graphPolicy);
}

void Add::preAddGraphs(std::function<void(std::shared_ptr<mod::graph::Graph>, IsomorphismPolicy)> add) const {
	for(const auto &g : graphs)
		add(g, graphPolicy);
}

void Add::printInfo(PrintSettings settings) const {
	settings.indent() << "Add";
	std::ostream &s = settings.s;
	if(onlyUniverse) s << "Universe";
	s << ":";
	if(generator) {
		s << '\n';
		++settings.indentLevel;
		settings.indent() << "function = ";
		generator->print(s);
		--settings.indentLevel;
	} else {
		for(const auto &g : graphs)
			s << " " << g->getName();
	}
	s << '\n';
	++settings.indentLevel;
	printBaseInfo(settings);
}

bool Add::isConsumed(const lib::graph::Graph *g) const {
	return false;
}

void Add::executeImpl(PrintSettings settings, const GraphState &input) {
	if(settings.verbosity >= PrintSettings::V_Add) {
		if(onlyUniverse)
			settings.indent() << "AddUniverse";
		else
			settings.indent() << "AddSubset";
		if(generator)
			settings.s << " (dynamic):" << std::endl;
		else
			settings.s << " (static):" << std::endl;
	}
	std::vector<std::shared_ptr<mod::graph::Graph>> graphsToAdd;
	if(generator) {
		graphsToAdd = (*generator)();
		if(std::any_of(graphsToAdd.begin(), graphsToAdd.end(), [](const auto &g) {
			return !g;
		}))
			throw LogicError("A graph returned in a dynamic add strategy is a null pointer.");
	} else {
		graphsToAdd = graphs;
	}
	if(settings.verbosity >= PrintSettings::V_Add) {
		++settings.indentLevel;
		// print the graphs, 5 per line
		for(int i = 0; i != graphsToAdd.size(); ++i) {
			if(i % 5 == 0 && i != 0) settings.s << std::endl;
			if(i % 5 == 0) settings.indent();
			else settings.s << " ";
			settings.s << graphsToAdd[i]->getName();
		}
		settings.s << std::endl;
		--settings.indentLevel;
	}
	switch(graphPolicy) {
	case IsomorphismPolicy::Check:
		for(const std::shared_ptr<mod::graph::Graph> &g : graphsToAdd) {
			// TODO: we need to add it as a vertex for backwards compatibility,
			//       do this more elegantly
			// TODO: actually, if this is a static add strategy, the graphs are probably already added,
			//       at least with the usual execute function.
			getExecutionEnv().tryAddGraph(g);
			getExecutionEnv().trustAddGraphAsVertex(g);
		}
		break;
	case IsomorphismPolicy::TrustMe:
		for(const auto &g : graphsToAdd)
			getExecutionEnv().trustAddGraphAsVertex(g);
		break;
	}
	output = new GraphState(input);
	if(onlyUniverse) {
		for(const auto &g : graphsToAdd)
			output->addToUniverse(&g->getGraph());
	} else {
		for(const auto &g : graphsToAdd)
			output->addToSubset(&g->getGraph());
	}
}

} // namespace mod::lib::DG::Strategies