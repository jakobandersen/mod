
.. _creatingPyMODExt:

Creating a Python Extension
===========================

PyMØD is primarily Python bindings for libMØD made using
`Boost.Python <http://www.boost.org/libs/python/>`_.
The following shows a template for creating a Python extension with C++
functions (e.g., to extend libMØD).
The complete source code for the example can be found `here <../_static/examples/pymod_extension/>`__.

Library Source and Headers
--------------------------

For this examples the following C++ source with header should be exposed
in the Python module. Header:

.. literalinclude:: ../_static/examples/pymod_extension/stuff.hpp
    :language: cpp

Source:

.. literalinclude:: ../_static/examples/pymod_extension/stuff.cpp
    :language: cpp

Exposing the Interface
----------------------

See the Boost.Python documentation for instructions how to expose C++ code.
Below is the code for creating our simple Python extension.

.. literalinclude:: ../_static/examples/pymod_extension/pyModule.cpp
    :language: cpp

The example exposes a bit of extra functionality for a sanity check.
Python will ``dlopen`` libMØD twice As both the PyMØD module and our extension
requires it. The library uses static variables and strange things might happen
if multiple instances of these exist. The MØD wrapper script changes the dlopen
flags (setting ``RTLD_GLOBAL`` and ``RTLD_NOW``) which should prevent multiple
instances of the library. The function ``magicLibraryValue`` can be used to check
that this is true (see the test script below).

Building With CMake
-------------------

We can use CMake to build the example:

.. literalinclude:: ../_static/examples/pymod_extension/CMakeLists.txt
    :language: cmake

Testing the Extension
---------------------

After configuring and building there should be a shared library ``awesome.so`` which contains
the extension. Executing the following script using the wrapper script in the same folder
as the shared libray should now work:

.. literalinclude:: ../_static/examples/pymod_extension/test.py
    :language: py

Command to execute:

.. code-block:: sh

    mod -f test.py
