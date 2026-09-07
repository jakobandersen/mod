#include <mod/py/Error.hpp>

#include <ostream>

// see http://stackoverflow.com/questions/2261858/boostpython-export-custom-exception
// and http://stackoverflow.com/questions/9620268/boost-python-custom-exception-class
// and http://stackoverflow.com/questions/11448735/boostpython-export-custom-exception-and-inherit-from-pythons-exception

namespace mod::Py {

MethodOverrideError::MethodOverrideError(std::string &&method, std::string &&baseClass, std::string &&error)
	: Exception("Error with '" + std::move(method) + "' in class derived from '" + std::move(baseClass) + "': " + std::move(error)) {}

namespace {

PyObject *exportException(const std::string &name) {
	std::string scopeName = py::extract<std::string>(py::scope().attr("__name__"));
	std::string qualifiedName = scopeName + "." + name;

	PyObject *exType = PyErr_NewException(qualifiedName.c_str(), PyExc_Exception, 0);
	if(!exType) py::throw_error_already_set();
	py::scope().attr(name.c_str()) = py::handle<>(py::borrowed(exType));
	return exType;
}

#define MOD_PY_ExportException(Name) {                                            \
	py::class_<Name>(#Name "_", py::no_init);                                     \
	PyObject *exType = exportException(#Name);                                    \
	py::register_exception_translator<Name>([exType](const Name &ex) {            \
		py::object exPy(ex); /* wrap the C++ exception */                         \
		py::object exTypePy(py::handle<>(py::borrowed(exType)));                  \
		/* add the wrapped exception to the Python exception */                   \
		exTypePy.attr("cause") = exPy;                                            \
		PyErr_SetString(exType, ex.what());                                       \
	});                                                                           \
}

} // namespace

void Error_doExport() {
	// rst: .. exception:: FatalError
	// rst:
	// rst:		See :cpp:class:`FatalError`.
	MOD_PY_ExportException(FatalError);
	// rst: .. exception:: InputError
	// rst:
	// rst:		See :cpp:class:`InputError`.
	MOD_PY_ExportException(InputError);
	// rst: .. exception:: LogicError
	// rst:
	// rst:		See :cpp:class:`LogicError`.
	MOD_PY_ExportException(LogicError);
	// rst: .. exception:: TermParsingError
	// rst:
	// rst:		See :cpp:class:`TermParsingError`.
	MOD_PY_ExportException(TermParsingError);
	// rst: .. exception:: StereoDeductionError
	// rst:
	// rst:		See :cpp:class:`StereoDeductionError`.
	MOD_PY_ExportException(StereoDeductionError);
	// rst: .. exception:: MethodOverrideError
	// rst:
	// rst:		When thrown there is at least basic exception safety.
	// rst:		This exception is thrown if a Python class derives from a C++ class,
	// rst:		but there is some error with an overriden method, including the method not being implemented.
	MOD_PY_ExportException(MethodOverrideError);
}

} // namespace mod::Py