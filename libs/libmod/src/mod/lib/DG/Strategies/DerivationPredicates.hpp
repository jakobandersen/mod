#ifndef MOD_LIB_DG_STRATEGIES_DERIVATIONPREDICATES_HPP
#define MOD_LIB_DG_STRATEGIES_DERIVATIONPREDICATES_HPP

#include <mod/dg/Strategies.hpp>
#include <mod/lib/DG/Strategies/Strategy.hpp>

namespace mod::lib::DG::Strategies {

struct DerivationPredicate : Strategy {
protected:
	DerivationPredicate(std::shared_ptr<mod::Function<bool(const mod::Derivation &)>> predicate,
	                    std::unique_ptr<Strategy> strat);
public:
	virtual ~DerivationPredicate() override;
	virtual void preAddGraphs(std::function<void(std::shared_ptr<mod::graph::Graph>, IsomorphismPolicy)> add) const override;
	virtual void forEachRule(std::function<void(const lib::rule::Rule &)> f) const override;
	virtual void printInfo(PrintSettings settings) const override;
	virtual const GraphState &getOutput() const override;
	virtual bool isConsumed(const lib::graph::Graph *g) const override;
protected:
	virtual void printName(std::ostream &s) const = 0;
	virtual void pushPredicate(std::shared_ptr<mod::Function<bool(const mod::Derivation &)> > pred) = 0;
	virtual void popPredicate() = 0;
private:
	virtual void setExecutionEnvImpl() override;
	virtual void executeImpl(PrintSettings settings, const GraphState &input) override;
protected:
	std::shared_ptr<mod::Function<bool(const mod::Derivation &)>> predicate;
	std::unique_ptr<Strategy> strat;
};

struct LeftPredicate : DerivationPredicate {
	LeftPredicate(std::shared_ptr<mod::Function<bool(const mod::Derivation &)>> predicate,
	              std::unique_ptr<Strategy> strat);
	virtual std::unique_ptr<Strategy> clone() const override;
private:
	void printName(std::ostream &s) const override;
	void pushPredicate(std::shared_ptr<mod::Function<bool(const mod::Derivation &)>> pred) override;
	void popPredicate() override;
};

struct RightPredicate : DerivationPredicate {
	RightPredicate(std::shared_ptr<mod::Function<bool(const mod::Derivation &)>> predicate,
	               std::unique_ptr<Strategy> strat);
	virtual std::unique_ptr<Strategy> clone() const override;
private:
	void printName(std::ostream &s) const override;
	void pushPredicate(std::shared_ptr<mod::Function<bool(const mod::Derivation &)>> pred) override;
	void popPredicate() override;
};

} // namespace mod::lib::DG::Strategies

#endif // MOD_LIB_DG_STRATEGIES_DERIVATIONPREDICATES_HPP