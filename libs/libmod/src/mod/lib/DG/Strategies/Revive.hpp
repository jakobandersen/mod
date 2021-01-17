#ifndef MOD_LIB_DG_STRATEGIES_REVIVE_HPP
#define MOD_LIB_DG_STRATEGIES_REVIVE_HPP

#include <mod/lib/DG/Strategies/Strategy.hpp>

namespace mod::lib::DG::Strategies {

struct Revive : Strategy {
	Revive(Strategy *strat);
	virtual ~Revive() override;
	virtual Strategy *clone() const override;
	virtual void preAddGraphs(std::function<void(std::shared_ptr<graph::Graph>, IsomorphismPolicy)> add) const override;
	virtual void forEachRule(std::function<void(const lib::Rules::Real &)> f) const override;
	virtual void printInfo(PrintSettings settings) const override;
	virtual bool isConsumed(const lib::Graph::Single *g) const override;
private:
	virtual void setExecutionEnvImpl() override;
	virtual void executeImpl(PrintSettings settings, const GraphState &input) override;
private:
	Strategy *strat;
	std::vector<const lib::Graph::Single *> revivedGraphs;
};

} // namespace mod::lib::DG::Strategies

#endif // MOD_LIB_DG_STRATEGIES_REVIVE_HPP