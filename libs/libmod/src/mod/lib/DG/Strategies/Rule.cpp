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

namespace mod::lib::DG::Strategies {

Rule::Rule(std::shared_ptr<rule::Rule> r)
		: Strategy(std::max(get_num_connected_components(get_labelled_left(r->getRule().getDPORule())),
		                    get_num_connected_components(get_labelled_right(r->getRule().getDPORule())))),
		  r(r), rRaw(&r->getRule()) {
	assert(get_num_connected_components(get_labelled_left(rRaw->getDPORule())) > 0);
}

Rule::Rule(const lib::Rules::Real *r)
		: Strategy(std::max(get_num_connected_components(get_labelled_left(r->getDPORule())),
		                    get_num_connected_components(get_labelled_right(r->getDPORule())))),
		  rRaw(r) {
	assert(get_num_connected_components(get_labelled_left(rRaw->getDPORule())) > 0);
}

std::unique_ptr<Strategy> Rule::clone() const {
	if(r) return std::make_unique<Rule>(r);
	else return std::make_unique<Rule>(rRaw);
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
	for(const auto *g: temp)
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

void handleBoundRulePair(int verbosity, IO::Logger logger, Context context, const BoundRule &brp) {
	assert(brp.rule);
	// TODO: use a smart pointer so the rule for sure is deallocated, even though we do a 'continue'
	const lib::Rules::Real &r = *brp.rule;
	const auto &rDPO = r.getDPORule();
	assert(r.isOnlyRightSide()); // otherwise, it should have been deallocated.
	// All max component results should be only right side
	mod::Derivation d;
	d.r = context.r;
	for(const lib::Graph::Single *g: brp.boundGraphs) d.left.push_back(g->getAPIReference());
	{ // left predicate
		bool result = context.executionEnv.checkLeftPredicate(d);
		if(!result) {
			if(verbosity >= PrintSettings::V_DerivationPredicatesFail)
				logger.indent() << "Skipping " << r.getName() << " due to leftPredicate" << std::endl;
			return;
		}
	}
	if(verbosity >= PrintSettings::V_RuleApplication) {
		logger.indent() << "Splitting " << r.getName() << " into "
		                << get_num_connected_components(get_labelled_right(rDPO))
		                << " graphs" << std::endl;
		++logger.indentLevel;
	}
	const std::vector<const lib::Graph::Single *> &educts = brp.boundGraphs;
	d.right = splitRule(
			rDPO, context.executionEnv.labelSettings.type, context.executionEnv.labelSettings.withStereo,
			[&context](std::unique_ptr<lib::Graph::Single> gCand) {
				return context.executionEnv.checkIfNew(std::move(gCand));
			},
			[verbosity, &logger](std::shared_ptr<graph::Graph> gWrapped, std::shared_ptr<graph::Graph> gPrev) {
				if(verbosity >= PrintSettings::V_RuleApplication)
					logger.indent() << "Discarding product " << gWrapped->getName()
					                << ", isomorphic to other product " << gPrev->getName()
					                << "." << std::endl;
			}
	);
	if(verbosity >= PrintSettings::V_RuleApplication)
		--logger.indentLevel;
	if(d.right.empty()) {
		if(verbosity >= V_RuleApplication)
			logger.indent(1) << "Discarding derivation, empty result." << std::endl;
		return;
	}

	{ // right predicates
		bool result = context.executionEnv.checkRightPredicate(d);
		if(!result) {
			if(verbosity >= PrintSettings::V_DerivationPredicatesFail)
				logger.indent() << "Skipping " << r.getName() << " due to rightPredicate" << std::endl;
			return;
		}
	}
	{ // now the derivation is good, so add the products to output
		if(getConfig().dg.putAllProductsInSubset) {
			for(const auto &g: d.right)
				context.output->addToSubset(&g->getGraph());
		} else {
			for(const auto &g: d.right)
				if(!context.output->isInUniverse(&g->getGraph()))
					context.output->addToSubset(&g->getGraph());
		}
		for(const auto &g: d.right)
			context.executionEnv.addProduct(g);
	}
	std::vector<const lib::Graph::Single *> rightGraphs;
	rightGraphs.reserve(d.right.size());
	for(const std::shared_ptr<graph::Graph> &g: d.right)
		rightGraphs.push_back(&g->getGraph());
	lib::DG::GraphMultiset gmsLeft(educts), gmsRight(std::move(rightGraphs));
	bool inserted = context.executionEnv.suggestDerivation(gmsLeft, gmsRight, &context.r->getRule());
	if(inserted) {
		for(const lib::Graph::Single *g: educts)
			context.consumedGraphs.insert(g);
	}
}

template<typename GraphRange>
unsigned int bindGraphs(PrintSettings settings, Context context,
                        const GraphRange &graphRange,
                        const std::vector<BoundRule> &rules,
                        std::vector<BoundRule> &outputRules,
                        Rules::GraphAsRuleCache &graphAsRuleCache) {
	unsigned int processedRules = 0;

	for(const lib::Graph::Single *g: graphRange) {
		if(context.executionEnv.doExit()) break;
		for(const BoundRule &p: rules) {
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
			const lib::Rules::Real &rFirst = graphAsRuleCache.getBindRule(g)->getRule();
			const lib::Rules::Real &rSecond = *p.rule;
			lib::RC::Super mm(
					std::max(0, settings.verbosity - PrintSettings::V_RCMorphismGenBase),
					settings,
					true, true);
			lib::RC::composeRuleRealByMatchMaker(rFirst, rSecond, mm, reporter, context.executionEnv.labelSettings);
			for(const BoundRule &brp: resultRules) {
				processedRules++;
				if(context.executionEnv.doExit()) delete brp.rule;
				else if(brp.rule->isOnlyRightSide()) {
					handleBoundRulePair(settings.verbosity, settings, context, brp);
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
			settings.indent() << "Exit requested, skipping." << std::endl;
		return;
	}

	const auto &subset = input.getSubset();
	const auto &universe = input.getUniverse();

	// partition such that the subset is first
	for(int i = 0; i != subset.size(); ++i)
		assert(subset.begin()[i] == universe[subset.getIndices()[i]]);

	std::vector<bool> inSubset(universe.size(), false);
	for(int idx: subset.getIndices())
		inSubset[idx] = true;

	std::vector<const lib::Graph::Single *> graphs = universe;
	auto subsetEnd = graphs.begin();
	for(auto iter = graphs.begin(); iter != graphs.end(); ++iter) {
		const auto offset = iter - graphs.begin();
		if(inSubset[offset]) {
			std::iter_swap(subsetEnd, iter);
			++subsetEnd;
		}
	}
	assert(subsetEnd - graphs.begin() == subset.size());

	Context context{r, getExecutionEnv(), output, consumedGraphs};
	std::vector<BoundRule> inputRules{{rRaw, {}, 0}};
	for(int round = 0; round != get_num_connected_components(get_labelled_left(rRaw->getDPORule())); ++round) {
		const auto firstGraph = graphs.begin();
		const auto lastGraph = round == 0 ? subsetEnd : graphs.end();

		const auto onOutput = [verbosity = settings.verbosity, context]
				(IO::Logger logger, BoundRule br) -> bool {
			if(br.rule->isOnlyRightSide()) {
				handleBoundRulePair(verbosity, logger, context, br);
				delete br.rule;
			}
			return true;
		};
		std::vector<BoundRule> outputRules = bindGraphs(
				settings.ruleApplicationVerbosity(), settings,
				round,
				firstGraph, lastGraph, inputRules,
				getExecutionEnv().graphAsRuleCache,
				getExecutionEnv().labelSettings,
				getExecutionEnv().doRuleIsomorphism,
				onOutput);
		if(round != 0) {
			// in round 0 the inputRules is the actual original input rule, so don't delete it
			for(auto &br: inputRules)
				delete br.rule;
		}
		std::swap(inputRules, outputRules);
	} // for each round based on numComponents
	assert(inputRules.empty());
}

} // namespace mod::lib::DG::Strategies