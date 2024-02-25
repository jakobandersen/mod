.. default-domain:: py
.. py:currentmodule:: mod
.. cpp:namespace:: mod

.. _mod-wrapper:

****************************
The Wrapper Script
****************************

.. program:: mod

The script ``mod`` is a convenience script primarily for starting the Python
interpreter and after its exit execute PostMØD (the summary generator).
The execution can however be customised somewhat, e.g., by running the
interpreter through ``valgrind`` and/or ``gdb``/``lldb``, or switching the
interpreter into interactive mode.

The wrapper script does the following.

#. Clear the folder ``out/``. If it does not exist, if will be created.
   All printing functions in libMØD assumes ``out/`` is created.
#. Create an input Python script (``out/input``) with some
   :ref:`default code <mod-wrapper-preamble>`,
   and then code based on the command line parameters
   (see :option:`-f` and :option:`-e`).
#. Run the Python interpreter on the input script,
   optionally through debugging tools.
#. If the exit code of the executable is not 0, exit with that exit code.
#. If PostMØD is not selected to be run, exit with code 0.
#. Run PostMØD and exit with its exit code.


Usage
#####

.. code-block:: bash

  mod [options]

Meta Options
============

.. option:: --help, -h

  Shows the help output of the script.

.. option:: --version, -v

  Print the version of MØD and exit
  (the version is always printed in the start).

Input Options
=============

.. option:: -f <filename>

  Append ``include("filename")`` to the input script (see :py:func:`include`).
  This option can be repeated to execute multiple files in sequence.

.. option:: -e <code>

  Append the given code (and a line break) to the input script.
  This option can be repeated to execute multiple code snippets in sequence.

.. envvar:: MOD_NO_DEPRECATED

  When this environment variable is set to a non-empty string, then the string
  ``config.common.ignoreDeprecation = False`` is executed first.
  This can be used to force errors when scripts use deprecated functionality.


Execution Customization
=======================

.. option:: -i

  Use interactive mode for the Python interpreter (i.e., pass ``-i`` as
  additional argument to the interpreter), but disable the log. If ``ipython3``
  is available it will be used as interpreter, otherwise ``python3`` is used.
  See also :envvar:`MOD_PYTHON` and :envvar:`MOD_IPYTHON`.

.. option:: -q

  Use quiet mode. Certain messages are not printed.

.. envvar:: MOD_PYTHON

  Set this environment variable to the executable to start in non-interactive
  mode. If not set it will use a Python interpreter appropriate for the compiled
  bindings. This interpreter was found during configuration of the build system.
  When executing the wrapper script the exact command can be seen in the output.

.. envvar:: MOD_IPYTHON

  Set this environment variable to the executable to start in interactive
  mode (when :option:`-i` is given).
  If not set ``ipython3`` will be used, if it can be found, otherwise it will
  fall back to standard Python interpreter as if :option:`-i` was not given.

  .. warning:: The standard ``ipython3`` may not be appropriate for the compiled bindings,
    which may result in a crash during import of the ``mod`` module.

.. envvar:: MOD_DEBUGGER

  Set this environmentt variable to the executable to use with :option:`--debug`
  is given and :option:`--memcheck` is not given.
  When not set the string ``gdb --args`` or ``lldb --`` is used, if GDB or LLDB
  is available.


Post-processing Options
=======================

.. option:: --nopost

  Do not run PostMØD after the selected executable has finished.

.. option:: -j <N>

  Give ``-j <N>`` to :ref:`PostMØD <mod_post>` (if executed).
  
.. option:: clean

  Do not run anything, but remove the ``out`` and ``summary`` folders.

Debugging Options
=================

.. option:: --profile

  Run the interpreter through `Valgrind <http://valgrind.org/>`_
  with ``--tool=callgrind``.
  This option takes precedence over the ``memcheck`` option.

.. option:: --memcheck

  Run the interpreter through `Valgrind <http://valgrind.org/>`_ with
  standard options for memory check.
  If ``--debug`` is given as well, the options ``--vgdb=yes --vgdb-error=0``
  are also given to ``valgrind`` such that a debugger (e.g. GDB or LLDB) can be
  attached.

.. option:: --vgArgs <args>

  If either :option:`--profile` or :option:`--memcheck` is used, this inserts
  ``<args>`` as an extra argument for ``valgrind``.

.. option:: --debug

  Run the interpreter through `GDB <http://www.gnu.org/software/gdb/>`_,
  or `LLDB <https://lldb.llvm.org/>`_.
  If :envvar:`MOD_DEBUGGER` it set, then this string is prepended to form a complete command,
  otherwise if ``gdb`` is available, then ``gdb --args`` is preprended,
  otherwise if ``lldb``is available, then ``lldb --`` is prepended,
  otherwise the script stops with an error.

  If :option:`--memcheck` is given as well, then it takes preceedence, but it will add extra
  flags to start a gdbserver. This will make Valgrind wait for a debugger to be attached.
  Valgrind will print instructions for how to attach GDB.


.. _mod-wrapper-preamble:

PyMØD Preamble
##############

When the wrapper script creates the input script, it will start with a small
preamble of useful code before the user-defined code.
The complete code, including the preamble, given to the Python interpereter can
be seen in ``out/input`` after execution of the wrapper script.
The preamble has the following effects.

* Setting ``RTLD_GLOBAL`` in the interpreters ``dlopen`` flags (see
  :ref:`creatingPyMODExt`)
* ``import mod`` and ``from mod import *``
* Defining the following function both in global scope and in the ``mod``
  module.

  .. py:function:: include(fName, checkDup=True, putDup=True, skipDup=True) 
  
	Include the file ``fName`` in somewhat the same way that the ``#include``
	directive in the C/C++ preprocessor includes files. Paths are handled
	relatively using the functions :py:func:`prefixFilename`,
	:py:func:`pushFilePrefix`, and :py:func:`popFilePrefix`.
    
	:param str fName: the file to be included.
	:param bool checkDup: check if the file has already been included before
		(with ``putDup=True``)
	:param bool putDup: remember that this file has been included in subsequent
		calls.
	:param bool skipDup: skip inclusion instead of exiting if the file has
		already been included before (with ``putDup=True``)


Plugins (Loading of Additional Modules)
#######################################

It can be useful to always import additional modules when using MØD.
This can be achieved by writing a file in the following format::

  name: <module name>
  path: <Python import path>

When ``mod`` uses such a file, it will append ``<Python import path>`` to
``PYTHONPATH``,
and insert both ``import <module name>`` and ``from <module name> import *``
into the :ref:`preamble <mod-wrapper-preamble>`.
The plugin specification file must be placed in a directory that is also
mentioned in the following environment variable.

.. envvar:: MOD_PLUGIN_PATH

  A colon separated list of paths to search for plugins. Non-directory paths
  are skipped.
  All files (non-recursively) in each specified directory are checked for
  plugin information.
