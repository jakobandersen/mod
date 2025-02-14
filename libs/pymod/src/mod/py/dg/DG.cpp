#include <mod/py/Common.hpp>

#include <mod/Derivation.hpp>
#include <mod/dg/Builder.hpp>
#include <mod/dg/DG.hpp>
#include <mod/dg/GraphInterface.hpp>
#include <mod/dg/Printer.hpp>
#include <mod/graph/Graph.hpp>
#include <mod/graph/Printer.hpp>

#include <mod/Function.hpp>

namespace mod::dg::Py {
namespace {

DG::HyperEdge (DG::*findEdgeVertices)(const std::vector<DG::Vertex> &,
                                      const std::vector<DG::Vertex> &) const = &DG::findEdge;
DG::HyperEdge (DG::*findEdgeGraphs)(const std::vector<std::shared_ptr<mod::graph::Graph> > &,
                                    const std::vector<std::shared_ptr<mod::graph::Graph> > &) const = &DG::findEdge;

// see https://stackoverflow.com/questions/19062657/is-there-a-way-to-wrap-the-function-return-value-object-in-python-using-move-i

std::shared_ptr<Builder> DG_build(std::shared_ptr<DG> dg_,
								  std::shared_ptr<Function<void(dg::DG::Vertex)>> onNewVertex,
                                  std::shared_ptr<Function<void(dg::DG::HyperEdge)>> onNewHyperEdge) {
	return std::make_shared<Builder>(dg_->build(onNewVertex, onNewHyperEdge));
}

} // namespace

void DG_doExport() {
	using Load = std::shared_ptr<DG> (*)(const std::vector<std::shared_ptr<mod::graph::Graph>> &,
	                                     const std::vector<std::shared_ptr<rule::Rule>> &,
	                                     const std::string &,
	                                     IsomorphismPolicy, int);

	// rst: .. class:: DG
	// rst:
	// rst:		The derivation graph class. A derivation graph is a directed multi-hypergraph
	// rst:		:math:`\mathcal{H} = (V, E)`. Each hyperedge :math:`e\in E` is thus an ordered pair
	// rst:		:math:`(e^+, e^-)` of multisets of vertices, the sources and the targets.
	// rst:		Each vertex is annotated with a graph, and each hyperedge is annotated with list of transformation rules.
	// rst:		A derivation graph is constructed incrementally using a :class:`DG.Builder` obtained from the :meth:`build()`
	// rst:		function. When the obtained builder is destructed the derivation graph becomes locked and can no longer be modified.
	// rst:
	py::class_<DG, std::shared_ptr<DG>, boost::noncopyable>("DG", py::no_init)
			// rst:		.. method:: __init__(*, labelSettings=LabelSettings(LabelType.String, LabelRelation.Isomorphism), \
			// rst:		                        graphDatabase=[], \
			// rst:		                        graphPolicy=IsomorphismPolicy.Check)
			// rst:
			// rst:			Create an empty unlocked derivation graph object.
			// rst:
			// rst:			:param LabelSettings labelSettings: defines which category the derivation graph object works in.
			// rst:				All morphism calculations (monomorphism and isomorphism) are thus defined by the :class:`LabelType`,
			// rst:				while the :class:`LabelRelation` is used for for monomorphism enumeration.
			// rst:			:param graphDatabase: an initial graph database.
			// rst:				Any subsequently added or constructed graph for this object
			// rst:				will be checked for isomorphism against the graph database.
			// rst:			:type graphDatabase: list[Graph]
			// rst:			:param IsomorphismPolicy graphPolicy: the policy for how the graphs of ``graphDatabase`` are checked for
			// rst:				isomorphism against each other initially. Only use ``IsomorphismPolicy.TrustMe`` if you are
			// rst:				absolutely sure that the graphs are unique up to isomorphism.
			// rst:			:raises: :class:`LogicError` if ``graphPolicy == IsomorphismPolicy.Check`` and two graphs
			// rst:				in ``graphDatabase`` are different objects but represents isomorphic graphs.
			// rst:			:raises: :class:`LogicError` if there is a ``None`` in ``graphDatabase``.
			.def("__init__", py::make_constructor(&DG::make))
					// rst:		.. attribute:: id
					// rst:
					// rst:			The unique instance id among all :class:`DG` objects.
					// rst:
					// rst:			:type: int
			.add_property("id", &DG::getId)
			.def(str(py::self))
					// rst:		.. attribute:: labelSettings
					// rst:
					// rst:			(Read-only) The label settings for the derivation graph.
					// rst:
					// rst:			:type: LabelSettings
			.add_property("labelSettings", &DG::getLabelSettings)
					//------------------------------------------------------------------
					// rst:		.. attribute:: hasActiveBuilder
					// rst:
					// rst:			(Read-only) Whether :meth:`build` has been called and the returned :class:`DG.Builder` is still active.
					// rst:
					// rst:			:type: bool
			.add_property("hasActiveBuilder", &DG::hasActiveBuilder)
					// rst:		.. attribute:: locked
					// rst:
					// rst:			(Read-only) Whether the derivation graph is locked or not.
					// rst:
					// rst:			:type: bool
			.add_property("locked", &DG::isLocked)
					//------------------------------------------------------------------
					// rst:		.. attribute:: numVertices
					// rst:
					// rst:			(Read-only) The number of vertices in the derivation graph.
					// rst:
					// rst:			:type: int
					// rst:			:raises: :class:`LogicError` if neither ``hasActiveBuilder`` nor ``isLocked``.
			.add_property("numVertices", &DG::numVertices)
					// rst:		.. attribute:: vertices
					// rst:
					// rst:			(Read-only) An iterable of all vertices in the derivation graph.
					// rst:
					// rst:			:type: DG.VertexRange
					// rst:			:raises: :class:`LogicError` if neither ``hasActiveBuilder`` nor ``isLocked``.
			.add_property("vertices", &DG::vertices)
					// rst:		.. attribute:: numEdges
					// rst:
					// rst:			(Read-only) The number of hyperedges in the derivation graph.
					// rst:
					// rst:			:type: int
					// rst:			:raises: :class:`LogicError` if neither ``hasActiveBuilder`` nor ``isLocked``.
			.add_property("numEdges", &DG::numEdges)
					// rst:		.. attribute:: edges
					// rst:
					// rst:			(Read-only) An iterable of all hyperedges in the derivation graph.
					// rst:
					// rst:			:type: DG.EdgeRange
					// rst:			:raises: :class:`LogicError` if neither ``hasActiveBuilder`` nor ``isLocked``.
			.add_property("edges", &DG::edges)
					//------------------------------------------------------------------
					// rst:		.. method:: findVertex(g)
					// rst:
					// rst:			:param Graph g: the graph to find a vertex which has it associated.
					// rst:			:returns: a vertex descriptor for which the given graph is associated,
					// rst:				or a null descriptor if no such vertex exists.
					// rst:			:rtype: DG.Vertex
					// rst:			:raises: :class:`LogicError` if neither ``hasActiveBuilder`` nor ``isLocked``.
					// rst:			:raises: :class:`LogicError` if ``g`` is ``None``.
			.def("findVertex", &DG::findVertex)
					// rst:		.. method:: findEdge(sources, targets)
					// rst:		            findEdge(sourceGraphs, targetGraphs)
					// rst:
					// rst:			:param sources: the list of source vertices the resulting hyperedge must have.
					// rst:			:type sources: list[DG.Vertex]
					// rst:			:param targets: the list of targets vertices the resulting hyperedge must have.
					// rst:			:type targets: list[DG.Vertex]
					// rst:			:param sourceGraphs: the list of graphs that must be associated with the source vertices the resulting hyperedge must have.
					// rst:			:type sourceGraphs: list[Graph]
					// rst:			:param targetGraphs: the list of graphs that must be associated with the targets vertices the resulting hyperedge must have.
					// rst:			:type targetGraphs: list[Graph]
					// rst:			:returns: a hyperedge with the given sources and targets.
					// rst:			  If no such hyperedge exists in the derivation graph then a null edge is returned.
					// rst:			  In the second version, the graphs are put through :meth:`findVertex` first.
					// rst:			:rtype: DG.HyperEdge
					// rst:			:raises: :class:`LogicError` if a vertex descriptor is null, or does not belong to the derivation graph.
					// rst:			:raises: :class:`LogicError` if neither ``hasActiveBuilder`` nor ``isLocked``.
			.def("findEdge", findEdgeVertices)
			.def("findEdge", findEdgeGraphs)
					//------------------------------------------------------------------
					// rst:		.. method:: build(onNewVertex=None, onNewHyperEdge=None)
					// rst:
					// rst:			:param onNewVertex: a callback invoked when a new vertex is added to the underlying derivation graph.
					// rst: 		:type onNewVertex: Optional[Callable[[DG.Vertex], None]]
					// rst:			:param onNewHyperEdge: a callback invoked when a new hyperedge is added to the underlying derivation graph.
					// rst: 		:type onNewVertex: Optional[Callable[[DG.HyperEdge], None]]
					// rst:			:returns: an RAII-style object which can be used to construct the derivation graph.
					// rst:				It can be used as a context manager in a ``with``-statement (see the documentation of :class:`DG.Builder`).
					// rst:				On destruction of an active builder object the associated DG object becomes locked for further modifications.
					// rst:			:rtype: DG.Builder
					// rst:			:raises: :class:`LogicError` if the DG already has an active builder (see :attr:`hasActiveBuilder`).
					// rst:			:raises: :class:`LogicError` if the DG is locked (see :attr:`locked`).
			.def("build", &DG_build)
					// rst:		.. attribute:: graphDatabase
					// rst:
					// rst:			All graphs known to the derivation graph.
					// rst:
					// rst:			:type: list[Graph]
			.add_property("_graphDatabase",
			              py::make_function(&DG::getGraphDatabase, py::return_value_policy<py::copy_const_reference>()))
					// rst:		.. attribute:: createdGraphs
					// rst:
					// rst:			The graphs that have been constructed by builder functions.
					// rst:
					// rst:			:type: list[Graph]
			.add_property("_createdGraphs",
			              py::make_function(&DG::getCreatedGraphs, py::return_value_policy<py::copy_const_reference>()))
					// rst:		.. method:: print(printer=DGPrinter(), data=None)
					// rst:
					// rst:			Print the derivation graph in style of a hypergraph. The appearance and structure
					// rst:			of the visualisation can optionally be configured by giving a DG printer and/or data object.
					// rst:
					// rst:			:param DGPrinter printer: the printer to use governing the appearance.
					// rst:			:param DGPrintData data: the extra data to use encoding the structure of the graph.
					// rst:			:returns: the name of the PDF-file that will be compiled in post-processing and the name of the coordinate tex-file used.
					// rst:			:rtype: tuple[str, str]
			.def("print", &DG::print)
					// rst:		.. method:: printNonHyper()
					// rst:
					// rst:			Print the derivation graph in style of a digraph, where each edge represents a hyperedge.
					// rst:			Each vertex in the depiction then represents a multiset of vertices in the hypergraph.
					// rst:
					// rst:			:returns: the name of the PDF-file that will be compiled in post-processing.
					// rst:			:rtype: str
			.def("printNonHyper", &DG::printNonHyper)
					// rst:		.. method:: dump()
					// rst:		            dump(filename)
					// rst:
					// rst:			Exports the derivation graph to a file, including associated graphs and rules.
					// rst:			Use :meth:`load` or :meth:`DG.Builder.load` to import the derivation graph again.
					// rst:
					// rst:			:param str filename: the name of the file to save the dump to.
					// rst:				If non is given an auto-generated name in the ``out/`` folder is used.
					// rst:				If an empty string is given, it is treated as if non is given.
					// rst:
					// rst:				.. note:: The filename is being used literally, i.e., it is not being
					// rst:					prefixed according to the current script location as input filenames are.
					// rst:			:returns: the filename of the exported derivation graph.
					// rst:			:rtype: str
					// rst:			:raises: :class:`LogicError` if the DG is not :attr:`locked`.
					// rst:			:raises: :class:`LogicError` if the target file can not be opened.
			.def("dump", static_cast<std::string (DG::*)() const>(&DG::dump))
			.def("dump", static_cast<std::string (DG::*)(const std::string &) const>(&DG::dump))
					// rst:		.. method:: listStats()
					// rst:
					// rst:			Lists various statistics for the derivation graph.
					// rst:
					// rst:			:raises: :class:`LogicError` if the DG has not been calculated.
			.def("listStats", &DG::listStats)
					// rst:		.. staticmethod:: load(graphDatabase, ruleDatabase, f, graphPolicy=IsomorphismPolicy.Check, verbosity=2)
					// rst:
					// rst:			Load a derivation graph dump as a locked object.
					// rst:			Use :func:`DG.Builder.load` to load a dump into a derivation graph under construction.
					// rst:
					// rst:			This is done roughly by making a :class:`DG` with the given `graphDatabase` and `graphPolicy`.
					// rst:			The label settings are retrieved from the dump file.
					// rst:			Vertices with graphs and hyperedges with rules are then added from the dump.
					// rst:			Any graph in the dump which is isomorphic to a given graph is replaced by the given graph.
					// rst:			The same procedure is done for the rules.
					// rst:			If a graph/rule is not found in the given lists, a new object is instantiated and used.
					// rst:			In the end the derivation graph is locked.
					// rst:
					// rst:			.. note:: If the dump to be loaded was made by version 0.10 or earlier, it does not contain the full rules
					// rst:				but only the rule name. It is then crucial that the names of the given rules match with those used to
					// rst:				create the dump in the first place.
					// rst:
					// rst:			See :cpp:func:`dg::DG::load` for an explanation of the verbosity levels.
					// rst:
					// rst:			:param graphDatabase: A list of graphs that will be given as graph database to :func:`__init__`.
					// rst:			:type graphDatabase: list[Graph]
					// rst:			:param ruleDatabase: A list of rules used as explained above.
					// rst:			:type ruleDatabase: list[Rule]
					// rst:			:param f: a DG dump file to load.
					// rst:			:type f: str or CWDPath
					// rst:			:param IsomorphismPolicy graphPolicy: the policy that will be given as graph policy to :func:`__init__`.
					// rst:			:returns: the loaded derivation graph.
					// rst:			:rtype: DG
					// rst:			:raises: the same exceptions :func:`__init__` raises related to ``graphDatabase`` and ``graphPolicy``.
					// rst:			:raises: :class:`LogicError` if there is a ``None`` in ``ruleDatabase``.
					// rst:			:raises: :class:`InputError` if the file can not be opened or its content is bad.
			.def("load", static_cast<Load>(&DG::load))
			.staticmethod("load");

	// rst: .. method:: diffDGs(dg1, dg2)
	// rst:
	// rst:		Compare two derivation graphs and lists the difference.
	// rst:		This is not a general isomorphism check; two vertices are equal if they have
	// rst:		the same graph attached. Edges are equal if the head and tail sets are equal
	// rst:		and if the attached rule is the same.
	// rst:
	// rst:		:param dg1: the first derivation graph.
	// rst:		:type dg1: :class:`DG`
	// rst:		:param dg2: the second derivation graph.
	// rst:		:type dg2: :class:`DG`
	py::def("diffDGs", &DG::diff);
}

} // namespace mod::dg::Py
