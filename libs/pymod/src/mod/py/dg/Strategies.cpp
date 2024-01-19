#include <mod/py/Common.hpp>

#include <mod/dg/DG.hpp>
#include <mod/dg/Strategies.hpp>

// rst: This section describes two interfaces for the derivation graph strategies;
// rst: the basic API and an embedded language which is built on the basic API.
// rst: Usually the embedded strategy language is easiest and sufficient for constructing strategies.
// rst:
// rst: The semantics of the individual strategies are described in :ref:`dgStrat`.
// rst: Note that a :class:`DGStrat` is a representation of a strategy and must be given to a derivation graph to be evaluated.
// rst:
// rst: .. _dg_edsl:
// rst:
// rst: The Embedded Strategy Language
// rst: ###############################
// rst:
// rst: The strategy language is really a collection of proxy classes with a lot of operator overloading, thus
// rst: the normal syntax and semantics of Python applies.
// rst:
// rst: The following is the grammar for the strategies.
// rst:
// rst: .. productionlist:: dgStrat
// rst:    strat: strats
// rst:         : `strat` ">>" `strat`
// rst:         : rule
// rst:         : "addSubset(" graphs ")"
// rst:         : "addUniverse(" graphs ")"
// rst:         : "execute(" executeFunc ")"
// rst:         : "filterSubset(" filterPred ")"
// rst:         : "filterUniverse(" filterPred ")"
// rst:         : "leftPredicate[" derivationPred "](" `strat` ")"
// rst:         : "rightPredicate[" derivationPred "](" `strat` ")"
// rst:         : "repeat" [ "[" int "]" ] "(" strat ")"
// rst:         : "revive(" `strat` ")"
// rst:
// rst: A ``strats`` must be an iterable of :token:`~dgStrat:strat`, e.g., an iterable of :class:`Rule`.
// rst: A ``graphs`` can either be a single :class:`Graph`, an iterable of graphs,
// rst: or a function taking no arguments and returning a list of graphs.
// rst:
// rst: The functions in the language have the following signatures.
// rst:
// rst: .. function:: addSubset(g, *gs, graphPolicy=IsomorphismPolicy.Check)
// rst:               addUniverse(g, *gs, graphPolicy=IsomorphismPolicy.Check)
// rst:
// rst:		Depending on ``g`` it calls either :func:`DGStrat.makeAddStatic` or :func:`DGStrat.makeAddDynamic`.
// rst:
// rst:		:param g: graph(s) to add, or a callback to compute them.
// rst:		:type g: Graph or Iterable[Graph] or Callable[[], Iterable[Graph]]
// rst:		:param gs: a variable amount of additional arguments with graphs,
// rst:			unless ``g`` is a callback, then no additional arguments may be given.
// rst:		:type gs: Graph or Iterable[Graph]
// rst:		:param IsomorphismPolicy graphPolicy: the policy to use when adding the graphs.
// rst:			When :attr:`IsomorphismPolicy.Check` is given, and a graph is added which is isomorphic to an existing
// rst:			graph in the internal database of the :class:`DG` the strategy is executed on, then
// rst:			a :class:`LogicError` is thrown.
// rst:
// rst: .. function:: execute(f)
// rst:
// rst:		:returns: the result of :func:`DGStrat.makeExecute`.
// rst:
// rst: .. function:: filterSubset(p)
// rst:               filterUniverse(p)
// rst:
// rst:		:returns: the result of the corresponding :func:`DGStrat.makeFilter`.
// rst:
// rst:
// rst: .. data:: leftPredicate
// rst:           rightPredicate
// rst:
// rst:		Objects of unspecified type which can be used as ``obj[pred](strat)``.
// rst:		This will call respectively :func:`DGStrat.makeLeftPredicate` or :func:`DGStrat.makeRightPredicate`.
// rst:
// rst: .. data:: repeat
// rst:
// rst:		An object of unspecified type which can be used either as ``repeat(strat)`` or ``repeat[limit](strat)``.
// rst:		This will call :func:`DGStrat.makeRepeat`.
// rst:
// rst:
// rst: .. function:: revive(strat)
// rst:
// rst:		:returns: the result of :func:`DGStrat.makeRevive`.
// rst:

namespace mod::dg::Py {

void Strategy_doExport() {
	// rst: The Basic API
	// rst: #################
	// rst:

	std::shared_ptr<Strategy> (*makeAdd_static)(bool,
	                                            const std::vector<std::shared_ptr<graph::Graph>> &,
	                                            IsomorphismPolicy) = &Strategy::makeAdd;
	std::shared_ptr<Strategy> (*makeAdd_dynamic)(bool,
	                                             const std::shared_ptr<mod::Function<std::vector<std::shared_ptr<graph::Graph>>()>>,
	                                             IsomorphismPolicy) = &Strategy::makeAdd;
	// rst: .. class:: DGStrat
	// rst: 
	auto pyStrat = py::class_<Strategy, std::shared_ptr<Strategy>, boost::noncopyable>("DGStrat", py::no_init)
			.def(str(py::self))
					// rst:		.. staticmethod:: makeAddStatic(onlyUniverse, graphs, graphPolicy)
					// rst:
					// rst:			:param bool onlyUniverse: if the strategy is :ref:`strat-addUniverse` or :ref:`strat-addSubset`.
					// rst:			:param graphs: the graphs to be added by the strategy.
					// rst:			:type graphs: list[Graph]
					// rst:			:param IsomorphismPolicy graphPolicy: refers to the checking of each added graph against the internal graph database.
					// rst:			:returns: an :ref:`strat-addUniverse` strategy if ``onlyUniverse`` is ``True``, otherwise an :ref:`strat-addSubset` strategy.
					// rst:			:rtype: DGStrat
					// rst:			:raises: :class:`LogicError` if there is a ``None`` in ``graphs``.
			.def("makeAddStatic", makeAdd_static).staticmethod("makeAddStatic")
					// rst:		.. staticmethod:: makeAddDynamic(onlyUniverse, graphsFunc, graphPolicy)
					// rst:
					// rst:			:param bool onlyUniverse: if the strategy is :ref:`strat-addUniverse` or :ref:`strat-addSubset`.
					// rst:			:param graphsFunc: a function returning the graphs to be added by the strategy.
					// rst:			:type graphsFunc: Callable[[], list[Graph]]
					// rst:			:param IsomorphismPolicy graphPolicy: refers to the checking of each added graph against the internal graph database.
					// rst:			:returns: an :ref:`strat-addUniverse` strategy if ``onlyUniverse`` is ``True``, otherwise an :ref:`strat-addSubset` strategy.
					// rst:			:rtype: DGStrat
			.def("makeAddDynamic", makeAdd_dynamic).staticmethod("makeAddDynamic")
					// rst:		.. staticmethod:: makeSequence(strats)
					// rst:
					// rst:			:param strats: the strategies to evaluate in sequence.
					// rst:			:type strats: list[DGStrat]
					// rst:			:returns: a :ref:`strat-sequence` strategy.
					// rst:			:rtype: DGStrat
					// rst:			:raises: :class:`LogicError` if the given list of strategies is empty.
					// rst:			:raises: :class:`LogicError` if there is a ``None`` in ``strats``.
			.def("makeSequence", &Strategy::makeSequence).staticmethod("makeSequence")
					// rst:		.. staticmethod:: makeParallel(strats)
					// rst:
					// rst:			:param strats: the sub-strategies to evaluate.
					// rst:			:type strats: list[DGStrat]
					// rst:			:returns: a :ref:`strat-parallel` strategy.
					// rst:			:rtype: DGStrat
					// rst:			:raises: :class:`LogicError` if `strats` is empty.
					// rst:			:raises: :class:`LogicError` if there is a ``None`` in ``strats``.
			.def("makeParallel", &Strategy::makeParallel).staticmethod("makeParallel")
					// rst:		.. staticmethod:: makeFilter(alsoUniverse, p)
					// rst:
					// rst:			:param bool alsoUniverse: if the strategy is :ref:`strat-filterUniverse` or :ref:`strat-filterSubset`.
					// rst:			:param p: the filtering predicate being called for each graph in either the subset or the universe.
					// rst:				The predicate is called with the graph and the graph state as arguments, and a bool stating whether
					// rst:				the call is the first in the filtering process.
					// rst:			:type p: Callable[[Graph, DGStrat.GraphState, bool], bool]
					// rst:			:returns: a :ref:`strat-filterUniverse` strategy if ``onlyUniverse`` is ``True``, otherwise a :ref:`strat-filterSubset` strategy.
					// rst:			:rtype: DGStrat
			.def("makeFilter", &Strategy::makeFilter).staticmethod("makeFilter")
					// rst:		.. staticmethod:: makeExecute(func)
					// rst:
					// rst:			:param func: A function being executed when the strategy is evaluated.
					// rst:			:type func: Callable[[DGStrat.GraphState], None]
					// rst:			:returns: an :ref:`strat-execute` strategy.
					// rst:			:rtype: DGStrat
			.def("makeExecute", &Strategy::makeExecute).staticmethod("makeExecute")
					// rst:		.. staticmethod:: makeRule(r)
					// rst:
					// rst:			:param Rule r: the rule to make into a strategy.
					// rst:			:returns: a :ref:`strat-rule` strategy.
					// rst:			:rtype: DGStrat
					// rst:			:raises: :class:`LogicError` is ``r`` is ``None``.
			.def("makeRule", &Strategy::makeRule).staticmethod("makeRule")
					// rst:		.. staticmethod:: makeLeftPredicate(p, strat)
					// rst:
					// rst:			:param p: the predicate to be called on each candidate derivation.
					// rst:				Even though the predicate is called with a :class:`Derivation` object, only the left side and the rule of the object is valid.
					// rst:			:type p: Callable[[Derivation], bool]
					// rst:			:param DGStrat strat: the sub-strategy to be evaluated under the constraints of the left predicate.
					// rst:			:returns: a :ref:`strat-leftPredicate` strategy.
					// rst:			:rtype: DGStrat
					// rst:			:raises: :class:`LogicError` if ``strat`` is ``None``.
			.def("makeLeftPredicate", &Strategy::makeLeftPredicate).staticmethod("makeLeftPredicate")
					// rst:		.. staticmethod:: makeRightPredicate(p, strat)
					// rst:
					// rst:			:param p: the predicate to be called on each candidate derivation.
					// rst:			:type p: Callable[[Derivation], bool]
					// rst:			:param DGStrat strat: the sub-strategy to be evaluated under the constraints of the right predicate.
					// rst:			:returns: a :ref:`strat-rightPredicate` strategy.
					// rst:			:rtype: DGStrat
					// rst:			:raises: :class:`LogicError` if ``strat`` is ``None``.
			.def("makeRightPredicate", &Strategy::makeRightPredicate).staticmethod("makeRightPredicate")
					// rst:		.. staticmethod:: makeRevive(strat)
					// rst:
					// rst:			:param DGStrat strat: the strategy to encapsulate.
					// rst:			:returns: a :ref:`strat-revive` strategy.
					// rst:			:rtype: DGStrat
					// rst:			:raises: :class:`LogicError` if ``strat`` is ``None``.
			.def("makeRevive", &Strategy::makeRevive).staticmethod("makeRevive")
					// rst:		.. staticmethod:: makeRepeat(limit, strat)
					// rst:
					// rst:			:param int limit: the maximum number of iterations.
					// rst:			:param DGStrat strat: the strategy to be repeated.
					// rst:			:returns: a :ref:`strat-repeat` strategy.
					// rst:			:rtype: DGStrat
					// rst:			:raises: :class:`LogicError` if ``limit`` is negative.
					// rst:			:raises: :class:`LogicError` if ``strat`` is ``None``.
			.def("makeRepeat", &Strategy::makeRepeat).staticmethod("makeRepeat");

	{
		auto scope = py::scope(pyStrat);
		// rst:		.. class:: GraphState
		// rst:
		// rst:			This class represents a graph state with a subset :math:`S` and a universe :math:`U` fulfilling :math:`S\subseteq U`.
		// rst:
		py::class_<Strategy::GraphState>("GraphState", py::no_init)
				// rst:			.. attribute:: subset
				// rst:
				// rst:				The subset :math:`\mathcal{S}`.
				// rst:
				// rst:				:type: list[Graph]
				// rst:
				.add_property("_subset",
				              py::make_function(&Strategy::GraphState::getSubset, py::return_internal_reference<1>()))
						// rst:			.. attribute:: universe
						// rst:
						// rst:				The universe :math:`\mathcal{U}`.
						// rst:
						// rst:				:type: list[Graph]
						// rst:
				.add_property("_universe",
				              py::make_function(&Strategy::GraphState::getUniverse, py::return_internal_reference<1>()));
	}
}

} // namespace mod::dg::Py