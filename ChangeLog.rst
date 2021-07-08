.. cpp:namespace:: mod
.. py:currentmodule:: mod

Changes
#######


v0.13.0 (2021-07-08)
====================

Incompatible Changes
--------------------

- The package name has been changed to simply "MØD".
- Use more C++17 features, making some code not compile with GCC 7.
- Clang 9 seems to produce wrong code for PyMØD, resultining in
  segmentation faults during module import.
- Require Sphinx 3.5
- The return type of :cpp:func:`rule::Rule::getLabelType` has changed
  from using ``boost::optional`` to ``std::optional``.
- Change the GraphCanon submodule from a relative path to the Github
  repository.
- Rename the C++ graph loading functions

  - ``graph::Graph::gmlString`` to :cpp:func:`graph::Graph::fromGMLString`
  - ``graph::Graph::gml``       to :cpp:func:`graph::Graph::fromGMLFile`
  - ``graph::Graph::graphDFS``  to :cpp:func:`graph::Graph::fromDFS`
  - ``graph::Graph::smiles``    to :cpp:func:`graph::Graph::fromSMILES`
  - ``graph::Graph::makeGraph`` to :cpp:func:`graph::Graph::create`
- Rename the C++ rule loading functions

  - ``rule::Rule::ruleGMLString`` to :cpp:func:`rule::Rule::fromGMLString`
  - ``rule::Rule::ruleGML``       to :cpp:func:`rule::Rule::fromGMLFile`
- Add ``warnings`` parameter to :cpp:func:`graph::Graph::create`.
- Fix ``rcCommon`` to consistently enumerate common subgraphs that are not
  necessarily vertex-induced.
  Use ``config.rc.useBoostCommonSubgraph = False`` to switch to the old
  behaviour.
- The ``BUILD_DOC`` option for building from source now defaults to ``OFF``.
- Add :cpp:class:`rule::CompositionMatch`/:py:class:`RCMatch`.
- The file parameter for :py:func:`DG.load` and :py:func:`DGBuilder.load`
  has been changed name from ``file`` to ``f``.
- :py:func:`Graph.fromSMILES` has changed order of parameters,
  ``add`` is now the last one.


New Features
------------

- Added ``printCombined`` parameter to
  :cpp:func:`rule::Rule::print`/:py:meth:`Rule.print`
  to optionally print a figure where the rule is depicted as a single
  combined graph.
  This was previously always printed, but now it defaults to off.
- Added <-operator to
  :cpp:class:`graph::Union`/:py:class:`UnionGraph`,
  :cpp:class:`rule::Rule::LeftGraph`/:py:class:`Rule.LeftGraph`,
  :cpp:class:`rule::Rule::ContextGraph`/:py:class:`Rule.ContextGraph`, and
  :cpp:class:`rule::Rule::RightGraph`/:py:class:`Rule.RightGraph`.
- Added :cpp:func:`dg::Printer::getTikzpictureOption`,
  :cpp:func:`dg::Printer::setTikzpictureOption`,
  :py:attr:`DGPrinter.tikzpictureOption`.
- Added :cpp:func:`dg::DG::printNonHyper`/:py:meth:`DG.printNonHyper`.
- Allow ``limit=0`` for repeat strategies,
  :cpp:func:`dg::Strategy::makeRepeat`/:py:meth:`DGStrat.makeRepeat`.
- Added overload for :cpp:func:`dg::DG::dump`/:py:meth:`DG.dump` that takes a
  target filename as argument.
- Add the static methods

  - :py:func:`Graph.fromGMLString` (the same as :py:func:`graphGMLString`)
  - :py:func:`Graph.fromGMLFile`   (the same as :py:func:`graphGML`)
  - :py:func:`Graph.fromDFS`       (the same as :py:func:`graphDFS`)
  - :py:func:`Graph.fromSMILES`    (the same as :py:func:`smiles`)
  - :py:func:`Rule.fromGMLString`  (the same as :py:func:`ruleGMLString`)
  - :py:func:`Rule.fromGMLFile`    (the same as :py:func:`ruleGML`)
- Allow dot (``.``) bonds in :ref:`SMILES <graph-smiles>` strings.
- Add the following functions for loading a possibly disconnected graph:

  - :cpp:func:`graph::Graph::fromSMILESMulti`/:py:func:`Graph.fromSMILESMulti`
  - :cpp:func:`graph::Graph::fromGMLStringMulti`/:py:func:`Graph.fromGMLStringMulti`
  - :cpp:func:`graph::Graph::fromGMLFileMulti`/:py:func:`Graph.fromGMLFileMulti`
- Add :envvar:`MOD_PYTHON` and :envvar:`MOD_IPYTHON` to overwrite the
  interpreter the :ref:`wrapper script <mod-wrapper>` executes.
- Add :cpp:func:`graph::Graph::getLoadingWarnings`/:py:attr:`Graph.loadingWarnings`.


Bugs Fixed
----------

- :cpp:func:`rule::Rule::fromGMLFile`/:py:func:`Rule.fromGMLFile` and
  :cpp:func:`rule::Rule::fromGMLString`/:py:func:`Rule.fromGMLString`:

  - Fixed typos in a few error messages.
  - Actually fail loading when errors in constraints are encountered.

- Fix v0.12 problem with RPATH handling of ``libmod``.
- Doc, fix infinite search.
- Fix exception visibility on macOS so they can be caught outside the library.
- Tests, set C++ standard in CMake tests.
- Added missing ``operator<`` to :cpp:class:`graph::Union::Vertex`.
- Fix error handling to throw the right exception with better message when the
  file can not be opened for the functions
  :cpp:func:`graph::Graph::fromGMLFile`/:py:func:`Graph.fromGMLFile`,
  :cpp:func:`rule::Rule::fromGMLFile`/:py:func:`Rule.fromGMLFile`,
  :cpp:func:`dg::DG::load`/:py:meth:`DG.load`,
  :cpp:func:`dg::Builder::load`/:py:meth:`DGBuilder.load`.
- Fixes to support Boost 1.76.
- Fixes to support GCC 11.
- Build system, use ``add_custom_command`` to avoid recompilation of
  docs and Latex format file on install.
  Fixes problem with failing to import ``sphinx``
  when running ``sudo make install``.


Other
-----

- Test, set C++ standard in CMake tests.
- Doc, fix description of :py:class:`RCExpExp` and :py:class:`RCExpComposeCommon`.
- Doc, for libMØD classes, make a synopsis with links to declarations.
- Doc, fix documentation for :cpp:func:`post::makeUniqueFilePrefix` so it is
  documented to be in namespace ``post``.
- Docker, for building the Ubuntu image, download Boost from the new URL.
- Conda, require a newer Graphviz version with rsvg from conda-forge instead of
  custom version.
- Refresh the messages from and the documentation on :ref:`mod-wrapper`.
- Doc, clarify use of ``pip`` may need ``--user`` for home folder installation.
- Doc, properly document that a :py:class:`CWDPath` is a valid argument for

  - :py:func:`DG.load`,
  - :py:func:`DGBuilder.load`,
  - :py:func:`Graph.fromGMLFile`, and
  - :py:func:`Rule.fromGMLFile`.
- Doc, clarify conditions on methods in :cpp:class:`dg::DG`/:py:class:`DG`
  regarding "hasActiveBuilder" and "isLocked".


v0.12.0 (2021-01-18)
====================

Incompatible Changes
--------------------

- Require C++17.
- Require Boost 1.73 to avoid CMake issue in 1.72.
- Require GraphCanon 0.5.
- Require Sphinx 3.4.
- Moved compilation instructions from :ref:`installation` to :ref:`compiling`.
- Renamed ``DGStratGraphState`` to :py:class:`DGStrat.GraphState`.
- Swap parameters for :cpp:func:`dg::DG::print` to be consistent with
  :py:func:`DG.print`.
- The function ``mod::makeUniqueFilePrefix()`` has been renamed to
  :cpp:func:`mod::post::makeUniqueFilePrefix`.
- Names for the left, context, and right graph of :py:class:`Rule`
  has been moved and renamed to be nested types of :py:class:`Rule`.
- Names for graph interface types for
  :py:class:`Graph`,
  :py:class:`Rule`,
  :py:class:`Rule.LeftGraph`,
  :py:class:`Rule.ContextGraph`, and
  :py:class:`Rule.RightGraph`
  have been moved and renamed to be nested types of their graph class.
- Names for graph automorphism types for :py:class:`Graph`
  have been moved and renamed to be nested types of :py:class:`Graph`.
- :cpp:func:`rule::Composer::eval`/:py:func:`RCEvaluator.eval` now returns a list
  of results, possibly with duplicates, instead of only a collection of unique rules.


New Features
------------

- A pre-compiled Conda installation is now available on Linux,
  see :ref:`installation` and https://anaconda.org/jakobandersen/mod.
- Added new higher-level installation instructions at :ref:`installation`,
  with documentation for how to install via Conda and using the Docker image.
- Update ``bindep.txt`` and :ref:`quick-start` guide for Arch.
- Add ``Brewfile`` to to make installation of dependencies much easier on macOS.
- Improved verbose output from "add" strategies during
  :cpp:func:`dg::Builder::execute`/:py:func:`DGBuilder.execute`.
- Improved rule application performance when evaluating
  :ref:`rule strategies <strat-rule>` and executing
  :cpp:func:`dg::Builder::apply`/:py:meth:`DGBuilder.apply`.
- Added a relaxed mode to 
  :cpp:func:`dg::Builder::apply`/:py:meth:`DGBuilder.apply`
  via the ``onlyProper`` parameter.
- Add missing ``graph`` attributes to vertices and edges of the
  four graph interfaces of :py:class:`Rule`.
- Add new class :cpp:class:`graph::Union`/:py:class:`UnionGraph`.
- Improve handling of pre-compiled Latex format files used in the
  post-processor (thanks also to Nikolai Nøjgaard):

  - Add build option to not install the file during normal installation.
    See ``-DBUILD_POST_MOD_FMT`` in :ref:`installation`.
  - Teach the post-processor to dynamically compile the format file if it is
    not installed.
  - Add options :option:`mod_post --install-format`
    and :option:`mod_post --install-format-sudo`
    for installing/updating the pre-compiled format file after MØD was
    installed. This can be used on an installation configured with
    ``-DBUILD_POST_MOD_FMT=off`` or for resolving a
    :ref:`known issue <issue-fmt>`.

- Make :cpp:class:`AtomData`/:py:class:`AtomData` LessThanComparable.
- Make build work on macOS and add installation instructions.
- Rule composition: when using ``rcCommon``, skip duplicate overlaps yielded by
  the underlying enumeration algorithm.


Bugs Fixed
----------

- Flush stdout in the end of
  :cpp:func:`dg::ExecuteResult::list`/:py:func:`DGExecuteResult.list`.
- Fix printing/stringification of a null vertices for
  :cpp:class:`graph::Graph`/:py:class:`Graph`,
  :cpp:class:`rule::Rule`/:py:class:`Rule`,
  :cpp:class:`rule::Rule::LeftGraph`/:py:class:`Rule.LeftGraph`,
  :cpp:class:`rule::Rule::ContextGraph`/:py:class:`Rule.ContextGraph`,
  :cpp:class:`rule::Rule::RightGraph`/:py:class:`Rule.ContextGraph`.
- Fix conversion of :py:class:`Graph.Edge` to ``bool``.
- Fix rule printing when hydrogens are collapsed to prevent occasional
  Latex errors on the form
  ``! Package pgf Error: No shape named `v-coord-<num>' is known.``.
- Add missing ``operator!=`` for :cpp:class:`AtomData`.
- macOS build fixes:

  - Infinite recursion in doc building, due to missing toctrees.
  - Disable leak sanitizer when using AppleClang as compiler.
  - Disable more compiler/link options not in AppleClang.

Other
-----

- Doc, rearrange and deduplicate documentation for graph interfaces
  in PyMØD.
  Introduce the :ref:`py-protocols` section which documents common protocols
  that several classes implement.
  The documentation of the following classes has been simplified by mostly
  referring to these protocols:

  - :py:class:`Graph`
  - :py:class:`Rule`

- Doc, improve documentation of the graph interfaces of :py:class:`Rule`.


v0.11.0 (2020-08-31)
====================

Incompatible Changes
--------------------

- Bump version requirement of Boost to 1.72.
- :cpp:func:`dg::Printer::pushVertexVisible`/:py:func:`DGPrinter.pushVertexVisible`,
  :cpp:func:`dg::Printer::pushVertexLabel`/:py:func:`DGPrinter.pushVertexLabel`, and
  :cpp:func:`dg::Printer::pushVertexColour`/:py:func:`DGPrinter.pushVertexColour`
  now requies a callback taking a
  :cpp:class:`dg::DG::Vertex`/:py:class:`DGVertex`, instead of a
  :cpp:class:`graph::Graph`/:py:class:`Graph` and
  :cpp:class:`dg::DG`/:py:class:`DG`.
  The previous style is removed in libMØD and deprecated in PyMØD.
- :cpp:func:`dg::DG::HyperEdge::print`/:py:func:`DGHyperEdge.print`
  now throws exceptions if either no rules are associated with the hyperedge
  or if at least one of the associated rules does not lead to a derivation.
- :cpp:class:`dg::PrintData`/:py:class:`DGPrintData`, many interface changes,
  including proper argument checking.
- ``dg::DG::dumpImport()`` has been renamed to :cpp:func:`dg::DG::load`.
- ``dgDump()`` has been renamed to :py:func:`DG.load`.
- :cpp:func:`dg::DG::load`/:py:func:`DG.load` has additional arguments
  and pre-conditions.
- Do not install a pkg-config file. It was broken and there doesn't seem to be
  an easy way to fix it.


New Features
------------

- Added :cpp:func:`dg::Builder::apply`/:py:meth:`DGBuilder.apply`
  as a lower-level function for computing proper direct derivations.
- :cpp:func:`graph::Graph::fromSMILES`/:py:meth:`smiles`:

  - Generalize the parser to accept almost arbitrary strings as symbols inside
    brackets. See :ref:`graph-smiles`.
    This is only allowed when passing ``allowPartial=True`` to
    :py:meth:`smiles`.
  - Generalize the parser to accept ring-bonds and branches in mixed order.
  - Generalize the parser to accept non-standard charges:
    ``+++``, ``++``, ``---``, ``--``, and magnitudes larger than +/-9.

- Added the PyMØD submodule :ref:`epim`.
- Added :cpp:enum:`SmilesClassPolicy`/:py:class:`SmilesClassPolicy`
  argument to :cpp:func:`graph::Graph::fromSMILES`/:py:meth:`smiles`.
- Support using either Open Babel 2 or 3 as dependency.
- Make :py:attr:`DGPrinter.graphPrinter` writeable as well.
- Make :cpp:class:`graph::Printer`/:py:class:`GraphPrinter` equality comparable.
- Added :cpp:func:`dg::Printer::setGraphvizPrefix`/:cpp:func:`dg::Printer::getGraphvizPrefix`/:py:attr:`DGPrinter.graphvizPrefix`.
- Added ``makeUniqueFilePrefix``/:py:func:`makeUniqueFilePrefix`.
- Improve verbosity level 8 information from
  :cpp:func:`dg::Builder::execute`/:py:func:`DGBuilder.execute` to the universe
  size.
- Make :cpp:class:`LabelSettings`/:py:class:`LabelSettings`
  equality comparable.
- Added :cpp:func:`dg::Builder::load`/:py:func:`DGBuilder.load`.
- Added :cpp:func:`rngUniformReal`/:py:func:`rngUniformReal`.


Bugs Fixed
----------

- Fix handling of null pointers:

  - :cpp:class:`Derivation`/:py:class:`Derivation` printing.
  - :cpp:class:`Derivations`/:py:class:`Derivations` printing.
  - :cpp:func:`dg::Builder::addDerivation`/:py:meth:`DGBuilder.apply`.
  - :cpp:func:`dg::Builder::execute`
  - :cpp:func:`dg::DG::make`/:py:meth:`DG.__init__`
  - :cpp:func:`dg::DG::findVertex`/:py:meth:`DG.findVertex`
  - (:cpp:func:`dg::DG::findEdge`/:py:meth:`DG.findEdge`)
  - Static and dynamic "add" strategies,
    :cpp:func:`dg::Strategy::makeAdd`/:py:meth:`DGStrat.makeAddStatic`
    and :py:meth:`DGStrat.makeAddDynamic`.
  - Sequence strategies,
    :cpp:func:`dg::Strategy::makeSequence`/:py:meth:`DGStrat.makeSequence`
  - Rule strategies,
    :cpp:func:`dg::Strategy::makeRule`/:py:meth:`DGStrat.makeRule`
  - Parallel strategies,
    :cpp:func:`dg::Strategy::makeParallel`/:py:meth:`DGStrat.makeParallel`
  - Filter strategies,
    :cpp:func:`dg::Strategy::makeFilter`
  - Execute strategies,
    :cpp:func:`dg::Strategy::makeExecute`
  - Left/right predicate strategies,
    :cpp:func:`dg::Strategy::makeLeftPredicate`/:py:meth:`DGStrat.makeLeftPredicate`,
    :cpp:func:`dg::Strategy::makeRightPredicate`/:py:meth:`DGStrat.makeRightPredicate`
  - Revive strategies,
    :cpp:func:`dg::Strategy::makeRevive`/:py:meth:`DGStrat.makeRevive`
  - Repeat strategies,
    :cpp:func:`dg::Strategy::makeRepeat`/:py:meth:`DGStrat.makeRepeat`

- Fix handling of empty functions given as callbacks:

  - :cpp:func:`dg::Printer::pushVertexVisible`,
  - :cpp:func:`dg::Printer::pushEdgeVisible`,
  - :cpp:func:`dg::Printer::pushVertexLabel`,
  - :cpp:func:`dg::Printer::pushEdgeLabel`,
  - :cpp:func:`dg::Printer::pushVertexColour`,
  - :cpp:func:`dg::Printer::pushEdgeColour`,
  - :cpp:func:`dg::Printer::setRotationOverwrite`, and
  - :cpp:func:`dg::Printer::setMirrorOverwrite`.

- :cpp:func:`graph::Graph::fromSMILES`/:py:meth:`smiles`:

  - Improve parsing error messages.
  - Fix missing external ID for bracketed wildcard atoms with class label,
    e.g., ``[*:42]``.
  - Fix handling of an atom which contains a ring-closure and ring-opening
    using the same ID, e.g., ``C1CCCP11NNNN1``.
  - When there is a bond mismatch in a ring closure (e.g., ``C-1CCCC=1``),
    throw a :cpp:class:`InputError`/:py:class:`InputError` instead of
    a :cpp:class:`FatalError`/:py:class:`FatalError`.

- :py:class:`Isotope` and :py:class:`Charge` are now comparable with integers.
- :cpp:func:`dg::DG::print`/:py:meth:`DG.print`, fix missing labels on shortcut
  edges when using a :cpp:class:`dg::Printer`/:py:class:`DGPrinter` with
  "labels as Latex math" set to false.
- :cpp:func:`dg::Builder::addAbstract`/:py:meth:`DGBuilder.addAbstract`:

  - Improve parsing error messages.
  - Fix assertion on non-ASCII input.
- :py:meth:`include`, read files in binary instead of ASCII.
- PostMØD: scale figures based on height as well to avoid them being clipped.
  Thanks to Christoph Flamm.
- Fix :cpp:func:`rule::Rule::getGMLString`/:py:meth:`Rule.getGMLString` to not
  perform coordinate instantiation when not needed.
- Fix Python export of :py:class:`Rule.ContextGraph.Vertex`.
- Properly throw exceptions from all ``pop`` functions in
  :cpp:class:`dg::Printer`/:py:class:`DGPrinter` when there is nothing to pop.
- PostMØD: remove extranous escape of a quote in AWK script in ``coordsFromGV``.
- Graph printing, fix coordinate overwrite when printing the same graph
  multiple times, but with different rotation or mirror settings,
  the layout of the last printing would be used for all of them.
  Those with non-zero rotation and mirroring now have their own file name.
- DG printing: fix bending of head/tail arrows when a tail vertex is also a
  head vertex so arrows don't overlap.
- `#8 <https://github.com/jakobandersen/mod/issues/8>`__:
  remove some linker flags when AppleClang is used.


Other
-----

- Doc, update theming again to increase readability.
- Doc, add more formal API for the
  :ref:`embedded strategy language for derivation graphs <dg_edsl>`.
- Doc, fix typo resulting in missing documentation of

  - :py:attr:`AtomData.atomId`
  - :py:attr:`AtomData.isotope`
  - :py:attr:`DGVertex.inDegree`
  - :py:attr:`DGVertex.outDegree`

- Doc, various typo fixes.
- :ref:`mod <mod-wrapper>`, don't log output when invoked with
  :option:`--debug <mod --debug>`.
- Doc, clarify that
  :py:func:`DGPrinter.pushVertexVisible`,
  :py:func:`DGPrinter.pushEdgeVisible`,
  :py:func:`DGPrinter.pushVertexLabel`,
  :py:func:`DGPrinter.pushEdgeLabel`,
  :py:func:`DGPrinter.pushVertexColour`,
  :py:func:`DGPrinter.pushEdgeColour`,
  :py:func:`DGPrinter.setRotationOverwrite`,
  :py:func:`DGPrinter.setMirrorOverwrite`
  accepts a constant as well as a callback.
- Doc, fix callback type for
  :py:func:`DGPrinter.setRotationOverwrite` and
  :py:func:`DGPrinter.setMirrorOverwrite`.
  They must take a :py:class:`Graph`, not a :py:class:`GraphPrinter`.
- Doc, add return type to :py:func:`DG.findEdge`.
- Added ``bindep.txt`` and ``requirements.txt`` to make installation of
  dependencies much easier.
  The installation instructions are updated with a :ref:`quick-start` guide and
  notes on the use of the dependency files.
- CMake, default ``BUILD_EXAMPLES=on``.



v0.10.0 (2020-02-05)
====================

Incompatible Changes
--------------------

- ``dg::DG::abstract``/``dgAbstract`` has been removed. Use
  :cpp:func:`dg::Builder::addAbstract`/:py:func:`DGBuilder.addAbstract`
  instead. Added slightly better documentation as well, :ref:`dg_abstract-desc`.
- ``dg::DG::derivations`` has been removed. Use the repeated calls
  to :cpp:func:`dg::Builder::addDerivation` instead.
- ``dg::DG::ruleComp`` and ``dg::DG::calc()`` has been removed.
  Use the new :cpp:func:`dg::Builder::execute` instead.
- ``dgRuleComp`` and ``DG.calc`` has been deprecated,
  and their implementation is now based on :py:meth:`DGBuilder.execute`.
  Use :py:meth:`DGBuilder.execute` directly instead.
- The implementation of ``dgDerivations`` has changed and the function
  is now deprecated. Use repeated calls to
  :py:meth:`DGBuilder.addDerivation` instead.
- :cpp:func:`dg::Strategy::makeAdd` overloads,
  :py:meth:`DGStrat.makeAddStatic`, and :py:meth:`DGStrat.makeAddDynamic`
  now requires another argument of type
  :cpp:enum:`IsomorphismPolicy`/:py:class:`IsomorphismPolicy`.
- :ref:`strat-addSubset` and :ref:`strat-addUniverse` now accepts a new optional
  keyword argument ``graphPolicy`` of type :py:class:`IsomorphismPolicy`.
- ``dg::DG::list``/``DG.list`` has been removed,
  use :cpp:func:`dg::ExecuteResult::list`/:py:meth:`DGExecuteResult.list`
  instead.
- Information from strategies has been updated.


New Features
------------

- Added new incremental build interface for :py:class:`DG`/:cpp:class:`dg::DG`.
  It includes:

  - :py:meth:`DG.__init__`/:cpp:func:`dg::DG::make` for constructing a
    derivation graph with this new interface.
  - :py:meth:`DG.build`/:cpp:func:`dg::DG::build` for obtaining an RAII-style
    proxy object for controlling the construction
    (:py:class:`DGBuilder`/:cpp:class:`dg::Builder`).
  - :py:attr:`DG.hasActiveBuilder`/:cpp:func:`dg::DG::hasActiveBuilder`
  - :py:attr:`DG.locked`/:cpp:func:`dg::DG::isLocked`

- Added :py:class:`Derivations`/:cpp:class:`Derivations` as an alternative
  to :py:class:`Derivation`/:cpp:class:`Derivation` which contains a list
  of rules instead of at most a single rule.
  The latter is implicitly convertible to the former.
- :py:class:`Rule` now has an overloaded operator ``<``.
- :py:class:`IsomorphismPolicy`/:cpp:enum:`IsomorphismPolicy` has been added
  to help configure various algorithms by users.
- Added :py:attr:`DG.labelSettings`/:cpp:func:`dg::DG::getLabelSettings`.
- Added :envvar:`MOD_NO_DEPRECATED` to make it easier to find usage of
  deprecated behaviour.
- Added :py:func:`Rule.isomorphicLeftRight`/:cpp:func:`rule::Rule::isomorphicLeftRight`.


Bugs Fixed
----------

- Changed assert to a proper error message at code related to Open Babel.
  If MØD, or an extension library, is loaded with ``dlopen`` without the
  ``RTLD_GLOBAL`` flag, there can be multiple copies of Open Babel symbols at
  the same time, which prevent MØD from accessing Open Babel operations..
- Document and check proper preconditions on :cpp:class:`dg::DG`/:py:class:`DG`.
- Document and check precondition on
  :cpp:func:`dg::DG::HyperEdge::getInverse`/:py:attr:`DGHyperEdge.inverse`,
  that it is only avilable after the DG is locked.
- Properly throw an exception if
  :py:meth:`DGStrat.makeSequence`/:cpp:func:`dg::Strategy::makeSequence`
  if given an empty list of strategies.
- Properly implementing stringification of
  :py:class:`LabelType`/:cpp:enum:`LabelType`,
  :py:class:`LabelRelation`/:cpp:enum:`LabelRelation`,
  :py:class:`LabelSettings`/:cpp:class:`LabelSettings`, and
  :py:class:`IsomorphismPolicy`/:cpp:enum:`IsomorphismPolicy`.
- Build: disallow use of experimental Boost CMake support due to a linking
  problem.


Other
-----

- Various fixes for documentation formatting including new themeing.
- Installation, highlight the more relevant ``CMAKE_PREFIX_PATH`` instead of
  ``CMAKE_PROGRAM_PATH``.
- Bump recommended lower bound on GCC version to 6.1 in the documentation.
- Updated documentation for :cpp:class:`mod::Derivation`/:py:class:`Derivation`.
- Documentation, added explicit example section.


v0.9.0 (2019-08-02)
===================

Incompatible Changes
--------------------

- Change to CMake as build system.
  See :ref:`installation` for how to build the package,
  or used it as a submodule in another CMake project.
- Now requires v0.4 of
  `GraphCanon <https://github.com/jakobandersen/graph_canon>`__
  (and `PermGroup <https://github.com/jakobandersen/perm_group>`__).
- :cpp:func:`dg::DG::getGraphDatabase` now returns a :cpp:any:`std::vector`
  instead of a :cpp:any:`std::unordered_set`.
- Hide internal symbols in the library to increase optimization opportunities,
  and hide symbols in library dependencies.
  Libraries linking against libmod may stop linking, but configuration options
  has been added to disable symbol hiding.


New Features
------------

- ``dgDump``/``dg::DG::dump`` should now be much, much faster
  in parsing the input file and loading the contained derivation graph.
- ``dgRuleComp``/``dg::DG::ruleComp`` should now be much faster
  during calculation.
- Added :py:func:`Graph.instantiateStereo`/:cpp:func:`graph::Graph::instantiateStereo`.
- Added :py:func:`rngReseed`/:cpp:func:`rngReseed`.


Bugs Fixed
----------

- Fixed off-by-one error in DG dump loading, ``dgDump``/``dg::DG::dump``.
- Fixed issues with ``auto`` in function signatures which is not yet in the C++ standard.


Other
-----

- The functions :py:func:`prefixFilename`, :py:func:`pushFilePrefix`, and :py:func:`popFilePrefix`
  used by the :py:func:`include` function are now documented.
  A new class :py:class:`CWDPath` has been added.
- Use interprocedural/link-time optimization as default.
  It can be disabled with a configuration option.


v0.8.0 (2019-04-04)
===================

Incompatible Changes
--------------------

- Now requires v0.3 of
  `GraphCanon <https://github.com/jakobandersen/graph_canon>`__
  (and `PermGroup <https://github.com/jakobandersen/perm_group>`__).
- ``graph::Graph::getMolarMass``/``Graph.molarMass`` has been removed.
- Python interface: remove auto-generated hash-functions from all classes.
  Note, most code broken by this was already silemtly broken.
- Python interface: consistently disable all custom attributes on all classes.
- Removed ``dg::Strategy::GraphState::getHyperEdges``/``DGStratGraphState.hyperEdges``.
  Use the graph interface of :cpp:any:`dg::DG`/:py:obj:`DG` instead.
- All atoms, including hydrogens, are now present with ids in strings from
  :cpp:any:`graph::Graph::getSmilesWithIds`/:py:obj:`Graph.smilesWithIds`.
- :cpp:any:`dg::DG::print`/:py:obj:`DG.print` now returns a pair of strings,
  instead of just one string. The first entry is the old return value.
  The second entry is the tex-file to depend on for layout coordinates.
- SMILES parsing: disallow isotope 0 as it is equivalent to not specifying an isotope.
- All classes in the Python interface without a custom hash function has their
  hash function removed. This is to prevent inconsistencies between hash and equality.


New Features
------------

- Added support for isotopes (see :ref:`mol-enc`).
- Added :cpp:any:`graph::Graph::getExactMass`/:py:obj:`Graph.exactMass`.
- Added optional ``printInfo`` parameter to
  ``dg::DG::calc``/``DG.calc``.
  to allow disabling of messages to stdout during calculation.
- The graph interface on :cpp:any:`dg::DG`/:py:obj:`DG` can now be used before and during
  calculation.
- Added include of the PGFPlots package in the summary preamble.
- Added :cpp:any:`AtomId::symbol`/:py:obj:`AtomId.symbol`.
- Added an ``add`` parameter to :py:obj:`graphGMLString`, :py:obj:`graphGML`,
  :py:obj:`graphDFS`, :py:obj:`smiles`, :py:obj:`ruleGMLString`, and :py:obj:`ruleGML`.
  It controls whether the graph/rule is appended to :py:obj:`inputGraphs`/:py:obj:`inputRules`
  or not. It defaults to ``True``.
- Add :cpp:any:`graph::Graph::getGraphDFSWithIds`/:py:obj:`Graph.graphDFSWithIds`
  for getting a string annotated with the internal vertex ids in form of the class labels.
  This mirrors the previously added :cpp:any:`graph::Graph::getSmilesWithIds`/:py:obj:`Graph.smilesWithIds`.
- Improve error messages from GML parsing of lists.
- Changed the return type of :cpp:func:`dg::DG::getGraphDatabase` from a `std::set` to a `std::unordered_set`.
- :cpp:func:`dg::DG::HyperEdge::print`/:py:func:`DGHyperEdge.print` now returns a list of file data.
- The vertices and edges of all graph interfaces now have a conversion to bool:

  - :cpp:class:`graph::Graph::Vertex`/:py:class:`Graph.Vertex`,
    :cpp:class:`graph::Graph::Edge`/:py:class:`Graph.Edge`
  - :cpp:class:`rule::Rule::Vertex`/:py:class:`Rule.Vertex`,
    :cpp:class:`rule::Rule::Edge`/:py:class:`Rule.Edge`
  - :cpp:class:`rule::Rule::LeftGraph::Vertex`/:py:class:`Rule.LeftGraph.Vertex`,
    :cpp:class:`rule::Rule::LeftGraph::Edge`/:py:class:`Rule.LeftGraph.Edge`
  - :cpp:class:`rule::Rule::ContextGraph::Vertex`/:py:class:`Rule.ContextGraph.Vertex`,
    :cpp:class:`rule::Rule::ContextGraph::Edge`/:py:class:`Rule.ContextGraph.Edge`
  - :cpp:class:`rule::Rule::RightGraph::Vertex`/:py:class:`Rule.RightGraph.Vertex`,
    :cpp:class:`rule::Rule::RightGraph::Edge`/:py:class:`Rule.RightGraph.Edge`
  - :cpp:class:`dg::DG::Vertex`/:py:class:`DGVertex`,
    :cpp:class:`dg::DG::HyperEdge`/:py:class:`DGHyperEdge`

- The vertices of all graph interfaces now have a proper hash support.
- Added :cpp:func:`dg::Printer::setRotationOverwrite`/:py:func:`DGPrinter.setRotationOverwrite`
  and :cpp:func:`dg::Printer::setMirrorOverwrite`/:py:func:`DGPrinter.setMirrorOverwrite`.


Bugs Fixed
----------

- Throw :cpp:any:`InputError`/:py:obj:`InputError` when loading a DG dump
  when a rule in the dump can not be linked to a rule from the user.
- Fix molecule decoding of atoms with negative charge and a radical.
- Fix dangling reference bug in first-order term handling.
- Fix inifiinite loop bug in first-order term handling.
- Remove extraneous template parameter lists to make it compile on GCC 8.
- Fix the documentation of
  :py:obj:`Graph.minExternalId`, :py:obj:`Graph.maxExternalId`,
  :py:obj:`Rule.minExternalId`, and :py:obj:`Rule.maxExternalId`.
  It was not being rendered.
- Fixed documentation of the constructor for :cpp:class:`AtomData`.
- Fix dangling references in morphism callbacks.
- Make sure Open Babel is not called in some cases where it is not needed.
- Find the library file for Boost.Python for Boost >= 1.67.
- Fix ambiguity between variadic arguments and function parameter packs,
  making term morphisms and stereo morphisms slow.
- Removed sanity check from GraphDFS loading which dominated the run time.
- Document :py:obj:`inputGraphs` and :py:obj:`inputRules`.


Other
-----

- Now compiles with ``-fno-stack-protector`` (some OS distributions messes with default flags).
- The Makefile from ``mod --get-latex`` now cleans ``.vrb``, ``.snm``, and ``.nav`` files as well.


v0.7.0 (2018-03-08)
===================

Incompatible Changes
--------------------

- Boost >= 1.64 is now required.
- Two new libraries,
  `GraphCanon <https://github.com/jakobandersen/graph_canon>`__ and
  `PermGroup <https://github.com/jakobandersen/perm_group>`__, are now required dependencies.
- Sphinx 1.7.1 is now required for building the documentation.
- :cpp:any:`dg::DG::HyperEdge::print`/:py:obj:`DGHyperEdge.print`
  now also takes an argument for colouring vertices/edges
  that are not matched by the rule. The default is now that matched vertices/edges
  are the default colour, while those that are not matched are grey.
- Most of the outer interface headers have now been moved to subfolders,
  and their content has been moved to corresponding namespaces.
  Several classes has been slightly renamed during the move,
  and some headers has been split into multiple header files.
- The previously deprecated class ``DerivationRef`` has now been removed along with
  ``DG.derivations`` in the Python interface.
- The previously deprecated method ``DG.vertexGraphs`` in the Python interface
  has been removed.


New Features
------------

- Added functions to map external ids of graphs and rules to internal vertices.
- Added functions to get vertex coordinates for rules.
- :cpp:any:`dg::DG::print`/:py:obj:`DG.print`
  now returns the name of the PDF-file that will be created in post-processing.
- Add :cpp:any:`dg::Printer::setWithInlineGraphs`/:py:obj:`DGPrinter.withInlineGraphs`
  to input raw tex files for graphs in nodes instead of compiled PDFs.
  The generated tex code for the graphs is different to ensure unique Tikz node names.
- Add ``inline`` as a special argument for the graph Latex macros to input the raw tex file,
  instead of including a compiled PDF.
- Add :cpp:any:`graph::Graph::getSmilesWithIds`/:py:obj:`Graph.smilesWithIds`
  for getting a string annotated with the internal vertex ids in form of the class labels.
- The automorphism group of each graph is now available.


Experimental New Features
-------------------------

- Vertex/edge labels in graphs/rules can now be interpreted as first-order terms.
  Syntactic unification is then performed during morphism finding.
  See where :cpp:any:`LabelSettings`/:py:obj:`LabelSettings` is being used.
  Each rule has an optional :cpp:any:`LabelType`/:py:obj:`LabelType`
  to signify whether it was designed for use with
  :cpp:any:`LabelType::String`/:py:obj:`LabelType.String` or
  :cpp:any:`LabelType::Term`/:py:obj:`LabelType.Term`.
  Some algorithms will check this property for safety reasons, but the check can be disbled.
- There is now a prototype-implementation of http://doi.org/10.1007/978-3-319-61470-0_4,
  for adding stereo-information to graphs/rules.
  Use :cpp:any:`LabelSettings`/:py:obj:`LabelSettings` objects to enable it.
  See the paper for examples on how to use it. The full framework will be implemented and
  documented in a future version.


Bugs Fixed
----------

- Multiple rules for the same derivation is now properly recorded.
- Fix documentation of :cpp:any:`rule::Rule::makeInverse`/:py:obj:`Rule.makeInverse`,
  it throws :cpp:any:`LogicError`/:py:obj:`LogicError`
  not :cpp:any:`InputError`/:py:obj:`InputError`.
- Set the name of a rule from :cpp:any:`rule::Rule::makeInverse`/:py:obj:`Rule.makeInverse`
  to something more descriptive.
- Fix graph/rule depiction bug with non-zero rotation.
- Fix DG dump loading to also load derivations with no rules.
- Don't crash when trying :cpp:any:`dg::DG::findVertex`/:py:obj:`DG.findVertex`
  with a graph not in the derivation graph.
- Don't crash when trying to print derivations with multiple rules.
- Fix documentation formatting errors.
- #2, throw exceptions from ``dg::DG::ruleComp``/``dgRuleComp``
  and ``dg::DG::calc``/``DG.calc`` when isomorphic graphs are given.
- Throw more appropriate exception if :cpp:any:`dg::DG::print`/:py:obj:`DG.print`
  is called before ``dg::DG::calc``/``DG.calc``.
- Various issues in graph/rule depiction related to positioning of hydrogens, charges, etc.
- Build system: give better error messages if a file is given where a path is expected.
- The produced SMILES strings are now truely canonical, as the new
  `GraphCanon <https://github.com/jakobandersen/graph_canon>`__ library is used.
- Fix :cpp:any:`AtomData`/:py:obj:`AtomData` to properly print radicals on uncharged atoms.
- Throw more friendly exceptions when loading graphs/rules that have loop/parallel edges.


Other
-----

- The documentation now has a "Known Issues" section, describing an issue where
  post-processing may fail if ``pdflatex`` has been upgraded after installation


v0.6.0 (2016-12-22)
===================

Incompatible Changes
--------------------

- A C++14 compiler is now required (e.g., GCC 5.1 or later).
- The required Boost version is now either 1.59 or at least 1.61.
  Version 1.60 do not work due to https://github.com/boostorg/python/issues/56.
- Make filenames in post-processing more Latex friendly.
- Rules specified in GML using ``constrainAdj`` must now enclose the operator in double quotes.  
- DG: remove most of the interface related to ``DerivationRef``.
  Use the graph interface instead. E.g.,

  - Deprecate ``DerivationRef``. They now interconvert with
    :cpp:any:`dg::DG::HyperEdge`/:py:obj:`DGHyperEdge`.
  - Change ``DG::getDerivationRef`` into :cpp:any:`dg::DG::findEdge`/:py:obj:`DG.findEdge`.
  - Make ``DG.derivations`` return the edges instead in the Python interface.
    It is removed in the C++ interface. It will be removed from Python in the future.
  - Remove ``DG::inDerivations`` and ``DG::outDerivations``.
  - Deprecate ``DG::vertexGraphs`` in Python, remove in C++.

- Move the graph interface for DG and Graph into separate headers:
  ``DGGraphInterface.h`` and ``GraphGraphInterface.h``.
- Move ``GraphPrinter`` into a separate header.
- Move ``DGPrinter`` and ``DGPrintData`` into a separate header.
- All SBML features have been removed from the library.
- The deprecated ``DG::printMatrix`` function has been removed.
- ``dg::DG::calc``/``DG.calc`` will no longer print a message when it is done.
- :cpp:any:`dg::DG::print`/:py:obj:`DG.print` by default now only prints the hypergraph rendering.
  (For now, set ``Config::DG::printNonHyper`` to enable printing of the non-hypergraph rendering)
- :cpp:any:`graph::Graph::print`/:py:obj:`Graph.print` and
  :cpp:any:`rule::Rule::print`/:py:obj:`Rule.print` will now emit only one depiction when
  the two printers are equal.
- :cpp:any:`rule::Rule::print`/:py:obj:`Rule.print`, change the default colours used to indicate
  changes. Now different colours are used in L, K, R.
- :py:obj:`DGHyperEdge.print`, change the default match colour.
- Add < operator to vertices and edges of Graph, Rule, and DG.


New Features
------------

- :cpp:any:`rule::Rule::makeInverse`/:py:obj:`Rule.makeInverse`.
- Reimplementation of GML parsing. It is now less strict with respect to ordering.
- Rule application constraint that checks the shortest path between two given vertices.
- Interactive mode for the wrapper script (option ``-i``).
  It will use IPython as interpreter if it is available.
- The molecule model now includes radicals. The SMILES format has been extended to support
  radicals as well.
- Plugin infrastructure to load additional Python modules when using the wrapper script.
- Graph interface for rules: for a rule :math:`L \leftarrow K\rightarrow R``, the three graphs
  can be accessed. The rule it self acts as the graph that is the pushout of the rule span.
- Graph loading: the ids used in GML and GraphDFS, as well as the class labels in SMILES can now
  be converted into vertices for the loaded graphs. If the class labels of a SMILES string are not
  unique, then none of them are available for querying.
- Add ``-v`` as alias for ``--version`` in the wrapper script.
- Add quite mode, ``-q``, to the wrapper script.
- Add :cpp:any:`graph::Printer::setRotation`/:py:obj:`GraphPrinter.rotation`.
  Internally computed coordinates will be rotated by the set amount of degrees.
 

Bugs Fixed
----------

- ``operator<<`` for Derivation: only try to print the rule if there is one.
- Properly throw an exception when graph GML parsing fails.
- Don't throw an exception while throwing an exception when graphs are disconnected.
- Fix bug in checking of certain ``constrainAdj`` during certain types of rule composition.
- Properly handle empty vertex/hyperedge ranges for DGs. Thanks to Robert Haas for reporting.


v0.5.0 (2016-03-07)
===================

Initial public version.
