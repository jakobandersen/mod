#ifndef MOD_LIB_DG_STRATEGIES_STRATEGY_HPP
#define MOD_LIB_DG_STRATEGIES_STRATEGY_HPP

#include <mod/dg/Strategies.hpp>
#include <mod/lib/DG/NonHyper.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/Rules/GraphAsRuleCache.hpp>

#include <iosfwd>
#include <vector>

namespace mod::lib::DG::Strategies {
class GraphState;

struct ExecutionEnv {
	ExecutionEnv(LabelSettings labelSettings, bool doRuleIsomorphism, Rules::GraphAsRuleCache &graphAsRuleCache)
			: labelSettings(labelSettings), doRuleIsomorphism(doRuleIsomorphism), graphAsRuleCache(graphAsRuleCache) {}
	virtual ~ExecutionEnv() {};
	// May throw LogicError if exists.
	virtual void tryAddGraph(std::shared_ptr<graph::Graph> g) = 0;
	virtual bool trustAddGraph(std::shared_ptr<graph::Graph> g) = 0;
	virtual bool trustAddGraphAsVertex(std::shared_ptr<graph::Graph> g) = 0;
	virtual bool doExit() const = 0;
	// the right side is always empty
	virtual bool checkLeftPredicate(const mod::Derivation &d) const = 0;
	// but here everything is defined
	virtual bool checkRightPredicate(const mod::Derivation &d) const = 0;
	virtual std::shared_ptr<graph::Graph> checkIfNew(std::unique_ptr<lib::Graph::Single> g) const = 0;
	virtual bool addProduct(std::shared_ptr<graph::Graph> g) = 0;
	virtual bool
	isDerivation(const GraphMultiset &gmsSrc, const GraphMultiset &gmsTar, const lib::Rules::Real *r) const = 0;
	virtual bool
	suggestDerivation(const GraphMultiset &gmsSrc, const GraphMultiset &gmsTar, const lib::Rules::Real *r) = 0;
	virtual void pushLeftPredicate(std::shared_ptr<mod::Function<bool(const mod::Derivation &)> > pred) = 0;
	virtual void pushRightPredicate(std::shared_ptr<mod::Function<bool(const mod::Derivation &)> > pred) = 0;
	virtual void popLeftPredicate() = 0;
	virtual void popRightPredicate() = 0;
public:
	const LabelSettings labelSettings;
	const bool doRuleIsomorphism;
	Rules::GraphAsRuleCache &graphAsRuleCache;
};

struct PrintSettings : IO::Logger {
	explicit PrintSettings(std::ostream &s, bool withUniverse) : IO::Logger(s), withUniverse(withUniverse) {}
	explicit PrintSettings(std::ostream &s, bool withUniverse, int verbosity)
			: IO::Logger(s), withUniverse(withUniverse), verbosity(verbosity) {}
	int ruleApplicationVerbosity() const {
		return std::max(0, verbosity - V_DerivationPredicatesPred);
	}
public:
	bool withUniverse; // mostly used for printInfo
	int verbosity = 2; // mostly used for execute
public: // constants for verbosity 'levels'
	static constexpr int
			V_Repeat = 2,
			V_RepeatBreak = 4,
			V_Sequence = V_RepeatBreak,
			V_Revive = V_RepeatBreak,
			V_Parallel = V_RepeatBreak,
			V_DerivationPredicates = V_RepeatBreak,
			V_Filter = V_RepeatBreak,
			V_Rule = V_RepeatBreak,
			V_Add = V_RepeatBreak,
			V_Execute = V_RepeatBreak,
			V_DerivationPredicatesPred = V_DerivationPredicates + 2,
			V_FilterPred = V_DerivationPredicatesPred,
	// The rest should be moved to RuleApplicationUtils
	V_RuleBinding = V_DerivationPredicatesPred + 2,
			V_RuleApplication = V_RuleBinding + 2,
			V_DerivationPredicatesFail = V_RuleApplication,
			V_RCMorphismGenBase = 48 /* meaning 48 is no info, 50 is gen info, and 60 is rc info */;
};

struct Strategy {
	Strategy(int maxComponents);
	Strategy(const Strategy &) = delete;
	Strategy &operator=(const Strategy &) = delete;
	virtual ~Strategy();
	virtual std::unique_ptr<Strategy> clone() const = 0;
	void setExecutionEnv(ExecutionEnv &env);
	virtual void preAddGraphs(std::function<void(std::shared_ptr<graph::Graph>, IsomorphismPolicy)> add) const = 0;
	virtual void forEachRule(std::function<void(const lib::Rules::Real &)> f) const = 0;
	int getMaxComponents() const;
	void execute(PrintSettings settings, const GraphState &input);
	virtual void printInfo(PrintSettings settings) const = 0;
	virtual const GraphState &getOutput() const;
	virtual bool isConsumed(const lib::Graph::Single *g) const = 0;
protected:
	ExecutionEnv &getExecutionEnv();
	void printBaseInfo(PrintSettings settings) const;
private:
	virtual void setExecutionEnvImpl();
	virtual void executeImpl(PrintSettings settings, const GraphState &input) = 0;
private:
	ExecutionEnv *env = nullptr;
	const int maxComponents;
protected:
	const GraphState *input = nullptr;
	GraphState *output = nullptr;
protected:
	static int calcMaxNumComponents(const std::vector<std::unique_ptr<Strategy>> &strats);
};

} // namespace mod::lib::DG::Strategies

#endif // MOD_LIB_DG_STRATEGIES_STRATEGY_HPP