#ifndef MOD_LIB_DG_STRATEGIES_SEQUENCE_HPP
#define MOD_LIB_DG_STRATEGIES_SEQUENCE_HPP

#include <mod/lib/DG/Strategies/Strategy.hpp>

#include <vector>

namespace mod::lib::DG::Strategies {

struct Sequence : Strategy {
	// pre: !strats.empty()
	Sequence(std::vector<std::unique_ptr<Strategy>> strats);
	virtual ~Sequence() override;
	virtual std::unique_ptr<Strategy> clone() const override;
	virtual void preAddGraphs(std::function<void(std::shared_ptr<mod::graph::Graph>, IsomorphismPolicy)> add) const override;
	virtual void forEachRule(std::function<void(const lib::rule::Rule &)> f) const override;
	virtual void printInfo(PrintSettings settings) const override;
	virtual const GraphState &getOutput() const override;
	virtual bool isConsumed(const lib::graph::Graph *g) const override;
private:
	virtual void setExecutionEnvImpl() override;
	virtual void executeImpl(PrintSettings settings, const GraphState &input) override;
private:
	std::vector<std::unique_ptr<Strategy>> strats;
};

} // namespace mod::lib::DG::Strategies

#endif // MOD_LIB_DG_STRATEGIES_SEQUENCE_HPP