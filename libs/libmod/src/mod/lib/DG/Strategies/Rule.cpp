#include "Rule.hpp"

#include <mod/Config.hpp>
#include <mod/Derivation.hpp>
#include <mod/Misc.hpp>
#include <mod/rule/Rule.hpp>
#include <mod/lib/DG/RuleApplicationUtils.hpp>
#include <mod/lib/DG/Strategies/GraphState.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/RC/ComposeRuleReal.hpp>
#include <mod/lib/RC/MatchMaker/Super.hpp>

namespace mod {
namespace lib {
namespace DG {
namespace Strategies {

Rule::Rule(std::shared_ptr<rule::Rule> r)
		: Strategy(std::max(r->getRule().getDPORule().numLeftComponents, r->getRule().getDPORule().numRightComponents)),
		  r(r), rRaw(&r->getRule()) {
	assert(rRaw->getDPORule().numLeftComponents > 0);
}

Rule::Rule(const lib::Rules::Real *r)
		: Strategy(std::max(r->getDPORule().numLeftComponents, r->getDPORule().numRightComponents)), rRaw(r) {
	assert(r->getDPORule().numLeftComponents > 0);
}

Strategy *Rule::clone() const {
	if(r) return new Rule(r);
	else return new Rule(rRaw);
}

void Rule::preAddGraphs(std::function<void(std::shared_ptr<graph::Graph>, IsomorphismPolicy)> add) const {}

void Rule::forEachRule(std::function<void(const lib::Rules::Real &)> f) const {
	f(*this->rRaw);
}

void Rule::printInfo(PrintSettings settings) const {
	settings.indent() << "Rule: " << r->getName() << '\n';
	++settings.indentLevel;
	printBaseInfo(settings);
	settings.indent() << "consumed =";
	std::vector<const lib::Graph::Single *> temp(begin(consumedGraphs), end(consumedGraphs));
	std::sort(begin(temp), end(temp), lib::Graph::Single::nameLess);
	for(const auto *g : temp)
		settings.s << " " << g->getName();
	settings.s << '\n';
}

bool Rule::isConsumed(const lib::Graph::Single *g) const {
	return consumedGraphs.find(g) != consumedGraphs.end();
}

namespace {

struct Context {
	const std::shared_ptr<rule::Rule> &r;
	ExecutionEnv &executionEnv;
	GraphState *output;
	std::unordered_set<const lib::Graph::Single *> &consumedGraphs;
};

void handleBoundRulePair(PrintSettings settings, Context context, const BoundRule &brp) {
	assert(brp.rule);
	// use a smart pointer so the rule for sure is deallocated, even though we do a 'continue'
	const lib::Rules::Real &r = *brp.rule;
	const auto &rDPO = r.getDPORule();
	assert(
			r.isOnlyRightSide()); // otherwise, it should have been deallocated. All max component results should be only right side
	mod::Derivation d;
	d.r = context.r;
	for(const lib::Graph::Single *g : brp.boundGraphs) d.left.push_back(g->getAPIReference());
	{ // left predicate
		bool result = context.executionEnv.checkLeftPredicate(d);
		if(!result) {
			if(settings.verbosity >= PrintSettings::V_DerivationPredicatesFail)
				settings.indent() << "Skipping " << r.getName() << " due to leftPredicate" << std::endl;
			return;
		}
	}
	if(settings.verbosity >= PrintSettings::V_RuleApplication)
		settings.indent() << "Splitting " << r.getName() << " into "
		                  << rDPO.numRightComponents << " graphs:" << std::endl;
	const std::vector<const lib::Graph::Single *> &educts = brp.boundGraphs;
	d.right = splitRule(
			rDPO, context.executionEnv.labelSettings.type, context.executionEnv.labelSettings.withStereo,
			[&context](std::unique_ptr<lib::Graph::Single> gCand) {
				return context.executionEnv.checkIfNew(std::move(gCand));
			},
			[&settings](std::shared_ptr<graph::Graph> gWrapped, std::shared_ptr<graph::Graph> gPrev) {
				if(settings.verbosity >= PrintSettings::V_RuleApplication)
					settings.indent() << "Discarding product " << gWrapped->getName()
					                  << ", isomorphic to other product " << gPrev->getName()
					                  << "." << std::endl;
			}
	);

	if(getConfig().dg.onlyProduceMolecules.get()) {
		for(std::shared_ptr<graph::Graph> g : d.right) {
			if(!g->getIsMolecule()) {
				IO::log() << "Error: non-molecule produced; '" << g->getName() << "'" << std::endl
				          << "Derivation is:" << std::endl
				          << "\tEducts:" << std::endl;
				for(const lib::Graph::Single *g : educts)
					IO::log() << "\t\t'" << g->getName() << "'\t" << g->getGraphDFS().first << std::endl;
				IO::log() << "\tProducts:" << std::endl;
				for(std::shared_ptr<graph::Graph> g : d.right)
					IO::log() << "\t\t'" << g->getName() << "'\t" << g->getGraphDFS() << std::endl;
				IO::log() << "Rule is '" << context.r->getName() << "'" << std::endl;
				std::exit(1);
			}
		}
	}
	{ // right predicates
		bool result = context.executionEnv.checkRightPredicate(d);
		if(!result) {
			if(settings.verbosity >= PrintSettings::V_DerivationPredicatesFail)
				settings.indent() << "Skipping " << r.getName() << " due to rightPredicate" << std::endl;
			return;
		}
	}
	{ // now the derivation is good, so add the products to output
		if(getConfig().dg.putAllProductsInSubset.get()) {
			for(const auto &g : d.right)
				context.output->addToSubset(0, &g->getGraph());
		} else {
			for(const auto &g : d.right)
				if(!context.output->isInUniverse(&g->getGraph()))
					context.output->addToSubset(0, &g->getGraph());
		}
		for(const auto &g : d.right)
			context.executionEnv.addProduct(g);
	}
	std::vector<const lib::Graph::Single *> rightGraphs;
	rightGraphs.reserve(d.right.size());
	for(const std::shared_ptr<graph::Graph> &g : d.right)
		rightGraphs.push_back(&g->getGraph());
	lib::DG::GraphMultiset gmsLeft(educts), gmsRight(std::move(rightGraphs));
	bool inserted = context.executionEnv.suggestDerivation(gmsLeft, gmsRight, &context.r->getRule());
	if(inserted) {
		for(const lib::Graph::Single *g : educts)
			context.consumedGraphs.insert(g);
	}
}

template<typename GraphRange>
unsigned int bindGraphs(PrintSettings settings, Context context,
                        const GraphRange &graphRange,
                        const std::vector<BoundRule> &rules,
                        std::vector<BoundRule> &outputRules) {
	unsigned int processedRules = 0;

	for(const lib::Graph::Single *g : graphRange) {
		if(context.executionEnv.doExit()) break;
		for(const BoundRule &p : rules) {
			if(context.executionEnv.doExit()) break;
			if(settings.verbosity >= PrintSettings::V_RuleApplication) {
				settings.indent() << "Trying to bind " << g->getName() << " to " << p.rule->getName() << ":" << std::endl;
				++settings.indentLevel;
			}
			std::vector<BoundRule> resultRules;
			BoundRuleStorage ruleStore(settings.verbosity >= PrintSettings::V_RuleApplication,
			                           settings,
			                           context.executionEnv.labelSettings.type,
			                           context.executionEnv.labelSettings.withStereo, resultRules, p, g);
			auto reporter = [&ruleStore](std::unique_ptr<lib::Rules::Real> r) {
				ruleStore.add(r.release());
				return true;
			};
			assert(p.rule);
			const lib::Rules::Real &rFirst = g->getBindRule()->getRule();
			const lib::Rules::Real &rSecond = *p.rule;
			lib::RC::Super mm(
					std::max(0, settings.verbosity - PrintSettings::V_RCMorphismGenBase),
					settings,
					true, true);
			lib::RC::composeRuleRealByMatchMaker(rFirst, rSecond, mm, reporter, context.executionEnv.labelSettings);
			for(const BoundRule &brp : resultRules) {
				processedRules++;
				if(context.executionEnv.doExit()) delete brp.rule;
				else if(brp.rule->isOnlyRightSide()) {
					handleBoundRulePair(settings, context, brp);
					delete brp.rule;
				} else outputRules.push_back(brp);
			}
			if(settings.verbosity >= PrintSettings::V_RuleApplication)
				--settings.indentLevel;
		}
	}
	return processedRules;
}

} // namespace 

void Rule::executeImpl(PrintSettings settings, const GraphState &input) {
	if(settings.verbosity >= PrintSettings::V_Rule) {
		settings.indent() << "Rule: " << r->getName() << std::endl;
		++settings.indentLevel;
	}

	if(getExecutionEnv().labelSettings.withStereo) {
		// let's trigger deduction errors early
		try {
			get_stereo(rRaw->getDPORule());
		} catch(StereoDeductionError &e) {
			std::stringstream ss;
			ss << "\nStereo deduction error in rule '" << rRaw->getName() << "'.";
			e.append(ss.str());
			throw;
		}
	}
	if(getExecutionEnv().labelSettings.type == LabelType::Term) {
		const auto &term = get_term(rRaw->getDPORule());
		if(!isValid(term)) {
			std::string msg = "Parsing failed for rule '" + rRaw->getName() + "'. " + term.getParsingError();
			throw TermParsingError(std::move(msg));
		}
	}

	output = new GraphState(input.getUniverse());
	if(getExecutionEnv().doExit()) {
		if(settings.verbosity >= PrintSettings::V_Rule)
			settings.indent() << "Exit requrested, skipping." << std::endl;
		return;
	}
	std::vector<std::vector<BoundRule> > intermediaryRules(rRaw->getDPORule().numLeftComponents + 1);
	{
		BoundRule p;
		p.rule = rRaw;
		intermediaryRules[0].push_back(p);
	}
	Context context{r, getExecutionEnv(), output, consumedGraphs};
	const auto &subset = input.getSubset(0);
	const auto &universe = input.getUniverse();
	for(unsigned int i = 1; i <= rRaw->getDPORule().numLeftComponents; i++) {
		if(settings.verbosity >= PrintSettings::V_RuleBinding) {
			settings.indent() << "Component bind round " << i << " with ";
			++settings.indentLevel;
			if(i == 1) {
				if(!getConfig().dg.ignoreSubset.get()) {
					IO::log() << subset.size() << " graphs";
				} else {
					IO::log() << universe.size() << " graphs";
				}
			} else {
				IO::log() << universe.size() << " graphs and " << intermediaryRules[i - 1].size() << " intermediaries";
			}
			IO::log() << std::endl;
		}

		std::size_t processedRules = 0;
		if(i == 1) {
			if(!getConfig().dg.ignoreSubset.get()) {
				processedRules = bindGraphs(settings, context, subset, intermediaryRules[0], intermediaryRules[1]);
			} else {
				processedRules = bindGraphs(settings, context, universe, intermediaryRules[0], intermediaryRules[1]);
			}
		} else {
			processedRules = bindGraphs(settings, context, universe, intermediaryRules[i - 1], intermediaryRules[i]);
			for(BoundRule &p : intermediaryRules[i - 1]) {
				delete p.rule;
				p.rule = nullptr;
			}
		}
		if(settings.verbosity >= PrintSettings::V_RuleBinding) {
			settings.indent() << "Processing of " << processedRules << " intermediary rules done" << std::endl;
			--settings.indentLevel;
		}
		if(context.executionEnv.doExit()) break;
	}
	assert(intermediaryRules.back().empty());
}

} // namespace Strategies
} // namespace DG
} // namespace lib
} // namespace mod
