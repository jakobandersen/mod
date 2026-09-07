#ifndef MOD_CAUSALITY_STOCHSIM_HPP
#define MOD_CAUSALITY_STOCHSIM_HPP

#include <mod/BuildConfig.hpp>
#include <mod/causality/ForwardDecl.hpp>
#include <mod/causality/EventTrace.hpp>
#include <mod/dg/DG.hpp>
#include <mod/dg/GraphInterface.hpp>

#include <optional>

// rst:
// rst:	.. versionadded:: 1.1
// rst:
// rst: The :class:`causality::Simulator` class implements the Stochastic Simulation Algorithm, also known as the Gillespie algorithm,
// rst: but with the ability to generate the underlying network as needed,
// rst: similarly to how a derivation graph, :class:`dg::DG`, can be created,
// rst: and in particular the generation through graph transformation rules and strategies, :ref:`dgStrat`.
// rst:

namespace mod::lib::Causality {
struct ChoiceAccess;
}
namespace mod::causality {

// rst:
// rst: .. class:: causality::Simulator
// rst:
// rst:		The main class for performing stochastic simulations of chemical systems.
// rst:
struct MOD_DECL Simulator {
	struct DrawTimeExponential;
	struct DrawFunction;
	struct DrawMassAction;
public:
	// rst:		.. type:: ExpandNetwork = std::function<bool(\
	// rst:			std::shared_ptr<dg::Builder>, \
	// rst:			const std::vector<std::shared_ptr<graph::Graph>> &, \
	// rst:			const std::vector<std::shared_ptr<graph::Graph>> &)>
	using ExpandNetwork = std::function<bool(
			std::shared_ptr<dg::Builder>,
			const std::vector<std::shared_ptr<graph::Graph>> &,
			const std::vector<std::shared_ptr<graph::Graph>> &)>;
public:
	// rst:		.. function:: Simulator(LabelSettings labelSettings, \
	// rst:			const std::vector<std::shared_ptr<graph::Graph>> &graphDatabase, \
	// rst:			IsomorphismPolicy graphPolicy, \
	// rst:			ExpandNetwork expandNetwork, \
	// rst:			const std::vector<std::pair<std::shared_ptr<graph::Graph>, int>> &initialState, \
	// rst:			std::function<std::shared_ptr<DrawFunction>(const Marking&)> draw, \
	// rst:			std::function<double(double)> drawTime, \
	// rst:			bool withSetCompare)
	// rst:
	// rst:			:param LabelSettings labelSettings: a settings object that will be passed to :func:`dg::DG::make`.
	// rst:			:param list[Graph] graphDatabase: a list of graphs that will be passed to :func:`dg::DG::make`.
	// rst:			:param IsomorphismPolicy graphPolicy: the policy that will be passed to :func:`dg::DG::make`.
	// rst:			:param expandNetwork: a callback which will be called when the simulator needs new hyperedge/reaction information.
	// rst:				The callback will be given:
	// rst:
	// rst:				1. the DG builder object for the underlying derivation graph.
	// rst:				2. a list of graphs that were newly discovered in the last iteration.
	// rst:				3. a list of all graphs in the current state.
	// rst:
	// rst:				The callback must return a boolean which indicates whether it should be called again.
	// rst:
	// rst:			:param initialState: the initial simulation state in terms of the number of	copies of each graph/molecule.
	// rst:				The graphs/molecules not mentioned are not considered part of the simulation yet,
	// rst:				and are thus implicitly assumed to be 0.
	// rst:
	// rst:				.. caution:: If you use input flow through :class:`DrawMassAction` then the input rate is only
	// rst:					queried once a graph/molecule is known to the simulation, and you must thus mention them
	// rst:					in ``initialState`` if they should be queried immediately in the simulation.
	// rst:
	// rst:			:param draw: The simulator will initially create a :class:`dg::DG` which is given to this function.
	// rst:				It must then return a callable that is used in each simulation step to draw the next hyperedge/reaction.
	// rst:			:param drawTime: The function to use for drawing the time increment in each simulation step.
	// rst:				The function will be given the reactivity, and must return the time increment.
	// rst:				If a null function is given, it will default to an instance of :class:`DrawTimeExponential`.
	// rst:			:param bool withSetCompare: Whether to skip network expansion if a larger subset of graphs has been used for expansion before.
	// rst:			:throws: :class:`LogicError` if `draw` is a null function.
	// rst:			:throws: :class:`LogicError` if ``draw(dg)`` returns a null function for the internally created derivation graph ``dg``.
	Simulator(LabelSettings labelSettings,
	          const std::vector<std::shared_ptr<graph::Graph>> &graphDatabase,
	          IsomorphismPolicy graphPolicy,
	          ExpandNetwork expandNetwork,
	          const std::vector<std::pair<std::shared_ptr<graph::Graph>, int>> &initialState,
	          std::function<std::shared_ptr<DrawFunction>(const Marking&)> draw,
	          std::function<double(double)> drawTime,
	          bool withSetCompare);
	// rst:		.. function:: ~Simulator()
	~Simulator();
public:
	// rst:		.. function:: std::shared_ptr<dg::DG> getDG() const
	// rst:
	// rst:			:returns: the internal :class:`dg::DG` underlying the simulation.
	std::shared_ptr<dg::DG> getDG() const;
	// rst:		.. function:: int getIteration() const
	// rst:
	// rst:			:returns: the current iteration number. It starts at 0 and is incremented in the beginning of each iteration.
	int getIteration() const;
	// rst:		.. function:: double getTime() const
	// rst:
	// rst:			:returns: the current simulation time.
	double getTime() const;
	// rst:		.. function:: int state(dg::DG::Vertex v) const
	// rst:		              int state(std::shared_ptr<graph::Graph> g) const
	// rst:
	// rst:			:returns: the number of occurrences of the given vertex/graph in the current state.
	int state(dg::DG::Vertex v) const;
	int state(std::shared_ptr<graph::Graph> g) const;
	// rst:		.. function:: const EventTrace &getTrace() const
	// rst:
	// rst:			:returns: a copy of the event trace for the entire simulation.
	const EventTrace &getTrace() const;
	// rst:		.. function:: bool isNetworkOpen() const
	// rst:
	// rst:			:returns: whether the underlying network is still open for expansion.
	// rst:				See also the ``keepNetworkOpen`` parameter of :func:`simulate`.
	bool isNetworkOpen() const;
	// rst:		.. function:: void closeNetwork()
	// rst:
	// rst:			Manually close the network for expansion, instead of letting the :func:`simulate` method do it.
	// rst:			The method does nothing if the network is already closed.
	// rst:			If the user saved the :class:`dg::Builder` object in the expansion callback,
	// rst:			this method also does nothing.
	void closeNetwork();
public:
	// rst:		.. function:: void setOnIterationBegin(std::function<void(Simulator &)> f, int interval)
	// rst:
	// rst:			Set/remove a callback invoked in the very beginning of iterations of the simulation.
	// rst:			just after the iteration count has been incremented.
	// rst:			It is called at every ``interval`` th iteration,
	// rst:			The callback is invoked with the simulator object as argument.
	// rst:			Give a null function to remove the current callback.
	// rst:
	// rst:			:throws: :class:`LogicError` if `interval` is non-positive.
	void setOnIterationBegin(std::function<void(Simulator &)> f, int interval);
	// rst:		.. function:: void setOnIterationEnd(std::function<bool(Simulator &)> f)
	// rst:
	// rst:			Set/remove the callback invoked in the very end of each iteration of the simulation,
	// rst:			after the drawn action has been carried out and the time advanced.
	// rst:			It is called with the simulator object,
	// rst:			and it must return a boolean indicating whether to continue the simulation, i.e., `true` means continue.
	void setOnIterationEnd(std::function<bool(Simulator &)> f);
	// rst:		.. function:: void setOnDeadlock(std::function<void(Simulator &)> f)
	// rst:
	// rst:			Set/remove the callback invoked if there are no events out of the current state.
	// rst:			It is called with the simulator object as argument.
	void setOnDeadlock(std::function<void(Simulator &)> f);
	// rst:		.. function:: void setOnExpand(std::function<void(Simulator &)> f)
	// rst:
	// rst:			Set/remove the callback invoked when the simulator is about to request events out of the current state.
	// rst:			It is called with the simulator object as argument.
	void setOnExpand(std::function<void(Simulator &)> f);
	// rst:		.. function:: void setOnExpandAvoided(std::function<void(Simulator &)> f)
	// rst:
	// rst:			Set/remove the callback invoked when the simulator detected is already had all events out of the current state.
	// rst:			It is called with the simulator object as argument.
	void setOnExpandAvoided(std::function<void(Simulator &)> f);
public:
	// rst:		.. function:: const EventTrace &simulate(std::optional<double> time, bool advanceToEndTime, std::optional<int> iterations, bool keepNetworkOpen)
	// rst:
	// rst:			Start/continue the simulation.
	// rst:
	// rst:			Simulate an additional amount of time or number of iterations, whichever is reached first,
	// rst:			or until no further events are possible (a deadlock).
	// rst:
	// rst:			:param time: the additional amount of time to simulate,	or `std::nullopt` for unbounded.
	// rst:			:param advanceToEndTime: if a time bound is given and the simulation stops due to this bound,
	// rst:				advance the current time to the time bound,	instead of staying at the time of the last event.
	// rst:			:param iterations: the additional number of iterations to simulate, or `std::nullopt` for unbounded.
	// rst:			:param keepNetworkOpen: if `false` the internal `dg::Builder` object will be deleted before returning.
	// rst:			:returns: `getTrace()`
	const EventTrace &simulate(std::optional<double> time, bool advanceToEndTime, std::optional<int> iterations, bool keepNetworkOpen);
private:
	struct Pimpl;
	std::unique_ptr<Pimpl> p;
};

// rst:
// rst: .. class:: causality::Simulator::DrawTimeExponential
// rst:
// rst:		A shorthand for drawing time from an exponential distribution.
// rst:
struct MOD_DECL Simulator::DrawTimeExponential {
	// rst:		.. function:: double operator()(double activitySum) const
	// rst:
	// rst:			:param activitySum: the total sum of activity in the system.
	// rst:			:returns: :math:`\frac{-\ln r}{activitySum}`, where :math:`r` is a random number in :math:`[0, 1)` drawn with :func:`rngUniformReal`.
	double operator()(double activitySum) const;
};

// rst:
// rst: .. class:: causality::Simulator::DrawFunction
// rst:
// rst:		The base class that event drawing functions must inherit from.
// rst:
// rst:		In each iteration a :class:`causality::Simulator` must draw the next event that should happen.
// rst:		How to do this drawing can be customized, but such a drawing function needs detailed information
// rst:		about the underlying network and as the network expands, the drawing function must be kept in sync with the network-
// rst:		The drawing function customization is therefore done when a slightly indirect manner.
// rst:		Instead of giving the drawing function directly, you give a function that can create a drawing function.
// rst:		That is, the :class:`causality::Simulator` will create an internal :class:`causality::Marking` and give it as
// rst:		argument to the function you give. Your function must then create an actual drawing function,
// rst:		which must inherit from :class:`causality::Simulator::DrawFunction` and implement the appropriate methods.
// rst:
// rst:		For an example of a drawing function, see :class:`causality::Simulator::DrawMassAction::Function`,
// rst:		and its creator function :class:`causality::Simulator::DrawMassAction`, which is the one users interact with.
// rst:
struct MOD_DECL Simulator::DrawFunction {
	// rst:		.. class:: Choice
	// rst:
	// rst:			The class used in the return type of :func:`causality::Simulator::DrawFunction::draw`.
	// rst:			It is a more efficient representation of a :type:`causality::Action`.
	// rst:
	struct Choice {
		// rst:			.. function:: Choice() = default
		// rst:			              Choice(Action a)
		// rst:			              Choice(InputAction a)
		// rst:			              Choice(OutputAction a)
		// rst:			              Choice(EdgeAction a)
		// rst:
		// rst:				Construct either a null choice or an actual choice from an action.
		Choice() = default;
		Choice(Action a);
		Choice(InputAction a);
		Choice(OutputAction a);
		Choice(EdgeAction a);
	public:
		// rst:			.. function:: std::optional<Action> asAction() const
		// rst:
		// rst:				:returns: a conversion the choice into an action.
		std::optional<Action> asAction() const;
	private:
		friend class mod::lib::Causality::ChoiceAccess;
		Choice(std::size_t type, std::size_t index, std::shared_ptr<dg::DG> dg)
			: type(type), index(index), dg(dg) {}
	private:
		std::size_t type = -1, index = -1;
		std::shared_ptr<dg::DG> dg;
	};
public:
	// rst:		.. function:: virtual ~DrawFunction()
	virtual ~DrawFunction();
	// rst:		.. function:: virtual void syncSize() = 0
	// rst:
	// rst:			Called whenever the underlying derivation graph has changed size.
	// rst:			If the drawing function has internal data structures, this method
	// rst:			is where such data structures can be resized.
	// rst:			The derivation graph must be given to this object by its creator.
	virtual void syncSize() = 0;
	// rst:		.. function:: virtual std::pair<Choice, double> draw() = 0
	// rst:
	// rst:			Called in order to draw the next event.
	// rst:
	// rst:			:returns: the drawn action to take and a number indicating the activity of the system.
	virtual std::pair<Choice, double> draw() = 0;
	// rst:		.. function:: virtual void stateUpdated() = 0
	// rst:
	// rst:			Updates internal caches after the state has been updated.
	// rst:			This function must be invoked inbetween calls to `draw`.
	virtual void stateUpdated() = 0;
};

// rst:
// rst: .. class:: causality::Simulator::DrawMassAction
// rst:
// rst:		A creator for a drawing function implementing the law of mass action.
// rst:
// rst:		It supports assigning a rate for input actions, output actions, and reactions,
// rst:		by taking a callback (or constant) for each type.
// rst:		To avoid the overhead of calling these callbacks in each iteration, a returned rate can be cached.
// rst:		Therefore, the return value of each callback (or the constant of each type) is a pair
// rst:		with the first entry being the rate, and the second entry a boolean indicating whether rate should be cached.
// rst:
// rst:		Each of the rate function can also be set to a null function,
// rst:		which means a default rate is used: input rate 0.0, reaction rate 1.0, output rate 0.0.
// rst:
struct MOD_DECL Simulator::DrawMassAction {
	struct Function;
public:
	// rst:		.. function:: DrawMassAction(std::function<std::pair<double, bool>(dg::DG::Vertex)> inputRate, \
	// rst:		                  std::function<std::pair<double, bool>(dg::DG::HyperEdge)> reactionRate, \
	// rst:		                  std::function<std::pair<double, bool>(dg::DG::Vertex)> outputRate, \
	// rst:		                  const std::string &implementationName)
	// rst:
	// rst:			:param inputRate: the rate used for pseudo-reactions for creating molecules.
	// rst:			:param reactionRate: the rate used for each reaction in the system.
	// rst:			:param outputRate: the rate used for pseudo-reactions for destroying molecules.
	// rst:			:param implementationName: the name of the internal implementation to use.
	DrawMassAction(std::function<std::pair<double, bool>(dg::DG::Vertex)> inputRate,
	               std::function<std::pair<double, bool>(dg::DG::HyperEdge)> reactionRate,
	               std::function<std::pair<double, bool>(dg::DG::Vertex)> outputRate,
	               const std::string &implementationName);
	// rst:		.. function:: ~DrawMassAction()
	~DrawMassAction();
	// rst:		.. function:: std::shared_ptr<Function> operator()(const Marking &state)
	// rst:
	// rst:			:param state: the state of the underlying simulation.
	// rst:				It must be kept alive as long as the returned drawing function is kept alive.
	// rst:			:returns: a drawing function implementing the law of mass action.
	std::shared_ptr<Function> operator()(const Marking &state);
private:
	std::function<std::pair<double, bool>(dg::DG::Vertex)> inputRate;
	std::function<std::pair<double, bool>(dg::DG::HyperEdge)> reactionRate;
	std::function<std::pair<double, bool>(dg::DG::Vertex)> outputRate;
	std::string implementationName;
};

// rst:
// rst: .. class:: causality::Simulator::DrawMassAction::Function
// rst:
// rst:		A helper class for performing stochastic simulations where
// rst:		events are drawn according to the law of mass action.
// rst:		Importantly, if the underlying derivation graph is enlarged then
// rst:		:func:`syncSize` must be called before calling :func:`draw`.
// rst:
struct MOD_DECL Simulator::DrawMassAction::Function : Simulator::DrawFunction {
	// rst:		.. function:: Function(const Marking &state,\
	// rst: 	                  std::function<std::pair<double, bool>(dg::DG::Vertex)> inputRate, \
	// rst: 	                  std::function<std::pair<double, bool>(dg::DG::HyperEdge)> reactionRate, \
	// rst: 	                  std::function<std::pair<double, bool>(dg::DG::Vertex)> outputRate, \
	// rst: 	                  const std::string &implementation)
	// rst:
	// rst:			Construct a new instance, that will draw based on the given state.
	// rst:			The return value of the rate callbacks must be 1) the rate and 2) a boolean telling whether
	// rst:			the library should cache the rate. If `true` no more calls with the same argument will be made.
	// rst:			Each of the rate functions may be an empty `std::function` (i.e., default constructed),
	// rst:			which means a default rate is used: input rate 0.0, reaction rate 1.0, output rate 0.0.
	// rst:
	Function(const Marking &state,
			 std::function<std::pair<double, bool>(dg::DG::Vertex)> inputRate,
			 std::function<std::pair<double, bool>(dg::DG::HyperEdge)> reactionRate,
			 std::function<std::pair<double, bool>(dg::DG::Vertex)> outputRate,
			 const std::string &implementation);
	// rst:		.. function:: ~Function()
	~Function();
	// rst:		.. function:: virtual void syncSize() override
	virtual void syncSize() override;
	// rst:		.. function:: virtual std::pair<DrawFunction::Choice, double> draw() override
	virtual std::pair<Choice, double> draw() override;
	// rst:		.. function:: virtual void stateUpdated() override
	virtual void stateUpdated() override;
private:
	struct Pimpl;
	std::unique_ptr<Pimpl> p;
};

} // namespace mod::causality

#endif // MOD_CAUSALITY_STOCHSIM_HPP