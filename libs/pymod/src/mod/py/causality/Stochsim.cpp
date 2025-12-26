#include <mod/py/Common.hpp>

#include <mod/Function.hpp>
#include <mod/causality/Petri.hpp>
#include <mod/causality/Stochsim.hpp>

namespace mod::causality::Py {
namespace {

std::shared_ptr<DrawMassActionFunction>
DrawMassActionFunction_ctor(std::shared_ptr<dg::DG> dg_,
                            std::shared_ptr<mod::Function<std::pair<double, bool>(dg::DG::Vertex)>> inputRate,
                            std::shared_ptr<mod::Function<std::pair<double, bool>(dg::DG::HyperEdge)>> reactionRate,
                            std::shared_ptr<mod::Function<std::pair<double, bool>(dg::DG::Vertex)>> outputRate) {
	return std::make_shared<DrawMassActionFunction>(dg_,
	                                                toStdFunction(inputRate),
	                                                toStdFunction(reactionRate),
	                                                toStdFunction(outputRate));
}

std::shared_ptr<SimulatorImpl>
SimulatorImpl_ctor() {
	return std::make_shared<SimulatorImpl>();
}

} // namespace

void Stochsim_doExport() {
	// rst:
	// rst: The :class:`causality.Simulator` class implements the Stochastic Simulation Algorithm, also known as the Gillespie algorithm,
	// rst: but with the ability to generate the underlying network as needed using graph transformation rules,
	// rst: similarly to how a derivation graph, :class:`DG`, can be expanded using strategies, :ref:`dgStrat`.
	// rst:
	// rst:
	// rst:	.. versionadded:: 1.1
	// rst:
	// rst:	.. todo:: check version added before release

	// rst:
	// rst: .. class:: causality.Simulator
	// rst:
	// rst:		The main class for performing stochastic simulations of chemical systems.
	// rst:
	// rst:		.. method:: __init__(*, labelSettings, graphDatabase, expandNetwork
	// rst:
	// rst:			:param LabelSettings labelSettings: a settings object that will be passed to :func:`DG.__init__`.
	// rst:			:param list[Graph] graphDatabase: a list of graphs that will be	passed to :func:`DG.__init__`.
	// rst:			:param expandNetwork: a callback which will be called when the simulator needs new hyperedge/reaction information.
	// rst:				The callback will be given:
	// rst:
	// rst:				1. the DG builder object for the underlying derivation graph.
	// rst:				2. a list of graphs that were newly discovered in the last iteration.
	// rst:				3. a list of all graphs in the current state.
	// rst:
	// rst:				The callback must return a boolean which indicates whether it should be called again.
	// rst:
	// rst:				For example, if expansion is done via some :py:class:`DGStrat`, ``strat``, a reasonable callback could be
	// rst:
	// rst:				.. code-block:: python
	// rst:
	// rst:					def expandNetwork(b, subset, universe):
	// rst:						b.execute(addSubset(subset) >> addUniverse(universe) >> strat, verbosity=0)
	// rst:						return True  # call again when the simulation
	// rst:
	// rst:				If you wish to simply provide a static network without dynamic expansion, your callback could be
	// rst:
	// rst:				.. code-block:: python
	// rst:
	// rst:					def expandNetwork(b, subset, universe):
	// rst:						# use the DG.Builder b to add reactions/hyperedges as needed to create the network
	// rst:						return False  # don't call again, we have added everything we need.
	// rst:
	// rst:				The :py:class:`ExpandByStrategy` class is a shorthand for this type of callback.
	// rst:			:type expandNetwork: Callable[[DG.Builder, list[Graph], list[Graph]], bool]
	// rst:			:param initialState: the initial simulation state in terms of the number of	copies of each graph/molecule.
	// rst:				The graphs/molecules not mentioned assumed to be 0.
	// rst:			:type initialState: dict[Graph, int]
	// rst:			:param draw: The simulator will initially create a :class:`DG` which is given to this function.
	// rst:				It must then return a callable that is used in each simulation step to draw the next hyperedge/reaction.
	// rst:				Defaults to a default constructed instance of :class:`DrawMassAction`.
	// rst:			:type draw: Callable[[DG], DrawFunction]
	// rst:			:param drawTime: The function to use for drawing the time increment in each simulation step.
	// rst:				The function will be given the reactivity, and must return the time increment.
	// rst:				Defaults to an instance of :py:class:`DrawTimeExponential`.
	// rst:			:type drawTime: Callable[[float], float]
	// rst:			:param bool withSetCompare: Whether to skip network expansion if a larger subset of graphs has been used for expansion before.
	// rst:				Defaults to ``True``.
	// rst:
	// rst:		.. property:: dg
	// rst:
	// rst:			The internal :class:`DG` underlying the simulation.
	// rst:
	// rst:			:type: DG
	// rst:
	// rst:		.. property:: iteration
	// rst:
	// rst:			The current iteration number. It starts at 0 and is incremented in the beginning of each iteration.
	// rst:
	// rst:			:type: int
	// rst:
	// rst:		.. property:: time
	// rst:
	// rst:			The current simulation time.
	// rst:
	// rst:			:type: float
	// rst:
	// rst:		.. method:: state(vg)
	// rst:
	// rst:			:param vg: the vertex/graph to query the current state with.
	// rst:			:type vg: DG.Vertex or Graph
	// rst:			:returns: the number of occurrences of the given vertex/graph in the current state.
	// rst:
	// rst:		.. property:: trace
	// rst:
	// rst:			References to the event trace for the entire simulation.
	// rst:			It should not be modified.
	// rst:
	// rst:			:type: EventTrace
	// rst:
	// rst:		.. attribute:: onIterationBegin
	// rst:
	// rst:			A callback invoked in the very beginning of each iteration of the simulation,
	// rst:			just after :attr:`iteration` has been incremented.
	// rst:			It is called with the simulator object as argument.
	// rst:
	// rst:			:type: Callable[[Simulator], None]
	// rst:
	// rst:		.. attribute:: onIterationEnd
	// rst:
	// rst:			A callback invoked in the very end of each iteration of the simulation,
	// rst:			after the drawn action has been carried out and the time advanced.
	// rst:			It is called with the simulator object, the drawn action, and the time increment,
	// rst:			and it must return a boolean indicating whether to continue the simulation, i.e., ``True`` means continue.
	// rst:
	// rst:			:type: Callable[[Simulator, EdgeAction | InputAction | OutputAction, float], bool]
	// rst:
	// rst:		.. attribute:: onDeadlock
	// rst:
	// rst:			A callback invoked if there are no events out of the current state.
	// rst:			It is called with the simulator object as argument.
	// rst:
	// rst:			:type: Callable[[Simulator], None]
	// rst:
	// rst:		.. attribute:: onExpand
	// rst:
	// rst:			A callback invoked when the simulator is about to request events out of the current state.
	// rst:			It is called with the simulator object as argument.
	// rst:
	// rst:			:type: Callable[[Simulator], None]
	// rst:
	// rst:		.. attribute:: onExpandAvoided
	// rst:
	// rst:			A callback invoked when the simulator detected is already had all events out of the current state.
	// rst:			It is called with the simulator object as argument.
	// rst:
	// rst:			:type: Callable[[Simulator], None]
	// rst:
	// rst:		.. method:: simulate(*, time, advanceToEndTime,	iterations, keepNetworkOpen=False)
	// rst:
	// rst:			Start/continue the simulation.
	// rst:
	// rst:			Simulate an additional amount of time or number of iterations, whichever is reached first,
	// rst:			or until no further events are possible (a deadlock).
	// rst:
	// rst:			:param Optional[float] time: the additional amount of time to simulate,	or ``None`` for unbounded. Defaults to ``None``.
	// rst:			:param bool advanceToEndTime: if a time bound is given and the simulation stops due to this bound,
	// rst:				advance the current time to the time bound,	instead of staying at the time of the last event. Defaults to ``False``.
	// rst:			:param Optional[int] iterations: the additional number of iterations to simulate, or ``None`` for unbounded. Defaults to ``None``.
	// rst:			:param bool keepNetworkOpen: is ``False`` the internal `DG.Builder` object will be deleted before returning. Defaults to ``False``.
	// rst:			:returns: a references to the event trace for the entire simulation.
	// rst:				It should not be modified.
	// rst:			:rtype: EventTrace
	// rst:

	// rst:
	// rst:	.. class:: causality.Simulator.DrawTimeExponential
	// rst:
	// rst:		A shorthand for drawing time from an exponential distribution.
	// rst:
	// rst:		.. method:: call(activitySum)
	// rst:
	// rst:			:param float rateSum: the total sum of activity in the system.
	// rst:			:returns: :math:`\frac{-\ln r}{activitySum}`, where :math:`r` is a random number in :math:`[0, 1)` drawn with :func:`rngUniformReal`.
	// rst:			:rtype: float
	// rst:

	// rst:
	// rst:	.. class:: causality.Simulator.ExpandByStrategy
	// rst:
	// rst:		A shorthand for an expansion callback that executes a strategy.
	// rst:
	// rst:		.. method:: __init__(strat)
	// rst:
	// rst:			:param strat: a strategy to execute each time more neighbourhood is needed for the simulation.
	// rst:				It can be any object that can be used as a strategy, see :ref:`dgStrat`.
	// rst:
	// rst:		.. method:: __call__(b, s, u)
	// rst:
	// rst:			Executes the stored strategy on the given subset and universe.
	// rst:
	// rst:			:param DG.Builder b: the builder for the derivation graph underlying the simulation.
	// rst:			:param list[Graph] s: the set of new molecules in this iteration.
	// rst:			:param list[Graph] u: the set of molecules in the current state.
	// rst:

	// rst:
	// rst:	.. class:: causality.Simulator.DrawMassAction
	// rst:
	// rst:		A creator for a drawing function implementing the law of mass action.
	// rst:
	// rst:		It supports assigning a rate for input actions, output actions, and reactions,
	// rst:		by taking a callback (or constant) for each type.
	// rst:		To avoid the overhead of calling these callbacks in each iteration, a returned rate can be cached.
	// rst:		Therefore, the return value of each callback (or the constant of each type) is a pair
	// rst:		with the first entry being the rate, and the second entry a boolean indicating whether rate should be cached.
	// rst:
	// rst:		Each of the rate function can also be set to ``None``,
	// rst:		which means a default rate is used: input rate 0.0, reaction rate 1.0, output rate 0.0.
	// rst:
	// rst:		:param inputRate: the rate used for pseudo-reactions for creating molecules. Defaults to ``None``.
	// rst:		:type inputRate: Callable[[DG.Vertex], Tuple[float, bool]] or Tuple[float, bool] or None
	// rst:		:param reactionRate: the rate used for each reaction in the system. Defaults to ``None``.
	// rst:		:type reactionRate: Callable[[DG.Vertex], Tuple[float, bool]] or Tuple[float, bool] or None
	// rst:		:param outputRate: the rate used for pseudo-reactions for destroying molecules. Defaults to ``None``.
	// rst:		:type outputRate: Callable[[DG.Vertex], Tuple[float, bool]]	or Tuple[float, bool] or None
	// rst:
	// rst:		.. method:: __call__(dg)
	// rst:
	// rst:			:param DG dg: the derivation graph underlying the simulation.
	// rst:			:returns: a drawing function implementing the law of mass action.
	// rst:			:rtype: DrawMassAction.Function
	// rst:

	py::class_<DrawMassActionFunction>("_DrawMassActionFunction", py::no_init)
			.def("__init__", py::make_constructor(&DrawMassActionFunction_ctor))
			.def("syncSize", &DrawMassActionFunction::syncSize)
			.def("draw", &DrawMassActionFunction::draw);

	// rst:
	// rst: .. class:: causality.DrawFunction
	// rst:
	// rst:		The protocol (see :ref:`py-protocols`) that event drawing functions must implement.
	// rst:
	// rst:		In each iteration a :class:`causality.Simulator` must draw the next event that should happen.
	// rst:		How to do this drawing can be customized, but as such a drawing function needs detailed information
	// rst:		about the underlying network and must be kept in sync with this network as it expands,
	// rst:		the customization is done when a slightly indirect manner.
	// rst:		Instead of giving the drawing function directly, you give a function that can create a drawing function.
	// rst:		That is, the :class:`causality.Simulator` will create an internal :class:`DG` and give it as argument to
	// rst:		the function you give. Your function must then create an actual drawing function,
	// rst:		which must adhere to the interface specified by this :class:`causality.DrawFunction` protocol.
	// rst:
	// rst:		For an example of a drawing function, see :class:`causality.Simulator.DrawMassAction.Function`,
	// rst:		and its creator function :class:`causality.Simulator.DrawMassAction`, which is the one users interact with.
	// rst:
	// rst:		.. method:: syncSize()
	// rst:
	// rst:			Called whenever the underlying derivation graph has changed size.
	// rst:			If the drawing function has internal data structures, this method
	// rst:			is where such data structures can be resized.
	// rst:			The derivation graph must be given to this object by its creator.
	// rst:
	// rst:		.. method:: draw(marking)
	// rst:
	// rst:			Called in order to draw the next event.
	// rst:
	// rst:			:param Marking marking: the current state of the simulation.
	// rst:				This may not be changed.
	// rst:			:returns: the drawn action to take and a number indicating the activity of the system.
	// rst:			:rtype: tuple[Action, float]
	// rst:


	py::class_<SimulatorImpl, boost::noncopyable>("_SimulatorImpl", py::no_init)
			.def("__init__", py::make_constructor(&SimulatorImpl_ctor))
			.add_property("iteration", &SimulatorImpl::getIteration)
			.add_property("time", &SimulatorImpl::getTime, &SimulatorImpl::setTime_delete)
			.def("doIteration", &SimulatorImpl::doIteration);
}

} // namespace mod::causality::Py