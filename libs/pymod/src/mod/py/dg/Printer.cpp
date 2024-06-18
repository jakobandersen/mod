#include <mod/py/Common.hpp>

#include <mod/Function.hpp>
#include <mod/dg/DG.hpp>
#include <mod/dg/GraphInterface.hpp>
#include <mod/dg/Printer.hpp>
#include <mod/graph/Printer.hpp>

namespace mod::dg::Py {
namespace {

void Printer_pushVertexVisible(Printer &printer, std::shared_ptr<mod::Function<bool(DG::Vertex)>> f) {
	printer.pushVertexVisible(mod::toStdFunction(f));
}

void Printer_pushEdgeVisible(Printer &printer, std::shared_ptr<mod::Function<bool(DG::HyperEdge)>> f) {
	printer.pushEdgeVisible(mod::toStdFunction(f));
}

void Printer_pushVertexLabel(Printer &printer, std::shared_ptr<mod::Function<std::string(DG::Vertex)>> f) {
	printer.pushVertexLabel(mod::toStdFunction(f));
}

void Printer_pushEdgeLabel(Printer &printer, std::shared_ptr<mod::Function<std::string(DG::HyperEdge)>> f) {
	printer.pushEdgeLabel(mod::toStdFunction(f));
}

void Printer_pushVertexColour(Printer &printer, std::shared_ptr<mod::Function<std::string(DG::Vertex)> > f,
                              bool extendToEdges) {
	printer.pushVertexColour(mod::toStdFunction(f), extendToEdges);
}

void Printer_pushEdgeColour(Printer &printer, std::shared_ptr<mod::Function<std::string(DG::HyperEdge)> > f) {
	printer.pushEdgeColour(mod::toStdFunction(f));
}

void
Printer_setRotationOverwrite(Printer &printer, std::shared_ptr<mod::Function<int(std::shared_ptr<graph::Graph>)> > f) {
	printer.setRotationOverwrite(mod::toStdFunction(f));
}

void
Printer_setMirrorOverwrite(Printer &printer, std::shared_ptr<mod::Function<bool(std::shared_ptr<graph::Graph>)> > f) {
	printer.setMirrorOverwrite(mod::toStdFunction(f));
}

void Printer_setImageOverride(Printer &printer,
                              std::shared_ptr<mod::Function<std::pair<std::string, std::string>(
		                              DG::Vertex, int)>> f) {
	printer.setImageOverwrite(mod::toStdFunction(f));
}

} // namespace

void Printer_doExport() {
	// rst: .. class:: DGPrintData
	// rst:
	// rst:		This class is used to hold extra data about how a specific derivation graph
	// rst:		is visualised.
	// rst:
	// rst:		The idea is that in the visualized network each vertex/hyperedge is specified
	// rst:		by a pair :math:`(id, dup)` where :math:`id` is the ID of the vertex/hyperedge
	// rst:		and :math:`dup` is an versioning integer that can be specified in objects of this class.
	// rst:
	// rst:		Initially, each vertex/hyperedge has only one version: duplicate number 0.
	// rst:		The duplication is primarily specified via the hyperedges, with the duplication
	// rst:		of vertices being induced afterwards.
	// rst:		Thus hyperedge duplicates are managed by :func:`makeDuplicate` and :func:`removeDuplicate`,
	// rst:		while the vertex duplicates are managed implicitly by :func:`reconnectSource` and :func:`reconnectTarget`.
	// rst:		In the end, when the data is used for printing, it will be compiled to form the actual duplication data.
	// rst:
	py::class_<PrintData>("DGPrintData", py::no_init)
			// rst:		.. method:: __init__(dg)
			// rst:
			// rst:			Construct a data object where all derivations have a single version, duplicate number 0,
			// rst:			connected to version 0 of all heads and tails.
			// rst:
			// rst:			:param DG dg: the derivation graph to hold data for.
			// rst:			:raises: :class:`LogicError` if ``not dg.isLocked``.
			.def(py::init<std::shared_ptr<DG> >())
					// rst:		.. attribute:: dg
					// rst:
					// rst:			(Read-only) The derivation graph the object holds data for.
			.add_property("dg", &PrintData::getDG)
					// rst:		.. method:: makeDuplicate(e, eDup)
					// rst:
					// rst:			Create another version of the given hyperedge and give it the given duplicate number.
					// rst:			It will connect to duplicate 0 of all head and tail vertices.
					// rst:
					// rst:			:param DG.HyperEdge e: a reference to the derivation to duplicate.
					// rst:			:param int eDup: the duplicate number for the new version of the derivation.
					// rst:			:raises: :class:`LogicError` if ``not e``.
					// rst:			:raises: :class:`LogicError` if ``e.dg != dg``.
					// rst:			:raises: :class:`LogicError` if duplicate ``eDup`` already exists for ``e``.
			.def("makeDuplicate", &PrintData::makeDuplicate)
					// rst:		.. method:: removeDuplicate(e, eDup)
					// rst:
					// rst:			Remove the version of the given hyperedge with the given duplicate number.
					// rst:
					// rst:			:param DG.HyperEdge e: a reference to the derivation to duplicate.
					// rst:			:param int eDup: the duplicate number for the version of the derivation to remove.
					// rst:			:raises: :class:`LogicError` if ``not e``.
					// rst:			:raises: :class:`LogicError` if ``e.dg != dg``.
					// rst:			:raises: :class:`LogicError` if duplicate ``eDup`` does not exist for ``e``.
			.def("removeDuplicate", &PrintData::removeDuplicate)
					// rst:		.. method:: reconnectSource(e, eDup, v, vDupTar)
					// rst:
					// rst:			For the given hyperedge duplicate, reconnect the given source vertex to the given duplicate of that source.
					// rst:			If the vertex is a source multiple times, then an arbitrary one of them is reconnected.
					// rst:
					// rst:			:param DG.HyperEdge e: a reference to the derivation to reconnect.
					// rst:			:param int eDup: the duplicate number of the derivation to reconnect.
					// rst:			:param DG.Vertex v: a source vertex to reconnect.
					// rst:			:param int vDupTar: the new duplicate number for the source vertex.
					// rst:			:raises: :class:`LogicError` if ``not e``.
					// rst:			:raises: :class:`LogicError` if ``e.dg != dg``.
					// rst:			:raises: :class:`LogicError` if ``not v``.
					// rst:			:raises: :class:`LogicError` if ``v.dg != dg``.
					// rst:			:raises: :class:`LogicError` if ``v`` is not a source vertex of ``e``.
					// rst:			:raises: :class:`LogicError` if duplicate ``eDup`` does not exist for ``e``.
			.def("reconnectSource", &PrintData::reconnectSource)
					// rst:		.. method:: reconnectTarget(e, eDup, v, vDupTar)
					// rst:
					// rst:			For the given hyperedge duplicate, reconnect the given head to the given duplicate of that head.
					// rst:			If the vertex is a head multiple times, then an arbitrary one of them is reconnected.
					// rst:
					// rst:			:param DG.HyperEdge e: a reference to the derivation to reconnect.
					// rst:			:param int eDup: the duplicate number of the derivation to reconnect.
					// rst:			:param DG.Vertex v: a target vertex to reconnect.
					// rst:			:param int vDupTar: the new duplicate number for the target vertex.
					// rst:			:raises: :class:`LogicError` if ``not e``.
					// rst:			:raises: :class:`LogicError` if ``e.dg != dg``.
					// rst:			:raises: :class:`LogicError` if ``not v``.
					// rst:			:raises: :class:`LogicError` if ``v.dg != dg``.
					// rst:			:raises: :class:`LogicError` if ``v`` is not a target vertex of ``e``.
					// rst:			:raises: :class:`LogicError` if duplicate ``eDup`` does not exist for ``e``.
			.def("reconnectTarget", &PrintData::reconnectTarget);

	graph::Printer &(Printer::*Printer_getGraphPrinter)() = &Printer::getGraphPrinter;
	void (*Printer_setGraphPrinter)(Printer &, const graph::Printer &) = [](Printer &p, const graph::Printer &gp) {
		p.getGraphPrinter() = gp;
	};
	// rst: .. class:: DGPrinter
	// rst:
	// rst:		This class is used to configure how derivation graphs are visualised,
	// rst:		how much is visualised and which extra properties are printed.
	// rst:
	py::class_<Printer, boost::noncopyable>("DGPrinter")
			// rst:		.. attribute:: graphPrinter
			// rst:
			// rst:			Access the :class:`GraphPrinter` used when printing images of graphs.
			// rst:			Note that assignment to the property copies the argument.
			// rst:
			// rst:			The initial printer has :meth:`GraphPrinter.enableAll` called,
			// rst:			and then indices are removed (:attr:`GraphPrinter.withIndex` set to ``False``).
			// rst:
			// rst:			:type: GraphPrinter
			.add_property("graphPrinter",
			              py::make_function(Printer_getGraphPrinter, py::return_internal_reference<1>()),
			              Printer_setGraphPrinter)
					// rst:		.. attribute:: withShortcutEdges
					// rst:
					// rst:			Control whether hyperedges :math:`(T, H)` with :math:`|T| = |H| = 1`
					// rst:			are printed as a simple arrow without the usual rectangle.
					// rst:
					// rst:			:type: bool
			.add_property("withShortcutEdges", &Printer::getWithShortcutEdges, &Printer::setWithShortcutEdges)
					// rst:		.. attribute:: withGraphImages
					// rst:
					// rst:			Control whether each vertex is printed with a image of its graph in it.
					// rst:
					// rst:			:type: bool
			.add_property("withGraphImages", &Printer::getWithGraphImages, &Printer::setWithGraphImages)
					// rst:		.. attribute:: labelsAsLatexMath
					// rst:
					// rst:			Control whether labels on vertices and hyperedges are put inline :math:`\LaTeX` math.
					// rst:			Specifically, a label ``label`` is written as ``$\mathrm{label'}$``, with ``label'`` being
					// rst:			``label`` with all space characters escaped.
					// rst:
					// rst:			:type: bool
			.add_property("labelsAsLatexMath", &Printer::getLabelsAsLatexMath, &Printer::setLabelsAsLatexMath)
					// rst:		.. method:: pushVertexVisible(f)
					// rst:
					// rst:			Add another function or constant controlling the visibility of vertices.
					// rst:			All visibility functions must return ``true`` for a vertex to be visible.
					// rst:
					// rst:			:param f: the function or constant to push for specifying vertex visibility.
					// rst:			:type f: Callable[[DG.Vertex], bool] or bool
			.def("pushVertexVisible", &Printer_pushVertexVisible)
					// rst:		.. method:: popVertexVisible()
					// rst:
					// rst:			Remove the last pushed vertex visibility function.
					// rst:
					// rst:			:raises: :class:`LogicError` if no callback is left to pop.
			.def("popVertexVisible", &Printer::popVertexVisible)
					// rst:		.. method:: pushEdgeVisible(f)
					// rst:
					// rst:			Add another function or constant controlling the visibility of hyperedges.
					// rst:			All visibility functions must return ``true`` for a hyperedge to be visible.
					// rst:
					// rst:			:param f: the function or constant to push for specifying hyperedge visibility.
					// rst:			:type f: Callable[[DG.HyperEdge], bool] or bool
			.def("pushEdgeVisible", &Printer_pushEdgeVisible)
					// rst:		.. method:: popEdgeVisible()
					// rst:
					// rst:			Remove the last pushed hyperedge visibility function.
					// rst:
					// rst:			:raises: :class:`LogicError` if no callback is left to pop.
			.def("popEdgeVisible", &Printer::popEdgeVisible)
					// rst:		.. attribute:: withShortcutEdgesAfterVisibility
					// rst:
					// rst:			Control whether hyperedges are depicted as simple arrows when all but 1 head and tail are hidden.
					// rst:
					// rst:			:type: bool
			.add_property("withShortcutEdgesAfterVisibility", &Printer::getWithShortcutEdgesAfterVisibility,
			              &Printer::setWithShortcutEdgesAfterVisibility)
					// rst:		.. attribute:: vertexLabelSep
					// rst:
					// rst:			The string used as separator between each part of each vertex label.
					// rst:
					// rst:			:type: str
			.add_property("vertexLabelSep",
			              py::make_function(&Printer::getVertexLabelSep,
			                                py::return_value_policy<py::copy_const_reference>()),
			              &Printer::setVertexLabelSep)
					// rst:		.. attribute:: edgeLabelSep
					// rst:
					// rst:			The string used as separator between each part of each edge label.
					// rst:
					// rst:			:type: str
			.add_property("edgeLabelSep",
			              py::make_function(&Printer::getEdgeLabelSep,
			                                py::return_value_policy<py::copy_const_reference>()),
			              &Printer::setEdgeLabelSep)
					// rst:		.. method:: pushVertexLabel(f)
					// rst:
					// rst:			Add another function or constant for vertex labelling.
					// rst:			The result of this function is added to the end of each label.
					// rst:
					// rst:			:param f: the function or constant to push for labelling vertices.
					// rst:			:type f: Callable[[DG.Vertex], str] or str
			.def("pushVertexLabel", &Printer_pushVertexLabel)
					// rst:		.. method:: popVertexLabel()
					// rst:
					// rst:			Remove the last pushed vertex labelling function.
					// rst:
					// rst:			:raises: :class:`LogicError` if no callback is left to pop.
			.def("popVertexLabel", &Printer::popVertexLabel)
					// rst:		.. method:: pushEdgeLabel(f)
					// rst:
					// rst:			Add another function or constant for edge labelling. The result of this function is appended to each label.
					// rst:
					// rst:			:param f: the function or constant to push for labelling edges.
					// rst:			:type f: Callable[[DG.HyperEdge], str] or str
			.def("pushEdgeLabel", &Printer_pushEdgeLabel)
					// rst:		.. method:: popEdgeLabel()
					// rst:
					// rst:			Remove the last pushed edge labelling function.
					// rst:
					// rst:			:raises: :class:`LogicError` if no callback is left to pop.
			.def("popEdgeLabel", &Printer::popEdgeLabel)
					// rst:		.. attribute:: withGraphName
					// rst:
					// rst:			Control whether graph names are appended to the vertex labels (see also :meth:`DGPrinter.pushVertexLabel`).
					// rst:
					// rst:			:type: bool
			.add_property("withGraphName", &Printer::getWithGraphName, &Printer::setWithGraphName)
					// rst:		.. attribute:: withRuleName
					// rst:
					// rst:			Control whether rule names are appended to the edge labels (see also :meth:`DGPrinter.pushEdgeLabel`).
					// rst:
					// rst:			:type: bool
			.add_property("withRuleName", &Printer::getWithRuleName, &Printer::setWithRuleName)
					// rst:		.. attribute:: withRuleId
					// rst:
					// rst:			Control whether rule IDs are appended to the edge labels (see also :meth:`DGPrinter.pushEdgeLabel`).
					// rst:
					// rst:			:type: bool
			.add_property("withRuleId", &Printer::getWithRuleId, &Printer::setWithRuleId)
					// rst:		.. attribute:: withInlineGraphs
					// rst:
					// rst:			Control whether graph depictions should be precompiled, or be included inline in the DG figure.
					// rst:
					// rst:			:type: bool
			.add_property("withInlineGraphs", &Printer::getWithInlineGraphs, &Printer::setWithInlineGraphs)
					// rst:		.. method:: pushVertexColour(f, extendToEdges=True)
					// rst:
					// rst:			Add another function or constant for colouring vertices. The final colour of a vertex is the result of the first colour function returning a non-empty string.
					// rst:			The functions are evaluated in the order they are pushed and the resulting string is used directly as a colour in Tikz.
					// rst:			A hyperedge is also coloured if at least one head and one tail *can* be coloured with a colour for which ``extendToEdges`` is ``True``.
					// rst:			In this case, the hyperedge (and a subset of the head and tail connectors) is coloured with the first applicable colour.
					// rst:			The edge extension of vertex colour takes lower precedence than explicitly added hyperedge colouring functions.
					// rst:
					// rst:			:param f: the function to push for colouring vertices.
					// rst:			:type f: Callable[[DG.Vertex], str] or str
					// rst:			:param bool extendToEdges: whether some hyperedges are coloured as well (see above).
			.def("pushVertexColour", &Printer_pushVertexColour)
					// rst:		.. method:: popVertexColour()
					// rst:
					// rst:			Remove the last pushed vertex colouring function.
					// rst:
					// rst:			:raises: :class:`LogicError` if no callback is left to pop.
			.def("popVertexColour", &Printer::popVertexColour)
					// rst:		.. method:: pushEdgeColour(f)
					// rst:
					// rst:			Add another function or constant for colouring hyperedges. The final colour of a hyperedge (and all of its head and tail connectors) is the result of the
					// rst:			first colour function returning a non-empty string.
					// rst:
					// rst:			:param f: the function or constant to push for colouring hyperedges.
					// rst:			:type f: Callable[[DG.HyperEdge], str] or str
			.def("pushEdgeColour", &Printer_pushEdgeColour)
					// rst:		.. method:: popEdgeColour()
					// rst:
					// rst:			Remove the last pushed hyperedge colouring function.
					// rst:
					// rst:			:raises: :class:`LogicError` if no callback is left to pop.
			.def("popEdgeColour", &Printer::popEdgeColour)
					// rst:		.. method:: setRotationOverwrite(f)
					// rst:
					// rst:			Overwrite the rotation set in the nested :class:`GraphPrinter`. The given function will be used
					// rst:			to set the rotation of each printed graph.
					// rst:
					// rst:			:param f: the constant or the function called on each graph to retrieve the rotation to render it with.
					// rst:			:type f: Callable[[Graph], int] or int
			.def("setRotationOverwrite", &Printer_setRotationOverwrite)
					// rst:		.. method:: setMirrorOverwrite(f)
					// rst:
					// rst:			Overwrite the mirror set in the nested :class:`GraphPrinter`. The given function will be used
					// rst:			to set the mirror of each printed graph.
					// rst:
					// rst:			:param f: the function called on each graph to retrieve the mirror to render it with.
					// rst:			:type f: Callable[[Graph], bool] or bool
			.def("setMirrorOverwrite", &Printer_setMirrorOverwrite)
					// rst:		.. method:: setImageOverwrite(f)
					// rst:
					// rst:			Overwrite the image generation for graphs depicted in the vertices of the printed derivation graph.
					// rst:			For each duplicate of each vertex to be depicted, the given callback is called.
					// rst:			It must then return two strings:
					// rst:
					// rst:			1. Either
					// rst:
					// rst:				- an empty string if the standard depiction should be used, in which case the second string is ignored, or
					// rst:				- the filename of the PDF that should be included in the final compiled figure.
					// rst:
					// rst:			2. Either
					// rst:
					// rst:				- an empty string if no additional post-processing command is needed, or
					// rst:				- a string of Bash code which will be inserted in the post-processing instructions.
					// rst:				  For example, one can write out source code in the callback, and then return a command that compiles
					// rst:				  that code into the PDF needed by inclusion.
					// rst:
					// rst:			The image overwrite can be removed by calling with ``None``.
					// rst:
					// rst:			:param f: the callback to use, or ``None`` to remove an existing callback.
					// rst:			:type f: Callable[[DG.Vertex, int], tuple[str, str]] or None
			.def("setImageOverwrite", &Printer_setImageOverride)
					// rst:		.. attribute:: graphvizPrefix
					// rst:
					// rst:			The string that will be inserted into generated DOT files,
					// rst:			just after the graph declaration.
					// rst:
					// rst:			:type: str
			.add_property("graphvizPrefix",
			              py::make_function(&Printer::getGraphvizPrefix,
			                                py::return_value_policy<py::copy_const_reference>()),
			              &Printer::setGraphvizPrefix)
					// rst:		.. attribute:: tikzpictureOption
					// rst:
					// rst:			Access the string that will be inserted into generated Tikz files,
					// rst:			in the options for the ``\tikzpicture`` macro used for the DG.
					// rst:
					// rst:			:type: str
			.add_property("tikzpictureOption",
			              py::make_function(&Printer::getTikzpictureOption,
			                                py::return_value_policy<py::copy_const_reference>()),
			              &Printer::setTikzpictureOption);
}

} // namespace mod::dg::Py
