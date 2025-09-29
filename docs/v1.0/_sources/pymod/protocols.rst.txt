.. _py-protocols:

.. default-domain:: py

.. py:currentmodule:: mod.protocols
.. cpp:namespace:: mod

**********************************************************
Protocols
**********************************************************

In Python a protocol is a specification for structural subtyping
(static duck-typing),
e.g., see https://www.python.org/dev/peps/pep-0544/.
This is very similar to what is called a "concept" in C++,
e.g., see https://en.wikipedia.org/wiki/Concept_(generic_programming).
Python for example has the Iterator protocol that specifies how to implement
objects one can iterate over.

This page describes protocols in MØD that are implemented by several classes
in the Python interface.
Note that these classes are not real types, but merely serves as a
documentation device.


.. class:: Graph

	The undirected graph protocol.

	In addition to the following elements, a graph is also equality comparable.

	.. attribute:: numVertices

		(Read-only) The number of vertices in the graph.

		:type: int

	.. attribute:: vertices

		(Read-only) An iterable of all vertices in the graph.

		:type: VertexRange

	.. attribute:: numEdges

		(Read-only) The number of edges in the graph.

		:type: int

	.. attribute:: edges

		(Read-only) An iterable of all edges in the graph.

		:type: EdgeRange

	.. class:: Vertex

		A descriptor of either a vertex in a graph, or a null vertex.

		Besides the operations below, descriptors are

		- equality comparable,
		- totally ordered, and
		- hashable

		.. method:: __init__()

			Constructs a null descriptor.

		.. :method:: __bool__()

			:returns: ``not isNull()``
			:rtype: bool

		.. method:: isNull()

			:returns: whether this is a null descriptor or not.
			:rtype: bool

		.. py:attribute:: id

			(Read-only) The index of the vertex. A non-negative number.
			The IDs are not guaranteed in general to be consequtive.
			See the documentation for concrete graphs for sepcific additional
			guarantees.

			:type: int
			:raises: :class:`~mod.LogicError` if it is a null descriptor.

		.. attribute:: graph

			(Read-only) The graph the vertex belongs to.

			:type: Graph
			:raises: :class:`~mod.LogicError` if it is a null descriptor.


		.. attribute:: degree

			(Read-only) The degree of the vertex.

			:type: int
			:raises: :class:`~mod.LogicError` if it is a null descriptor.


		.. attribute:: incidentEdges

			(Read-only) A range of incident edges to this vertex.

			:type: Graph.IncidentEdgeRange
			:raises: :class:`~mod.LogicError` if it is a null descriptor.


	.. class:: Edge

		A descriptor of either an edge in a graph, or a null edge.

		Besides the operations below, descriptors are

		- equality comparable, and
		- totally ordered

		.. method:: __init__()

			Constructs a null descriptor.

		.. method:: __bool__()

			:returns: ``not isNull()``
			:rtype: bool

		.. method:: isNull()

			:returns: whether this is a null descriptor or not.
			:rtype: bool

		.. attribute:: graph

			(Read-only) The graph the edge belongs to.

			:type: Graph
			:raises: :class:`~mod.LogicError` if it is a null descriptor.

		.. attribute:: source

			(Read-only) The source vertex of the edge.

			:type: Graph.Vertex
			:raises: :class:`~mod.LogicError` if it is a null descriptor.

		.. attribute:: target

			(Read-only) The target vertex of the edge.

			:type: Graph.Vertex
			:raises: :class:`~mod.LogicError` if it is a null descriptor.


	.. class:: VertexRange

		An iterable of all vertices in the graph.

	.. class:: EdgeRange

		An iterable of all edges in the graph.

	.. class:: IncidentEdgeRange

		An iterable of all edges incident to a specific vertex in the graph.



.. class:: LabelledGraph

	An extension of the :class:`Graph` protocol,
	with access to label information on vertices and edges.

	.. class:: Vertex

		An extension of the :class:`Graph.Vertex` protocol.

		.. py:attribute:: stringLabel

			(Read-only) The string label of the vertex.

			:type: str
			:raises: :class:`~mod.LogicError` if it is a null descriptor.

		.. attribute:: atomId

			(Read-only) The atom ID of the vertex.

			:type: AtomId
			:raises: :class:`~mod.LogicError` if it is a null descriptor.

		.. attribute:: isotope

			(Read-only) The isotope of the vertex.

			:type: Isotope
			:raises: :class:`~mod.LogicError` if it is a null descriptor.

		.. attribute:: charge

			(Read-only) The charge of the vertex.

			:type: Charge
			:raises: :class:`~mod.LogicError` if it is a null descriptor.

		.. :attribute:: radical

			(Read-only) The radical status of the vertex.

			:type: bool
			:raises: :class:`~mod.LogicError` if it is a null descriptor.

		.. method:: printStereo()
		            printStereo(p)

			Print the stereo configuration for the vertex.

			:param GraphPrinter p: the printing options used for the depiction.
			:returns: the name of the PDF-file that will be compiled in
				post-processing.
			:rtype: str
			:raises: :class:`~mod.LogicError` if it is a null descriptor.


	.. class:: Edge

		An extension of the :class:`Graph.Edge` protocol.

		.. attribute:: stringLabel

			(Read-only) The string label of the edge.

			:type: str
			:raises: :class:`~mod.LogicError` if it is a null descriptor.

		.. attribute:: bondType

			(Read-only) The bond type of the edge.

			:type: BondType
			:raises: :class:`~mod.LogicError` if it is a null descriptor.


.. class:: VertexMap

	The protocol implemented by classes that represent a map of vertices
	from a domain :class:`Graph` to a codomain :class:`Graph`.

	.. attribute:: domain

		(Read-only) The graph for which the vertices form the domain of the map.

		:type: :class:`Graph`

	.. attribute:: codomain

		(Read-only) The graph for which the vertices form the codomain of the map.

		:type: :class:`Graph`

	.. method:: __getitem__(v)

		:param Graph.Vertex v: the vertex to return the image of.
		:returns: the image of the given domain vertex.
			May return a null vertex if the map is partial.
		:rtype: Graph.Vertex
		:raises: :class:`~mod.LogicError` if ``v`` is a null vertex.
		:raises: :class:`~mod.LogicError` if ``v`` does not belong to the domain graph for the map.

	.. method:: inverse(v)

		:param Graph.Vertex v: the codomain vertex to return the domain vertex for.
		:returns: the domain vertex that maps to the given codomain vertex.
			May return a null vertex if non exist.
		:rtype: Graph.Vertex
		:raises: :class:`~mod.LogicError` if ``v`` is a null vertex.
		:raises: :class:`~mod.LogicError` if ``v`` does not belong to the codomain graph for the map.
