#include <mod/py/Common.hpp>

#include <mod/Function.hpp>
#include <mod/dg/Printer.hpp>
#include <mod/hyperflow/Printer.hpp>

namespace mod::hyperflow::Py {
namespace {

void Printer_pushInEdgeLabel(Printer &printer, std::shared_ptr<mod::Function<std::string(dg::DG::Vertex)>> f) {
	printer.pushInEdgeLabel(mod::toStdFunction(f));
}

void Printer_pushOutEdgeLabel(Printer &printer, std::shared_ptr<mod::Function<std::string(dg::DG::Vertex)>> f) {
	printer.pushOutEdgeLabel(mod::toStdFunction(f));
}

} // namespace 

void Printer_doExport() {
	dg::Printer &(Printer::*Printer_getDGPrinter)() = &Printer::getDGPrinter;
	// rst: .. class:: hyperflow.Printer
	// rst:
	// rst:		An object of this class holds information on how derivation graphs with flow are visualised.
	// rst:
	py::class_<Printer, boost::noncopyable>("Printer")
			// rst:		.. attribute:: dgPrinter
			// rst:
			// rst:			(Read-only) Retrieve the :class:`DGPrinter` used when printing solutions.
			// rst:
			// rst:			:type: DGPrinter
			.add_property("dgPrinter", py::make_function(Printer_getDGPrinter, py::return_internal_reference<1>()))
					// rst:		.. attribute:: withFlowLabels
					// rst:
					// rst:			Control whether a flow label is appended to each hyperedge label (see also :meth:`DGPrinter.pushEdgeLabel`).
					// rst:
					// rst:			:type: bool
			.add_property("withFlowLabels", &Printer::getWithFlowLabels, &Printer::setWithFlowLabels)
					// rst:		.. method:: pushInEdgeLabel(f)
					// rst:
					// rst:			Add another function for in-edge labelling.
					// rst:			The result of this function is appended to each label using the edge label separator
					// rst:			of the underlying :class:`DGPrinter`.
					// rst:
					// rst:			:param f: the function to push for labelling in-edges.
					// rst:			:type f: Callable[DG.Vertex], str] or str
			.def("pushInEdgeLabel", &Printer_pushInEdgeLabel)
					// rst:		.. method:: popInEdgeLabel()
					// rst:
					// rst:			Remove the last pushed in-edge labelling function.
					// rst:
					// rst:			:raises: :class:`LogicError` if no callback is left to pop.
			.def("popInEdgeLabel", &Printer::popInEdgeLabel)
					// rst:		.. method:: pushOutEdgeLabel(f)
					// rst:
					// rst:			Add another function for out-edge labelling.
					// rst:			The result of this function is appended to each label using the edge label separator
					// rst:			of the underlying :class:`DGPrinter`.
					// rst:
					// rst:			:param f: the function to push for labelling out-edges.
					// rst:			:type f: Callable[DG.Vertex], str] or str
			.def("pushOutEdgeLabel", &Printer_pushOutEdgeLabel)
					// rst:		.. method:: popOutEdgeLabel()
					// rst:
					// rst:			Remove the last pushed out-edge labelling function.
					// rst:
					// rst:			:raises: :class:`LogicError` if no callback is left to pop.
			.def("popOutEdgeLabel", &Printer::popOutEdgeLabel)
					// rst:		.. attribute:: printUnfiltered
					// rst:
					// rst:			Control whether the unfiltered version of a solution is printed.
					// rst:
					// rst:			:type: bool
			.add_property("printUnfiltered", &Printer::getPrintUnfiltered, &Printer::setPrintUnfiltered)
					// rst:		.. attribute:: printFiltered
					// rst:
					// rst:			Control whether the filtered version of a solution is printed.
					// rst:
					// rst:			:type: bool
			.add_property("printFiltered", &Printer::getPrintFiltered, &Printer::setPrintFiltered)
					// rst:		.. attribute:: unfilteredFlowColour
					// rst:
					// rst:			When printing the unfiltered version, use this colour on vertices and edges with non-zero flow.
					// rst:			Use an empty string to not colour them.
					// rst:
					// rst:			:type: str
			.add_property("unfilteredFlowColour", &Printer::getUnfilteredFlowColour, &Printer::setUnfilteredFlowColour);
}

} // namespace mod::hyperflow::Py