#include <mod/py/Error.hpp>

#include <mod/Function.hpp>
#include <mod/causality/Petri.hpp>
#include <mod/causality/Stochsim.hpp>

namespace mod::causality::Py {
namespace {

Simulator::DrawMassAction *
DrawMassAction_ctor(std::shared_ptr<mod::Function<std::pair<double, bool>(dg::DG::Vertex)>> inputRate,
                    std::shared_ptr<mod::Function<std::pair<double, bool>(dg::DG::HyperEdge)>> reactionRate,
                    std::shared_ptr<mod::Function<std::pair<double, bool>(dg::DG::Vertex)>> outputRate,
                    const std::string &implementation) {
	return new Simulator::DrawMassAction(
			toStdFunction(inputRate),
			toStdFunction(reactionRate),
			toStdFunction(outputRate),
			implementation
			);
}

std::shared_ptr<Simulator>
Simulator_ctor(LabelSettings labelSettings,
               const std::vector<std::shared_ptr<graph::Graph>> &graphDatabase,
               IsomorphismPolicy graphPolicy,
               std::shared_ptr<mod::Function<bool(
		               std::shared_ptr<dg::Builder>,
		               const std::vector<std::shared_ptr<graph::Graph>> &,
		               const std::vector<std::shared_ptr<graph::Graph>> &)>> expandNetwork,
               const std::vector<std::pair<std::shared_ptr<graph::Graph>, int>> &initialState,
               std::shared_ptr<mod::Function<std::shared_ptr<Simulator::DrawFunction>(const Marking &)>> draw,
               std::shared_ptr<mod::Function<double(double)>> drawTime,
               bool withSetCompare) {
	return std::make_shared<Simulator>(labelSettings, graphDatabase, graphPolicy, toStdFunction(expandNetwork),
	                                   initialState, toStdFunction(draw), toStdFunction(drawTime), withSetCompare);
}

void Simulator_setOnIterationBegin(Simulator &self, std::shared_ptr<mod::Function<void(Simulator &)>> f, int interval) {
	self.setOnIterationBegin(toStdFunction(f), interval);
}

void Simulator_setOnIterationEnd(Simulator &self, std::shared_ptr<mod::Function<bool(Simulator &)>> f) {
	self.setOnIterationEnd(toStdFunction(f));
}

void Simulator_setOnDeadlock(Simulator &self, std::shared_ptr<mod::Function<void(Simulator &)>> f) {
	self.setOnDeadlock(toStdFunction(f));
}

void Simulator_setOnExpand(Simulator &self, std::shared_ptr<mod::Function<void(Simulator &)>> f) {
	self.setOnExpand(toStdFunction(f));
}

void Simulator_setOnExpandAvoided(Simulator &self, std::shared_ptr<mod::Function<void(Simulator &)>> f) {
	self.setOnExpandAvoided(toStdFunction(f));
}

struct DrawFunctionWrapper : Simulator::DrawFunction, py::wrapper<DrawFunctionWrapper> {
	virtual void syncSize() override {
		if(py::override f = this->get_override("syncSize")) {
			f();
		} else {
			throw mod::Py::MethodOverrideError("syncSize", "causality.Simulator.DrawFunction", "method not found.");
		}
	}

	virtual std::pair<Choice, double> draw() override {
		if(py::override f = this->get_override("draw")) {
			py::tuple pyRet = f();
			if(py::len(pyRet) != 2)
				throw mod::Py::MethodOverrideError("draw", "causality.Simulator.DrawFunction",
				                                   " returned tuple does not have length 2.");

			return f();
		} else {
			throw mod::Py::MethodOverrideError("draw", "causality.Simulator.DrawFunction", "method not found.");
		}
	}

	virtual void stateUpdated() override {
		if(py::override f = this->get_override("stateUpdated")) {
			f();
		} else {
			throw mod::Py::MethodOverrideError("stateUpdated", "causality.Simulator.DrawFunction", "method not found.");
		}
	}
};

const EventTrace &Simulator_simulate(Simulator &self, py::object pyTime, bool advanceToEndTime,
                                     py::object pyIterations, bool keepNetworkOpen) {
	std::optional<double> time;
	if(!pyTime.is_none()) time = py::extract<double>(pyTime);
	std::optional<int> iterations;
	if(!pyIterations.is_none()) iterations = py::extract<int>(pyIterations);
	return self.simulate(time, advanceToEndTime, iterations, keepNetworkOpen);
}

} // namespace

void Stochsim_doExport() {
	// rst:
	// rst:	.. versionadded:: 1.1
	// rst:
	// rst: The :class:`causality.Simulator` class implements the Stochastic Simulation Algorithm, also known as the Gillespie algorithm,
	// rst: but with the ability to generate the underlying network as needed,
	// rst: similarly to how a derivation graph, :class:`DG`, can be created,
	// rst: and in particular the generation through graph transformation rules and strategies, :ref:`dgStrat`.
	// rst:

	// rst:
	// rst: .. class:: causality.Simulator
	// rst:
	// rst:		The main class for performing stochastic simulations of chemical systems.
	// rst:
	py::scope simulatorScope = py::class_<Simulator, boost::noncopyable>("Simulator", py::no_init)
	                           // rst:		.. method:: __init__(*, labelSettings=LabelSettings(LabelType.String, LabelRelation.Isomorphism), \
	                           // rst:		                     graphDatabase=[], graphPolicy=IsomorphismPolicy.Check, expandNetwork, \
	                           // rst:		                     initialState, draw=..., drawTime=..., withSetCompare=...)
	                           // rst:
	                           // rst:			:param LabelSettings labelSettings: a settings object that will be passed to :func:`DG.__init__`.
	                           // rst:			:param list[Graph] graphDatabase: a list of graphs that will be passed to :func:`DG.__init__`.
	                           // rst:			:param IsomorphismPolicy graphPolicy: the policy that will be passed to :func:`DG.__init__`.
	                           // rst:			:param expandNetwork: a callback which will be called when the simulator needs new hyperedge/reaction information.
	                           // rst:				The callback will be given:
	                           // rst:
	                           // rst:				1. the DG builder object for the underlying derivation graph.
	                           // rst:				2. a list of graphs that were newly discovered in the last iteration.
	                           // rst:				3. a list of all graphs in the current state.
	                           // rst:
	                           // rst:				The callback must return a boolean which indicates whether it should be called again.
	                           // rst:
	                           // rst:				If you have a derivation graph expansion strategy, :class:`DGStrat`,
	                           // rst:				you can use :class:`ExpandByStrategy` as a shorthand to create an appropriate callback.
	                           // rst:				E.g., if you simply want to use all loaded rules you can give ``ExpandByStrategy(inputRules)``
	                           // rst:				as the callback.
	                           // rst: 				
	                           // rst:				If you wish to simply provide a static network without dynamic expansion, your callback could be
	                           // rst:
	                           // rst:				.. code-block:: python
	                           // rst:
	                           // rst:					def expandNetwork(b, subset, universe):
	                           // rst:						# use the DG.Builder b to add reactions/hyperedges as needed to create the network
	                           // rst:						return False  # don't call again, we have added everything we need.
	                           // rst:
	                           // rst:			:type expandNetwork: Callable[[DG.Builder, list[Graph], list[Graph]], bool]
	                           // rst:			:param initialState: the initial simulation state in terms of the number of	copies of each graph/molecule.
	                           // rst:				The graphs/molecules not mentioned are not considered part of the simulation yet,
	                           // rst:				and are thus implicitly assumed to be 0.
	                           // rst:				
	                           // rst:				.. caution:: If you use input flow through :class:`DrawMassAction` then the input rate is only
	                           // rst:					queried once a graph/molecule is known to the simulation, and you must thus mention them
	                           // rst:					in ``initialState`` if they should be queried immediately in the simulation.
	                           // rst:				
	                           // rst:			:type initialState: dict[Graph, int]
	                           // rst:			:param draw: The simulator will initially create a :class:`DG` which is given to this function.
	                           // rst:				It must then return a callable that is used in each simulation step to draw the next hyperedge/reaction.
	                           // rst:				Defaults to a default constructed instance of :class:`DrawMassAction`.
	                           // rst:			:type draw: Callable[[DG], DrawFunction]
	                           // rst:			:param drawTime: The function to use for drawing the time increment in each simulation step.
	                           // rst:				The function will be given the reactivity, and must return the time increment.
	                           // rst:				Defaults to an instance of :class:`DrawTimeExponential`.
	                           // rst:			:type drawTime: Callable[[float], float]
	                           // rst:			:param bool withSetCompare: Whether to skip network expansion if a larger subset of graphs has been used for expansion before.
	                           // rst:				Defaults to ``True``.
	                           // rst:			:raises: :class:`LogicError` if ``draw(dg)`` returns ``None`` for the internally created derivation graph ``dg``.
	                           .def("__init__", py::make_constructor(&Simulator_ctor))
	                           // rst:		.. property:: dg
	                           // rst:
	                           // rst:			(Read-only) The internal :class:`DG` underlying the simulation.
	                           // rst:
	                           // rst:			:type: DG
	                           .add_property("dg", &Simulator::getDG)
	                           // rst:		.. property:: iteration
	                           // rst:
	                           // rst:			(Read-only) The current iteration number. It starts at 0 and is incremented in the beginning of each iteration.
	                           // rst:
	                           // rst:			:type: int
	                           .add_property("iteration", &Simulator::getIteration)
	                           // rst:		.. property:: time
	                           // rst:
	                           // rst:			(Read-only) The current simulation time.
	                           // rst:
	                           // rst:			:type: float
	                           .add_property("time", &Simulator::getTime)
	                           // rst:		.. method:: state(vg)
	                           // rst:
	                           // rst:			:param vg: the vertex/graph to query the current state with.
	                           // rst:			:type vg: DG.Vertex or Graph
	                           // rst:			:returns: the number of occurrences of the given vertex/graph in the current state.
	                           .def("state", static_cast<int (Simulator::*)(dg::DG::Vertex) const>(&Simulator::state))
	                           .def("state",
	                                static_cast<int (Simulator::*)(std::shared_ptr<graph::Graph>) const>(&
		                                Simulator::state))
	                           // rst:		.. property:: trace
	                           // rst:
	                           // rst:			(Read-only) Returns a copy of the event trace for the entire simulation.
	                           // rst:
	                           // rst:			:type: EventTrace
	                           .add_property("trace", py::make_function(&Simulator::getTrace,
	                                                                    py::return_value_policy<
	                                                                    py::copy_const_reference>()))
							   // rst:		.. attribute:: isNetworkOpen
							   // rst:
							   // rst:			(Read-only) Query whether the underlying network is still open for expansion.
							   // rst:			See also the ``keepNetworkOpen`` parameter of :meth:`simulate`.
							   // rst:
							   // rst:			:type: bool
							   .add_property("isNetworkOpen", &Simulator::isNetworkOpen)
							   // rst:		.. method:: closeNetwork()
							   // rst:
							   // rst:			Manually close the network for expansion, instead of letting the :meth:`simulate` method do it.
							   // rst:			The method does nothing if the network is already closed.
							   // rst:			If the user saved the :class:`DG.Builder` object in the expansion callback,
							   // rst:			this method also does nothing.
							   .def("closeNetwork", &Simulator::closeNetwork)
	                           // rst:		.. method:: setOnIterationBegin(callback, interval)
	                           // rst:
	                           // rst:			Set/remove a callback invoked in the very beginning of iterations of the simulation.
	                           // rst:			just after :attr:`iteration` has been incremented.
	                           // rst:			It is called at every ``interval`` th iteration,
	                           // rst:			The callback is invoked with the simulator object as argument.
	                           // rst:
	                           // rst:			:param callback: The callback to set, or ``None`` to remove the callback.
	                           // rst:			:type callback: None or Callable[[Simulator], None]
	                           // rst:			:raises: :class:`LogicError` if ``interval`` is non-positive.
	                           .def("setOnIterationBegin", &Simulator_setOnIterationBegin)
	                           // rst:		.. attribute:: onIterationEnd
	                           // rst:
	                           // rst:			(Read-only) A callback invoked in the very end of each iteration of the simulation,
	                           // rst:			after the drawn action has been carried out and the time advanced.
	                           // rst:			It is called with the simulator object,
	                           // rst:			and it must return a boolean indicating whether to continue the simulation, i.e., ``True`` means continue.
	                           // rst:
	                           // rst:			:type: Callable[[Simulator], bool]
	                           .add_property("onIterationEnd", &mod::Py::noGet, &Simulator_setOnIterationEnd)
	                           // rst:		.. attribute:: onDeadlock
	                           // rst:
	                           // rst:			(Read-only) A callback invoked if there are no events out of the current state.
	                           // rst:			It is called with the simulator object as argument.
	                           // rst:
	                           // rst:			:type: Callable[[Simulator], None]
	                           .add_property("onDeadlock", &mod::Py::noGet, &Simulator_setOnDeadlock)
	                           // rst:		.. attribute:: onExpand
	                           // rst:
	                           // rst:			(Read-only) A callback invoked when the simulator is about to request events out of the current state.
	                           // rst:			It is called with the simulator object as argument.
	                           // rst:
	                           // rst:			:type: Callable[[Simulator], None]
	                           .add_property("onExpand", &mod::Py::noGet, &Simulator_setOnExpand)
	                           // rst:		.. attribute:: onExpandAvoided
	                           // rst:
	                           // rst:			(Read-only) A callback invoked when the simulator detected is already had all events out of the current state.
	                           // rst:			It is called with the simulator object as argument.
	                           // rst:
	                           // rst:			:type: Callable[[Simulator], None]
	                           .add_property("onExpandAvoided", &mod::Py::noGet, &Simulator_setOnExpandAvoided)
	                           // rst:		.. method:: simulate(*, time=None, advanceToEndTime=False,	iterations=None, keepNetworkOpen=False)
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
	                           // rst:			:param bool keepNetworkOpen: if ``False`` the internal :py:class:`DG.Builder` object will be deleted before returning. Defaults to ``False``.
	                           // rst:			:returns: a copy of the event trace for the entire simulation.
	                           // rst:			:rtype: EventTrace
	                           .def("simulate", py::make_function(&Simulator_simulate,
	                                                              py::return_value_policy<py::copy_const_reference>()));

	// rst:
	// rst:	.. class:: causality.Simulator.DrawTimeExponential
	// rst:
	// rst:		A shorthand for drawing time from an exponential distribution.
	// rst:
	py::class_<Simulator::DrawTimeExponential>("DrawTimeExponential")
			// rst:		.. method:: call(activitySum)
			// rst:
			// rst:			:param float activitySum: the total sum of activity in the system.
			// rst:			:returns: :math:`\frac{-\ln r}{activitySum}`, where :math:`r` is a random number in :math:`[0, 1)` drawn with :func:`rngUniformReal`.
			// rst:			:rtype: float
			// rst:
			.def("__call__", &Simulator::DrawTimeExponential::operator());

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
	// rst:		.. method:: __call__(b, subset, universe)
	// rst:
	// rst:			Executes the stored strategy on the given subset and universe, and returns ``True``.
	// rst: 		It is equivalent to the following callback implementation.
	// rst:
	// rst:			.. code-block:: python
	// rst:
	// rst:				def expandNetwork(b, subset, universe):
	// rst:					b.execute(addSubset(subset) >> addUniverse(universe) >> strat, verbosity=0)
	// rst:					return True  # call again when the simulation
	// rst:
	// rst:			:param DG.Builder b: the builder for the derivation graph underlying the simulation.
	// rst:			:param list[Graph] s: the set of new molecules in this iteration.
	// rst:			:param list[Graph] u: the set of molecules in the current state.
	// rst:			:returns: ``True``
	// rst:			:rtype: bool
	// rst:

	// rst:
	// rst: .. class:: causality.Simulator.DrawFunction
	// rst:
	// rst:		The base class that event drawing functions must inherit from.
	// rst:
	// rst:		In each iteration a :class:`causality.Simulator` must draw the next event that should happen.
	// rst:		How to do this drawing can be customized, but such a drawing function needs detailed information
	// rst:		about the underlying network and as the network expands, the drawing function must be kept in sync with the network-
	// rst:		The drawing function customization is therefore done when a slightly indirect manner.
	// rst:		Instead of giving the drawing function directly, you give a function that can create a drawing function.
	// rst:		That is, the :class:`causality.Simulator` will create an internal :class:`causality.Marking` and give it as
	// rst:		argument to the function you give. Your function must then create an actual drawing function,
	// rst:		which must inherit from :class:`causality.Simulator.DrawFunction` and implement the appropriate methods.
	// rst:
	// rst:		For an example of a drawing function, see :class:`causality.Simulator.DrawMassAction.Function`,
	// rst:		and its creator function :class:`causality.Simulator.DrawMassAction`, which is the one users interact with.
	// rst:
	{
		py::scope drawFunctionScope =
	py::class_<DrawFunctionWrapper, std::shared_ptr<DrawFunctionWrapper>, boost::noncopyable>("DrawFunction")
			// rst:		.. method:: syncSize()
			// rst:
			// rst:			Called whenever the underlying derivation graph has changed size.
			// rst:			If the drawing function has internal data structures, this method
			// rst:			is where such data structures can be resized.
			// rst:			The derivation graph must be given to this object by its creator.
			// rst:
			.def("syncSize", py::pure_virtual(&Simulator::DrawFunction::syncSize))
			// rst:		.. method:: draw()
			// rst:
			// rst:			Called in order to draw the next event.
			// rst:
			// rst:			:returns: the drawn action to take and a number indicating the activity of the system.
			// rst:			:rtype: tuple[Choice, float]
			// rst:
			.def("draw", py::pure_virtual(&Simulator::DrawFunction::draw))
			// rst: 	.. method:: stateUpdated()
			// rst:
			// rst:			Updates internal caches after the state has been updated.
			// rst:			This function must be invoked inbetween calls to :py:meth:`draw`.
			.def("stateUpdated", py::pure_virtual(&Simulator::DrawFunction::stateUpdated));
		py::implicitly_convertible<std::shared_ptr<DrawFunctionWrapper>, std::shared_ptr<Simulator::DrawFunction>>();
		py::implicitly_convertible<std::shared_ptr<Simulator::DrawMassAction::Function>,
								   std::shared_ptr<Simulator::DrawFunction>>();

		// rst:
		// rst: .. class:: causality.Simulator.DrawFunction.Choice
		// rst:
		// rst:		The class used in the return type of :meth:`causality.Simulator.DrawFunction.draw`.
		// rst:		It a union type of :class:`causality.EdgeAction`, :class:`causality.InputAction`, and :class:`causality.OutputAction`.
		// rst:		It can be constructed from either of these types.
		// rst:
		// rst:		.. method:: asAction()
		// rst:
		// rst:			:returns: the represented action.
		// rst:			:rtype: :class:`causality.EdgeAction` | :class:`causality.InputAction` | :class:`causality.OutputAction`.
		py::class_<Simulator::DrawFunction::Choice>("Choice")
			.def("asAction", &Simulator::DrawFunction::Choice::asAction);
		py::implicitly_convertible<EdgeAction, Simulator::DrawFunction::Choice>();
		py::implicitly_convertible<InputAction, Simulator::DrawFunction::Choice>();
		py::implicitly_convertible<OutputAction, Simulator::DrawFunction::Choice>();
	}

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
	{
		py::scope drawMassActionScope
				= py::class_<Simulator::DrawMassAction>("DrawMassAction", py::no_init)
				  .def("__init__", py::make_constructor(&DrawMassAction_ctor))
				  // rst:		.. method:: __call__(state)
				  // rst:
				  // rst:			:param Marking state: the state of the underlying simulation.
				  // rst:				It must be kept alive as long as the returned drawing function is kept alive.
				  // rst:			:returns: a drawing function implementing the law of mass action.
				  // rst:			:rtype: DrawMassAction.Function
				  .def("__call__", &Simulator::DrawMassAction::operator());

		// rst:
		// rst:		.. class:: Function(causality.Simulator.DrawFunction)
		py::class_<Simulator::DrawMassAction::Function, std::shared_ptr<Simulator::DrawMassAction::Function>,
				   boost::noncopyable>("Function", py::no_init)
				.def("syncSize", &Simulator::DrawMassAction::Function::syncSize)
				.def("draw", &Simulator::DrawMassAction::Function::draw)
				.def("stateUpdated", &Simulator::DrawMassAction::Function::stateUpdated);
	}
}

} // namespace mod::causality::Py
