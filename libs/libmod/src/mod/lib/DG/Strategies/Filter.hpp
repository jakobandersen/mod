#ifndef MOD_LIB_DG_STRATEGY_FILTER_HPP
#define MOD_LIB_DG_STRATEGY_FILTER_HPP

#include <mod/dg/Strategies.hpp>
#include <mod/lib/DG/Strategies/Strategy.hpp>

namespace mod::lib::DG::Strategies {

struct Filter : Strategy {
	Filter(std::shared_ptr<mod::Function<bool(std::shared_ptr<mod::graph::Graph>, const dg::Strategy::GraphState &,
	                                          bool)>> filterFunc, bool filterUniverse);
	virtual std::unique_ptr<Strategy> clone() const override;
	virtual void
	preAddGraphs(std::function<void(std::shared_ptr<mod::graph::Graph>, IsomorphismPolicy)> add) const override {}
	virtual void forEachRule(std::function<void(const lib::rule::Rule &)> f) const override {}
	virtual void printInfo(PrintSettings settings) const override;
	virtual bool isConsumed(const lib::graph::Graph *g) const override;
private:
	virtual void executeImpl(PrintSettings settings, const GraphState &input) override;
private:
	std::shared_ptr<mod::Function<bool(std::shared_ptr<mod::graph::Graph>, const dg::Strategy::GraphState &,
	                                   bool)>> filterFunc;
	bool filterUniverse;
};

} // namespace mod::lib::DG::Strategies

#endif // MOD_LIB_DG_STRATEGY_FILTER_HPP