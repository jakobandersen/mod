#include <mod/py/Common.hpp>

#include <mod/causality/Petri.hpp>
#include <mod/dg/GraphInterface.hpp>

namespace mod::causality::Py {

void Petri_doExport() {
	// rst: .. class:: causality.Net
	// rst:
	// rst:		Adaptation of a derivation graph into a Petri net.
	// rst:		Importantly, if the underlying derivation graph is enlarged then
	// rst:		:func:`syncSize` must be called before calling certain other
	// rst:		methods on the object.
	// rst:
	// rst:		.. versionadded:: 1.1
	// rst:
	// rst:		.. todo:: check version added before release
	// rst:
	py::class_<Net, std::shared_ptr<Net>, boost::noncopyable>("Net", py::no_init)
			// rst:		.. method:: __init__(dg)
			// rst:
			// rst:			Calls :meth:`syncSize`.
			// rst:
			// rst:			:param DG dg: the derivation graph to adapt.
			// rst:			:returns: a new Petri net, adapting the given derivation graph.
			// rst:			:raises: :class:`LogicError` if ``dg`` is ``None``.
			// rst:			:raises: :class:`LogicError` if neither ``dg.hasActiveBuilder`` nor ``dg.locked``.
			.def("__init__", py::make_constructor(&Net::make))
			.def(str(py::self))
					// rst:		.. attribute:: dg
					// rst:
					// rst:			(Read-only) The underlying derivation graph.
					// rst:
					// rst:			:type: DG
			.add_property("dg", &Net::getDG)
					// rst:		.. method:: syncSize()
					// rst:
					// rst:			Enlarges the internal data structures to the current size of the underlying derivation graph.
			.def("syncSize", &Net::syncSize)
					// rst:		.. method:: getPostPlaces(e)
					// rst:
					// rst:			:param DG.HyperEdge e: the hyperedge to find unique targets for.
					// rst:			:returns: a list of unique targets for the given hyperedge.
					// rst:			:rtype: list[DG.Vertex]
					// rst:
					// rst:			Requires :meth:`syncSize` to have been called since the last time the underlying derivation graph has changed size.
			.def("getPostPlaces", &Net::getPostPlaces);

	// rst: .. class:: causality.Marking
	// rst:
	// rst:		Representation of a marking on a Petri net.
	// rst:		Importantly, if the underlying derivation graph is enlarged then
	// rst:		:func:`syncSize` must be called before calling certain other
	// rst:		methods on the object.
	// rst:
	// rst:		.. versionadded:: 1.1
	// rst:
	// rst:		.. todo:: check version added before release
	// rst:
	py::class_<Marking>("Marking", py::no_init)
			// rst:		.. method:: __init__(net)
			// rst:
			// rst:			:param Net net: the Petri net to construct a marking for.
			// rst:			:raises: :class:`LogicError` if ``net`` is ``None``.
			.def(py::init<std::shared_ptr<Net>>())
					// rst:		.. attribute:: net
					// rst:
					// rst:			(Read-only) The underlying Petri net.
					// rst:
					// rst:			:type: Net
			.add_property("net", &Marking::getNet)
					// rst:		.. method:: syncSize()
					// rst:
					// rst:			Enlarges the internal data structures to the current size of the underlying derivation graph.
					// rst:
					// rst:			Calls ``net.syncSize()``.
			.def("syncSize", &Marking::syncSize)
			.def(str(py::self))
					// rst:		.. method:: __eq__()
			.def(py::self == py::self)
					// rst:		.. method:: add(v, c)
					// rst:		            add(g, c)
					// rst:
					// rst:			Add `c` tokens on the place `v`.
					// rst:			The second version is equivalent to ``self.add(self.net.dg.findVertex(g), c)``.
					// rst:
					// rst:			:param DG.Vertex v: the place to add tokens to.
					// rst:			:param Graph g: the graph associated to the place to add tokens to.
					// rst:			:param int c: the amount of tokens to add.
					// rst:			:returns: the new total token count on `v`.
					// rst:			:rtype: int
					// rst:			:raises: :class:`LogicError` if ``not v``.
					// rst:			:raises: :class:`LogicError` if ``v.dg != self.net.dg``.
					// rst:			:raises: :class:`LogicError` if ``c < 0``.
					// rst:			:raises: :class:`LogicError` if `g is None`.
					// rst:			:raises: :class:`LogicError` if `!self.net.dg.findVertex(g)`.
					// rst:
					// rst:			Requires :func:`syncSize` to have been called since the last time the underlying derivation graph has changed size.
			.def("add", static_cast<int (Marking::*)(dg::DG::Vertex, int)>(&Marking::add))
			.def("add", static_cast<int (Marking::*)(std::shared_ptr<mod::graph::Graph>, int)>(&Marking::add))
					// rst:		.. method:: remove(v, c)
					// rst:		            remove(g, c)
					// rst:
					// rst:			Remove `c` tokens from the place `v`.
					// rst:			The second version is equivalent to ``self.remove(self.net.dg.findVertex(g), c)``.
					// rst:
					// rst:			:param DG.Vertex v: the place to remove tokens from.
					// rst:			:param Graph g: the graph associated to the place to remove tokens from.
					// rst:			:param int c: the amount of tokens to remove.
					// rst:			:returns: the new total token count on `v`.
					// rst:			:rtype: int
					// rst:			:raises: :class:`LogicError` if ``not v``.
					// rst:			:raises: :class:`LogicError` if ``v.dg != self.net.dg``.
					// rst:			:raises: :class:`LogicError` if ``c < 0``.
					// rst:			:raises: :class:`LogicError` if not enough tokens are left to remove all ``c``.
					// rst:			:raises: :class:`LogicError` if `g is None`.
					// rst:			:raises: :class:`LogicError` if `!self.net.dg.findVertex(g)`.
					// rst:
					// rst:			Requires :func:`syncSize` to have been called since the last time the underlying derivation graph has changed size.
			.def("remove", static_cast<int (Marking::*)(dg::DG::Vertex, int)>(&Marking::remove))
			.def("remove", static_cast<int (Marking::*)(std::shared_ptr<mod::graph::Graph>, int)>(&Marking::remove))
					// rst:		.. method:: __getitem__(v)
					// rst:		            __getitem__(g)
					// rst:
					// rst:			:param DG.Vertex v: the place to query the number of tokens for.
					// rst:			:param Graph g: the graph associated to the place to query the number of tokens for.
					// rst:			:returns: the token count for `v`.
					// rst:				The second version is equivalent to ``self[self.net.dg.findVertex(g)]``.
					// rst:			:rtype: int
					// rst:			:raises: :class:`LogicError` if ``not v``.
					// rst:			:raises: :class:`LogicError` if ``v.dg != self.net.dg``.
					// rst:			:raises: :class:`LogicError` if `g is None`.
					// rst:			:raises: :class:`LogicError` if `!self.net.dg.findVertex(g)`.
					// rst:
					// rst:			Requires :func:`syncSize` to have been called since the last time the underlying derivation graph has changed size.
			.def("__getitem__", static_cast<int (Marking::*)(dg::DG::Vertex) const>(&Marking::operator[]))
			.def("__getitem__", static_cast<int (Marking::*)(std::shared_ptr<mod::graph::Graph>) const>(&Marking::operator[]))
					// rst:		.. attribute:: numTokens
					// rst:
					// rst:			(Read-only) The total number of tokens in the marking.
					// rst:
					// rst:			:type: int
			.add_property("numTokens", &Marking::getNumTokens)
					// rst:		.. method:: getAllEnabled()
					// rst:
					// rst:			:returns: a list of all hyperedges currently enabled for firing.
					// rst:			:rtype: list[DG.HyperEdge]
			.def("getAllEnabled", &Marking::getAllEnabled)
					// rst:		.. method:: getNonZeroPlaces()
					// rst:
					// rst:			:returns: a list of all vertices with tokens.
					// rst:			:rtype: list[DG.Vertex]
			.def("getNonZeroPlaces", &Marking::getNonZeroPlaces)
					// rst:		.. method:: getEmptyPostPlaces(e)
					// rst:
					// rst:			:param DG.HyperEdge e: the edge to query for empty post places.
					// rst:			:returns: a list of all target vertices of the given hyperedge that do not have any tokens.
					// rst:				The list represents a set, so if vertex is a target multiple times it will only be included once.
					// rst:			:rtype: list[DG.Vertex]
					// rst:			:raises: :class:`LogicError` if ``not e``.
					// rst:			:raises: :class:`LogicError` if ``e.dg != self.net.dg``.
			.def("getEmptyPostPlaces", &Marking::getEmptyPostPlaces)
					// rst:		.. method:: isEnabled(e)
					// rst:
					// rst:			:param DG.HyperEdge e: the edge to query for being enabled.
					// rst:			:returns: whether the given hyperedge is enabled for firing.
					// rst:			:rtype: bool
					// rst:			:raises: :class:`LogicError` if ``not e``.
					// rst:			:raises: :class:`LogicError` if ``e.dg != self.net.dg``.
			.def("isEnabled", &Marking::isEnabled)
					// rst:		.. method:: fire(e)
					// rst:
					// rst:			:param DG.HyperEdge e: the edge to fire.
					// rst:			:raises: :class:`LogicError` if ``not e``.
					// rst:			:raises: :class:`LogicError` if ``e.dg != self.net.dg``.
					// rst:			:raises: :class:`LogicError` if ``not self.isEnabled(e)``.
			.def("fire", &Marking::fire);

	py::class_<MarkingSet, boost::noncopyable>("MarkingSet")
			.def("addIfNotSubset", &MarkingSet::addIfNotSubset);
}

} // namespace mod::causality::Py