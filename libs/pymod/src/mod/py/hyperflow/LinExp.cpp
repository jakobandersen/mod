#include <mod/py/Common.hpp>

#include <mod/hyperflow/LinExp.hpp>
#include <mod/graph/Graph.hpp>

namespace mod::hyperflow::Py {

void LinExp_doExport() {
	// rst:
	// rst: .. _FlowLinExpAndConstraints-label:
	// rst:
	// rst:
	// rst: This part describes variable specifiers, linear expressions, and linear constraints.
	// rst: See :ref:`flowCommon-varSpecLinExp` for a higher-level description of these entities.
	// rst:
	// rst: Linear expressions (:class:`hyperflow.LinExp`) and constraints (:class:`hyperflow.LinConstraint`)
	// rst: can be implicitly created from variable specifiers and operators, meaning one rarely
	// rst: need to instantiate them explicitly.
	// rst: However creating the zero-expression might sometimes be useful (e.g., in a sum expression)
	// rst: and can be done simply with the expression ``hyperflow.LinExp()``.
	// rst:
	// rst: A variable specifier is a representation of either a single variable or a sum of variables.
	// rst: The addition, multiplication and division operators are overloaded for variable specifiers and
	// rst: linear expressions such that expressions can be written in the natural manner.
	// rst:
	// rst: The operators ``<=``, ``==`` and ``>=`` are overloaded on linear expressions to make the creation of
	// rst: linear constraints easier. However, one argument to the operator must be a linear expression while
	// rst: the other argument must be a number.
	// rst: 
	// rst: All variable specifiers are defined in module scope. See :ref:`flowCommon` for an overview of the specifiers.
	// rst:

	// rst: .. class:: hyperflow.VarSumVertex
	// rst:
	// rst: 	Represents the sum of all variables indicated by the given :attr:`id`.
	// rst: 	Can be indexed to retrieve a specifier for a single variable in the set.
	// rst:
	py::object class_VarSumVertex = py::class_<VarSumVertex>("VarSumVertex", py::no_init)
			// rst: 	.. method:: __init__(id)
			// rst:
			// rst: 		Construct a variable specifier for a sum of variables associated with
			// rst: 		vertices in the underlying derivation graph.
			// rst:
			// rst: 		:param str id: The ID for the variable set to represent.
			.def(py::init<std::string>())
			.def(str(py::self))
					// rst: 	.. method:: __getitem__(x)
					// rst:
					// rst: 		Select the variable associated with the derivation graph vertex which represents the given object.
					// rst: 		When the resulting specifier is given to a flow model,
					// rst: 		if the given object is a vertex, then it must belong to the same derivation graph as the one
					// rst: 		underlying the model.
					// rst: 		If the given object is a graph, a vertex in the underlying derivation graph must exist
					// rst: 		which has the graph associated.
					// rst:
					// rst: 		:param x: a graph or vertex used to select a vertex in the underlying derivation graph.
					// rst: 		:type x: DG.Vertex or Graph
					// rst: 		:returns: a variable specifier for a single variable in the set represented by this object.
					// rst: 		:rtype: VarVertex or VarVertexGraph
					// rst: 		:raises: `LogicError` if ``not x``.
			.def("__getitem__", static_cast<
					VarVertex (VarSumVertex::*)(dg::DG::Vertex) const
					>(&VarSumVertex::operator[]))
			.def("__getitem__", static_cast<
					VarVertexGraph (VarSumVertex::*)(std::shared_ptr<mod::graph::Graph>) const
					>(&VarSumVertex::operator[]))
					// rst: 	.. attribute:: id
					// rst:
					// rst: 		The ID for this specifier.
					// rst:
					// rst: 		:type: str
			.add_property("id", &VarSumVertex::id);
	// rst: .. class:: hyperflow.VarVertex
	// rst:
	// rst: 	Represents a variable associated with a single vertex in a derivation graph.
	// rst:
	py::class_<VarVertex>("VarVertex", py::no_init)
			// rst: 	.. method:: __init__(id, v)
			// rst:
			// rst: 		:param str id: The ID for the variable set to select variables from.
			// rst: 		:param DG.Vertex v: The vertex used to select a variable from the variable set.
			// rst:
			// rst: 		:raises: `LogicError` if ``not v``.
			.def(py::init<std::string, dg::DG::Vertex>())
			.def(str(py::self))
					// rst: 	.. attribute:: id
					// rst:
					// rst: 		The ID for the underlying set specifier.
					// rst:
					// rst: 		:type: str
			.add_property("id", &VarVertex::id)
					// rst: 	.. attribute:: vertex
					// rst:
					// rst: 		The graph used to index into the variable set.
					// rst:
					// rst: 		:type: DG.Vertex
			.add_property("vertex", &VarVertex::v);
	// rst: .. class:: hyperflow.VarVertexGraph
	// rst:
	// rst: 	Represents a variable associated with a single vertex in a derivation graph.
	// rst: 	This is done implicitly by a graph, and when this specifier is later given to a flow model
	// rst: 	a vertex with the graph associated must exist in the underlying derivation graph.
	// rst:
	py::class_<VarVertexGraph>("VarVertexGraph", py::no_init)
			// rst: 	.. method:: __init__(id, g)
			// rst:
			// rst: 		:param str id: The ID for the variable set to select variables from.
			// rst: 		:param Graph g: The graph used to select a variable from the variable set.
			// rst:
			// rst: 		:raises: `LogicError` if ``not g``.
			.def(py::init<std::string, std::shared_ptr<mod::graph::Graph>>())
			.def(str(py::self))
					// rst: 	.. attribute:: id
					// rst:
					// rst: 		The ID for the underlying set specifier.
					// rst:
					// rst: 		:type: str
			.add_property("id", &VarVertexGraph::id)
					// rst: 	.. attribute:: graph
					// rst:
					// rst: 		The graph used to index into the variable set.
					// rst:
					// rst: 		:type: Graph
			.add_property("graph", &VarVertexGraph::g);


	// rst: .. class:: hyperflow.VarSumEdge
	// rst:
	// rst: 	Represents the sum of all variables indicated by the given :attr:`id`.
	// rst: 	Can be indexed to retrieve a specifier for a single variable in the set.
	// rst:
	py::object class_VarSumEdge = py::class_<VarSumEdge>("VarSumEdge", py::no_init)
			// rst: 	.. method:: __init__(id)
			// rst:
			// rst: 		Construct a variable specifier for a sum of variables associated with
			// rst: 		edges in the underlying derivation graph.
			// rst:
			// rst: 		:param str id: The ID for the variable set to represent.
			.def(py::init<std::string>())
			.def(str(py::self))
					// rst: 	.. method:: __getitem__(e)
					// rst:
					// rst: 		Select the variable associated with the derivation graph edge which represents the given object.
					// rst: 		When the resulting specifier is given to a flow model, the given edge must be long to the same
					// rst: 		derivation graph as the one underlying the model.
					// rst:
					// rst: 		:param DG.HyperEdge e: an edge used to select a variable.
					// rst: 		:returns: a variable specifier for a single variable in the set represented by this object.
					// rst: 		:rtype: VarEdge
					// rst:
			.def("__getitem__", &VarSumEdge::operator[])
					// rst: 	.. attribute:: id
					// rst:
					// rst: 		The ID for this specifier.
					// rst:
					// rst: 		:type: str
			.add_property("id", &VarSumEdge::id);
	// rst: .. class:: hyperflow.VarEdge
	// rst:
	// rst: 	Represents a variable associated with an edge in a derivation graph.
	// rst:
	py::class_<VarEdge>("VarEdge", py::no_init)
			// rst: 	.. method:: __init__(id, e)
			// rst:
			// rst: 		:param str id: The ID for the variable set to select variables from.
			// rst: 		:param DG.HyperEdge e: The edge used to select a variable from the variable set.
			// rst:
			.def(py::init<std::string, dg::DG::HyperEdge>())
			.def(str(py::self))
					// rst: 	.. attribute:: id
					// rst:
					// rst: 		The ID for the underlying set specifier.
					// rst:
					// rst: 		:type: str
			.add_property("id", &VarEdge::id)
					// rst: 	.. attribute:: edge
					// rst:
					// rst: 		The edge used to index into the variable set.
					// rst:
					// rst: 		:type: DG.HyperEdge
			.add_property("edge", &VarEdge::edge);


	// rst: .. class:: hyperflow.VarSumCustom
	// rst:
	// rst: 	Represents the sum of all variables indicated by the given :attr:`id`.
	// rst: 	Can be indexed to retrieve a specifier for a single variable in the set.
	// rst:
	py::object class_VarSumCustom = py::class_<VarSumCustom>("VarSumCustom", py::no_init)
			// rst: 	.. method:: __init__(id)
			// rst:
			// rst: 		Construct a variable specifier for a sum of custom variables.
			// rst:
			// rst: 		:param str id: The ID for the variable set to represent.
			.def(py::init<std::string>())
			.def(str(py::self))
					// rst: 	.. method:: __getitem__(id)
					// rst:
					// rst: 		Select the variable associated with the given name.
					// rst: 		When the resulting specifier is given to a flow model that variable must have been created
					// rst: 		in the specification in some way.
					// rst:
					// rst: 		:param str e: a name used to select a variable.
					// rst: 		:returns: a variable specifier for a single variable in the set represented by this object.
					// rst: 		:rtype: VarCustom
					// rst:
			.def("__getitem__", &VarSumCustom::operator[])
					// rst: 	.. attribute:: id
					// rst:
					// rst: 		The ID for this specifier.
					// rst:
					// rst: 		:type: str
			.add_property("id", &VarSumCustom::id);
	// rst: .. class:: hyperflow.VarCustom
	// rst:
	// rst: 	Represents a custom variable.
	// rst:
	py::class_<VarCustom>("VarCustom", py::no_init)
			// rst: 	.. method:: __init__(id, name)
			// rst:
			// rst: 		:param str id: The ID for the variable set to select variables from.
			// rst: 		:param str name: The name used to select a variable from the variable set.
			// rst:
			.def(py::init<std::string, std::string>())
			.def(str(py::self))
					// rst: 	.. attribute:: id
					// rst:
					// rst: 		The ID for the underlying set specifier.
					// rst:
					// rst: 		:type: str
			.add_property("id", &VarCustom::id)
					// rst: 	.. attribute:: name
					// rst:
					// rst: 		The name used to index into the variable set.
					// rst:
					// rst: 		:type: str
			.add_property("name", &VarCustom::name);


	// rst: .. class:: hyperflow.Var
	// rst:
	// rst: 	Represents any variable specifier. They are all implicitly convertible to this type.
	// rst:
	py::object class_Var = py::class_<Var>("Var", py::no_init)
			.def(str(py::self));
	py::implicitly_convertible<VarSumVertex, Var>();
	py::implicitly_convertible<VarVertex, Var>();
	py::implicitly_convertible<VarVertexGraph, Var>();
	py::implicitly_convertible<VarSumEdge, Var>();
	py::implicitly_convertible<VarEdge, Var>();
	py::implicitly_convertible<VarSumCustom, Var>();
	py::implicitly_convertible<VarCustom, Var>();

	// rst: .. class:: hyperflow.LinExp
	// rst:
	// rst: 	Represents a linear expression over variable specifiers.
	// rst:
	// rst: 	The usual arithmetic operators are overloaded on variable specifiers and this class
	// rst: 	to automatically create expressions.
	// rst:
	py::class_<LinExp>("LinExp")
			.def(py::init<LinExp>())
			.def(str(py::self))
			.def(py::self += LinExp())
			.def(py::self -= LinExp())
			.def(py::self + py::self)
			.def(py::self - py::self)
			.def(+py::self)
			.def(-py::self)
			.def(py::self *= double()) // Boost.Python does overload 'resolution' by last-registered-first,
			.def(py::self *= int()) // and a Python int matches a double, but not the other way around
			.def(py::self * double())
			.def(double() * py::self)
			.def(py::self * int())
			.def(int() * py::self)
			.def(py::self <= double())
			.def(py::self == double())
			.def(py::self >= double());
	py::implicitly_convertible<Var, LinExp>();

	// rst: .. class:: hyperflow.LinConstraint
	// rst:
	// rst: 	Represents a linear constraint with a :class:`hyperflow.LinExp` on one side,
	// rst: 	and a constant on the other side.
	// rst: 	The operators ``<=``, ``==``, and ``>=`` are overloaded on linear expressions
	// rst: 	to automatically create constraints.
	// rst:
	py::class_<LinConstraint>("LinConstraint", py::no_init)
			.def(str(py::self));


	// rst: Pre-defined Variable Specifiers
	// rst: ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	// rst:
	// rst: Each of the variable specifiers are defined in the submodule ``hyperflow.vars``,
	// rst: and then import in the root module, so they are directly accessible.
	// rst:
	// https://stackoverflow.com/questions/29006439/boost-python-multiple-modules-in-one-shared-object
	py::scope HyperflowVarModule = py::object(py::handle<>(py::borrowed(PyImport_AddModule("mod.hyperflow.vars"))));
	// Note: these are simply shortcuts defined for convenience.
	// rst: Base Model
	// rst: """"""""""
	// rst:
	// rst: .. data:: hyperflow.vars.inFlow
	// rst:           inFlow
	// rst:
	// rst: 	:type: hyperflow.VarSumVertex
	py::scope().attr(vars::inFlow.id.c_str()) = vars::inFlow;
	// rst: .. data:: hyperflow.vars.outFlow
	// rst:           outFlow
	// rst:
	// rst: 	:type: hyperflow.VarSumVertex
	py::scope().attr(vars::outFlow.id.c_str()) = vars::outFlow;
	// rst: .. data:: hyperflow.vars.isInUsed
	// rst:           isInUsed
	// rst:
	// rst: 	:type: hyperflow.VarSumVertex
	py::scope().attr(vars::isInUsed.id.c_str()) = vars::isInUsed;
	// rst: .. data:: hyperflow.vars.isOutUsed
	// rst:           isOutUsed
	// rst:
	// rst: 	:type: hyperflow.VarSumVertex
	py::scope().attr(vars::isOutUsed.id.c_str()) = vars::isOutUsed;
	// rst: .. data:: hyperflow.vars.isInLessOut
	// rst:           isInLessOut
	// rst:
	// rst: 	:type: hyperflow.VarSumVertex
	py::scope().attr(vars::isInLessOut.id.c_str()) = vars::isInLessOut;
	// rst: .. data:: hyperflow.vars.isInGreaterOut
	// rst:           isInGreaterOut
	// rst:
	// rst: 	:type: hyperflow.VarSumVertex
	py::scope().attr(vars::isInGreaterOut.id.c_str()) = vars::isInGreaterOut;
	// rst: .. data:: hyperflow.vars.isInOutZero
	// rst:           isInOutZero
	// rst:
	// rst: 	:type: hyperflow.VarSumVertex
	py::scope().attr(vars::isInOutZero.id.c_str()) = vars::isInOutZero;
	// rst: .. data:: hyperflow.vars.vertexFlow
	// rst:           vertexFlow
	// rst:
	// rst: 	:type: hyperflow.VarSumVertex
	py::scope().attr(vars::vertexFlow.id.c_str()) = vars::vertexFlow;
	// rst: .. data:: hyperflow.vars.isVertexUsed
	// rst:           isVertexUsed
	// rst:
	// rst: 	:type: hyperflow.VarSumVertex
	py::scope().attr(vars::isVertexUsed.id.c_str()) = vars::isVertexUsed;
	// rst: .. data:: hyperflow.vars.transitInternalFlow
	// rst:           transitInternalFlow
	// rst:
	// rst: 	:type: hyperflow.VarSumVertex
	py::scope().attr(vars::transitInternalFlow.id.c_str()) = vars::transitInternalFlow;
	// rst: .. data:: hyperflow.vars.edgeFlow
	// rst:           edgeFlow
	// rst:
	// rst: 	:type: hyperflow.VarSumEdge
	py::scope().attr(vars::edgeFlow.id.c_str()) = vars::edgeFlow;
	// rst: .. data:: hyperflow.vars.isEdgeUsed
	// rst:           isEdgeUsed
	// rst:
	// rst: 	:type: hyperflow.VarSumEdge
	py::scope().attr(vars::isEdgeUsed.id.c_str()) = vars::isEdgeUsed;
	// rst: .. data:: hyperflow.vars.isBothReverseUsed
	// rst:           isBothReverseUsed
	// rst:
	// rst: 	:type: hyperflow.VarSumEdge
	py::scope().attr(vars::isBothReverseUsed.id.c_str()) = vars::isBothReverseUsed;
	// rst:
	// rst: OverallCatalysis
	// rst: """"""""""""""""
	// rst:
	// rst: .. data:: hyperflow.vars.isOverallCata
	// rst:           isOverallCata
	// rst:
	// rst: 	:type: hyperflow.VarSumVertex
	py::scope().attr(vars::isOverallCata.id.c_str()) = vars::isOverallCata;
	// rst:
	// rst: OverallAutocatalysis
	// rst: """"""""""""""""""""
	// rst:
	// rst: .. data:: hyperflow.vars.isOverallAutocata
	// rst:           isOverallAutocata
	// rst:
	// rst: 	:type: hyperflow.VarSumVertex
	py::scope().attr(vars::isOverallAutocata.id.c_str()) = vars::isOverallAutocata;


	// Deprecated ones
	py::scope().attr("vertex") = vars::vertexFlow;
	py::scope().attr("transitInternal") = vars::transitInternalFlow;
	py::scope().attr("edge") = vars::edgeFlow;
}

} // namespace mod::hyperflow::Py