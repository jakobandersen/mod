################################################
MØD
################################################

This is the documentation for the MØD software package.
The sources can be found in the GitHub repository:
`<http://github.com/jakobandersen/mod>`_.
For additional information see the webpage: `<http://mod.imada.sdu.dk>`_.

If you use MØD in your research you may want to cite [AFMS-MØD]_,
and other relevant publications, depending on the components used.
A discussion of rule-based modelling of chemistry can be found in [AFMS-Levels]_.
See :ref:`references` for a full list of publications.

The package contains the following components.

* :ref:`libmod`, a C++ shared library (``mod``)
	The main library.
* :ref:`pymod`, a Python module (``mod``)
	Python bindings for the library and extra functionality for easier usage of
	many features.
* :ref:`mod_post`, a Bash script (``mod_post``)
	The post-processor for compiling figures and summaries.
* :ref:`mod-wrapper`, a Bash script (``mod``)
	A convenience wrapper for invoking ``python3`` with user-supplied code.
	The wrapper handles output and invokes the post-processor.
	Additionally, it can run ``python3`` through ``gdb``
	and/or ``valgrind`` (either normal memcheck or callgrind).


Contents
========

.. toctree::
	:maxdepth: 1
	:numbered:

	installation
	compiling
	libmod/index
	pymod/index
	postmod/index
	exe/index
	graphModel/index
	formats/index
	dgStrat/index
	examples/index

	knownIssues
	changes
	references


Contributors
============

* `Jakob Lykke Andersen <https://imada.sdu.dk/~jlandersen>`__: main author.
* `Nikolai Nøjgaard <https://imada.sdu.dk/~nojgaard>`__:
  author of EpiM which was a Python module of MØD (see [AHMNP-EpiM]_).
