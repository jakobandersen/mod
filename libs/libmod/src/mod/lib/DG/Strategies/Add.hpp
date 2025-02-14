#ifndef MOD_LIB_DG_STRATEGIES_ADD_HPP
#define MOD_LIB_DG_STRATEGIES_ADD_HPP

#include <mod/lib/DG/Strategies/Strategy.hpp>

namespace mod {
template<typename Sig>
class Function;
} // namespace mod
namespace mod::lib::DG::Strategies {

struct Add : Strategy {
	// pre: no nullptrs in graphs
	Add(const std::vector<std::shared_ptr<mod::graph::Graph>> graphs, bool onlyUniverse, IsomorphismPolicy graphPolicy);
	Add(const std::shared_ptr<mod::Function<std::vector<std::shared_ptr<mod::graph::Graph> >()> > generator,
	    bool onlyUniverse, IsomorphismPolicy graphPolicy);
	virtual ~Add() override;
	virtual std::unique_ptr<Strategy> clone() const override;
	virtual void preAddGraphs(std::function<void(std::shared_ptr<mod::graph::Graph>, IsomorphismPolicy)> add) const override;
	virtual void forEachRule(std::function<void(const lib::rule::Rule &)> f) const override {}
	virtual void printInfo(PrintSettings settings) const override;
	virtual bool isConsumed(const lib::graph::Graph *g) const override;
private:
	virtual void executeImpl(PrintSettings settings, const GraphState &input) override;
private:
	const std::vector<std::shared_ptr<mod::graph::Graph> > graphs;
	const std::shared_ptr<mod::Function<std::vector<std::shared_ptr<mod::graph::Graph> >()> > generator;
	const bool onlyUniverse;
	const IsomorphismPolicy graphPolicy;
};

} // namespace mod::lib::DG::Strategies

#endif // MOD_LIB_DG_STRATEGIES_ADD_HPP