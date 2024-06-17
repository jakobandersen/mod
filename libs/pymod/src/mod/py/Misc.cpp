#include <mod/py/Common.hpp>

#include <mod/Config.hpp>
#include <mod/Misc.hpp>

#include <mod/py/Function.hpp>

#include <cstdint>

namespace mod::Py {

// rst:
// rst: .. function:: prefixFilename(name)
// rst:
// rst: 	Utility function for converting script-relative paths to current-working-directory-relative paths
// rst: 	(see also :func:`include`).
// rst: 	This function is used in all PyMØD functions that takes a filename as argument.
// rst: 	To circumvent this prefixing use the :class:`CWDPath` class.
// rst:
// rst: 	:returns: `name` prefixed with all strings pushed with :func:`pushFilePrefix`
// rst: 		and popped yet.
// rst: 	:rtype: str
// rst:
// rst: .. function:: pushFilePrefix(s)
// rst:
// rst: 	Push another prefix used in :func:`prefixFilename`.
// rst: 	The prefixes are concatenated in the order they are pushed.
// rst: 	No directory delimiters are inserted, so they must explicitly be part of the pushed prefixes.
// rst: 	If the argument starts with ``/``, then during concatenation all previously pushed prefixes are ignored.
// rst:
// rst: 	:param str s: the string to push as a prefix to be concatenated in :func:`prefixFilename`.
// rst:
// rst: .. function:: popFilePrefix()
// rst:
// rst: 	Pop a previously pushed prefix.
// rst:
// rst: .. class:: CWDPath
// rst:
// rst:		A dummy class to wrap a filename in to disable prefixing.
// rst:
// rst:		For example, if a graph GML file is to be loaded relative to the current file
// rst:		(assuming :func:`include` is used for script inclusion), then it can be done with
// rst:
// rst:		.. code-block:: python
// rst:
// rst:			g = graphGML(f)
// rst:
// rst:		If the file is located relative to the current working directory
// rst:		(i.e., where the :any:`mod` wrapper script were invoked from), then the graph loading should be done as
// rst:
// rst:		.. code-block:: python
// rst:
// rst:			g = graphGML(CWDPath(f))
// rst:
// rst:		.. method:: __init__(f)
// rst:
// rst:			Wrap a filename.
// rst:

namespace {

std::uintptr_t magicLibraryValue() {
	return reinterpret_cast<std::uintptr_t> (&mod::getConfig());
}

} // namespace

void Misc_doExport() {
	// rst: .. function:: magicLibraryValue()
	// rst:
	// rst:		:returns: a number unique for each instantiation of libMØD.
	// rst:			This can be used to check for the diamond problem for shared libraries.
	// rst:		:rtype: int
	py::def("magicLibraryValue", &magicLibraryValue);

	// rst: .. function:: version()
	// rst:
	// rst:		:returns: the version of MØD.
	// rst:		:rtype: str
	py::def("version", &mod::version);

	// rst: .. function:: rngReseed(seed)
	// rst:
	// rst: 		Reseed the random bit generator used in the library.
	py::def("rngReseed", &mod::rngReseed);
	// rst: .. function:: rngUniformReal()
	// rst:
	// rst: 		:returns: a uniformly random real number between 0 and 1.
	// rst: 	 	:rtype: float
	py::def("rngUniformReal", &mod::rngUniformReal);

	py::def("strFromDump", &mod::strFromDump);
	py::def("showDump", &mod::showDump);

	py::def("printGeometryGraph", &mod::printGeometryGraph);
}

} // namespace mod::Py