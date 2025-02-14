#ifndef MOD_LIB_DG_STRATEGIES_EXECUTE_HPP
#define MOD_LIB_DG_STRATEGIES_EXECUTE_HPP

#include <mod/dg/Strategies.hpp>
#include <mod/lib/DG/Strategies/Strategy.hpp>

namespace mod::lib::DG::Strategies {

struct Execute : Strategy {
	Execute(std::shared_ptr<mod::Function<void(const dg::Strategy::GraphState &)>> func);
	virtual std::unique_ptr<Strategy> clone() const override;
	virtual void preAddGraphs(std::function<void(std::shared_ptr<mod::graph::Graph>, IsomorphismPolicy)> add) const override;
	virtual void forEachRule(std::function<void(const lib::rule::Rule &)> f) const override {}
	virtual void printInfo(PrintSettings settings) const override;
	virtual const GraphState &getOutput() const override;
	virtual bool isConsumed(const lib::graph::Graph *g) const override;
private:
	virtual void executeImpl(PrintSettings settings, const GraphState &input) override;
private:
	std::shared_ptr<mod::Function<void(const dg::Strategy::GraphState &)> > func;
};

} // namespace mod::lib::DG::Strategies

#endif // MOD_LIB_DG_STRATEGIES_EXECUTE_HPP