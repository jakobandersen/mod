Compiling and Linking Client Code
=================================

The build system installs not only header files and the actual library,
but also import files that other CMake-based projects can use to link against libMØD.
The source code listed below can be found `here <../_static/examples/libmod_cmake/>`__.

For example, we could make a simple program from the following source:

.. literalinclude:: /_static/examples/libmod_cmake/main.cpp
	:language: c++
	:tab-width: 3

The program can then be build as a CMake project with the following:

.. literalinclude:: /_static/examples/libmod_cmake/CMakeLists.txt
	:language: cmake
	:tab-width: 3

