.. _compiling:

.. py:currentmodule:: mod
.. cpp:namespace:: mod

Compiling from Source
=====================

.. _quick-start:

Quick Start
-----------

These are simplified instructions for Ubuntu, Fedora, Arch, and macOS.
See :ref:`below <compiling-conda>` for instructions for compiling in a Conda
environment.

If a step doesn't work or fit your case, please consult the sections below.

First, you will need to have a Python virtual environment for your installation of MØD
(if you use Ubuntu < 23.04 or any Fedora version, you don't need to, as they don't implement `PEP 668 <https://peps.python.org/pep-0668/>`__).
Here we will start with a completely new environment called ``mod-env``.
Go to anywhere you like that environment to be stored execute

.. code-block:: bash

	python3 -m venv --system-site-packages mod-env
	source mod-env/bin/activate

Then we can start the compilation.
Retrive the MØD sources, create auto-generated files, and install Python dependencies:

.. code-block:: bash

	git clone --recursive https://github.com/jakobandersen/mod.git
	cd mod
	./bootstrap.sh
	python3 -m pip install -r requirements.txt

Then install the rest of the dependencies:

.. tab-set::

	.. tab-item:: Ubuntu
		:sync: ubuntu

		.. code-block:: bash

			sudo apt install $(bindep -b | tr '\n' ' ')

		Then

		1. Install Boost from source, see :ref:`install_boost_python`.
		   Remember the installation path.
		2. Install Graphviz from source:

		   .. code-block:: bash

		      sudo apt install librsvg2-dev libpango1.0-dev
		      # Download a source archive from https://graphviz.org/download/source/
		      # Extract the archive and cd into the extracted folder.
		      ./configure
		      make -j <n> # where <n> is the number of CPU cores you have, e.g., 'make -j 8'
		      sudo make install

	.. tab-item:: Fedora
		:sync: fedora

		.. code-block:: bash

			sudo dnf install $(bindep -b | tr '\n' ' ')

	.. tab-item:: Arch
		:sync: arch

		.. code-block:: bash

			sudo pacman -Suy --noconfirm $(bindep -b | tr '\n' ' ')

	.. tab-item:: macOS
		:sync: macos

		.. code-block:: bash

			brew tap Homebrew/bundle  # may not be needed
			brew bundle # perhaps restart the terminal afterwards to pick up the new commands

Then we can begin the compilation and installation:

.. tab-set::

	.. tab-item:: Ubuntu
		:sync: ubuntu

		.. code-block:: bash

			mkdir build
			cd build
			# install to the virtual env folder
			cmake ../ -DCMAKE_INSTALL_PREFIX=$VIRTUAL_ENV -DCMAKE_PREFIX_PATH=path/to/boost
			# Build and install:
			make -j <n>  # where <n> is the number of CPU cores you have, e.g., 'make -j 8'
			make install

	.. tab-item:: Fedora/Arch/macOS

		.. code-block:: bash

			mkdir build
			cd build
			# install to the virtual env folder
			cmake ../ -DCMAKE_INSTALL_PREFIX=$VIRTUAL_ENV
			# Build and install:
			make -j <n>  # where <n> is the number of CPU cores you have, e.g., 'make -j 8'
			make install


.. _compiling-conda:

Quick Start in a Conda Environment
----------------------------------

Retrive the MØD sources and create auto-generated files:

.. code-block:: bash

	git clone --recursive https://github.com/jakobandersen/mod.git
	cd mod
	./bootstrap.sh

In ``conda/environment.yaml`` is a specification of the environment needed to compile.
You can either create a new enviroment, called ``mod-env`` by defualt:

.. code-block:: bash

	conda env create -f conda/environment.yaml 

Or update an existing enviroment, say ``my-env``, with the dependencies:

.. code-block:: bash

	conda env update --name my-env -f conda/environment.yaml

Install enought of Latex in your system, outside Conda:

.. tab-set::

	.. tab-item:: Ubuntu
		:sync: ubuntu

		.. code-block:: bash

			sudo apt install texlive-science texlive-pictures texlive-latex-extra lmodern

	.. tab-item:: Fedora
		:sync: fedora

		.. code-block:: bash

			sudo dnf install texlive-collection-mathscience texlive-collection-pictures texlive-collection-latexextra

	.. tab-item:: Arch
		:sync: arch

		.. code-block:: bash

			sudo pacman -Suy texlive-science texlive-picture texlive-latexextra

	.. tab-item:: macOS
		:sync: macos

		.. code-block:: bash

			brew install --cask mactex

Activate the environment and then proceed with compilation:

.. code-block:: bash

	mkdir build
	cd build
	# install to Conda environment folder
	cmake ../ -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX
	# Build and install:
	make -j <n>  # where <n> is the number of CPU cores you have, e.g., 'make -j 8'
	make install


From a Git Repository
---------------------

After a checkout of the desired version, do:

.. code-block:: bash

	git submodule update --init --recursive
	./bootstrap.sh

This is needed to first fetch certain dependencies and second
to generate build files, extract the API documentation,
and create the file ``VERSION`` based on the current commit.

See :ref:`source-build` on how to then build the package.


As Dependency of Another CMake Project
--------------------------------------

MØD supports use via ``add_subdirectory`` in CMake.
The target ``mod::libmod`` is exported,
which can be used with ``target_link_libraries`` to link against libMØD.
The version is in the variable ``mod_VERSION``.
Note that running ``./bootstrap.sh`` is still needed if the
source is a repository clone (e.g., a Git submodule).


.. _source-build:

From a Source Archive
---------------------

The package is build and installed from source as a CMake project.
Generally that means something like:

.. code-block:: bash

	mkdir build
	cd build
	cmake ../ <options>
	make -j <n>
	make install

A source archive can also be created with ``make dist``.

The following is a list of commonly used options for ``cmake``.
Additional options specific for MØD along with their default valule are also
listed.
See also :ref:`dependencies` for elaboration on some of them.

- ``-DCMAKE_INSTALL_PREFIX=<prefix>``, set a non-standard installation
  directory. Note also that the `GNUInstallDirs
  <https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html>`__
  module is used.
- ``-DCMAKE_BUILD_TYPE=<build type>``, set a non-standard build type.
  The default is `RelWithDebInfo
  <https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html?highlight=build_type#variable:CMAKE_BUILD_TYPE>`__.
  An additional build type ``OptDebug`` is available which adds the compilation
  flags ``-g -O3``.
- ``-DCMAKE_PREFIX_PATH=<paths>``, set a ``;``-separated list of paths used for
  finding most dependencies. The paths should generally be the prefixes
  specified when the dependency was installed.
  See also https://cmake.org/cmake/help/latest/variable/CMAKE_PREFIX_PATH.html#variable:CMAKE_PREFIX_PATH.
- ``-DBUILD_DOC=on``, whether to build documentation or not.
  This is forced to ``off`` when used via ``add_subdirectory``.
- ``-DBUILD_POST_MOD=on``, whether to build the post-processor or not.
- ``-DBUILD_POST_MOD_FMT=on``, whether to build the Latex format files used by
  the post-processor or not.
  Format files may need to be updated when the Latex compiler is updated,
  so when making a binary distribution of MØD that does not include the Latex
  compiler, it is probably a good idea to set this to ``off``.
  If the format files are not installed they will be compiled dynamically
  by the post-processor when needed, i.e., making each post-processor run
  slightly slower.
  Alternatively, the format file can be (re)installed by the post-processor
  using the
  :option:`mod_post --install-format`/:option:`mod_post --install-format-sudo`
  options.
- ``-DBUILD_PY_MOD=on``, whether to build the Python bindings or not.
- ``-DBUILD_PY_MOD_PIP=on``, whether to install the Python bindings via pip or
  not. The bindings are always installed in the ``<prefix>/lib`` folder, so
  a normal ``import`` in Python will probably not find the module.
  Having this setting on will enable a build of a fake Python package to be
  installed via ``pip`` in the default system folder. This fake package will
  redirect the import to the real location.
  This package can be uninstalled with ``pip uninstall mod-jakobandersen``.
- ``-DBUILD_TESTING=off``, whether to allow test building or not.
  This is forced to ``off`` when used via ``add_subdirectory``.
  When ``on`` the tests can be build with ``make tests`` and run with ``ctest``.
- ``-DBUILD_WITH_SANITIZERS=off``, whether to compile libraries and tests
  with sanitizers or not. This is forced to ``off`` when ``BUILD_COVERAGE=on``.
- ``-DBUILD_EXAMPLES=off``, whether to build and allow running of examples as
  tests or not.
  This is forced to ``off`` when used via ``add_subdirectory``.
  This is forced to ``off`` when ``BUILD_TESTING`` is ``off``.
- ``-DBUILD_COVERAGE=off``, whether to compile code and run tests with GCov.
  When ``on`` the sanitizers on tests will be disabled.
  After building the tests, execute ``make coverage_collect`` without parallel
  jobs to run tests. Afterwards, execute ``make coverage_build`` to compile the
  code coverage report.
- ``-DENABLE_SYMBOL_HIDING=on``, whether symbols internal to the library are
  hidden or not. Disabling this option may degrade performance, and should only
  be done while developing extensions to the C++ library.
- ``-DENABLE_DEP_SYMBOL_HIDING=on``, whether symbols from library dependencies
  are hidden or not. Disabling this option may make it slower to load the
  library at runtime.
- ``-DENABLE_IPO=on``, whether to use link-time optimization or not.
  Disabling this option may degrade performance, but speed up linking time.
  As default the link-time optimizer will only use 1 thread.
  To use more threads, e.g., 7, use the following options for configuration
  ``-DCMAKE_MODULE_LINKER_FLAGS="-flto=7" -DCMAKE_SHARED_LINKER_FLAGS="-flto=7"``,
  when using GCC as the compiler.
- ``-DUSE_NESTED_GRAPH_CANON=on``, whether to use the dependency GraphCanon
  from the Git submodule or not.
- ``-DUSE_NESTED_NLOHMANN_JSON=on``, whether to use the dependency
  `nlohmann/json <https://github.com/nlohmann/json>`__ from the Git submodule
  or not.
- ``-DWITH_OPENBABEL=on``, whether to enable/disable features depending on Open Babel.


.. _dependencies:

Dependencies
------------

Python dependencies are listed in ``requirements.txt`` (which includes
``requirements_nodoc.txt``).

Dependencies that can be satisfied via the system package manager are listed in
``bindep.txt``. Any missing package can be listed using the
`Bindep <https://docs.openstack.org/infra/bindep/readme.html>`__ program
(which can be installed via ``requirements.txt``).

The following is a detailed list of all dependencies and the CMake switches
related to them.

- This documentation requires (``-DBUILD_DOC=on``):

  - A supported version of `Sphinx <http://sphinx-doc.org>`__.
  - The Python package ``sphinx-autoapi`` (module name: ``autoapi``).

- libMØD:

  - A C++ compiler with reasonable C++17 support is needed.
  - `Boost <http://boost.org>`__ dev >= 1.76
    (use ``-DCMAKE_PREFIX_PATH=<path>`` for non-standard locations).
  - `GraphCanon <https://github.com/jakobandersen/graph_canon>`__ >= 0.5.
    This is fulfilled via a Git submodule (make sure to do
    ``git submodule update --init --recursive``),
    but if another source is needed, set ``-DUSE_NESTED_GRAPH_CANON=off``.
  - `nlohmann/json <https://github.com/nlohmann/json>`__ >= 3.7.3.
    This is fulfilled via a Git submodule (make sure to do
    ``git submodule update --init --recursive``),
    but if another source is needed, set ``-DUSE_NESTED_NLOHMANN_JSON=off``.
  - (optional) `Open Babel <http://openbabel.org>`__ dev, >= 2.3.2
    (``-DWITH_OPENBABEL=on``).

- PyMØD (``-DBUILD_PY_MOD=on``):

  - Python 3 dev
  - Boost.Python built with Python 3
  - (Optional) IPython 3

- PostMØD (``-DBUILD_POST_MOD=on``):

  - ``pdflatex`` available in the ``PATH`` or in ``CMAKE_PROGRAM_PATH``,
    with not too old packages (Tex Live 2012 or newer should work).
  - ``pdf2svg`` available in the ``PATH`` or in ``CMAKE_PROGRAM_PATH``.
  - ``dot`` and ``neato`` from Graphviz available in the ``PATH`` or in
    ``CMAKE_PROGRAM_PATH``.
    They must additionally be able to load SVG files and output as both SVG and
    PDF files (all via cairo).
    That is, in the output of ``dot -P`` the following edges must exist:

    - ``cairo_device_svg -> output_svg``
    - ``cairo_device_pdf -> output_pdf``
    - ``rsvg_loadimage_svg -> render_cairo``

  - If you install Graphviz from source, you can check if the status output in
    the end of ``configure`` includes the following two lines::

    	pangocairo:    Yes
    	rsvg:          Yes


.. _install_boost_python:

Boost and Boost.Python with Python 3
------------------------------------

A package with the sources of Boost can be downloaded from `http://boost.org`.


Basic Installation
""""""""""""""""""

Boost uses a custom build system which may be difficult to use.
A procedure for compiling and installing Boost with Python 3 for Boost.Python
is the following.

1. ``./bootstrap.sh --with-python=python3`` (optionally give
   ``--prefix=some/path`` to specify a non-standard installation path.
2. ``./b2`` (optionally give ``-j N`` (similar to GNU Make) to compile with
   multiple threads)
3. ``./b2 install`` (optionally give ``--prefix=some/path`` to specify a
   non-standard installation path (if not done earlier).

After ``bootstrap.sh`` has been run, the file ``project-config.jam``
has been created, which can be edited to customise installation path and a lot
of other things. All edits should be done before running ``b2``.


Non-standard Python Installation
""""""""""""""""""""""""""""""""

Passing ``--with-python=python3`` to ``bootstrap.sh`` should work.
This adds a line similar to "``using python : 3.7 ;``" to
``project-config.jam``.
After compilation (running ``b2``) the file ``stage/lib/libboost_python3.so``
should exist. If not, it did not detect Python 3 properly.

If Python is installed in a non-standard location, add the a line similar to
"``using python : 3.7 : python3 : /path/to/python/3/installtion/include ;``" to
``project-config.jam``, where the last path is the path to the
``include``-folder of the Python-installation.


Custom GCC Version
""""""""""""""""""

Before running ``b2`` create the file ``user-config.jam`` in the root of the
home dir (see `here
<http://www.boost.org/boost-build2/doc/html/bbv2/overview/configuration.html>`__
for the full documentation). Put a line similar to
"``using gcc : : /path/to/g++-10``" in the file.
