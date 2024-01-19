#include <mod/py/Common.hpp>

#include <mod/VertexMap.hpp>
#include <mod/graph/Graph.hpp>
#include <mod/graph/Automorphism.hpp>
#include <mod/graph/GraphInterface.hpp>
#include <mod/graph/Printer.hpp>

#include <mod/py/VertexMap.hpp>

#include <iomanip>

namespace mod::graph::Py {

void Graph_doExport() {
	std::pair<std::string, std::string>(Graph::*
	printWithoutOptions)() const = &Graph::print;
	std::pair<std::string, std::string>(Graph::*
	printWithOptions)(const graph::Printer&, const graph::Printer&) const = &Graph::print;

	// rst: .. class:: Graph
	// rst:
	// rst:		This class models an undirected graph with labels on vertices and edges,
	// rst:		without loops and without parallel edges.
	// rst:		See :ref:`graph-model` for more details.
	// rst:
	// rst:		The class implements the :class:`protocols.LabelledGraph`.
	// rst:		See :ref:`py-Graph/GraphInterface` for additional guarantees.
	// rst:
	py::class_<Graph, std::shared_ptr<Graph>, boost::noncopyable>("Graph", py::no_init)
			// rst:		.. attribute:: id
			// rst:
			// rst:			(Read-only) A unique instance id among :class:`Graph` objects.
			// rst:	
			// rst:			:type: int
			.add_property("id", &Graph::getId)
			.def(str(py::self))
					//------------------------------------------------------------------
					// LabelledGraph
			.add_property("numVertices", &Graph::numVertices)
			.add_property("vertices", &Graph::vertices)
			.add_property("numEdges", &Graph::numEdges)
			.add_property("edges", &Graph::edges)
					//------------------------------------------------------------------
					// rst:		.. method:: aut(labelSettings=LabelSettings(LabelType.String, LabelRelation.Isomorphism))
					// rst:
					// rst:			:param LabelSettings labelSettings: the label settings to use.
					// rst:			:returns: an object representing the automorphism group of the graph, with the given label settings.
					// rst:			:rtype: AutGroup
			.def("aut", &Graph::aut)
					//------------------------------------------------------------------
					// rst:		.. method:: print()
					// rst:		               print(first, second=None)
					// rst:
					// rst:			Print the graph, using either the default options or the options in ``first`` and ``second``.
					// rst:			If ``first`` and ``second`` are the same, only one depiction will be made.
					// rst:
					// rst:			:param GraphPrinter first: the printing options used for the first depiction.
					// rst:			:param GraphPrinter second: the printing options used for the second depiction.
					// rst:				If it is ``None`` then it is set to ``first``.
					// rst:			:returns: the names for the PDF-files that will be compiled in post-processing.
					// rst:				If ``first`` and ``second`` are the same, the two file prefixes are equal.
					// rst:			:rtype: tuple[str, str]
			.def("print", printWithoutOptions)
			.def("print", printWithOptions)
					// rst:		.. method:: printTermState
					// rst:
					// rst:			Print the term state for the graph.
			.def("printTermState", &Graph::printTermState)
					// rst:		.. method:: getGMLString(withCoords=False)
					// rst:
					// rst:			:returns: the :ref:`GML <graph-gml>` representation of the graph,
					// rst:			          optionally with generated 2D coordinates.
					// rst:			:rtype: str
					// rst:			:raises: :class:`LogicError` when coordinates are requested, but
					// rst:			         none can be generated.
			.def("getGMLString", &Graph::getGMLString)
					// rst:		.. method:: printGML(withCoords=False)
					// rst:
					// rst:			Print the :ref:`GML <graph-gml>` representation of the graph,
					// rst:			optionally with generated 2D coordinates.
					// rst:
					// rst:			:returns: the filename of the printed GML file.
					// rst:			:rtype: str
					// rst:			:raises: :class:`LogicError` when coordinates are requested, but
					// rst:			         none can be generated.
			.def("printGML", &Graph::printGML)
					// rst:		.. attribute:: name
					// rst:
					// rst:			The name of the graph. The default name includes the unique instance id.
					// rst:
					// rst:			:type: str
			.add_property("name", py::make_function(&Graph::getName, py::return_value_policy<py::copy_const_reference>()),
			              &Graph::setName)
					// rst:		.. attribute:: smiles
					// rst:
					// rst:			(Read-only) If the graph models a molecule, this is the canonical :ref:`SMILES string <graph-smiles>` for it.
					// rst:
					// rst:			:type: str
					// rst:			:raises: :class:`LogicError` if the graph is not a molecule.
			.add_property("smiles",
			              py::make_function(&Graph::getSmiles, py::return_value_policy<py::copy_const_reference>()))
					// rst:		.. attribute:: smilesWithIds
					// rst:
					// rst:			(Read-only) If the graph models a molecule, this is the canonical :ref:`SMILES string <graph-smiles>` for it,
					// rst:			that includes the internal vertex id as a class label on each atom.
					// rst:
					// rst:			:type: str
					// rst:			:raises: :class:`LogicError` if the graph is not a molecule.
			.add_property("smilesWithIds",
			              py::make_function(&Graph::getSmilesWithIds, py::return_value_policy<py::copy_const_reference>()))
					// rst:		.. attribute:: graphDFS
					// rst:
					// rst:			(Read-only) This is a :ref:`GraphDFS <format-graphDFS>` of the graph.
					// rst:
					// rst:			:type: str
			.add_property("graphDFS",
			              py::make_function(&Graph::getGraphDFS, py::return_value_policy<py::copy_const_reference>()))
					// rst:		.. attribute:: graphDFSWithIds
					// rst:
					// rst:			(Read-only) This is a :ref:`GraphDFS <format-graphDFS>` of the graph, where each vertex have an explicit id,
					// rst:			corresponding to its internal vertex id.
					// rst:
					// rst:			:type: str
			.add_property("graphDFSWithIds", py::make_function(&Graph::getGraphDFSWithIds,
			                                                   py::return_value_policy<py::copy_const_reference>()))
					// rst:		.. attribute:: linearEncoding
					// rst:
					// rst:			(Read-only) If the graph models a molecule this is the :ref:`SMILES string <graph-smiles>` string, otherwise it is the :ref:`GraphDFS <format-graphDFS>` string.
					// rst:
					// rst:			:type: str
			.add_property("linearEncoding", py::make_function(&Graph::getLinearEncoding,
			                                                  py::return_value_policy<py::copy_const_reference>()))
					// rst:		.. attribute:: isMolecule
					// rst:
					// rst:			(Read-only) Whether the graph models a molecule. See :ref:`mol-enc`.
					// rst:
					// rst:			:type: bool
			.add_property("isMolecule", &Graph::getIsMolecule)
					// rst:		.. attribute:: energy
					// rst:
					// rst:			(Read-only) If the graph models a molecule, this is some energy value.
					// rst:			The energy is calculated using Open Babel, unless already calculated or cached by :meth:`Graph.cacheEnergy`.
					// rst:
					// rst:			:type: float
			.add_property("energy", &Graph::getEnergy)
					// rst:		.. method:: cacheEnergy(e)
					// rst:
					// rst:			If the graph models a molecule, sets the energy to a given value.
					// rst:
					// rst:			:param float e: the value for the energy to be set.
			.def("cacheEnergy", &Graph::cacheEnergy)
					// rst:		.. attribute:: exactMass
					// rst:
					// rst:			(Read-only) The exact mass of the graph, if it is a molecule.
					// rst:			It is the sum of the exact mass of each atom, with the mass of electrons subtracted corresponding to the integer charge.
					// rst:			That is, the mass is :math:`\sum_a (mass(a) - mass(e)\cdot charge(a))`.
					// rst:			If an atom has no specified isotope, then the most abundant is used.
					// rst:
					// rst:			:type: float
					// rst:			:raises: :class:`LogicError` if it is not a molecule, including if some isotope has not been tabulated.
			.add_property("exactMass", &Graph::getExactMass)
					// rst:		.. method:: vLabelCount(label)
					// rst:
					// rst:			:param str label: some label for querying.
					// rst:			:returns: the number of vertices in the graph with the given label.
					// rst:			:rtype: int
			.def("vLabelCount", &Graph::vLabelCount)
					// rst:		.. method:: eLabelCount(label)
					// rst:
					// rst:			:param str label: some label for querying.
					// rst:			:returns: the number of edges in the graph with the given label.
					// rst:			:rtype: int
			.def("eLabelCount", &Graph::eLabelCount)
					// rst:		.. method:: isomorphism(codomain, maxNumMatches=1, labelSettings=LabelSettings(LabelType.String, LabelRelation.Isomorphism))
					// rst:		            monomorphism(codomain, maxNumMatches=1, labelSettings=LabelSettings(LabelType.String, LabelRelation.Isomorphism))
					// rst:
					// rst:			:param Graph codomain: the codomain graph for finding morphisms.
					// rst:			:param int maxNumMatches: the maximum number of isomorphisms/monomorphisms to search for.
					// rst:			:param LabelSettings labelSettings: the label settings to use during the search.
					// rst:			:returns: the number of isomorphisms/monomorphisms from this graph to ``other``, but at most ``maxNumMatches``.
					// rst:			:rtype: int
					// rst:			:raises LogicError: if ``codomain`` is null.
			.def("isomorphism", &Graph::isomorphism)
			.def("monomorphism", &Graph::monomorphism)
					// rst:		.. method:: enumerateIsomorphisms(codomain, callback,  labelSettings=LabelSettings(LabelType.String, LabelRelation.Isomorphism))
					// rst:		            enumerateMonomorphisms(codomain, callback,  labelSettings=LabelSettings(LabelType.String, LabelRelation.Isomorphism))
					// rst:
					// rst:			Perform (sub)structure search of this graph into the given codomain graph.
					// rst:			Whenever a match is found, the corresponding isomorphism/monomorphism is copied into a vertex map
					// rst:			and the given callback is invoked with it.
					// rst:
					// rst:			:param Graph codomain: the codomain graph for finding morphisms.
					// rst:			:param callback: the function to call with each found isomorphism/monomorphism.
					// rst:				If ``False`` is returned from it, then the search is stopped.
					// rst:			:type callback: Callable[[protocols.VertexMap], bool]
					// rst:			:param LabelSettings labelSettings: the label settings to use during the search.
					// rst:
					// rst:			:raises LogicError: if ``codomain`` is null.
					// rst:			:raises LogicError: if ``callback`` is null.
			.def("enumerateIsomorphisms", &Graph::enumerateIsomorphisms)
			.def("enumerateMonomorphisms", &Graph::enumerateMonomorphisms)
					// rst:		.. method:: makePermutation()
					// rst:
					// rst:			:returns: a graph isomorphic to this, but with the vertex indices randomly permuted.
					// rst:			:rtype: Graph
					// rst:
					// rst:			.. note: Currently, permuting stereo-information is not supported, and the resulting
					// rst:				graphs thus have any stereo-information stripped.
			.def("makePermutation", &Graph::makePermutation)
					// rst:		.. attribute:: image
					// rst:
					// rst:			(Write-only) A custom depiction for the graph. The depiction file used will be the string
					// rst:			returned by the given function, with ``.pdf`` appended.
					// rst:			The function will only be called once.
					// rst:
					// rst:			:type: Callable[[], str]
			.add_property("image", &mod::Py::noGet, &Graph::setImage)
					// rst:		.. attribute:: imageCommand
					// rst:
					// rst:			A command to be run in post-processing if a custom depiction is set.
					// rst:			The command is only run once.
					// rst:
					// rst:			:type: str
			.add_property("imageCommand", &Graph::getImageCommand, &Graph::setImageCommand)
					// rst:		.. method:: instantiateStereo()
					// rst:
					// rst: 			Make sure that stereo data is instantiated.
					// rst:
					// rst: 			:raises: :class:`StereoDeductionError` if the data was not instantiated and deduction failed.
			.def("instantiateStereo", &Graph::instantiateStereo)
					// ===================================================================
					// External data
					// ===================================================================
					// rst:		.. method:: getVertexFromExternalId(id)
					// rst:
					// rst:			If the graph was not loaded from an external data format, then this function
					// rst:			always return a null descriptor.
					// rst:			If the graph was loaded from a SMILES string, but *any* class label was not unique,
					// rst:			then the function always return a null descriptor.
					// rst:
					// rst:			.. note:: In general there is no correlation between external and internal ids.
					// rst:
					// rst:			:param int id: the external id to find the vertex descriptor for.
					// rst:			:returns: the vertex descriptor for the given external id.
					// rst:		            The descriptor is null if the external id was not used.
					// rst:			:rtype: Vertex
			.def("getVertexFromExternalId", &Graph::getVertexFromExternalId)
					// rst:		.. attribute:: minExternalId
					// rst:		               maxExternalId
					// rst:
					// rst:			(Read-only)
					// rst:			If the graph was not loaded from an external data format, then these attributes
					// rst:			are always return 0. Otherwise, they are the minimum/maximum external id from which
					// rst:			non-null vertices can be obtained from :meth:`getVertexFromExternalId`.
					// rst:			If no such minimum and maximum exists, then they are 0.
					// rst:
					// rst:			:type: int
			.add_property("minExternalId", &Graph::getMinExternalId)
			.add_property("maxExternalId", &Graph::getMaxExternalId)
					// rst:		.. attribute:: loadingWarnings
					// rst:
					// rst:			(Read-only)
					// rst:			The list of warnings stored when the graph was created from an external format.
					// rst:			Each entry is a message and then an indicator of whether
					// rst:			the warning was printed before construction (``True``), or was a silenced warning (``False``).
					// rst:
					// rst:			:raises: :class:`LogicError` if the graph does not have data from external loading
					// rst:			:type: List[Tuple[str, bool]]
			.add_property("loadingWarnings", &Graph::getLoadingWarnings)
					// rst:
					// rst: Loading Functions
					// rst: =================
					// rst:
					// rst:	.. staticmethod:: Graph.fromGMLString(s, name=None, add=True, printStereoWarnings=True)
					// rst:                    Graph.fromGMLFile(f, name=None, add=True, printStereoWarnings=True)
					// rst:
					// rst:		Load a graph in :ref:`GML <graph-gml>` format from a given string, ``s``,
					// rst:		or given file ``f``.
					// rst:		The graph must be connected.
					// rst:		If not, use :meth:`fromGMLStringMulti` or :meth:`fromGMLFileMulti`.
					// rst:
					// rst:		:param str s: the string with the :ref:`GML <graph-gml>` data to load from.
					// rst:		:param f: name of the :ref:`GML <graph-gml>` file to be loaded.
					// rst:		:type f: str or CWDPath
					// rst:		:param str name: the name of the graph. If none is given the default name is used.
					// rst:		:param bool add: whether to append the graph to :data:`inputGraphs` or not.
					// rst:		:param bool printStereoWarnings: whether to print warnings due to unhandled stereo information.
					// rst:		:returns: the loaded graph.
					// rst:		:rtype: Graph
					// rst:		:raises: :class:`InputError` on bad input.
			.def("fromGMLString", &Graph::fromGMLString)
			.staticmethod("fromGMLString")
			.def("fromGMLFile", &Graph::fromGMLFile)
			.staticmethod("fromGMLFile")
					// rst:	.. staticmethod:: Graph.fromGMLStringMulti(s, add=True, printStereoWarnings=True)
					// rst:                    Graph.fromGMLFileMulti(f, add=True, printStereoWarnings=True)
					// rst:
					// rst:		Load a set of graphs in :ref:`GML <graph-gml>` format from a given string, ``s``,
					// rst:		or given file ``f``,
					// rst:		with each graph being a connected component of the graph specified in the GML data.
					// rst:
					// rst:		See :meth:`fromGMLString` and :meth:`fromGMLFile`
					// rst:		for a description of the parameters and exceptions.
					// rst:
					// rst:		:returns: a list of the loaded graphs.
					// rst:		:rtype: list[Graph]
			.def("fromGMLStringMulti", &Graph::fromGMLStringMulti)
			.staticmethod("fromGMLStringMulti")
			.def("fromGMLFileMulti", &Graph::fromGMLFileMulti)
			.staticmethod("fromGMLFileMulti")
					// rst: .. staticmethod:: Graph.fromDFS(s, name=None, add=True)
					// rst:
					// rst:		Load a graph from a :ref:`GraphDFS <format-graphDFS>` string.
					// rst:		The graph must be connected.
					// rst:		If not, use :meth:`Graph.fromDFSMulti`.
					// rst:
					// rst:		:param str s: the :ref:`GraphDFS <format-graphDFS>` string to parse.
					// rst:		:param str name: the name of the graph. If none is given the default name is used.
					// rst:		:param bool add: whether to append the graph to :data:`inputGraphs` or not.
					// rst:		:returns: the loaded graph.
					// rst:		:rtype: Graph
					// rst:		:raises: :class:`InputError` on bad input.
			.def("fromDFS", &Graph::fromDFS)
			.staticmethod("fromDFS")
					// rst: .. staticmethod:: Graph.fromDFSMulti(s, add=True)
					// rst:
					// rst:		Load a set of graphs from a :ref:`GraphDFS <format-graphDFS>` string,
					// rst:		with each graph being a connected component of the graph specified in the DFS data.
					// rst:
					// rst:		:param str s: the :ref:`GraphDFS <format-graphDFS>` string to parse.
					// rst:		:param bool add: whether to append the graphs to :data:`inputGraphs` or not.
					// rst:		:returns: the loaded graphs.
					// rst:		:rtype: list[Graph]
					// rst:		:raises: :class:`InputError` on bad input.
			.def("fromDFSMulti", &Graph::fromDFSMulti)
			.staticmethod("fromDFSMulti")
					// rst: .. staticmethod:: Graph.fromSMILES(s, name=None, allowAbstract=False, classPolicy=SmilesClassPolicy.NoneOnDuplicate, add=True, printStereoWarnings=True)
					// rst:
					// rst:		Load a molecule from a :ref:`SMILES <graph-smiles>` string.
					// rst:		The molecule must be a connected graph. If not, use :meth:`fromSMILESMulti`.
					// rst:
					// rst:		:param str s: the :ref:`SMILES <graph-smiles>` string to parse.
					// rst:		:param str name: the name of the graph. If none is given the default name is used.
					// rst:		:param bool allowAbstract: whether to allow abstract vertex labels in bracketed atoms.
					// rst:		:param bool add: whether to append the graph to :data:`inputGraphs` or not.
					// rst:		:param bool printStereoWarnings: whether to print warnings due to unhandled stereo information.
					// rst:		:returns: the loaded molecule.
					// rst:		:rtype: Graph
					// rst:		:raises: :class:`InputError` on bad input.
			.def("fromSMILES",
			     static_cast<std::shared_ptr<Graph> (*)(
					     const std::string &, bool, SmilesClassPolicy, bool)>(&Graph::fromSMILES))
			.staticmethod("fromSMILES")
					// rst: .. staticmethod:: Graph.fromSMILESMulti(s, allowAbstract=False, classPolicy=SmilesClassPolicy.NoneOnDuplicate, add=True, printStereoWarnings=True)
					// rst:
					// rst:		Load a set of molecules from a :ref:`SMILES <graph-smiles>` string,
					// rst:		with each molecule being a connected component of the graph specified in the SMILES string.
					// rst:
					// rst:		See :meth:`fromSMILES` for a description of the parameters and exceptions.
					// rst:
					// rst:		:returns: a list of the loaded molecules.
					// rst:		:rtype: list[Graph]
			.def("fromSMILESMulti", static_cast<std::vector<std::shared_ptr<Graph>>(*)(
					const std::string &, bool, SmilesClassPolicy, bool)>(&Graph::fromSMILESMulti))
			.staticmethod("fromSMILESMulti")
					// rst: .. staticmethod:: Graph.fromMOLString(s, name=None, options=MDLOptions(), add=True)
					// rst:                   Graph.fromMOLFile(f, name=None, options=MDLOptions(), add=True)
					// rst:
					// rst:		Load a molecule in :ref:`MOL <graph-mdl>` format from a given string or file.
					// rst:		The molecule must be a connected graph.
					// rst:		If not, use :meth:`fromMOLStringMulti` and :meth:`fromMOLFileMulti`.
					// rst:
					// rst:		:param str s: the string to parse.
					// rst:		:param f: name of the file to load.
					// rst:		:type f: str or CWDPath
					// rst:		:param str name: the name of the graph. If none is given the default name is used.
					// rst:		:param MDLOptions options: the options to use for loading.
					// rst:		:param bool add: whether to append the graph to :data:`inputGraphs` or not.
					// rst:		:returns: the loaded molecule.
					// rst:		:rtype: :class:`Graph`
					// rst:		:raises: :class:`InputError` on bad input.
			.def("fromMOLString", &Graph::fromMOLString)
			.staticmethod("fromMOLString")
			.def("fromMOLFile", &Graph::fromMOLFile)
			.staticmethod("fromMOLFile")
					// rst: .. staticmethod:: Graph.fromMOLStringMulti(s, options=MDLOptions(), add=True)
					// rst:                   Graph.fromMOLFileMulti(f, options=MDLOptions(), add=True)
					// rst:
					// rst:		Load a set of molecules from a given string or file with :ref:`MOL <graph-mdl>` data,
					// rst:		with each molecule being a connected component of the graph specified in the data.
					// rst:
					// rst:		See :meth:`fromMOLString` and :meth:`fromMOLFile`
					// rst:		for a description of the parameters and exceptions.
					// rst:
					// rst:		:returns: a list of the loaded molecules.
					// rst:		:rtype: list[Graph]
			.def("fromMOLStringMulti", &Graph::fromMOLStringMulti)
			.staticmethod("fromMOLStringMulti")
			.def("fromMOLFileMulti", &Graph::fromMOLFileMulti)
			.staticmethod("fromMOLFileMulti")
					// rst: .. staticmethod:: Graph.fromSDString(s, options=MDLOptions(), add=True)
					// rst:                   Graph.fromSDFile(f, options=MDLOptions(), add=True)
					// rst:
					// rst:		Load a list of molecules in :ref:`SD <graph-mdl>` format from a given string or file,
					// rst:		with each molecule being a connected component of each of the graphs specified in the data.
					// rst:		If any graph is not connected, use :meth:`fromSDStringMulti` and :meth:`fromSDFileMulti` instead.
					// rst:
					// rst:		:param str s: the string to parse.
					// rst:		:param f: name of the file to load.
					// rst:		:type f: str or CWDPath
					// rst:		:param MDLOptions options: the options to use for loading.
					// rst:		:param bool add: whether to append the graphs to :data:`inputGraphs` or not.
					// rst:		:returns: a list of the loaded molecules.
					// rst:		:rtype: list of :class:`Graph`
					// rst:		:raises: :class:`InputError` on bad input.
			.def("fromSDString", &Graph::fromSDString)
			.staticmethod("fromSDString")
			.def("fromSDFile", &Graph::fromSDFile)
			.staticmethod("fromSDFile")
					// rst: .. staticmethod:: Graph.fromSDStringMulti(s, options=MDLOptions(), add=True)
					// rst:                   Graph.fromSDFileMulti(f, options=MDLOptions(), add=True)
					// rst:
					// rst:		Load a list of molecules in :ref:`SD <graph-mdl>` format from a given string or file.
					// rst:		Each molecule is returned as a list of graphs, with each corresponding to a connected
					// rst:		component of the MOL entry.
					// rst:
					// rst:		See :meth:`fromSDString` and :meth:`fromSDFile`
					// rst:		for a description of the parameters and exceptions.
					// rst:
					// rst:		:returns: a list of lists of the loaded molecules.
					// rst:			The items of the outer list correspond to each MOL entry in the SD data.
					// rst:		:rtype: list[list[Graph]]
			.def("fromSDStringMulti", &Graph::fromSDStringMulti)
			.staticmethod("fromSDStringMulti")
			.def("fromSDFileMulti", &Graph::fromSDFileMulti)
			.staticmethod("fromSDFileMulti");

	mod::Py::exportVertexMap<VertexMap<graph::Graph, graph::Graph>>("VertexMapGraphGraph");


	// rst: .. method:: graphGMLString(...)
	// rst:
	// rst:		Alias of :py:meth:`Graph.fromGMLString`.
	// rst: .. method:: graphGML(...)
	// rst:
	// rst:		Alias of :py:meth:`Graph.fromGMLFile`.
	// rst: .. method:: graphDFS(...)
	// rst:
	// rst:		Alias of :py:meth:`Graph.fromDFS`.
	// rst: .. method:: smiles(...)
	// rst:
	// rst:		Alias of :py:meth:`Graph.fromSMILES`.
	// rst:
	// rst:
	// rst: .. data:: inputGraphs
	// rst:
	// rst:		A list of graphs to which explicitly loaded graphs as default are appended.
	// rst:
	// rst:		:type: list[Graph]
	// rst:
}

} // namespace mod::graph::Py
