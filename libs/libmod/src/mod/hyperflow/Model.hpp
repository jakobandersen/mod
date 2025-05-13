#ifndef MOD_HYPERFLOW_MODEL_HPP
#define MOD_HYPERFLOW_MODEL_HPP

#include <mod/BuildConfig.hpp>
#include <mod/dg/DG.hpp>
#include <mod/hyperflow/ForwardDecl.hpp>
#include <mod/graph/ForwardDecl.hpp>

#include <iosfwd>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace mod {
template<typename>
struct Function;
} // namespace mod
namespace mod::hyperflow {

// rst-class: hyperflow::Model
// rst:
// rst:		A :class:`Model` object represents a hyperflow model on a given derivation graph.
// rst:		A model consists of a set of modules with the base module modelling the edge flow and input/output flow.
// rst:		Each module is accessed using proxy objects accessible as public members in the main model object.
// rst:
// rst-class-start:
class MOD_DECL Model : public std::enable_shared_from_this<Model> {
private:
	Model(std::unique_ptr<lib::HyperFlow::Flow> f);
public:
	~Model();
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const Model &dgFlow);
	// rst: .. function:: int getId() const
	// rst:
	// rst:		:returns: the unique instance ID for the object.
	int getId() const;
	// rst: .. function:: std::shared_ptr<dg::DG> getDG() const
	// rst:
	// rst:		:returns: the underlying derivation graph.
	std::shared_ptr<dg::DG> getDG() const;
public: // internal
	lib::HyperFlow::Flow &getFlow();
	const lib::HyperFlow::Flow &getFlow() const;
public:
	// rst: .. function:: bool isSpecificationLocked() const
	// rst:
	// rst:		:returns: whether the specification is locked for modifications.
	bool isSpecificationLocked() const;
	// rst: .. function:: void listSpecification() const
	// rst:
	// rst:		List the specification textually to standard output.
	void listSpecification() const;
public:
	// rst: .. function:: void addSource(dg::DG::Vertex v)
	// rst:
	// rst:		Add the vertex as a possible source.
	// rst:
	// rst:		:throws: :class:`LogicError` if `isSpecificationLocked()`.
	// rst:		:throws: :class:`LogicError` if `!v`.
	// rst:		:throws: :class:`LogicError` if  `v.getDG() != getDG()`.
	void addSource(dg::DG::Vertex v);
	// rst: .. function:: void addSource(std::shared_ptr<graph::Graph> g)
	// rst:
	// rst:		Equivalent to calling `addSource(getDG()->findVertex(g))`.
	void addSource(std::shared_ptr<graph::Graph> g);
	// rst: .. function:: std::vector<dg::DG::Vertex> getSources() const
	// rst:
	// rst:		Retrieve the list of source vertices for the model.
	std::vector<dg::DG::Vertex> getSources() const;
	// rst: .. function:: void addSink(dg::DG::Vertex v)
	// rst:
	// rst:		Add the vertex as a possible sink.
	// rst:
	// rst:		:throws: :class:`LogicError` if `isSpecificationLocked()`.
	// rst:		:throws: :class:`LogicError` if `!v`.
	// rst:		:throws: :class:`LogicError` if  `v.getDG() != getDG()`.
	void addSink(dg::DG::Vertex v);
	// rst: .. function:: void addSink(std::shared_ptr<graph::Graph> g)
	// rst:
	// rst:		Equivalent to calling `addSink(getDG()->findVertex(g))`.
	void addSink(std::shared_ptr<graph::Graph> g);
	// rst: .. function:: std::vector<dg::DG::Vertex> getSinks() const
	// rst:
	// rst:		Retrieve the list of sink vertices for the model.
	std::vector<dg::DG::Vertex> getSinks() const;
	// rst: .. function:: void exclude(dg::DG::Vertex v)
	// rst:
	// rst:		Exclude the vertex and all its incident edges from the model.
	// rst:		This will not only add a constraint to disallow flow through this vertex,
	// rst:     but will make some algorithms in various modules pretend the vertex and incident edges
	// rst:     were never part of the model in the first place.
	// rst:
	// rst:		:throws: :class:`LogicError` if `isSpecificationLocked()`.
	// rst:		:throws: :class:`LogicError` if `!v`.
	// rst:		:throws: :class:`LogicError` if  `v.getDG() != getDG()`.
	void exclude(dg::DG::Vertex v);
	// rst: .. function:: void exclude(std::shared_ptr<graph::Graph> g)
	// rst:
	// rst:		Equivalent to calling `exclude(getDG()->findVertex(g))`.
	void exclude(std::shared_ptr<graph::Graph> g);
	// rst: .. function:: std::vector<dg::DG::Vertex> getExcluded() const
	// rst:
	// rst:		Retrieve the list of excluded vertices for the model.
	std::vector<dg::DG::Vertex> getExcluded() const;
	// rst: .. function:: void separateIOInternalTransit(dg::DG::Vertex v)
	// rst:
	// rst:		Ensure that the expanded vertex of `v` has transit edges such that
	// rst:		flow going from the input edge or to the output edge can be distinguished
	// rst:		from flow going from the network and back to the network.
	// rst:
	// rst:		The vertex expansion is lazy, and thus calling this function is necessary
	// rst:		in order to, e.g., access the corresponding ``transitInternal`` variable.
	// rst:
	// rst:		:throws: :class:`LogicError` if `isSpecificationLocked()`.
	// rst:		:throws: :class:`LogicError` if `!v`.
	// rst:		:throws: :class:`LogicError` if  `v.getDG() != getDG()`.
	void separateIOInternalTransit(dg::DG::Vertex v);
	// rst: .. function:: void separateIOInternalTransit(std::shared_ptr<graph::Graph> g)
	// rst:
	// rst:		Equivalent to calling `separateIOInternalTransit(getDG()->findVertex(g))`.
	void separateIOInternalTransit(std::shared_ptr<graph::Graph> g);
	// rst: .. function:: std::vector<dg::DG::Vertex> getSeparatedIOInternalTransit() const
	// rst:
	// rst:		:returns: a list of vertices where :cpp:func:`separateIOInternalTransit` has been called.
	// rst:			Note, this does not mean that other vertices do not have separated transit edges.
	// rst:			For example, calling `setAllowIOReversal(true)` implies separation as well.
	std::vector<dg::DG::Vertex> getSeparatedIOInternalTransit() const;
	// rst: .. function:: void setAllowHyperLoops(bool value)
	// rst:               bool getAllowHyperLoops() const
	// rst:
	// rst:		Control or query whether flow is allowed through loop hyperedges.
	// rst:		I.e., hyperedges with identical source and target multisets.
	// rst:
	// rst:		:throws: (only set) :class:`LogicError` if `isSpecificationLocked()`.
	// rst:
	// rst:		:note: This setting may be changed when certain modules are enabled.
	void setAllowHyperLoops(bool value);
	bool getAllowHyperLoops() const;
	// rst: .. function:: void setAllowReversal(bool value)
	// rst:               bool getAllowReversal() const
	// rst:
	// rst:		Control or query whether flow may go through
	// rst:		one edge and then directly afterwards the inverse edge.
	// rst:
	// rst:		:throws: (only set) :class:`LogicError` if `isSpecificationLocked()`.
	// rst:
	// rst:		:note: This setting may be modified when certain modules are enabled.
	void setAllowReversal(bool value);
	bool getAllowReversal() const;
	// rst: .. function:: void setAllowIOReversal(bool value)
	// rst:               bool getAllowIOReversal() const
	// rst:
	// rst:		Control or query whether flow may go through
	// rst:		an input edge and directly afterwards through the corresponding output edge.
	// rst:
	// rst:		This implies that transit edges gets separated in all vertices,
	// rst:		as if :cpp:func:`separateIOInternalTransit` was called on all of them.
	// rst:
	// rst:		:throws: (only set) :class:`LogicError` if `isSpecificationLocked()`.
	// rst:
	// rst:		:note: This setting may be modified when certain modules are enabled.
	void setAllowIOReversal(bool value);
	bool getAllowIOReversal() const;
	// rst: .. function:: void setRelaxed(bool value)
	// rst:               bool getRelaxed() const
	// rst:
	// rst:		Controls whether the core flow variables are integer or continuous.
	// rst:		The default is ``False``, meaning integer. Using the relaxed model significantly
	// rst:		changes the meaning of solutions,
	// rst:		and all features that rely on flows being integer will be disabled.
	// rst:
	// rst:		:throws: (only set) :class:`LogicError` if `isSpecificationLocked()`.
	void setRelaxed(bool value);
	bool getRelaxed() const;
	// rst: .. function:: void setObjectiveFunction(LinExp func)
	// rst:
	// rst:		Set the objective function used when finding solutions.
	// rst:
	// rst:		:throws: :class:`LogicError` if `isSpecificationLocked()`.
	void setObjectiveFunction(LinExp func);
public: // user defined variables and constraints
	// rst: .. function:: void addBoolVariable(const std::string &name)
	// rst:               void addIntVariable(const std::string &name)
	// rst:               void addFloatVariable(const std::string &name)
	// rst:
	// rst:		Create a new custom boolean, integer, or floating point variable with the given name.
	// rst:
	// rst:		:returns: a handle to the variable.
	// rst:		:throws: :class:`LogicError` if `isSpecificationLocked()`.
	// rst:		:throws: :class:`LogicError` if :cpp:var`name` is already in use.
	VarCustom addBoolVariable(const std::string &name);
	VarCustom addIntVariable(const std::string &name);
	VarCustom addFloatVariable(const std::string &name);
	// rst: .. function:: std::vector<VarCustom> getCustomBoolVariables() const
	// rst:               std::vector<VarCustom> getCustomIntVariables() const
	// rst:               std::vector<VarCustom> getCustomFloatVariables() const
	// rst:
	// rst:		:returns: a list of handles to the variables added with
	// rst:			:func:`addBoolVariable`/:func:`addIntVariable`/:func:`addFloatVariable`.
	std::vector<VarCustom> getCustomBoolVariables() const;
	std::vector<VarCustom> getCustomIntVariables() const;
	std::vector<VarCustom> getCustomFloatVariables() const;
	// rst: .. function:: void addConstraint(const LinConstraint &constraint)
	// rst:
	// rst:		Add the given linear constraint to the model.
	// rst:
	// rst:		:throws: :class:`LogicError` if `isSpecificationLocked()`.
	void addConstraint(const LinConstraint &constraint);
public:
	// rst-nested: hyperflow::Model::OverallAutocatalysis
	// rst:
	// rst:		This class provides access to the module for :ref:`overall autocatalysis <flowCommon-overallAutocata>` of a flow model.
	// rst-nested-start:
	class OverallAutocatalysis {
		friend class Model;
		OverallAutocatalysis(Model &dgFlow);
	public:
		// rst:		.. function:: void enable()
		// rst:
		// rst:			Enable the module. This will automatically call `setAllowReversal(false)`
		// rst:			and `setAllowIOReversal(false)`.
		// rst:
		// rst:			:throws: :class:`LogicError` if `isSpecificationLocked()`.
		// rst:			:throws: during model creation, :class:`LogicError` if in relaxed mode.
		void enable();
		// rst:		.. function:: bool isEnabled()
		// rst:
		// rst:			:returns: whether the module is enabled.
		bool isEnabled() const;
		// rst:		.. function:: void setForceExistence(bool value)
		// rst:		              bool getForceExistence() const
		// rst:
		// rst:			Control or query whether a solution must be overall autocatalytic.
		// rst:
		// rst:			:throws: :class:`LogicError` if the module is not enabled.
		// rst:			:throws: (only set) :class:`LogicError` if `isSpecificationLocked()`.
		void setForceExistence(bool value);
		bool getForceExistence() const;
		// rst:		.. function:: void setStrictTransit(bool value)
		// rst:			          bool getStrictTransit() const
		// rst:
		// rst:			Control or query whether transit flow in overall autocatalytic vertices is restricted or not.
		// rst:
		// rst:			:throws: :class:`LogicError` if the module is not enabled.
		// rst:			:throws: (only set) :class:`LogicError` if `isSpecificationLocked()`.
		void setStrictTransit(bool value);
		bool getStrictTransit() const;
		// rst:		.. function:: void setBFSExclusive(bool value)
		// rst:			          bool getBFSExclusive() const
		// rst:
		// rst:			Control or query whether vertices must be exclusively overall autocatalytic,
		// rst:			as determined by breadth-first marking.
		// rst:
		// rst:			:throws: :class:`LogicError` if the module is not enabled.
		// rst:			:throws: (only set) :class:`LogicError` if `isSpecificationLocked()`.
		void setBFSExclusive(bool value);
		bool getBFSExclusive() const;
	private:
		Model &dgFlow;
	};
	// rst-nested-end:
	// rst: .. member:: OverallAutocatalysis overallAutocatalysis
	// rst:
	// rst:		The access object for the :ref:`overall autocatalysis module <flowCommon-overallAutocata>` of the flow model.
	OverallAutocatalysis overallAutocatalysis;
public:
	// rst-nested: hyperflow::Model::OverallCatalysis
	// rst:
	// rst:		This class provides access to the module for :ref:`overall catalysis <flowCommon-overallCata>` of a flow model.
	// rst-nested-start:
	class OverallCatalysis {
		friend class Model;
		OverallCatalysis(Model &dgFlow);
	public:
		// rst:		.. function:: void enable()
		// rst:
		// rst:			Enable the module. This will automatically call `setAllowReversal(false)`
		// rst: 			and `setAllowIOReversal(false)`.
		// rst:
		// rst:			:throws: :class:`LogicError` if `isSpecificationLocked()`.
		// rst:			:throws: during model creation, :class:`LogicError` if in relaxed mode.
		void enable();
		// rst:		.. function:: bool isEnabled()
		// rst:
		// rst:			:returns: whether the module is enabled.
		bool isEnabled() const;
		// rst:		.. function:: void setForceExistence(bool value)
		// rst:		              bool getForceExistence() const
		// rst:
		// rst:			Control or query whether a solution must be overall catalytic.
		// rst:
		// rst:			:throws: :class:`LogicError` if the module is not enabled.
		// rst:			:throws: (only set) :class:`LogicError` if `isSpecificationLocked()`.
		void setForceExistence(bool value);
		bool getForceExistence() const;
		// rst:		.. function:: void setStrictTransit(bool value)
		// rst:		              bool getStrictTransit() const
		// rst:
		// rst:			Control or query whether transit flow in overall catalytic vertices is restricted or not.
		// rst:
		// rst:			:throws: :class:`LogicError` if the module is not enabled.
		// rst:			:throws: (only set) :class:`LogicError` if `isSpecificationLocked()`.
		void setStrictTransit(bool value);
		bool getStrictTransit() const;
	private:
		Model &dgFlow;
	};
	// rst-nested-end:
	// rst: .. member:: OverallCatalysis overallCatalysis
	// rst:
	// rst:		The access object for the :ref:`overall catalysis module <flowCommon-overallCata>` of the flow model.
	OverallCatalysis overallCatalysis;
public: // solution definition
	// rst: .. function:: void addEnumerationVar(Var var)
	// rst:
	// rst:		Add the variables specified by the given variable specifier for solution enumeration.
	// rst:
	// rst:		The default variables are :var:`vars::edgeFlow`, :var:`vars::inFlow`, and :var:`vars::outFlow`.
	// rst:		These are removed the first time this function is called.
	// rst:
	// rst:		:throws: :class:`LogicError` if `isSpecificationLocked()`.
	void addEnumerationVar(Var var);
	// rst: .. function:: std::vector<Var> getEnumerationVars() const
	// rst:
	// rst:		Retrieve the list of variable specifiers used for solution enumeration.
	std::vector<Var> getEnumerationVars() const;
	// rst: .. function:: void addTransitEnumeration(dg::DG::Vertex v)
	// rst:
	// rst:		Add the transit edges of the vertex for solution enumeration.
	// rst:
	// rst:		:throws: :class:`LogicError` if `isSpecificationLocked()`.
	// rst: 		:throws: :class:`LogicError` if `!v`.
	// rst: 		:throws: :class:`LogicError` if  `v.getDG() != getDG()`.
	void addTransitEnumeration(dg::DG::Vertex v);
	// rst: .. function:: void addTransitEnumeration(std::shared_ptr<graph::Graph> g)
	// rst:
	// rst:		Equivalent to calling `addTransitEnumeration(getDG()->findVertex(g))`.
	void addTransitEnumeration(std::shared_ptr<graph::Graph> g);
	// rst: .. function:: std::vector<dg::DG::Vertex> getTransitEnumeration() const
	// rst:
	// rst:		Retrieve the list of vertices where the transit edges are used for solution enumeration.
	std::vector<dg::DG::Vertex> getTransitEnumeration() const;
	// rst: .. function:: void setAbsGap(int absGap)
	// rst:
	// rst:		Set the absolute gap in objective value between the optimal solution and the worst solution.
	// rst:		As default there is no constraint on this gap. Pass a negative number to reset to this unconstrained state.
	// rst:
	// rst:		:throws: :class:`LogicError` if `isSpecificationLocked()`.
	void setAbsGap(int absGap);
	// rst: .. function:: std::optional<int> getAbsGap()
	// rst:
	// rst:		:returns: the absolution gap in objective value.
	std::optional<int> getAbsGap();
public: // allowed both pre and post calculation
	// rst: .. function:: SolutionRange findSolutions(int maxNumSolutions)
	// rst:               SolutionRange findSolutions(int maxNumSolutions, int verbosity)
	// rst:               SolutionRange findSolutions(int maxNumSolutions, int verbosity, int ilpVerbosity)
	// rst:
	// rst:		Find the next up to :var:`maxNumSolutions` best solutions.
	// rst:
	// rst:		This may be called multiple times to find additional solutions in an incremental fashion.
	// rst:		After the first call the specification will be locked, i.e., `isSpecificationLocked()` will return `true`.
	// rst:
	// rst:		Calling with `maxNumSolutions` set to 0 will still lock the specification,
	// rst:		but will create the internal model.
	// rst:
	// rst:		:param verbosity: controls the amount of information printed during solution enumeration, defaults to 1:
	// rst:
	// rst:			- 0: print no information, and cap `ilpVerbosity` to 0, no matter the given value.
	// rst:			- 1: print status updates periodically.
	// rst:			- 2: print debug information.
	// rst:		:param ilpVerbosity: controls the amount of information printed by the underlying ILP solver, defaults to 1:
	// rst:
	// rst:			- 0: print no information.
	// rst:			- 1: print default information for the first solution only.
	// rst:			- 2: print default information when finding a solution.
	// rst:		:returns: a range of the newly found solutions.
	// rst:		:throws: :class:`LogicError` if :var:`maxNumSolutions` is less than 0.
	// rst:		:throws: :class:`LogicError` the first time it is called,
	// rst:			if an enabled module can not create its model.
	// rst:			See the documentation for each module.
	SolutionRange findSolutions(int maxNumSolutions);
	SolutionRange findSolutions(int maxNumSolutions, int verbosity);
	SolutionRange findSolutions(int maxNumSolutions, int verbosity, int ilpVerbosity);
	// rst: .. function:: std::string dump() const
	// rst:               std::string dump(const std::string &filename) const
	// rst:
	// rst:		Dump all model settings and all solutions found to a file. This file can be loaded in again.
	// rst:
	// rst:		:param filename: the name of the file to save the dump to.
	// rst:			If non is given an auto-generated name in the ``out/`` folder is used.
	// rst:			If an empty string is given, it is treated as if non is given.
	// rst:		:returns: the filename with the dumped model.
	std::string dump() const;
	std::string dump(const std::string &filename) const;
public: // post-calculation operations
	// rst: .. function:: SolutionRange getSolutions() const
	// rst:
	// rst:		:returns: a range of the solutions found so far.
	// rst:		:throws: :class:`LogicError` if `isSpecificationLocked()`.
	SolutionRange getSolutions() const;
	// rst: .. function:: ModelImplementationView getImplementationView() const;
	// rst:
	// rst:		:returns: a new view on the implementation of the hyperflow model.
	// rst:		:throws: :class:`LogicError` if `!isSpecificationLocked()`
	friend class ModelImplementationView;
	ModelImplementationView getImplementationView() const;
private:
	struct Pimpl;
	std::unique_ptr<Pimpl> p;
public:
	// rst: .. function:: static std::shared_ptr<Model> make(std::shared_ptr<dg::DG> dg_)
	// rst:               static std::shared_ptr<Model> make(std::shared_ptr<dg::DG> dg_, const std::string &ilpSolver)
	// rst:
	// rst:		:param ilpSolver: defaults to ``default``. See :cpp:func:`getAvailableILPSolvers`.
	// rst:		:returns: a new flow model over the given derivation graph.
	// rst:		:throws: :class:`LogicError` if `!dg`.
	// rst:		:throws: :class:`LogicError` if `!dg->isLocked()`.
	// rst:		:throws: :class:`LogicError` if `dg->numVertices() == 0`.
	static std::shared_ptr<Model> make(std::shared_ptr<dg::DG> dg_);
	static std::shared_ptr<Model> make(std::shared_ptr<dg::DG> dg_, const std::string &ilpSolver);
	// rst: .. function:: static std::shared_ptr<Model> copy(std::shared_ptr<Model> other)
	// rst:
	// rst:		:param other: a model to copy the specification from.
	// rst:		:returns: a new flow model starting with a copy of the specification from `other`.
	// rst:			The returned model is not locked.
	// rst:		:throws: :class:`LogicError` if `!other`.
	static std::shared_ptr<Model> copy(std::shared_ptr<Model> other);
	// rst: .. function:: static std::shared_ptr<Model> load(std::shared_ptr<dg::DG> dg_, const std::string &file)
	// rst:               static std::shared_ptr<Model> load(std::shared_ptr<dg::DG> dg_, const std::string &file, \
	// rst:                                                  const std::string &ilpSolver, int verbosity)
	// rst:
	// rst:		:param ilpSolver: defaults to ``default``. See :cpp:func:`getAvailableILPSolvers`.
	// rst:			Its value only matters if further solutions are enumerated after the model has been loaded.
	// rst:		:param verbosity: see :func:`findSolutions`.
	// rst:		:returns: a flow model (possibly with solutions) corresponding to the model stored in the given file.
	// rst:			The given derivation graph must match the derivation graph originally used to create the dump.
	// rst:		:throws: :class:`LogicError` if `!dg`.
	// rst:		:throws: :class:`InputError` on bad data or if the given derivation graph does not match the data.
	static std::shared_ptr<Model> load(std::shared_ptr<dg::DG> dg_, const std::string &file);
	static std::shared_ptr<Model> load(std::shared_ptr<dg::DG> dg_, const std::string &file,
	                                   const std::string &ilpSolver, int verbosity);
	// rst: .. function:: static std::shared_ptr<Model> loadString(std::shared_ptr<dg::DG> dg_, const std::string &data)
	// rst:               static std::shared_ptr<Model> loadString(std::shared_ptr<dg::DG> dg_, const std::string &data, \
	// rst:                                                        const std::string &ilpSolver, int verbosity)
	// rst:
	// rst:		:param ilpSolver: defaults to ``default``. See :cpp:func:`getAvailableILPSolvers`.
	// rst:			Its value only matters if further solutions are enumerated after the model has been loaded.
	// rst:		:param verbosity: see :func:`findSolutions`.
	// rst:		:returns: a flow model (possibly with solutions) corresponding to the model stored in the given string.
	// rst:			The given derivation graph must match the derivation graph originally used to create the dump.
	// rst:		:throws: :class:`LogicError` if `!dg`.
	// rst:		:throws: :class:`InputError` on bad data or if the given derivation graph does not match the data.
	static std::shared_ptr<Model> loadString(std::shared_ptr<dg::DG> dg_, const std::string &data);
	static std::shared_ptr<Model> loadString(std::shared_ptr<dg::DG> dg_, const std::string &data,
	                                         const std::string &ilpSolver, int verbosity);
};
// rst-class-end:

} // namespace mod::hyperflow

#endif // MOD_HYPERFLOW_MODEL_HPP