# Based on https://cmake.org/pipermail/cmake/2011-January/041666.html

function(find_python_module module)
	string(TOUPPER ${module} module_upper)
	if(NOT PY_${module_upper})
		if(ARGC GREATER 1 AND ARGV1 STREQUAL "REQUIRED")
			set(${module}_FIND_REQUIRED TRUE)
		endif()
		# A module's location is usually a directory, but for binary modules
		# it's a .so file.
		set(code "import re, ${module}; print(re.compile('/__init__.py.*').sub('',${module}.__file__))")
		execute_process(COMMAND "${Python3_EXECUTABLE}" "-c" "${code}"
				RESULT_VARIABLE _${module}_status
				OUTPUT_VARIABLE _${module}_stdout
				ERROR_VARIABLE _${module}_stderr
				ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
		if("${_${module}_status}" STREQUAL "0")
			set(PY_${module_upper} "${_${module}_stdout}" CACHE STRING
					"Location of Python module ${module}")
		endif()
	endif()
	find_package_handle_standard_args(${module} REQUIRED_VARS PY_${module_upper}
			REASON_FAILURE_MESSAGE "Got the following output:\n${_${module}_stdout}\n${_${module}_stderr}"
			FAIL_MESSAGE "Could not import Python module '${module}' with found interpreter ${Python3_EXECUTABLE}")
endfunction()