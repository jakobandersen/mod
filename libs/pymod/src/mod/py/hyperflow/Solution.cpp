#include <mod/py/Common.hpp>

#include <mod/dg/Printer.hpp>
#include <mod/hyperflow/Model.hpp>
#include <mod/hyperflow/LinExp.hpp>
#include <mod/hyperflow/Printer.hpp>
#include <mod/hyperflow/Solution.hpp>

namespace mod::hyperflow::Py {
namespace {

py::object Solution_objectiveValue(const Solution &s) {
	return std::visit([](const auto v) { return py::object(v); }, s.getObjectiveValue());
}

py::object Solution_eval(const Solution &s, const LinExp &exp) {
	return std::visit([](const auto v) { return py::object(v); }, s.eval(exp));
}

py::object Solution_print(const Solution &sol, const Printer &printer, const dg::PrintData &data) {
	auto res = sol.print(printer, data);
	auto first = res.first ? py::object(std::move(*res.first)) : py::object();
	auto second = res.second ? py::object(std::move(*res.second)) : py::object();
	return py::make_tuple(first, second);
}

} // namespace

void Solution_doExport() {
	// rst: .. class:: hyperflow.SolutionRange
	// rst:
	// rst:		A range of solutions for a flow model.
	// rst:
	py::class_<SolutionRange>("SolutionRange", py::no_init)
			// rst:		.. attribute:: model
			// rst:
			// rst:			(Read-only) The hyperflow model that produced these solutions.
			// rst:
			// rst:			:type: Model
			.add_property("model", &SolutionRange::getModel)
			.def(str(py::self))
					// rst:		.. attribute:: size
					// rst:
					// rst:			The number of solutions in the flow model.
					// rst:
					// rst:			:type: int
			.add_property("size", &SolutionRange::size)
					// rst:		.. method:: __len__()
					// rst:
					// rst:			:returns: the number of solutions in the flow model.
					// rst:
					// rst:			:rtype: int
			.def("__len__", &SolutionRange::size)
					// rst:		.. method:: __iter__()
					// rst:
					// rst:			:returns: an iterator for the contained solutions.
			.def("__iter__", py::iterator<SolutionRange>())
					// rst:		.. method:: __getitem__(i)
					// rst:
					// rst:			:param int i: the index into the range.
					// rst:			:returns: the ``i`` th solution in the range.
					// rst:			:rtype: Solution
			.def("__getitem__", &SolutionRange::operator[])
					// rst:		.. method:: list()
					// rst:
					// rst:			List overall information about each solution.
			.def("list", &SolutionRange::list)
					// rst:		.. method:: print(printer=Printer(), data=None)
					// rst:
					// rst:			Print solutions using the settings in the given printer and the structure information in the data object.
					// rst:
					// rst:			:param Printer printer: the printer to use governing the appearance.
					// rst:			:param DGPrintData data: the extra data to use encoding the structure of the graph.
			.def("print", static_cast<void (SolutionRange::*)(const Printer &,
			                                                  const dg::PrintData &) const>(&SolutionRange::print));

	// rst: .. class:: hyperflow.Solution
	// rst:
	// rst:		A solution for a flow model.
	// rst:
	py::class_<Solution>("Solution", py::no_init)
			// rst:		.. attribute:: model
			// rst:
			// rst:			(Read-only) The hyperflow model that produced the solution.
			// rst:
			// rst:			:type: Model
			.add_property("model", &Solution::getModel)
					// rst:		.. attribute:: id
					// rst:
					// rst:			(Read-only) An ID for the solution which is unique among
					// rst:			all solutions from the parent :class:`~hyperflow.Model` object.
					// rst:
					// rst:			:type: int
			.add_property("id", &Solution::getId)
			.def(str(py::self))
			.def(py::self == py::self)
			.def(py::self != py::self)
			.def(py::self < py::self)
					// rst:		.. attribute:: objectiveValue
					// rst:
					// rst:			(Read-only) The objective value of this solution.
					// rst:
					// rst:			:type: int or float
			.add_property("objectiveValue", &Solution_objectiveValue)
					// rst:		.. method:: eval(exp)
					// rst:
					// rst:			:param LinExp exp: the linear expression to be evaluated on the solution.
					// rst:			:returns: the value of the given linear expression evaluated on the solution.
					// rst:			:rtype: int or float
			.def("eval", &Solution_eval)
					// rst:		.. method:: list()
					// rst:
					// rst:			List overall information about the solution.
			.def("list", &Solution::list)
					// rst:		.. method:: print(printer=Printer(), data=None)
					// rst:
					// rst:			Print the solution using the settings in the given printer and the structure information in the data object.
					// rst:
					// rst:			:param Printer printer: the printer to use governing the appearance.
					// rst:			:param DGPrintData data: the extra data to use encoding the structure of the graph.
					// rst:			:returns: the name of the PDF-files that will be compiled in post-processing
					// rst:				for respectively the filtered and unfiltered depictions.
					// rst:				If either is not requested by the given ``printer`` the corresponding entry is ``None``.
					// rst:			:rtype: tuple[str, str]
			.def("print", &Solution_print);
}

} // namespace mod::hyperflow::Py