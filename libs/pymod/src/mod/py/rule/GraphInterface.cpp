#include <mod/py/Common.hpp>
#include <mod/py/VertexMap.hpp>

#include <mod/Chem.hpp>
#include <mod/VertexMap.hpp>
#include <mod/graph/Printer.hpp>
#include <mod/graph/Union.hpp>
#include <mod/rule/GraphInterface.hpp>

namespace mod::rule::Py {
namespace {

void exportLeft() {
	std::string (Rule::LeftGraph::Vertex::*
		printStereoWithoutOptions)() const = &Rule::LeftGraph::Vertex::printStereo;
	std::string (Rule::LeftGraph::Vertex::*
		printStereoWithOptions)(const graph::Printer &) const = &Rule::LeftGraph::Vertex::printStereo;

	py::object ruleObj = py::scope().attr("Rule");
	py::scope ruleScope = ruleObj;

	// rst: .. class:: Rule.LeftGraph
	// rst:
	// rst:		A proxy object representing the left graph of the rule.
	// rst:		The class implements the :class:`protocols.LabelledGraph` protocol,
	// rst:
	py::scope graphObj = py::class_<Rule::LeftGraph>("LeftGraph", py::no_init)
	                     .def(py::self == py::self)
	                     .def(py::self != py::self)
	                     .def(py::self < py::self)
	                     .def(str(py::self))
	                     .add_property("numVertices", &Rule::LeftGraph::numVertices)
	                     .add_property("vertices", &Rule::LeftGraph::vertices)
	                     .add_property("numEdges", &Rule::LeftGraph::numEdges)
	                     .add_property("edges", &Rule::LeftGraph::edges)
	                     // rst:		.. attribute:: rule
	                     // rst:
	                     // rst:			(Read-only) The rule the graph belongs to.
	                     // rst:
	                     // rst:			:type: Rule
	                     .add_property("rule", &Rule::LeftGraph::getRule);

	// rst:		.. class:: Vertex
	// rst:
	// rst:			A descriptor of either a vertex in a left side, or a null vertex.
	// rst:			Implements the :class:`protocols.LabelledGraph.Vertex` protocol.
	// rst:			Additionally, the :attr:`id <protocols.Graph.Vertex.id>` is in the range :math:`[0, numCoreVertices[`.
	// rst:
	py::class_<Rule::LeftGraph::Vertex>("Vertex", py::no_init)
			.def(py::init<>())
			.def(str(py::self))
			.def(py::self == py::self)
			.def(py::self != py::self)
			.def(py::self < py::self)
			.def("__hash__", &Rule::LeftGraph::Vertex::hash)
			.def("__bool__", &Rule::LeftGraph::Vertex::operator bool)
			.def("isNull", &Rule::LeftGraph::Vertex::isNull)
			.add_property("id", &Rule::LeftGraph::Vertex::getId)
			.add_property("graph", &Rule::LeftGraph::Vertex::getGraph)
			.add_property("degree", &Rule::LeftGraph::Vertex::getDegree)
			.add_property("incidentEdges", &Rule::LeftGraph::Vertex::incidentEdges)
			.add_property("stringLabel", py::make_function(&Rule::LeftGraph::Vertex::getStringLabel,
			                                               py::return_value_policy<py::copy_const_reference>()))
			.add_property("atomId", &Rule::LeftGraph::Vertex::getAtomId)
			.add_property("isotope", &Rule::LeftGraph::Vertex::getIsotope)
			.add_property("charge", &Rule::LeftGraph::Vertex::getCharge)
			.add_property("radical", &Rule::LeftGraph::Vertex::getRadical)
			.def("printStereo", printStereoWithoutOptions)
			.def("printStereo", printStereoWithOptions)
			// rst:			.. attribute:: core
			// rst:
			// rst:				(Read-only) The descriptor for this vertex in the core graph.
			// rst:
			// rst:				:type: Rule.Vertex
			.add_property("core", &Rule::LeftGraph::Vertex::getCore)
			// rst:			.. attribute:: rule
			// rst:
			// rst:				(Read-only) The rule the vertex belongs to.
			// rst:
			// rst:				:type: Rule
			// rst:				:raises: :class:`LogicError` if it is a null descriptor.
			.add_property("rule", &Rule::LeftGraph::Vertex::getRule);

	// rst:		.. class:: Edge
	// rst:
	// rst:			A descriptor of either an edge in a left side, or a null edge.
	// rst:			Implements the :class:`protocols.LabelledGraph.Edge` protocol.
	// rst:
	py::class_<Rule::LeftGraph::Edge>("Edge", py::no_init)
			.def(py::init<>())
			.def(str(py::self))
			.def(py::self == py::self)
			.def(py::self != py::self)
			.def(py::self < py::self)
			.def("__bool__", &Rule::LeftGraph::Edge::operator bool)
			.def("isNull", &Rule::LeftGraph::Edge::isNull)
			.add_property("graph", &Rule::LeftGraph::Edge::getGraph)
			.add_property("source", &Rule::LeftGraph::Edge::source)
			.add_property("target", &Rule::LeftGraph::Edge::target)
			.add_property("stringLabel", py::make_function(&Rule::LeftGraph::Edge::getStringLabel,
			                                               py::return_value_policy<py::copy_const_reference>()))
			.add_property("bondType", &Rule::LeftGraph::Edge::getBondType)
			// rst:			.. attribute:: core
			// rst:
			// rst:				(Read-only) The descriptor for this edge in the core graph.
			// rst:
			// rst:				:type: Rule.Edge
			.add_property("core", &Rule::LeftGraph::Edge::getCore)
			// rst:			.. attribute:: rule
			// rst:
			// rst:				(Read-only) The rule the edge belongs to.
			// rst:
			// rst:				:type: Rule
			// rst:				:raises: :class:`LogicError` if it is a null descriptor.
			.add_property("rule", &Rule::LeftGraph::Edge::getRule);

	// rst:		.. class:: VertexRange
	// rst:
	// rst:			Implements the :class:`protocols.Graph.VertexRange` protocol.
	// rst:
	py::class_<Rule::LeftGraph::VertexRange>("VertexRange", py::no_init)
			.def("__iter__", py::iterator<Rule::LeftGraph::VertexRange>());
	// rst:		.. class:: EdgeRange
	// rst:
	// rst:			Implements the :class:`protocols.Graph.EdgeRange` protocol.
	// rst:
	py::class_<Rule::LeftGraph::EdgeRange>("EdgeRange", py::no_init)
			.def("__iter__", py::iterator<Rule::LeftGraph::EdgeRange>());
	// rst:		.. class:: IncidentEdgeRange
	// rst:
	// rst:			Implements the :class:`protocols.Graph.IncidentEdgeRange` protocol.
	// rst:
	py::class_<Rule::LeftGraph::IncidentEdgeRange>("IncidentEdgeRange", py::no_init)
			.def("__iter__", py::iterator<Rule::LeftGraph::IncidentEdgeRange>());

	// rst: .. class:: VertexMapRuleLeftGraphUnionGraph
	// rst:
	// rst:		Implements the :class:`protocols.VertexMap` protocol.
	mod::Py::exportVertexMap<VertexMap<Rule::LeftGraph, graph::Union>>("VertexMapRuleLeftGraphUnionGraph");
}

void exportContext() {
	py::object ruleObj = py::scope().attr("Rule");
	py::scope ruleScope = ruleObj;

	// rst: .. class:: Rule.ContextGraph
	// rst:
	// rst:		A proxy object representing the context graph of the rule.
	// rst:		The class implements the :class:`protocols.Graph` protocol,
	// rst:
	py::scope graphObj = py::class_<Rule::ContextGraph>("ContextGraph", py::no_init)
	                     .def(py::self == py::self)
	                     .def(py::self != py::self)
	                     .def(py::self < py::self)
	                     .def(str(py::self))
	                     .add_property("numVertices", &Rule::ContextGraph::numVertices)
	                     .add_property("vertices", &Rule::ContextGraph::vertices)
	                     .add_property("numEdges", &Rule::ContextGraph::numEdges)
	                     .add_property("edges", &Rule::ContextGraph::edges)
	                     // rst:		.. attribute:: rule
	                     // rst:
	                     // rst:			(Read-only) The rule the graph belongs to.
	                     // rst:
	                     // rst:			:type: Rule
	                     .add_property("rule", &Rule::ContextGraph::getRule);

	// rst:		.. class:: Vertex
	// rst:
	// rst:			A descriptor of either a vertex in a context, or a null vertex.
	// rst:			Implements the :class:`protocols.Graph.Vertex` protocol.
	// rst:			Additionally, the :attr:`id <protocols.Graph.Vertex.id>` is in the range :math:`[0, numCoreVertices[`.
	// rst:
	py::class_<Rule::ContextGraph::Vertex>("Vertex", py::no_init)
			.def(py::init<>())
			.def(str(py::self))
			.def(py::self == py::self)
			.def(py::self != py::self)
			.def(py::self < py::self)
			.def("__hash__", &Rule::ContextGraph::Vertex::hash)
			.def("__bool__", &Rule::ContextGraph::Vertex::operator bool)
			.def("isNull", &Rule::ContextGraph::Vertex::isNull)
			.add_property("id", &Rule::ContextGraph::Vertex::getId)
			.add_property("graph", &Rule::ContextGraph::Vertex::getGraph)
			.add_property("degree", &Rule::ContextGraph::Vertex::getDegree)
			.add_property("incidentEdges", &Rule::ContextGraph::Vertex::incidentEdges)
			// rst:			.. attribute:: core
			// rst:
			// rst:				(Read-only) The descriptor for this vertex in the core graph.
			// rst:
			// rst:				:type: Rule.Vertex
			.add_property("core", &Rule::ContextGraph::Vertex::getCore)
			// rst:			.. attribute:: rule
			// rst:
			// rst:				(Read-only) The rule the vertex belongs to.
			// rst:
			// rst:				:type: Rule
			// rst:				:raises: :class:`LogicError` if it is a null descriptor.
			.add_property("rule", &Rule::ContextGraph::Vertex::getRule);

	// rst:		.. class:: Edge
	// rst:
	// rst:			A descriptor of either an edge in a context, or a null edge.
	// rst:			Implements the :class:`protocols.LabelledGraph.Edge` protocol.
	// rst:
	py::class_<Rule::ContextGraph::Edge>("Edge", py::no_init)
			.def(py::init<>())
			.def(str(py::self))
			.def(py::self == py::self)
			.def(py::self != py::self)
			.def(py::self < py::self)
			.def("__bool__", &Rule::ContextGraph::Edge::operator bool)
			.def("isNull", &Rule::ContextGraph::Edge::isNull)
			.add_property("graph", &Rule::ContextGraph::Edge::getGraph)
			.add_property("source", &Rule::ContextGraph::Edge::source)
			.add_property("target", &Rule::ContextGraph::Edge::target)
			// rst:			.. attribute:: core
			// rst:
			// rst:				(Read-only) The descriptor for this edge in the core graph.
			// rst:
			// rst:				:type: Rule.Edge
			.add_property("core", &Rule::ContextGraph::Edge::getCore)
			// rst:			.. attribute:: graph
			// rst:
			// rst:				(Read-only) The rule the edge belongs to.
			// rst:
			// rst:				:type: Rule
			// rst:				:raises: :class:`LogicError` if it is a null descriptor.
			.add_property("rule", &Rule::ContextGraph::Edge::getRule);

	// rst:		.. class:: VertexRange
	// rst:
	// rst:			Implements the :class:`protocols.Graph.VertexRange` protocol.
	// rst:
	py::class_<Rule::ContextGraph::VertexRange>("VertexRange", py::no_init)
			.def("__iter__", py::iterator<Rule::ContextGraph::VertexRange>());
	// rst:		.. class:: EdgeRange
	// rst:
	// rst:			Implements the :class:`protocols.Graph.EdgeRange` protocol.
	// rst:
	py::class_<Rule::ContextGraph::EdgeRange>("EdgeRange", py::no_init)
			.def("__iter__", py::iterator<Rule::ContextGraph::EdgeRange>());
	// rst:		.. class:: IncidentEdgeRange
	// rst:
	// rst:			Implements the :class:`protocols.Graph.IncidentEdgeRange` protocol.
	// rst:
	py::class_<Rule::ContextGraph::IncidentEdgeRange>("IncidentEdgeRange", py::no_init)
			.def("__iter__", py::iterator<Rule::ContextGraph::IncidentEdgeRange>());
}

void exportRight() {
	std::string (Rule::RightGraph::Vertex::*
		printStereoWithoutOptions)() const = &Rule::RightGraph::Vertex::printStereo;
	std::string (Rule::RightGraph::Vertex::*
		printStereoWithOptions)(const graph::Printer &) const = &Rule::RightGraph::Vertex::printStereo;

	py::object ruleObj = py::scope().attr("Rule");
	py::scope ruleScope = ruleObj;

	// rst: .. class:: Rule.RightGraph
	// rst:
	// rst:		A proxy object representing the right graph of the rule.
	// rst:		The class implements the :class:`protocols.LabelledGraph` protocol,
	// rst:
	py::scope graphObj = py::class_<Rule::RightGraph>("RightGraph", py::no_init)
	                     .def(py::self == py::self)
	                     .def(py::self != py::self)
	                     .def(py::self < py::self)
	                     .def(str(py::self))
	                     .add_property("numVertices", &Rule::RightGraph::numVertices)
	                     .add_property("vertices", &Rule::RightGraph::vertices)
	                     .add_property("numEdges", &Rule::RightGraph::numEdges)
	                     .add_property("edges", &Rule::RightGraph::edges)
	                     // rst:		.. attribute:: rule
	                     // rst:
	                     // rst:			(Read-only) The rule the graph belongs to.
	                     // rst:
	                     // rst:			:type: Rule
	                     .add_property("rule", &Rule::RightGraph::getRule);

	// rst:		.. class:: Vertex
	// rst:
	// rst:			A descriptor of either a vertex in a right side, or a null vertex.
	// rst:			Implements the :class:`protocols.LabelledGraph.Vertex` protocol.
	// rst:			Additionally, the :attr:`id <protocols.Graph.Vertex.id>` is in the range :math:`[0, numCoreVertices[`.
	// rst:
	py::class_<Rule::RightGraph::Vertex>("Vertex", py::no_init)
			.def(py::init<>())
			.def(str(py::self))
			.def(py::self == py::self)
			.def(py::self != py::self)
			.def(py::self < py::self)
			.def("__hash__", &Rule::RightGraph::Vertex::hash)
			.def("__bool__", &Rule::RightGraph::Vertex::operator bool)
			.def("isNull", &Rule::RightGraph::Vertex::isNull)
			.add_property("id", &Rule::RightGraph::Vertex::getId)
			.add_property("graph", &Rule::RightGraph::Vertex::getGraph)
			.add_property("degree", &Rule::RightGraph::Vertex::getDegree)
			.add_property("incidentEdges", &Rule::RightGraph::Vertex::incidentEdges)
			.add_property("stringLabel", py::make_function(&Rule::RightGraph::Vertex::getStringLabel,
			                                               py::return_value_policy<py::copy_const_reference>()))
			.add_property("atomId", &Rule::RightGraph::Vertex::getAtomId)
			.add_property("isotope", &Rule::RightGraph::Vertex::getIsotope)
			.add_property("charge", &Rule::RightGraph::Vertex::getCharge)
			.add_property("radical", &Rule::RightGraph::Vertex::getRadical)
			.def("printStereo", printStereoWithoutOptions)
			.def("printStereo", printStereoWithOptions)
			// rst:			.. attribute:: core
			// rst:
			// rst:				(Read-only) The descriptor for this vertex in the core graph.
			// rst:
			// rst:				:type: Rule.Vertex
			.add_property("core", &Rule::RightGraph::Vertex::getCore)
			// rst:			.. attribute:: rule
			// rst:
			// rst:				(Read-only) The rule the vertex belongs to.
			// rst:
			// rst:				:type: Rule
			// rst:				:raises: :class:`LogicError` if it is a null descriptor.
			.add_property("rule", &Rule::RightGraph::Vertex::getRule);

	// rst:		.. class:: Edge
	// rst:
	// rst:			A descriptor of either an edge in a right side, or a null edge.
	// rst:			Implements the :class:`protocols.LabelledGraph.Edge` protocol.
	// rst:
	py::class_<Rule::RightGraph::Edge>("Edge", py::no_init)
			.def(py::init<>())
			.def(str(py::self))
			.def(py::self == py::self)
			.def(py::self != py::self)
			.def(py::self < py::self)
			.def("__bool__", &Rule::RightGraph::Edge::operator bool)
			.def("isNull", &Rule::RightGraph::Edge::isNull)
			.add_property("graph", &Rule::RightGraph::Edge::getGraph)
			.add_property("source", &Rule::RightGraph::Edge::source)
			.add_property("target", &Rule::RightGraph::Edge::target)
			.add_property("stringLabel", py::make_function(&Rule::RightGraph::Edge::getStringLabel,
			                                               py::return_value_policy<py::copy_const_reference>()))
			.add_property("bondType", &Rule::RightGraph::Edge::getBondType)
			// rst:			.. attribute:: core
			// rst:
			// rst:				(Read-only) The descriptor for this edge in the core graph.
			// rst:
			// rst:				:type: Rule.Edge
			.add_property("core", &Rule::RightGraph::Edge::getCore)
			// rst:			.. attribute:: rule
			// rst:
			// rst:				(Read-only) The rule the edge belongs to.
			// rst:
			// rst:				:type: Rule
			// rst:				:raises: :class:`LogicError` if it is a null descriptor.
			.add_property("rule", &Rule::RightGraph::Edge::getRule);

	// rst:		.. class:: VertexRange
	// rst:
	// rst:			Implements the :class:`protocols.Graph.VertexRange` protocol.
	// rst:
	py::class_<Rule::RightGraph::VertexRange>("VertexRange", py::no_init)
			.def("__iter__", py::iterator<Rule::RightGraph::VertexRange>());
	// rst:		.. class:: EdgeRange
	// rst:
	// rst:			Implements the :class:`protocols.Graph.EdgeRange` protocol.
	// rst:
	py::class_<Rule::RightGraph::EdgeRange>("EdgeRange", py::no_init)
			.def("__iter__", py::iterator<Rule::RightGraph::EdgeRange>());
	// rst:		.. class:: IncidentEdgeRange
	// rst:
	// rst:			Implements the :class:`protocols.Graph.IncidentEdgeRange` protocol.
	// rst:
	py::class_<Rule::RightGraph::IncidentEdgeRange>("IncidentEdgeRange", py::no_init)
			.def("__iter__", py::iterator<Rule::RightGraph::IncidentEdgeRange>());

	// rst: .. class:: VertexMapRuleRightGraphUnionGraph
	// rst:
	// rst:		Implements the :class:`protocols.VertexMap` protocol.
	mod::Py::exportVertexMap<VertexMap<Rule::RightGraph, graph::Union>>("VertexMapRuleRightGraphUnionGraph");
}

void exportCore() {
	py::object ruleObj = py::scope().attr("Rule");
	py::scope ruleScope = ruleObj;

	// rst: .. class:: Rule.Vertex
	// rst:
	// rst:		Implements the :class:`protocols.Graph.Vertex` protocol.
	// rst:		Additionally, the :attr:`id <protocols.Graph.Vertex.id>` is in the range :math:`[0, numVertices[`.
	// rst:
	py::class_<Rule::Vertex>("Vertex", py::no_init)
			.def(py::init<>())
			.def(str(py::self))
			.def(py::self == py::self)
			.def(py::self != py::self)
			.def(py::self < py::self)
			.def("__hash__", &Rule::Vertex::hash)
			.def("__bool__", &Rule::Vertex::operator bool)
			.def("isNull", &Rule::Vertex::isNull)
			.add_property("id", &Rule::Vertex::getId)
			.add_property("graph", &Rule::Vertex::getGraph)
			.add_property("degree", &Rule::Vertex::getDegree)
			.add_property("incidentEdges", &Rule::Vertex::incidentEdges)
			// rst:		.. attribute:: rule
			// rst:
			// rst:			An alias for :attr:`protocols.Graph.Vertex.graph`.
			// rst:
			// rst:			:type: Rule
			.add_property("rule", &Rule::Vertex::getRule)
			// rst:		.. attribute:: left
			// rst:
			// rst:			(Read-only) A null descriptor if this vertex is not in the left graph,
			// rst:			otherwise the descriptor of this vertex in the left graph.
			// rst:
			// rst:			:type: LeftGraph.Vertex
			.add_property("left", &Rule::Vertex::getLeft)
			// rst:		.. attribute:: context
			// rst:
			// rst:			(Read-only) A null descriptor if this vertex is not in the context graph,
			// rst:			otherwise the descriptor of this vertex in the context graph.
			// rst:
			// rst:			:type: ContextGraph.Vertex
			.add_property("context", &Rule::Vertex::getContext)
			// rst:		.. attribute:: right
			// rst:
			// rst:			(Read-only) A null descriptor if this vertex is not in the right graph,
			// rst:			otherwise the descriptor of this vertex in the right graph.
			// rst:
			// rst:			:type: RightGraph.Vertex
			.add_property("right", &Rule::Vertex::getRight)
			// rst:		.. method:: get2DX(withHydrogens=True)
			// rst:
			// rst:			:returns: the x-coordinate in a 2D depiction of the rule.
			// rst:				Different sets of coordinates exists for rendering with and without certain hydrogens.
			// rst:			:rtype: float
			// rst:			:raises: :class:`LogicError` if it is a null descriptor, or
			// rst:				if ``withHydrogens`` is ``True`` and the vertex is a "clean" hydrogen.
			.def("get2DX", &Rule::Vertex::get2DX, py::arg("withHydrogens") = true)
			// rst:		.. method:: get2DY(withHydrogens=True)
			// rst:
			// rst:			:returns: the y-coordinate in a 2D depiction of the rule.
			// rst:				Different sets of coordinates exists for rendering with and without certain hydrogens.
			// rst:			:rtype: float
			// rst:			:raises: :class:`LogicError` if it is a null descriptor, or
			// rst:				if ``withHydrogens`` is ``True`` and the vertex is a "clean" hydrogen.
			.def("get2DY", &Rule::Vertex::get2DY, py::arg("withHydrogens") = true);

	// rst: .. class:: Rule.Edge
	// rst:
	// rst:		Implements the :class:`protocols.Graph.Edge` protocol.
	// rst:
	py::class_<Rule::Edge>("Edge", py::no_init)
			.def(py::init<>())
			.def(str(py::self))
			.def(py::self == py::self)
			.def(py::self != py::self)
			.def(py::self < py::self)
			.def("__bool__", &Rule::Edge::operator bool)
			.def("isNull", &Rule::Edge::isNull)
			.add_property("graph", &Rule::Edge::getGraph)
			.add_property("source", &Rule::Edge::source)
			.add_property("target", &Rule::Edge::target)
			// rst:		.. attribute:: rule
			// rst:
			// rst:			An alias for :attr:`protocols.Graph.Edge.graph`.
			// rst:
			// rst:			:type: Rule
			.add_property("rule", &Rule::Edge::getRule)
			// rst:		.. attribute:: left
			// rst:
			// rst:			(Read-only) A null descriptor if this edge is not in the left graph,
			// rst:			otherwise the descriptor of this edge in the left graph.
			// rst:
			// rst:			:type: LeftGraph.Edge
			.add_property("left", &Rule::Edge::getLeft)
			// rst:		.. attribute:: context
			// rst:
			// rst:			(Read-only) A null descriptor if this edge is not in the context graph,
			// rst:			otherwise the descriptor of this edge in the context graph.
			// rst:
			// rst:			:type: ContextGraph.Edge
			.add_property("context", &Rule::Edge::getContext)
			// rst:		.. attribute:: right
			// rst:
			// rst:			(Read-only) A null descriptor if this edge is not in the right graph,
			// rst:			otherwise the descriptor of this edge in the right graph.
			// rst:
			// rst:			:type: RightGraph.Edge
			.add_property("right", &Rule::Edge::getRight);

	// rst: .. class:: Rule.VertexRange
	// rst:
	// rst:		Implements the :class:`protocols.Graph.VertexRange` protocol,
	// rst:		in addition to the following functionality.
	// rst:
	py::class_<Rule::VertexRange>("VertexRange", py::no_init)
			.def("__iter__", py::iterator<Rule::VertexRange>())
			// rst:		.. method:: __getitem__(i)
			// rst:
			// rst:			:returns: the ``i``\ th vertex of the graph.
			// rst:			:rtype: Rule.Vertex
			.def("__getitem__", &Rule::VertexRange::operator[]);
	// rst: .. class:: Rule.EdgeRange
	// rst:
	// rst:		Implements the :class:`protocols.Graph.EdgeRange` protocol.
	// rst:
	py::class_<Rule::EdgeRange>("EdgeRange", py::no_init)
			.def("__iter__", py::iterator<Rule::EdgeRange>());
	// rst: .. class:: Rule.IncidentEdgeRange
	// rst:
	// rst:		Implements the :class:`protocols.Graph.IncidentEdgeRange` protocol.
	// rst:
	py::class_<Rule::IncidentEdgeRange>("IncidentEdgeRange", py::no_init)
			.def("__iter__", py::iterator<Rule::IncidentEdgeRange>());
}

} // namespace

void GraphInterface_doExport() {
	exportLeft();
	exportContext();
	exportRight();
	exportCore();
}

} // namespace mod::rule::Py