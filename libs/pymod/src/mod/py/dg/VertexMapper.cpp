#include <mod/py/Common.hpp>

#include <mod/dg/VertexMapper.hpp>
#include <mod/graph/Union.hpp>
#include <mod/rule/Rule.hpp>

namespace mod::dg::Py {

void VertexMapper_doExport() {
	// rst: .. class:: DGVertexMapper
	// rst:
	// rst:		A class for enumerating all valid vertex maps for a given :class:`DG.HyperEdge`.
	// rst:		That is, for such a hyperedge, collect the graphs associated with
	// rst:		respectively the source and target vertices, and create the disjoint union of those graphs.
	// rst:		Let the result be the graphs :math:`G'` and :math:`H'`.
	// rst:		Then each rule :math:`p = (L\leftarrow K\rightarrow R)` associated with the hyperedge,
	// rst:		generate direct derivations :math:`G\overset{p, m}{\Rightarrow} H`
	// rst:		where :math:`G` is isomorphic to :math:`G'` and :math:`H` is isomorphic to :math:`H'`.
	// rst:
	// rst:		Each recorded vertex map is a map :math:`V(G) \rightarrow V(H)`.
	// rst:		Those maps are available in this class.
	// rst:
	py::scope parent = py::class_<VertexMapper, boost::noncopyable>("DGVertexMapper", py::no_init)
			// rst:		.. method:: __init__(e, upToIsomorphismG=True, leftLimit=1, rightLimit=2**30)
			// rst:
			// rst:			Construct a vertex map holder. It will immediately calculate all vertex maps for the derivations underlying the
			// rst:			given hyperedge.
			// rst:			By default all maps :math:`V(G) \rightarrow V(H)` are enumerated.
			// rst:			To only enumerate a singular vertex map per rule, use ``upToIsomorphismG = True`` and ``leftLimit = rightLimit = 1``.
			// rst:
			// rst:			:param DG.HyperEdge e: the hyperedge to construct vertex maps for.
			// rst:			:param bool upToIsomrophismG: whether to enumerate all :math:`m`, or just those such that all bottom spans
			// rst:				:math:`(G\leftarrow D\rightarrow H)` up to isomorphism are generated.
			// rst:			:param int leftLimit: after bottom span generation, find this many isomorphisms back to the sources of the hyperedge.
			// rst:			:param int rightLimit: after bottom span generation, find this many isomorphisms back to the targets of the hyperedge.
			// rst:			:raises: :class:`LogicError` if ``not e``.
			.def(py::init<DG::HyperEdge, bool, int, int, int>(
					(py::args("upToIsomorphismG") = true,
					 py::args("leftLimit") = 1 << 30, py::args("rightLimit") = 1 << 30,
					 py::args("verbosity") = 0)))
					// rst:		.. attribute:: edge
					// rst:
					// rst:			(Read-only) The hyperedge to calculate vertex maps for.
					// rst:
					// rst:			:type: DG.HyperEdge
			.def_readonly("edge", &VertexMapper::getEdge)
					// rst:		.. attribute:: left
					// rst:		               right
					// rst:
					// rst:			:returns: the disjoint union of graphs from respectively the source and target vertices of the hyperedge.
					// rst:				That is, the graphs :math:`G` and :math:`H` that are the domain and codomain graphs of the calculated
					// rst:				vertex maps.
					// rst:			:type: UnionGraph
			.def_readonly("left", &VertexMapper::getLeft)
			.def_readonly("right", &VertexMapper::getRight)
					// rst:		.. method:: __iter__()
					// rst:
					// rst:			:returns: an iterator over the vertex maps,
					// rst:				see :meth:`__getitem__` for the specific type.
			.def("__iter__", py::iterator<VertexMapper>())
					// rst:		.. attribute:: size
					// rst:
					// rst:			(Read-only) The number of vertex maps calculated.
					// rst:
					// rst:			:type: int
			.def_readonly("size", &VertexMapper::size)
					// rst:		.. method:: __len__()
					// rst:
					// rst:			:returns: :attr:`size`
					// rst:			:rtype: int
			.def("__len__", &VertexMapper::size)
					// rst:		.. method:: __getitem__(i)
					// rst:
					// rst:			:param int i: the index of the map to retrieve.
					// rst:			:returns: the ``i``\ th vertex map.
					// rst:			:rtype: Result
					// rst:			:raises: :class:`LogicError` if ``i`` is out of range.
			.def("__getitem__", &VertexMapper::operator[]);

	// rst: .. class:: DGVertexMapper.Result
	// rst:
	// rst:		The value type returned for each vertex map.
	// rst:		the rule used to generate the map, and then the actual map.
	// rst:
	py::class_<VertexMapper::Result>("Result", py::no_init)
			// rst: 	.. attribute:: rule
			// rst:
			// rst: 		The rule used to generate the map.
			// rst:
			// rst: 		:type: Rule
			.def_readonly("rule", &VertexMapper::Result::r)
					// rst: 	.. attribute:: map
					// rst:
					// rst: 		The actual vertex map :math:`V(G) \rightarrow V(H)`.
					// rst:
					// rst: 		:type: VertexMapUnionGraphUnionGraph
			.def_readonly("map", &VertexMapper::Result::map);
}

} // namespace mod::dg::Py