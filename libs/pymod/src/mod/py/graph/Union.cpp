#include <mod/py/Common.hpp>

#include <mod/Chem.hpp>
#include <mod/graph/Graph.hpp>
#include <mod/graph/GraphInterface.hpp>
#include <mod/graph/Printer.hpp>
#include <mod/graph/Union.hpp>
#include <mod/VertexMap.hpp>

#include <mod/py/VertexMap.hpp>

#include <boost/python/stl_iterator.hpp>

namespace mod::graph::Py {
namespace {

Union *make(const py::object &o) {
	return new Union(std::vector<std::shared_ptr<Graph>>(
			py::stl_input_iterator<std::shared_ptr<Graph>>(o),
			py::stl_input_iterator<std::shared_ptr<Graph>>()
	));
}

void exportClass() {

	// rst: .. class:: UnionGraph
	// rst:
	// rst:		An adaptor for a multiset of :class:`Graph`\ s to present them
	// rst:		as their disjoint union.
	// rst:
	// rst:		It functions as an iterable of the adapted graphs,
	// rst:		and it implements the :class:`protocols.LabelledGraph` protocol.
	// rst:
	py::scope UnionScope = py::class_<Union>("UnionGraph")
			// rst:		.. method:: __init__()
			// rst:		            __init__(graphs)
			// rst:
			// rst:			Construct a graph representing the disjoint union of ``graphs``.
			// rst:			Thus, if no graphs are given the empty graph is constructed.
			// rst:
			// rst:			:param graphs: the list of graphs to adapt.
			// rst:			:type graphs: list[Graph]
			// rst:			:raises LogicError: if a given graph is ``None``.
			.def("__init__", py::make_constructor(&make))
			.def(py::self == py::self)
			.def(py::self != py::self)
			.def(py::self < py::self)
					// rst:		.. method:: __str__()
					// rst:
					// rst:			:rtype: str
			.def(str(py::self))
					// rst:		.. method:: __len__()
					// rst:
					// rst:			:returns: the number of adapted graphs.
					// rst:			:rtype: int
			.def("__len__", &Union::size)
					// rst:		.. method:: __iter__()
					// rst:
					// rst:			:returns: the range of graphs adapted by this object.
			.def("__iter__", py::iterator<Union>())
					// rst:		.. method:: __getitem__(i)
					// rst:
					// rst:			:returns: the ``i``\ th adapted graph.
					// rst:			:rtype: Graph
					// rst:			:raises: :class:`LogicError` if ``i`` is out of range.
			.def("__getitem__", &Union::operator[])
			.add_property("numVertices", &Union::numVertices)
			.add_property("vertices", &Union::vertices)
			.add_property("numEdges", &Union::numEdges)
			.add_property("edges", &Union::edges)
					// rst:		.. method:: printTermState
					// rst:
					// rst:			Print the term state for the graph.
			.def("printTermState", &Union::printTermState);


	// rst:		.. class:: Vertex
	// rst:
	// rst:			Implements the :class:`protocols.LabelledGraph.Vertex` protocol.
	// rst:			Additionally, the :attr:`id <protocols.Graph.Vertex.id>` is in the range :math:`[0, numVertices[`.
	// rst:
	// rst:			A non-null vertex represents a specific :class:`Graph.Vertex`.
	// rst:			As the same :class:`Graph` can be part of a :class:`UnionGraph` multiple times,
	// rst:			a vertex also has a graph index to determine which instance of a :class:`Graph`
	// rst:			in the :class:`UnionGraph` the vertex refers to.
	// rst:
	std::string(Union::Vertex::*
	printStereoWithoutOptions)() const = &Union::Vertex::printStereo;
	std::string(Union::Vertex::*
	printStereoWithOptions)(const graph::Printer&) const = &Union::Vertex::printStereo;
	py::class_<Union::Vertex>("Vertex", py::no_init)
			.def(py::init<>())
			.def(str(py::self))
			.def(py::self == py::self)
			.def(py::self != py::self)
			.def(py::self < py::self)
			.def("__hash__", &Union::Vertex::hash)
			.def("__bool__", &Union::Vertex::operator bool)
			.def("isNull", &Union::Vertex::isNull)
			.add_property("id", &Union::Vertex::getId)
			.add_property("graph", &Union::Vertex::getGraph)
			.add_property("degree", &Union::Vertex::getDegree)
			.add_property("incidentEdges", &Union::Vertex::incidentEdges)
			.add_property("stringLabel", py::make_function(&Union::Vertex::getStringLabel,
			                                               py::return_value_policy<py::copy_const_reference>()))
			.add_property("atomId", &Union::Vertex::getAtomId)
			.add_property("isotope", &Union::Vertex::getIsotope)
			.add_property("charge", &Union::Vertex::getCharge)
			.add_property("radical", &Union::Vertex::getRadical)
			.def("printStereo", printStereoWithoutOptions)
			.def("printStereo", printStereoWithOptions)
					// rst:			.. attribute:: graphIndex
					// rst:
					// rst:				(Read-only) The index of the graph in the owning :class:`UnionGraph` this vertex
					// rst:				is from. The index is thus in the range :math:`[0, len(graph)[`.
					// rst:
					// rst:				We can obtain the underlying :class:`Graph` this vertex, ``self``,
					// rst:				is from both directly via the :attr:`vertex` attribute as ``self.vertex.graph``,
					// rst:				or via the graph index as ``self.graph[self.graphIndex]``.
					// rst:
					// rst:				:type: int
					// rst:				:raises: :class:`LogicError` if it is a null vertex.
			.add_property("graphIndex", &Union::Vertex::getGraphIndex)
					// rst:			.. attribute:: vertex
					// rst:
					// rst:				(Read-only) The underlying :class:`Graph.Vertex` this vertex represents.
					// rst:
					// rst:				We can obtain the underlying :class:`Graph` this vertex, ``self``,
					// rst:				is from both directly via the :attr:`vertex` attribute as ``self.vertex.graph``,
					// rst:				or via the graph index as ``self.graph[self.graphIndex]``.
					// rst:
					// rst:				:type: Graph.Vertex
					// rst:				:raises: :class:`LogicError` if it is a null vertex.
			.add_property("vertex", &Union::Vertex::getVertex);

	// rst:		.. class:: Edge
	// rst:
	// rst:			Implements the :class:`protocols.LabelledGraph.Vertex` protocol.
	// rst:
	// rst:			A non-null edge represents a specific :class:`Graph.Edge`.
	// rst:			As the same :class:`Graph` can be part of a :class:`UnionGraph` multiple times,
	// rst:			an edge also has a graph index to determine which instance of a :class:`Graph`
	// rst:			in the :class:`UnionGraph` the edge refers to.
	// rst:
	py::class_<Union::Edge>("Edge", py::no_init)
			.def(py::init<>())
			.def(str(py::self))
			.def(py::self == py::self)
			.def(py::self != py::self)
			.def(py::self < py::self)
			.def("__bool__", &Union::Edge::operator bool)
			.def("isNull", &Union::Edge::isNull)
			.add_property("graph", &Union::Edge::getGraph)
			.add_property("source", &Union::Edge::source)
			.add_property("target", &Union::Edge::target)
			.add_property("stringLabel", py::make_function(&Union::Edge::getStringLabel,
			                                               py::return_value_policy<py::copy_const_reference>()))
			.add_property("bondType", &Union::Edge::getBondType)
					// rst:			.. attribute:: graphIndex
					// rst:
					// rst:				(Read-only) The index of the graph in the owning :class:`UnionGraph` this edge
					// rst:				is from. The index is thus in the range :math:`[0, len(graph)[`.
					// rst:
					// rst:				We can obtain the underlying :class:`Graph` this edge, ``self``,
					// rst:				is from both directly via the :attr:`edge` attribute as ``self.edge.graph``,
					// rst:				or via the graph index as ``self.graph[self.graphIndex]``.
					// rst:
					// rst:				:type: int
					// rst:				:raises: :class:`LogicError` if it is a null edge.
			.add_property("graphIndex", &Union::Edge::getGraphIndex)
					// rst:			.. attribute:: edge
					// rst:
					// rst:				(Read-only) The underlying :class:`Graph.Edge` this edge represents.
					// rst:
					// rst:				We can obtain the underlying :class:`Graph` this edge, ``self``,
					// rst:				is from both directly via the :attr:`edge` attribute as ``self.edge.graph``,
					// rst:				or via the graph index as ``self.graph[self.graphIndex]``.
					// rst:
					// rst:				:type: Graph.Edge
					// rst:				:raises: :class:`LogicError` if it is a null edge.
			.add_property("edge", &Union::Edge::getEdge);

	// rst:		.. class:: VertexRange
	// rst:
	// rst:			Implements the :class:`protocols.Graph.VertexRange` protocol,
	// rst:			in addition to the following functionality.
	// rst:
	py::class_<Union::VertexRange>("VertexRange", py::no_init)
			.def("__iter__", py::iterator<Union::VertexRange>())
					// rst:			.. method:: __getitem__(i)
					// rst:
					// rst:				:returns: the ``i``\ th vertex of the graph.
					// rst:				:rtype: UnionGraph.Vertex
			.def("__getitem__", &Union::VertexRange::operator[]);
	// rst:		.. class:: EdgeRange
	// rst:
	// rst:			Implements the :class:`protocols.Graph.EdgeRange` protocol.
	// rst:
	py::class_<Union::EdgeRange>("EdgeRange", py::no_init)
			.def("__iter__", py::iterator<Union::EdgeRange>());
	// rst:		.. class:: IncidentEdgeRange
	// rst:
	// rst:			Implements the :class:`protocols.Graph.IncidentEdgeRange` protocol.
	// rst:
	py::class_<Union::IncidentEdgeRange>("IncidentEdgeRange", py::no_init)
			.def("__iter__", py::iterator<Union::IncidentEdgeRange>());
}

} // namespace

void Union_doExport() {
	// export the class first, but it needs scoping, so do it in another function
	exportClass();

	// rst: .. class:: VertexMapUnionGraphUnionGraph
	// rst:
	// rst:		Implements the :class:`protocols.VertexMap` protocol.
	mod::Py::exportVertexMap<VertexMap<graph::Union, graph::Union>>("VertexMapUnionGraphUnionGraph");
}

} // namespace mod::graph::Py
