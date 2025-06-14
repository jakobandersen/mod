#include <mod/py/Common.hpp>

#include <mod/dg/DG.hpp>
#include <mod/dg/Printer.hpp>
#include <mod/hyperflow/Model.hpp>
#include <mod/hyperflow/ModelImplementationView.hpp>
#include <mod/hyperflow/LinExp.hpp>
#include <mod/hyperflow/Printer.hpp>
#include <mod/hyperflow/Solution.hpp>
#include <mod/graph/Graph.hpp>

namespace mod::hyperflow::Py {
namespace {

void setAbsGap(std::shared_ptr<Model> self, py::object o) {
	if(o.is_none()) self->setAbsGap(-1);
	else self->setAbsGap(py::extract<int>(o));
}

} // namespace

void Model_doExport() {
	using Model_findSolutions = SolutionRange (Model::*)(int, int, int);
	using Model_make = std::shared_ptr<Model> (*)(std::shared_ptr<dg::DG>, const std::string &);
	using Model_load = std::shared_ptr<Model> (*)(std::shared_ptr<dg::DG> dg, const std::string &, const std::string &,
	                                             int);
	// rst: .. class:: hyperflow.Model
	// rst: 
	// rst:		A :class:`~hyperflow.Model` object represents a flow model on a given derivation graph (see :ref:`flowCommon` for details).
	// rst: 	A model consists of a set of modules with the base module modelling the edge flow and input/output flow.
	// rst:		Each module is accessed using proxy objects accessible as attributes in the main model object.
	// rst: 
	py::scope ModelClass = py::class_<Model, std::shared_ptr<Model>, boost::noncopyable>("Model", py::no_init)
			// rst:		.. method:: __init__(dg, ilpSolver="default")
			// rst:		            __init__(model)
			// rst:
			// rst:			Creates a new hyperflow model over the given derivation graph,
			// rst:			or as a copy of an existing model.
			// rst:
			// rst:			:param DG dg: the derivation graph to create the flow model for over.
			// rst:			:param Model model: the other model to copy the specification from.
			// rst:				The copy can be further modified afterwards.
			// rst:			:param str ilpSolver: the ILP solver to use. See :func:`getAvailableILPSolvers`.
			// rst:			:raises: :class:`LogicError` if  ``not dg``.
			// rst:			:raises: :class:`LogicError` if  ``not dg.locked``.
			// rst:			:raises: :class:`LogicError` if  ``dg.numVertices`` is 0.
			// rst:			:raises: :class:`LogicError` if  ``not model``.
			.def("__init__", py::make_constructor(static_cast<Model_make>(&Model::make),
			                                      py::default_call_policies(),
			                                      (py::arg("dg"), py::arg("ilpSolver") = std::string("default"))))
			.def("__init__", py::make_constructor(&Model::copy,
			                                      py::default_call_policies(),
			                                      (py::arg("other"))))
			.def(str(py::self))
					// rst:		.. attribute:: id
					// rst:
					// rst:			(Read-only) The unique instance ID for the :class:`~hyperflow.Model` object.
					// rst:
					// rst:			:type: int
			.add_property("id", &Model::getId)
					// rst:		.. attribute:: dg
					// rst:
					// rst:			(Read-only) The underlying derivation graph.
					// rst:
					// rst:			:type: DG
			.add_property("dg", &Model::getDG)
					// =================================================================================================
					// rst:		.. attribute:: specificationLocked
					// rst:
					// rst:			(Read-only) Whether the specification is locked for modifications.
					// rst:
					// rst:			:type: bool
			.add_property("specificationLocked", &Model::isSpecificationLocked)
					// rst:		.. method:: listSpecification()
					// rst:
					// rst:			List the specification textually to standard output.
			.def("listSpecification", &Model::listSpecification)
					// =================================================================================================
					// rst:		.. method:: addSource(v)
					// rst:		            addSource(g)
					// rst:
					// rst:			Add a vertex as a possible source.
					// rst:			The version taking a graph as argument is equivalent to
					// rst:			calling ``addSource(self.dg.findVertex(g))``.
					// rst:
					// rst:			:param DG.Vertex v: the vertex to add as source.
					// rst:			:param Graph g: a graph representing a vertex to be added as a source.
					// rst:			:raises: :class:`LogicError` if ``specificationLocked``.
					// rst:			:raises: :class:`LogicError` if ``not v``.
					// rst:			:raises: :class:`LogicError` if  ``v`` does not belong to the underlying derivation graph.
					// rst:			:raises: (the graph version) :class:`LogicError` if it is not represented in the underlying DG.
			.def("addSource", static_cast<void (Model::*)(dg::DG::Vertex)>(&Model::addSource))
			.def("addSource", static_cast<void (Model::*)(std::shared_ptr<mod::graph::Graph>)>(&Model::addSource))
					// rst:		.. attribute:: sources
					// rst:
					// rst:			(Read-only) Retrieve the list of source vertices for the model.
					// rst:
					// rst:			:type: list[DG.Vertex]
			.add_property("sources", &Model::getSources)
					// rst:		.. method:: addSink(v)
					// rst:		            addSink(g)
					// rst:
					// rst:			Add a vertex as a possible sink.
					// rst:			The version taking a graph as argument is equivalent to
					// rst:			calling ``addSink(self.dg.findVertex(g))``.
					// rst:
					// rst:			:param DG.Vertex v: the vertex to add as sink.
					// rst:			:param Graph g: a graph representing a vertex to be added as a sink.
					// rst:			:raises: :class:`LogicError` if ``specificationLocked``.
					// rst:			:raises: :class:`LogicError` if ``not v``.
					// rst:			:raises: :class:`LogicError` if  ``v`` does not belong to the underlying derivation graph.
					// rst:			:raises: (the graph version) :class:`LogicError` if it is not represented in the underlying DG.
			.def("addSink", static_cast<void (Model::*)(dg::DG::Vertex)>(&Model::addSink))
			.def("addSink", static_cast<void (Model::*)(std::shared_ptr<mod::graph::Graph>)>(&Model::addSink))
					// rst:		.. attribute:: sinks
					// rst:
					// rst:			(Read-only) Retrieve the list of sink vertices for the model.
					// rst:
					// rst:			:type: list[DG.Vertex]
			.add_property("sinks", &Model::getSinks)
					// rst:		.. method:: exclude(v)
					// rst:		            exclude(g)
					// rst:
					// rst:			Exclude the vertex and all its incident edges from the model.
					// rst:			This will not only add a constraint to disallow flow through this vertex,
					// rst:			but will make some algorithms in various modules pretend the vertex and incident edges
					// rst:			were never part of the model in the first place.
					// rst:			The version taking a graph as argument is equivalent to
					// rst:			calling ``exclude(self.dg.findVertex(g))``.
					// rst:
					// rst:			:param DG.Vertex v: the vertex to exclude.
					// rst:			:param Graph g: a graph representing a vertex to be excluded.
					// rst:			:raises: :class:`LogicError` if ``specificationLocked``.
					// rst:			:raises: :class:`LogicError` if ``not v``.
					// rst:			:raises: :class:`LogicError` if  ``v`` does not belong to the underlying derivation graph.
					// rst:			:raises: (the graph version) :class:`LogicError` if it is not represented in the underlying DG.
			.def("exclude", static_cast<void (Model::*)(dg::DG::Vertex)>(&Model::exclude))
			.def("exclude", static_cast<void (Model::*)(std::shared_ptr<mod::graph::Graph>)>(&Model::exclude))
					// rst:		.. attribute:: excluded
					// rst:
					// rst:			(Read-only) Retrieve the list of excluded vertices for the model.
					// rst:
					// rst:			:type: list[DG.Vertex]
			.add_property("excluded", &Model::getExcluded)
					// rst:		.. method:: separateIOInternalTransit(v)
					// rst:		            separateIOInternalTransit(g)
					// rst:
					// rst:			Ensure that the expanded vertex of `v` has transit edges such that
					// rst:			flow going from the input edge or to the output edge can be distinguished
					// rst:			from flow going from the network and back to the network.
					// rst:
					// rst:			The vertex expansion is lazy, and thus calling this function is necessary
					// rst:			in order to, e.g., access the corresponding ``transitInternal`` variable.
					// rst:
					// rst:			The version taking a graph as argument is equivalent to
					// rst:			calling ``separateIOInternalTransit(self.dg.findVertex(g))``.
					// rst:
					// rst:			:param DG.Vertex v: the vertex to ensure the separation of transit edges for.
					// rst:			:param Graph g: a graph representing a vertex to ensure the separation of transit edges for.
					// rst:			:raises: :class:`LogicError` if ``specificationLocked``.
					// rst:			:raises: :class:`LogicError` if ``not v``.
					// rst:			:raises: :class:`LogicError` if  ``v`` does not belong to the underlying derivation graph.
					// rst:			:raises: (the graph version) :class:`LogicError` if it is not represented in the underlying DG.
			.def("separateIOInternalTransit",
			     static_cast<void (Model::*)(dg::DG::Vertex)>(&Model::separateIOInternalTransit))
			.def("separateIOInternalTransit",
			     static_cast<void (Model::*)(std::shared_ptr<mod::graph::Graph>)>(&Model::separateIOInternalTransit))
					// rst:		.. attribute:: separatedIOInternalTransit
					// rst:
					// rst:			(Read-only) Retrieve the list of vertices where
					// rst:			:meth:`separateIOInternalTransit` has been called.
					// rst:			Note, this does not mean that other vertices do not have separated transit edges.
					// rst:			For example, setting :attr:`allowIOReversal` to ``True`` implies separation as well.
					// rst:
					// rst:			:type: list[DG.Vertex]
			.add_property("separatedIOInternalTransit", &Model::getSeparatedIOInternalTransit)
					// rst:		.. attribute:: allowHyperLoops
					// rst:
					// rst:			Control or query whether flow is allowed through loop edges.
					// rst:			I.e., hyperedges with identical source and target multisets.
					// rst:
					// rst:			:type: bool
					// rst:			:raises: (only set) :class:`LogicError` if ``specificationLocked``.
					// rst:
					// rst:			:note: This setting may be changed when certain modules are enabled.
			.add_property("allowHyperLoops", &Model::getAllowHyperLoops, &Model::setAllowHyperLoops)
					// rst:		.. attribute:: allowReversal
					// rst:
					// rst:			Controls whether flow may go through
					// rst:			one edge and then directly afterwards the inverse edge.
					// rst:
					// rst:			:type: bool
					// rst:			:raises: (only set) :class:`LogicError` if ``specificationLocked``.
					// rst:
					// rst:			:note: This setting may be modified when certain modules are enabled.
			.add_property("allowReversal", &Model::getAllowReversal, &Model::setAllowReversal)
					// rst:		.. attribute:: allowIOReversal
					// rst:
					// rst:			Controls whether flow may go through
					// rst:			an input edge and directly afterwards through the corresponding output edge.
					// rst:
					// rst:			:type: bool
					// rst:			:raises: (only set) :class:`LogicError` if ``specificationLocked``.
					// rst:
					// rst:			:note: This setting may be modified when certain modules are enabled.
			.add_property("allowIOReversal", &Model::getAllowIOReversal, &Model::setAllowIOReversal)
					// rst:		.. attribute:: relaxed
					// rst:
					// rst:			Controls whether the core flow variables are integer or continuous.
					// rst:			The default is ``False``, meaning integer. Using the relaxed model significantly
					// rst:			changes the meaning of solutions,
					// rst:			and all features that rely on flows being integer will be disabled.
					// rst:
					// rst:			:type: bool
					// rst:			:raises: (only set) :class:`LogicError` if ``specificationLocked``.
			.add_property("relaxed", &Model::getRelaxed, &Model::setRelaxed)
					// rst:		.. attribute:: objectiveFunction
					// rst:
					// rst:			(Write-only) The objective function, which will be minimized
					// rst:			(see :ref:`py-hyperflow/LinExp`).
					// rst:
					// rst:			:type: LinExp
					// rst:			:raises: :class:`LogicError` if ``specificationLocked``.
			.add_property("objectiveFunction", &mod::Py::noGet, &Model::setObjectiveFunction)
					// =================================================================================================
					// rst:		.. method:: addBoolVariable(name)
					// rst:		            addIntVariable(name)
					// rst:		            addFloatVariable(name)
					// rst:
					// rst:			Create a new custom boolean, integer, or floating point variable with the given name.
					// rst:
					// rst:			:returns: a handle to the variable.
					// rst: 			:rtype: VarCustom
					// rst:			:raises: :class:`LogicError` if ``specificationLocked``.
					// rst:			:raises: :class:`LogicError` if ``name`` is already in use.
			.def("addBoolVariable", &Model::addBoolVariable)
			.def("addIntVariable", &Model::addIntVariable)
			.def("addFloatVariable", &Model::addFloatVariable)
					// rst:		.. attribute:: customBoolVariables
					// rst:		               customIntVariables
					// rst:		               customFloatVariables
					// rst:
					// rst:			:returns: a list of handles to the variables added with
					// rst:				:meth:`addBoolVariable`/:meth:`addIntVariable`/:meth:`addFloatVariable`.
					// rst: 		:type: list[VarCustom]
			.def_readonly("customBoolVariables", &Model::getCustomBoolVariables)
			.def_readonly("customIntVariables", &Model::getCustomIntVariables)
			.def_readonly("customFloatVariables", &Model::getCustomFloatVariables)
					// rst:		.. method:: addConstraint(c)
					// rst:
					// rst:			:param LinConstraint c: the linear constraint to add to the model (see :ref:`py-hyperflow/LinExp`).
					// rst:			:raises: :class:`LogicError` if ``specificationLocked``.
			.def("addConstraint", &Model::addConstraint)
					// modules
					// =================================================================================================
					// rst:		.. attribute:: overallAutocatalysis
					// rst:
					// rst:			The access object for the :ref:`overall autocatalysis module <flowCommon-overallAutocata>` of the flow model.
					// rst:
					// rst:			:type: OverallAutocatalysis
			.def_readonly("overallAutocatalysis", &Model::overallAutocatalysis)
					// rst:		.. attribute:: overallCatalysis
					// rst:
					// rst:			The access object for the :ref:`overall catalysis module <flowCommon-overallCata>` of the flow model.
					// rst:
					// rst:			:type: OverallCatalysis
			.def_readonly("overallCatalysis", &Model::overallCatalysis)
					// solution definition
					// =================================================================================================
					// rst:		.. method:: addEnumerationVar(var)
					// rst:
					// rst:			Add the variables specified by the given variable specifier for solution enumeration.
					// rst:
					// rst:			The default variables are :data:`edgeFlow`, :data:`inFlow`, and :data:`outFlow`.
					// rst:			These are removed the first time this function is called.
					// rst:
					// rst:			:param Var var: the variable specifier to add variables from.
					// rst:			:raises: :class:`LogicError` if ``specificationLocked``.
			.def("addEnumerationVar", &Model::addEnumerationVar)
					// rst:		.. attribute:: enumerationVars
					// rst:
					// rst:			(Read-only) Retrieve the list variable specifiers used for enumeration.
					// rst:
					// rst:			:type: list[Var]
			.add_property("enumerationVars", &Model::getEnumerationVars)
					// rst:		.. method:: addTransitEnumeration(v)
					// rst: 	            addTransitEnumeration(g)
					// rst:
					// rst:			Add the transit edges of the vertex for solution enumeration.
					// rst:			The version taking a graph as argument is equivalent to
					// rst:			calling ``addTransitEnumeration(self.dg.findVertex(g))``.
					// rst:
					// rst:			:param DG.Vertex v: the vertex to add transit edges for enumeration.
					// rst:			:param Graph g: a graph representing a vertex for which transit edges should be added.
					// rst:			:raises: :class:`LogicError` if ``specificationLocked``.
					// rst:			:raises: :class:`LogicError` if ``not v``.
					// rst:			:raises: (the graph version) :class:`LogicError` if it is not represented in the underlying DG.
			.def("addTransitEnumeration",
			     static_cast<void (Model::*)(dg::DG::Vertex)>(&Model::addTransitEnumeration))
			.def("addTransitEnumeration",
			     static_cast<void (Model::*)(std::shared_ptr<mod::graph::Graph>)>(&Model::addTransitEnumeration))
					// rst:		.. attribute:: transitEnumeration
					// rst:
					// rst:			(Read-only) Retrieve the list vertices where the transit edges are used for solution enumeration.
					// rst:
					// rst:			:type: list[DG.Vertex]
			.add_property("transitEnumeration", &Model::getTransitEnumeration)
					// rst:		.. attribute:: absGap
					// rst:
					// rst:			The absolute gap in objective value between the optimal solution and the worst solution that can be enumerated.
					// rst:			As default there is no constraint on this gap.
					// rst:			Set to ``None``, or a negative value, to reset to this unconstrained state.
					// rst:
					// rst:			:type: int or None
					// rst:			:throws: (only set) :class:`LogicError` if ``specificationLocked``.
			.add_property("absGap", &Model::getAbsGap, &setAbsGap)
					// =================================================================================================
					// rst:		.. method:: findSolutions(*, maxNumSolutions=1, verbosity=1, ilpVerbosity=1)
					// rst:
					// rst:			Find the next up to ``maxNumSolutions`` best solutions.
					// rst:
					// rst:			This may be called multiple times to find additional solutions in an incremental fashion.
					// rst:			After the first call the specification will be locked, i.e., ``specificationLocked`` will be ``True``.
					// rst:
					// rst:			Calling with `maxNumSolutions` set to 0 will still lock the specification,
					// rst:			but will create the internal model.
					// rst:
					// rst:			:param int maxNumSolutions: the maximum number of solutions to find.
					// rst:			:param int verbosity: see :cpp:func:`hyperflow::Model::findSolutions`.
					// rst:			:param int ilpVerbosity: see :cpp:func:`hyperflow::Model::findSolutions`.
					// rst:			:returns: a range of the newly found solutions.
					// rst:			:rtype: SolutionRange
					// rst:			:raises: :class:`LogicError` if ``maxNumSolutions`` is less than 0.
					// rst:			:raises: :class:`LogicError` the first time it is called,
					// rst:				if an enabled module can not create its model.
					// rst:				See the documentation for each module.
			.def("findSolutions", static_cast<Model_findSolutions>(&Model::findSolutions))
					// rst:		.. method:: dump()
					// rst:		            dump(filename)
					// rst:
					// rst:			Dump all model settings and all solutions found to a file, that can be loaded with :meth:`load`.
					// rst:
					// rst:			:param str filename: the name of the file to save the dump to.
					// rst:				If non is given an auto-generated name in the ``out/`` folder is used.
					// rst:				If an empty string is given, it is treated as if non is given.
					// rst:
					// rst:				.. note:: The filename is used literally, i.e., it is not
					// rst:					prefixed according to the current script location as input filenames are.
					// rst:			:returns: the filename with the dumped model.
					// rst:			:rtype: str
			.def("dump", static_cast<std::string (Model::*)() const>(&Model::dump))
			.def("dump", static_cast<std::string (Model::*)(const std::string &) const>(&Model::dump))
					// =================================================================================================
					// rst:		.. attribute:: solutions
					// rst:
					// rst:			(Read-only) A range of the solutions found so far.
					// rst:
					// rst:			:type: SolutionRange
					// rst:			:raises: :class:`LogicError` if ``not specificationLocked``.
			.add_property("solutions", &Model::getSolutions)
					// rst:		.. attribute:: implementationView
					// rst:
					// rst:			(Read-only) A new view on the implementation of the hyperflow model.
					// rst:
					// rst:			:type: ModelImplementationView
					// rst:			:throws: :class:`LogicError` if ``not specificationLocked``
			.add_property("implementationView", &Model::getImplementationView)
					// rst:		.. staticmethod:: load(dg, f, ilpSolver="default", verbosity=1)
					// rst:
					// rst:			:param DG dg: the derivation graph which the dumped flow model is build upon.
					// rst:			:param f: name of the file with the model to be loaded.
					// rst:			:type f: str or CWDPath
					// rst:			:param str ilpSolver: the ILP solver to use. See :func:`getAvailableILPSolvers`.
					// rst:			:param int verbosity: see :cpp:func:`hyperflow::Model::findSolutions`.
					// rst:			:returns: a flow model (possibly with solutions) corresponding to the model stored in the given file.
					// rst:				The given derivation graph must match the derivation graph originally used to create the dump.
					// rst:			:rtype: Model
					// rst:			:raises: :class:`LogicError` if  ``not dg``.
					// rst:			:raises: :class:`InputError` on bad data or if the given derivation graph does not match the data.
			.def("load", static_cast<Model_load>(&Model::load)) // overwritten in __init__.py, so keywords defined there
			.staticmethod("load")
					// rst:		.. staticmethod:: loadString(dg, s, ilpSolver="default", verbosity=1)
					// rst:
					// rst:			:param DG dg: the derivation graph which the dumped flow model is build upon.
					// rst:			:param str s: the string with the dump data to be loaded.
					// rst:			:param str ilpSolver: the ILP solver to use. See :func:`getAvailableILPSolvers`.
					// rst:			:param int verbosity: see :cpp:func:`hyperflow::Model::findSolutions`.
					// rst:			:returns: a flow model (possibly with solutions) corresponding to the model stored in the given string.
					//	rst:				The given derivation graph must match the derivation graph originally used to create the dump.
					// rst:			:rtype: Model
					// rst:			:raises: :class:`LogicError` if  ``not dg``.
					// rst:			:raises: :class:`InputError` on bad data or if the given derivation graph does not match the data.
			.def("loadString", static_cast<Model_load>(&Model::loadString),
			     (py::arg("dg"), py::arg("s"), py::arg("ilpSolver") = std::string("default"), py::arg("verbosity") = 1))
			.staticmethod("loadString");


	// rst:
	// rst: .. class:: hyperflow.Model.OverallAutocatalysis
	// rst:
	// rst:		This class provides access to the module for :ref:`overall autocatalysis <flowCommon-overallAutocata>` of a flow model.
	// rst:
	py::class_<Model::OverallAutocatalysis, boost::noncopyable>("OverallAutocatalysis", py::no_init)
			// rst:		.. method:: enable()
			// rst:
			// rst:			Enable the extension. This will also set ``allowReversal`` and ``allowIOReversal`` to ``False``.
			// rst:
			// rst:			:raises: :class:`LogicError` if :py:attr:`~hyperflow.Model.specificationLocked`.
			// rst:			:raises: during model creation, :class:`LogicError` if in relaxed mode.
			.def("enable", &Model::OverallAutocatalysis::enable)
					// rst:		.. attribute:: isEnabled
					// rst:
					// rst:			Whether the module is enabled.
					// rst:
					// rst:			:type: bool
			.add_property("isEnabled", &Model::OverallAutocatalysis::isEnabled)
					// rst:		.. attribute:: forceExistence
					// rst:
					// rst:			Controls whether a solution must be overall autocatalytic.
					// rst:
					// rst:			:type: bool
					// rst:			:raises: :class:`LogicError` if the module is not enabled.
					// rst:			:raises: (only set) :class:`LogicError` if :py:attr:`~hyperflow.Model.specificationLocked`.
			.add_property("forceExistence", &Model::OverallAutocatalysis::getForceExistence,
			              &Model::OverallAutocatalysis::setForceExistence)
					// rst:		.. attribute:: strictTransit
					// rst:
					// rst:			Controls whether transit flow in overall autocatalytic vertices is restricted or not.
					// rst:
					// rst:			:type: bool
					// rst:			:raises: :class:`LogicError` if the module is not enabled.
					// rst:			:raises: (only set) :class:`LogicError` if :py:attr:`~hyperflow.Model.specificationLocked`.
			.add_property("strictTransit", &Model::OverallAutocatalysis::getStrictTransit,
			              &Model::OverallAutocatalysis::setStrictTransit)
					// rst:		.. attribute:: bfsExclusive
					// rst:
					// rst:			Controls whether vertices must be exclusively overall autocatalytic,
					// rst:			as determined by breadth-first marking.
					// rst:
					// rst:			:type: bool
					// rst:			:raises: :class:`LogicError` if the module is not enabled.
					// rst:			:raises: (only set) :class:`LogicError` if :py:attr:`~hyperflow.Model.specificationLocked`.
			.add_property("bfsExclusive", &Model::OverallAutocatalysis::getBFSExclusive,
			              &Model::OverallAutocatalysis::setBFSExclusive);
	// rst:
	// rst: .. class:: hyperflow.Model.OverallCatalysis
	// rst:
	// rst:		This class provides access to the module for :ref:`overall catalysis <flowCommon-overallCata>` of a flow model.
	// rst:
	py::class_<Model::OverallCatalysis, boost::noncopyable>("OverallCatalysis", py::no_init)
			// rst:		.. method:: enable()
			// rst:
			// rst:			Enable the extension. This will also set ``allowReversal`` and ``allowIOReversal`` to ``False``.
			// rst:
			// rst:			:raises: :class:`LogicError` if :py:attr:`~hyperflow.Model.specificationLocked`.
			// rst:			:raises: during model creation, :class:`LogicError` if in relaxed mode.
			.def("enable", &Model::OverallCatalysis::enable)
					// rst:		.. attribute:: isEnabled
					// rst:
					// rst:			Whether the module is enabled.
					// rst:
					// rst:			:type: bool
			.add_property("isEnabled", &Model::OverallCatalysis::isEnabled)
					// rst:		.. attribute:: forceExistence
					// rst:
					// rst:			Controls whether a solution must be overall catalytic.
					// rst:
					// rst:			:type: bool
					// rst:			:raises: :class:`LogicError` if the module is not enabled.
					// rst:			:raises: (only set) :class:`LogicError` if :py:attr:`~hyperflow.Model.specificationLocked`.
			.add_property("forceExistence", &Model::OverallCatalysis::getForceExistence,
			              &Model::OverallCatalysis::setForceExistence)
					// rst:		.. attribute:: strictTransit
					// rst:
					// rst:			Controls whether transit flow in overall catalytic vertices is restricted or not.
					// rst:
					// rst:			:type: bool
					// rst:			:raises: :class:`LogicError` if the module is not enabled.
					// rst:			:raises: (only set) :class:`LogicError` if :py:attr:`~hyperflow.Model.specificationLocked`.
			.add_property("strictTransit", &Model::OverallCatalysis::getStrictTransit,
			              &Model::OverallCatalysis::setStrictTransit);
}

} // namespace mod::hyperflow::Py
