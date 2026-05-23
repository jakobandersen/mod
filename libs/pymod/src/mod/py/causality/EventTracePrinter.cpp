#include <mod/py/Common.hpp>

#include <mod/Function.hpp>
#include <mod/causality/EventTracePrinter.hpp>
#include <mod/dg/GraphInterface.hpp>

namespace mod::causality::Py {
namespace {

void Printer_pushOptions(EventTracePrinter &printer, std::shared_ptr<mod::Function<std::string(std::shared_ptr<dg::DG>)>> f) {
	printer.pushOptions(mod::toStdFunction(f));
}

void Printer_pushVertexVisible(EventTracePrinter &printer, std::shared_ptr<mod::Function<bool(dg::DG::Vertex)>> f) {
	printer.pushVertexVisible(mod::toStdFunction(f));
}

void Printer_pushVertexOptions(EventTracePrinter &printer, std::shared_ptr<mod::Function<std::string(dg::DG::Vertex)>> f) {
	printer.pushVertexOptions(mod::toStdFunction(f));
}

void Printer_setPreContent(EventTracePrinter &printer, std::shared_ptr<mod::Function<std::string(std::shared_ptr<dg::DG>)>> f) {
	printer.setPreContent(mod::toStdFunction(f));
}

void Printer_setPostContent(EventTracePrinter &printer, std::shared_ptr<mod::Function<std::string(std::shared_ptr<dg::DG>)>> f) {
	printer.setPostContent(mod::toStdFunction(f));
}

 } // namespace

void EventTracePrinter_doExport() {
	// rst: .. class:: causality.EventTracePrinter
	// rst:
	// rst:		This class is used to configure how event traces are visualised,
	// rst:		e.g., which vertices/graphs should be shown, colours, and axis configuration.
	// rst:
	// rst:		See :cpp:class:`causality::EventTracePrinter` for details on how the plot is rendered,
	// rst:		and how the rendering is influenced by the printer.
	// rst:
	// rst:		.. versionadded:: 1.1
	// rst:
	py::class_<EventTracePrinter>("EventTracePrinter")
			// rst:		.. attribute:: maxPointsPerVertex
			// rst:
			// rst:			Control how many points are rendered for each different vertex represented in the trace.
			// rst:			Data is then visualised at this amount of regular intervals on the time axis.
			// rst:			It defaults to 500.
			// rst:
			// rst:			:type: int
			.add_property("maxPointsPerVertex", &EventTracePrinter::getMaxPointsPerVertex,
			              &EventTracePrinter::setMaxPointsPerVertex)
					// rst:		.. attribute:: logTime
					// rst:
					// rst:			Control if the time axis should be logarithmic or not.
					// rst:			It defaults to ``false``.
					// rst:
					// rst:			:type: bool
			.add_property("logTime", &EventTracePrinter::getLogTime, &EventTracePrinter::setLogTime)
					// rst:		.. attribute:: logCount
					// rst:
					// rst:			Control if the count axis should be logarithmic or not.
					// rst:			It defaults to ``false``.
					// rst:
					// rst:			:type: bool
			.add_property("logCount", &EventTracePrinter::getLogCount, &EventTracePrinter::setLogCount)
					// rst:		.. method:: pushOptions(f)
					// rst:
					// rst:			Add another function or constant that adds options for the axis environment.
					// rst:			All options function results are applied.
					// rst:
					// rst:			:param f: the function or constant to push for specifying options.
					// rst:			:type f: Callable[[DG], str] or str
			.def("pushOptions", &Printer_pushOptions)
					// rst:		.. method:: popOptions()
					// rst:
					// rst:			Remove the last pushed options function.
					// rst:
					// rst:			:raises: :class:`LogicError` if no callback is left to pop.
			.def("popOptions", &EventTracePrinter::popOptions)
					// rst:		.. method:: pushVertexVisible(f)
					// rst:
					// rst:			Add another function or constant controlling the visibility of plots of vertices.
					// rst:			All visibility functions must return ``true`` for a plot of a vertex to be visible.
					// rst:
					// rst:			:param f: the function or constant to push for specifying vertex visibility.
					// rst:			:type f: Callable[[DG.Vertex], bool] or bool
			.def("pushVertexVisible", &Printer_pushVertexVisible)
					// rst:		.. method:: popVertexVisible()
					// rst:
					// rst:			Remove the last pushed vertex visibility function.
					// rst:
					// rst:			:raises: :class:`LogicError` if no callback is left to pop.
			.def("popVertexVisible", &EventTracePrinter::popVertexVisible)
					// rst:		.. method:: pushVertexOptions(f)
					// rst:
					// rst:			Add another function or constant setting options for the plot of each vertex.
					// rst:			All options function results are applied.
					// rst:
					// rst:			:param f: the function or constant to push for specifying vertex options.
					// rst:			:type f: Callable[[DG.Vertex], str] or str
			.def("pushVertexOptions", &Printer_pushVertexOptions)
					// rst:		.. method:: popVertexOptions()
					// rst:
					// rst:			Remove the last pushed vertex options function.
					// rst:
					// rst:			:raises: :class:`LogicError` if no callback is left to pop.
			.def("popVertexOptions", &EventTracePrinter::popVertexOptions)
					// rst:		.. method:: setPreContent(f)
					// rst:		            setPostContent(f)
					// rst:
					// rst:			Set a callback for adding additional content to the plot,
					// rst:			either before or after the primary data has been added.
					// rst:
					// rst:			:param f: the function or constant to set for adding content.
					// rst:			:type f: Callable[[DG.Vertex], str] or str
			.def("setPreContent", &Printer_setPreContent)
			.def("setPostContent", &Printer_setPostContent);

}

} // namespace mod::causality::Py