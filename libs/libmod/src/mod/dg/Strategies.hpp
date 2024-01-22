#ifndef MOD_DG_STRATEGIES_HPP
#define MOD_DG_STRATEGIES_HPP

#include <mod/BuildConfig.hpp>
#include <mod/dg/DG.hpp>
#include <mod/dg/GraphInterface.hpp>
#include <mod/rule/ForwardDecl.hpp>

#include <functional>
#include <iosfwd>
#include <memory>
#include <vector>

namespace mod {
struct Derivation;
template<typename Sig>
class Function;
} // namespace mod
namespace mod::dg {

// rst-class: dg::Strategy
// rst:
// rst:		Derivation graph strategies are used in :cpp:func:`Builder::execute`.
// rst:
// rst-class-start:
struct MOD_DECL Strategy {
	Strategy(const Strategy &) = delete;
	Strategy &operator=(const Strategy &) = delete;
public:
	// rst-nested: dg::Strategy::GraphState
	// rst:
	// rst:		This class represents a graph state with a subset :math:`S` and a universe :math:`U` fulfilling :math:`S\subseteq U`.
	// rst:
	// rst-nested-start:
	struct GraphState {
		GraphState(std::function<void(std::vector<std::shared_ptr<graph::Graph>> &)> fSubset,
		           std::function<void(std::vector<std::shared_ptr<graph::Graph>> &)> fUniverse);
		// rst:		.. function:: const std::vector<std::shared_ptr<graph::Graph>> &getSubset() const
		// rst:
		// rst:			:returns: the subset :math:`\mathcal{S}`.
		// rst:
		const std::vector<std::shared_ptr<graph::Graph>> &getSubset() const;
		// rst:		.. function:: const std::vector<std::shared_ptr<graph::Graph>> &getUniverse() const
		// rst:
		// rst:			:returns: the universe :math:`\mathcal{U}`
		// rst:
		const std::vector<std::shared_ptr<graph::Graph>> &getUniverse() const;
	private:
		mutable bool subsetInit = false, universeInit = false;
		mutable std::vector<std::shared_ptr<graph::Graph>> subset, universe;
		std::function<void(std::vector<std::shared_ptr<graph::Graph>> &)> fSubset, fUniverse;
	};
	// rst-nested-end:
public:
	explicit Strategy(std::unique_ptr<lib::DG::Strategies::Strategy> strategy);
	~Strategy();
	std::unique_ptr<Strategy> clone() const;
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const Strategy &strat);
	lib::DG::Strategies::Strategy &getStrategy();
private:
	struct Pimpl;
	std::unique_ptr<Pimpl> p;
public:
	// rst: .. function:: static std::shared_ptr<Strategy> \
	// rst:               makeAdd(bool onlyUniverse, const std::vector<std::shared_ptr<graph::Graph>> &graphs, IsomorphismPolicy graphPolicy)
	// rst:
	// rst:		:returns: an :ref:`strat-addUniverse` strategy if `onlyUniverse` is `true`, otherwise an :ref:`strat-addSubset` strategy.
	// rst:			The `graphPolicy` refers to the checking of each added graph against the internal graph database.
	// rst:		:throws: :class:`LogicError` if there is a `nullptr` in `graphs`.
	static std::shared_ptr<Strategy>
	makeAdd(bool onlyUniverse, const std::vector<std::shared_ptr<graph::Graph>> &graphs, IsomorphismPolicy graphPolicy);
	// rst: .. function:: static std::shared_ptr<Strategy> \
	// rst:               makeAdd(bool onlyUniverse, const std::shared_ptr<Function<std::vector<std::shared_ptr<graph::Graph>>()>> generator, IsomorphismPolicy graphPolicy)
	// rst:
	// rst:		:returns: an :ref:`strat-addUniverse` strategy if `onlyUniverse` is `true`, otherwise an :ref:`strat-addSubset` strategy.
	// rst:			The `graphPolicy` refers to the checking of each added graph against the internal graph database.
	static std::shared_ptr<Strategy>
	makeAdd(bool onlyUniverse, const std::shared_ptr<Function<std::vector<std::shared_ptr<graph::Graph>>()>> generator,
	        IsomorphismPolicy graphPolicy);
	// rst: .. function:: static std::shared_ptr<Strategy> makeSequence(const std::vector<std::shared_ptr<Strategy>> &strategies)
	// rst:
	// rst:		:returns: a :ref:`strat-sequence` strategy.
	// rst:		:throws: :class:`LogicError` if `strategies.empty()`.
	// rst:		:throws: :class:`LogicError` if there is a `nullptr` in `strategies`.
	static std::shared_ptr<Strategy> makeSequence(const std::vector<std::shared_ptr<Strategy>> &strategies);
	// rst: .. function:: static std::shared_ptr<Strategy> makeParallel(const std::vector<std::shared_ptr<Strategy>> &strategies)
	// rst:
	// rst:		:returns: a :ref:`strat-parallel` strategy.
	// rst:		:throws: :class:`LogicError` if `strategies` is empty.
	// rst:		:throws: :class:`LogicError` if there is a `nullptr` in `strategies`.
	static std::shared_ptr<Strategy> makeParallel(const std::vector<std::shared_ptr<Strategy>> &strategies);
	// rst: .. function:: static std::shared_ptr<Strategy> makeFilter(bool alsoUniverse, std::shared_ptr<Function<bool(std::shared_ptr<graph::Graph>, const Strategy::GraphState&, bool)>> filterFunc)
	// rst:
	// rst:		The filtering predicate will be called for each graph in either the subset or the universe.
	// rst:		The predicate is called with the graph and the graph state as arguments, and a bool stating whether
	// rst:		the call is the first in the filtering process.
	// rst:
	// rst:		:returns: a :ref:`strat-filterUniverse` strategy if `alsoUniverse` is `true`, otherwise a :ref:`strat-filterSubset` strategy.
	// rst:		:throws: :class:`LogicError` if `filterFunc` is a null pointer.
	static std::shared_ptr<Strategy> makeFilter(bool alsoUniverse,
	                                            std::shared_ptr<Function<bool(std::shared_ptr<graph::Graph>,
	                                                                          const Strategy::GraphState &,
	                                                                          bool)>> filterFunc);
	// rst: .. function:: static std::shared_ptr<Strategy> makeExecute(std::shared_ptr<Function<void(const Strategy::GraphState&)>> func)
	// rst:
	// rst:		:returns: an :ref:`strat-execute` strategy.
	// rst:		:throws: :class:`LogicError` if `func` is a null pointer.
	static std::shared_ptr<Strategy> makeExecute(std::shared_ptr<Function<void(const Strategy::GraphState &)>> func);
	// rst: .. function:: static std::shared_ptr<Strategy> makeRule(std::shared_ptr<rule::Rule> r)
	// rst:
	// rst:		:returns: a :ref:`strat-rule` strategy.
	// rst:		:throws: :class:`LogicError` if `r` is a `nullptr`.
	static std::shared_ptr<Strategy> makeRule(std::shared_ptr<rule::Rule> r);
	// rst: .. function:: static std::shared_ptr<Strategy> makeLeftPredicate(std::shared_ptr<Function<bool(const Derivation&)>> predicate, std::shared_ptr<Strategy> strategy)
	// rst:
	// rst:		Even though the predicate is called with a :class:`Derivation` object, only the left side and the rule of the object is valid.
	// rst:
	// rst:		:returns: a :ref:`strat-leftPredicate` strategy.
	// rst:		:throws: :class:`LogicError` if `predicate` or `strategy` is a `nullptr`.
	static std::shared_ptr<Strategy>
	makeLeftPredicate(std::shared_ptr<Function<bool(const Derivation &)>> predicate, std::shared_ptr<Strategy> strategy);
	// rst: .. function:: static std::shared_ptr<Strategy> makeRightPredicate(std::shared_ptr<Function<bool(const Derivation&) >> predicate, std::shared_ptr<Strategy> strategy)
	// rst:
	// rst:		:returns: a :ref:`strat-rightPredicate` strategy.
	// rst:		:throws: :class:`LogicError` if `predicate` or `strategy` is a `nullptr`.
	static std::shared_ptr<Strategy>
	makeRightPredicate(std::shared_ptr<Function<bool(const Derivation &)>> predicate,
	                   std::shared_ptr<Strategy> strategy);
	// rst: .. function:: static std::shared_ptr<Strategy> makeRevive(std::shared_ptr<Strategy> strategy)
	// rst:
	// rst:		:returns: a :ref:`strat-revive` strategy.
	// rst:		:throws: :class:`LogicError` if `strategy` is a `nullptr`.
	static std::shared_ptr<Strategy> makeRevive(std::shared_ptr<Strategy> strategy);
	// rst: .. function:: static std::shared_ptr<Strategy> makeRepeat(int limit, std::shared_ptr<Strategy> strategy)
	// rst:
	// rst:		:returns: a :ref:`strat-repeat` strategy.
	// rst:		:throws: :class:`LogicError` if `limit < 0`
	// rst:		:throws: :class:`LogicError` if `strategy` is a `nullptr`.
	static std::shared_ptr<Strategy> makeRepeat(int limit, std::shared_ptr<Strategy> strategy);
};
// rst-class-end:

} // namespace mod::dg

#endif // MOD_DG_STRATEGIES_HPP