#ifndef MOD_LIB_DG_STRATEGIES_RULE_HPP
#define MOD_LIB_DG_STRATEGIES_RULE_HPP

#include <mod/lib/DG/Strategies/Strategy.hpp>

#include <unordered_set>

namespace mod::lib::DG::Strategies {

struct Rule : Strategy {
	Rule(std::shared_ptr<mod::rule::Rule> r);
	Rule(const lib::rule::Rule *r);
	virtual std::unique_ptr<Strategy> clone() const override;
	virtual void preAddGraphs(std::function<void(std::shared_ptr<mod::graph::Graph>, IsomorphismPolicy)> add) const override;
	virtual void forEachRule(std::function<void(const lib::rule::Rule &)> f) const override;
	virtual void printInfo(PrintSettings settings) const override;
	virtual bool isConsumed(const lib::graph::Graph *g) const override;
private:
	virtual void executeImpl(PrintSettings settings, const GraphState &input) override;
private:
	std::shared_ptr<mod::rule::Rule> r;
	const lib::rule::Rule *rRaw;
	std::unordered_set<const lib::graph::Graph *> consumedGraphs; // all those from lhs of derivations
};

} // namespace mod::lib::DG::Strategies

#endif // MOD_LIB_DG_STRATEGIES_RULE_HPP