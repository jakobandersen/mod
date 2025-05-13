#include <mod/py/Common.hpp>

#include <iostream>

namespace mod::hyperflow::Py {
void LinExp_doExport();
void Model_doExport();
void ModelImplementationView_doExport();
void Printer_doExport();
void Solution_doExport();

void module_doExport() {
	// https://stackoverflow.com/questions/29006439/boost-python-multiple-modules-in-one-shared-object
	py::scope HyperflowModule = py::object(py::handle<>(py::borrowed(PyImport_AddModule("mod.hyperflow"))));
	LinExp_doExport();
	Model_doExport();
	ModelImplementationView_doExport();
	Printer_doExport();
	Solution_doExport();
}

} // namespace mod::causality::Py