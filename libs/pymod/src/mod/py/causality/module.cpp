#include <mod/py/Common.hpp>

#include <iostream>

namespace mod::causality::Py {
void EventTrace_doExport();
void EventTracePrinter_doExport();
void Petri_doExport();
void Stochsim_doExport();

void module_doExport() {
	// https://stackoverflow.com/questions/29006439/boost-python-multiple-modules-in-one-shared-object
	py::scope CausalityModule = py::object(py::handle<>(py::borrowed(PyImport_AddModule("mod.causality"))));
	EventTrace_doExport();
	EventTracePrinter_doExport();
	Petri_doExport();
	Stochsim_doExport();
}

} // namespace mod::causality::Py
