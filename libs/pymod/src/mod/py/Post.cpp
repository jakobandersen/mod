#include <mod/py/Common.hpp>

#include <mod/Post.hpp>

namespace mod::post::Py {

void Post_doExport() {
	// rst: .. function:: makeUniqueFilePrefix()
	// rst:
	// rst:		:returns: a string on the form ``out/iii_`` where ```iii``` is the next zero-padded integer
	// rst:			from an internal counter.
	// rst:		:rtype: str
	py::def("makeUniqueFilePrefix", &makeUniqueFilePrefix);


	// https://stackoverflow.com/questions/29006439/boost-python-multiple-modules-in-one-shared-object
	py::scope PostModule = py::object(py::handle<>(py::borrowed(PyImport_AddModule("mod.post"))));
	// rst: .. module:: post
	// rst:
	// rst:		The ``post`` submodule contains various functions to manipulate post-processing (:ref:`mod_post`).
	// rst:		Commands for the post-processor are written to the command file ``out/post.sh``
	// rst:		which the post-processor executes internally as a Bash script.
	// rst:
	// rst:		.. function:: command(line)
	// rst:
	// rst:			Write the given text to the command file and write a newline character.
	// rst:
	// rst:			:param str line: the text to be written.
	// rst:
	// rst:			.. warning:: The contents of the command file is executed without any security checks.
	py::def("command", &command);
	// rst:		.. function:: flushCommands()
	// rst:
	// rst:			Flush the command file buffer.
	py::def("flushCommands", &flushCommands);
	// rst:		.. function:: disableCommands()
	// rst:
	// rst:			Disable command writing and flushing, also for commands emitted internally in the library.
	py::def("disableCommands", &disableCommands);
	// rst:		.. function:: enableCommands()
	// rst:
	// rst:			Enable command writing and flushing, also for commands emitted internally in the library.
	py::def("enableCommands", &enableCommands);
	// rst:		.. function:: reopenCommandFile()
	// rst:
	// rst:			Reopen the command file, which may be useful if it was modified externally while open by the library.
	py::def("reopenCommandFile", &reopenCommandFile);
	// rst:		.. function:: summaryChapter(heading)
	// rst:
	// rst:			Command the post-processor to insert a ``\chapter`` macro in the summary.
	// rst:
	// rst:			:param str heading: the chapter heading to insert.
	py::def("summaryChapter", &summaryChapter);
	// rst:		.. function:: summarySection(heading)
	// rst:
	// rst:			Command the post-processor to insert a ``\section`` macro in the summary.
	// rst:
	// rst:			:param str heading: the section heading to insert.
	py::def("summarySection", &summarySection);
	// rst:		.. function:: summaryRaw(latexCode, file=...)
	// rst:
	// rst:			Command the post-processor to insert the given code verbatim in the summary.
	// rst:
	// rst:			:param str latexCode: the code to insert.
	// rst:			:param str file: if given then that will be appended to a unique prefix for the final filename the code is stored in.
	py::def("summaryRaw", static_cast<void (*)(const std::string &)>(&summaryRaw));
	py::def("summaryRaw", static_cast<void (*)(const std::string &, const std::string &)>(&summaryRaw));
	// rst:		.. function:: summaryInput(filename)
	// rst:
	// rst:			Command the post-processor to insert a ``\input`` macro in the summary.
	// rst:
	// rst:			:param str filename: the filename to input.
	py::def("summaryInput", &summaryInput);
	// rst:		.. function:: disableInvokeMake()
	// rst:		                  enableInvokeMake()
	// rst:
	// rst:			Disable/enable the invocation of Make in the post-processor.
	// rst:			The processing of commands and generation of Makefiles will still be carried out,
	// rst:			and Make invocation can be done manually afterwards through the post-processor
	py::def("disableInvokeMake", &disableInvokeMake);
	py::def("enableInvokeMake", &enableInvokeMake);
	// rst:		.. function:: disableCompileSummary()
	// rst:		                  enableCompileSummary()
	// rst:
	// rst:			Disable/enable the compilation of the final summary during post-processing.
	// rst:			The compilation can be invoked manually afterwards through the post-processor.
	py::def("disableCompileSummary", &disableCompileSummary);
	py::def("enableCompileSummary", &enableCompileSummary);
}

} // namespace mod::post::Py