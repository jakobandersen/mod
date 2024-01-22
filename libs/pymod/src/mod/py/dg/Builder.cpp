#include <mod/py/Common.hpp>

#include <mod/Derivation.hpp>
#include <mod/dg/Builder.hpp>

namespace mod::dg::Py {
namespace {

// see https://stackoverflow.com/questions/19062657/is-there-a-way-to-wrap-the-function-return-value-object-in-python-using-move-i

std::shared_ptr<ExecuteResult>
Builder_execute(std::shared_ptr<Builder> b, std::shared_ptr<Strategy> strategy, int verbosity,
                bool ignoreRuleLabelTypes) {
	return std::make_shared<ExecuteResult>(b->execute(strategy, verbosity, ignoreRuleLabelTypes));
}

} // namespace

void Builder_doExport() {
	using AddDerivation = DG::HyperEdge (Builder::*)(const Derivations &, IsomorphismPolicy);
	using AddHyperEdge = DG::HyperEdge (Builder::*)(const DG::HyperEdge &, IsomorphismPolicy);
	using Apply = std::vector<DG::HyperEdge> (Builder::*)(const std::vector<std::shared_ptr<graph::Graph> > &,
	                                                      std::shared_ptr<rule::Rule>, bool,
	                                                      int, IsomorphismPolicy);
	// rst: .. class:: DGBuilder
	// rst:
	// rst:		An RAII-style object obtained from :meth:`DG.build`.
	// rst:		On destruction of an active builder object the owning :class:`DG` will be locked
	// rst:		for further modifications.
	// rst:
	// rst:		The object can be used as a context manager:
	// rst:
	// rst:		.. code-block:: python
	// rst:
	// rst:			dg = DG()
	// rst:			with dg.build() as b:
	// rst:				# b is a DGBuilder
	// rst:			# b has now been destructed and dg is locked.
	// rst:
	// rst:		Otherwise one can manually use ``del`` on the obtained builder to trigger the destruction.
	// rst:
	py::class_<Builder, std::shared_ptr<Builder>, boost::noncopyable>("_DGBuilder", py::no_init)
			// rst:		.. attribute:: dg
			// rst:
			// rst:			The derivation graph this builder can modify.
			// rst:
			// rst:			:type: DG
			.add_property("dg", &Builder::getDG)
					// rst:		.. attribute:: isActive
					// rst:
					// rst:			Whether this object is associated with a :py:class:`DG`.
					// rst:
					// rst:			:type: bool
			.add_property("isActive", &Builder::isActive)
					// rst:		.. method:: addDerivation(d, graphPolicy=IsomorphismPolicy.Check)
					// rst:
					// rst:			Adds a hyperedge corresponding to the given derivation to the associated :class:`DG`.
					// rst:			If it already exists, only add the given rules to the edge.
					// rst:
					// rst:			:param Derivations d: a derivation to add a hyperedge for.
					// rst:			:param IsomorphismPolicy graphPolicy: the isomorphism policy for adding the given graphs.
					// rst:			:returns: the hyperedge corresponding to the given derivation.
					// rst:			:rtype: DGHyperEdge
					// rst:			:raises: :class:`LogicError` if ``d.left`` or ``d.right`` is empty.
					// rst:			:raises: :class:`LogicError` if a ``None``is in ``d.left``, ``d.right``, or ``d.rules``.
					// rst:			:raises: :class:`LogicError` if ``graphPolicy == IsomorphismPolicy.Check`` and a given graph object
					// rst:				is different but isomorphic to another given graph object or to a graph object already
					// rst:				in the internal graph database in the associated derivation graph.
			.def("addDerivation", static_cast<AddDerivation>(&Builder::addDerivation))
					// rst:		.. method:: addHyperEdge(e, graphPolicy=IsomorphismPolicy.Check)
					// rst:
					// rst:			Adds a hyperedge to the associated :class:`DG` from a copy of the given hyperedge
					// rst:			(from a different :class:`DG`).
					// rst:			If it already exists, only add the rules to the edge.
					// rst:
					// rst:			:param DGHyperEdge e: a hyperedge to copy.
					// rst:			:param IsomorphismPolicy graphPolicy: the isomorphism policy for adding the given graphs.
					// rst:			:returns: the hyperedge corresponding to the copy of the given hyperedge.
					// rst:			:rtype: DGHyperEdge
					// rst:			:raises: :class:`LogicError` if ``e`` is a null edge.
					// rst:			:raises: :class:`LogicError` if ``graphPolicy == IsomorphismPolicy.Check`` and a given graph object
					// rst:				is different but isomorphic to another given graph object or to a graph object already
					// rst:				in the internal graph database in the associated derivation graph.
			.def("addHyperEdge", static_cast<AddHyperEdge>(&Builder::addHyperEdge))
					// rst:		.. method:: execute(strategy, *, verbosity=2, ignoreRuleLabelTypes=False)
					// rst:
					// rst:			Execute the given strategy (:ref:`dgStrat`) and as a side effect add
					// rst:			vertices and hyperedges to the underlying derivation graph.
					// rst:
					// rst:			:param DGStrat strategy: the strategy to execute.
					// rst:			:param int verbosity: the level of verbosity of printed information during calculation.
					// rst:				See :cpp:func:`dg::Builder::execute` for explanations of the levels.
					// rst:			:param bool ignoreRuleLabelTypes: whether rules in the strategy should be checked beforehand for
					// rst:				whether they have an associated :class:`LabelType` which matches the one in the underlying derivation graph.
					// rst:			:returns: a proxy object for accessing the result of the execution.
					// rst:			:rtype: DGExecuteResult
					// rst:			:throws: :class:`LogicError` if a static "add" strategy has :attr:`IsomorphismPolicy.Check` as graph policy,
					// rst:				and it tries to add a graph object isomorphic to an already known, but different, graph object in the database.
					// rst:				This is checked before execution, so there is strong exception guarantee.
					// rst:			:throws: :class:`LogicError` if a dynamic "add" strategy has :attr:`IsomorphismPolicy.Check` as graph policy,
					// rst:				and it tries to add a graph object isomorphic to an already known, but different, graph object in the database.
					// rst:
					// rst:				.. warning:: This is checked during execution, so while the basic exception guarantee is provided,
					// rst:					there may be modifications to the underlying derivation graph.
					// rst:			:throws: :class:`LogicError` if a dynamic "add" strategy is executed where a returned graph is ``None``.
					// rst:
					// rst:				.. warning:: This is checked during execution, so while the basic exception guarantee is provided,
					// rst:					there may be modifications to the underlying derivation graph.
					// rst:			:throws: :class:`LogicError`: if ``ignoreRuleLabelTypes`` is ``False``, which is the default,
					// rst:				and a rule in the given strategy has an associated :class:`LabelType` which is different from the one
					// rst:				in the derivation graph.
			.def("execute", &Builder_execute)
					// rst:		.. method:: apply(graphs, r, onlyProper=True, verbosity=0, graphPolicy=IsomorphismPolicy.Check)
					// rst:
					// rst:			Compute direct derivations.
					// rst:
					// rst:			:param graphs: the graphs constituting the left-hand side of the computed direct derivations.
					// rst:			:type graphs: list[Graph]
					// rst:			:param bool onlyProper: when ``True``, then all of ``graphs`` must be used in each direct derivation.
					// rst:			:param Rule r: the rule to use for the direct derivations.
					// rst:			:param int verbosity: the level of verbosity of printed information during calculation.
					// rst:				See :cpp:func:`dg::Builder::apply` for explanations of the levels.
					// rst:			:param IsomorphismPolicy graphPolicy: the isomorphism policy for adding the given graphs.
					// rst:
					// rst:			:returns: a list of hyper edges representing the found direct derivations.
					// rst:				The list may contain duplicates if there are multiple ways of constructing
					// rst:				the same direct derivation when ignoring the specific match morphism.
					// rst:			:rtype: list[DGHyperEdge]
					// rst:			:raises: :class:`LogicError` if there is a ``None`` in ``graphs``.
					// rst:			:raises: :class:`LogicError` if ``r`` is ``None``.
					// rst:			:raises: :class:`LogicError` if ``graphPolicy == IsomorphismPolicy.Check`` and a given graph object
					// rst:				is different but isomorphic to another given graph object or to a graph object already
					// rst:				in the internal graph database in the associated derivation graph.
			.def("apply", static_cast<Apply>(&Builder::apply))
					// rst:		.. method:: addAbstract(description)
					// rst:
					// rst:			Add vertices and hyperedges based on the given abstract description.
					// rst:			The description must adhere to the grammar described at :ref:`dg_abstract-desc`.
					// rst:
					// rst:			For each vertex named in the description a graph object with a single vertex will be created.
					// rst:			The label of that vertex and the name of the graph is set to the given identifier.
					// rst:
					// rst:			:param str description: the description to parse into abstract derivations.
					// rst:			:raises: :class:`InputError` if the description could not be parsed.
			.def("addAbstract", &Builder::addAbstract)
					// rst:		.. method:: load(ruleDatabase, f, verbosity=2)
					// rst:
					// rst:			Load and add a derivation graph dump.
					// rst:			Use :func:`DG.load` to load a dump as a locked derivation graph.
					// rst:
					// rst:			The label settings of this DG and the ones retrieved from the dump file must match.
					// rst:			Vertices with graphs and hyperedges with rules are then added from the dump.
					// rst:			Any graph in the dump which is isomorphic to a graph in the internal graph database of the DG
					// rst:			is replaced by the given graph.
					// rst:			The same procedure is done for the rules, but compared against the given rules.
					// rst:			If a graph/rule is not found in the given lists, a new object is instantiated and used.
					// rst:
					// rst:			See :cpp:func:`dg::DG::load` for an explanation of the verbosity levels.
					// rst:
					// rst:			:param ruleDatabase: A list of rules used as explained above.
					// rst:			:type ruleDatabase: list[Rule]
					// rst:			:param f: a DG dump file to load.
					// rst:			:type f: str or CWDPath
					// rst:			:raises: :class:`LogicError` if there is a ``None`` in ``ruleDatabase``.
					// rst:			:raises: :class:`LogicError` if the label settings of the dump does not match those of this DG.
					// rst: 			:raises: :class:`InputError` if the file can not be opened or its content is bad.
			.def("load", &Builder::load);

	// rst: .. class:: DGExecuteResult
	// rst:
	// rst:		The result from calling :func:`DGBuilder.execute`.
	// rst:
	py::class_<ExecuteResult, std::shared_ptr<ExecuteResult>, boost::noncopyable>("DGExecuteResult", py::no_init)
			// rst:		.. attribute:: subset
			// rst:		                  universe
			// rst:
			// rst:			(Read-only) Respectively the subset and the universe computed
			// rst:			by the strategy execution (see also :ref:`dgStrat`).
			// rst:
			// rst:			:type: list[Graph]
			.add_property("subset", py::make_function(&ExecuteResult::getSubset,
			                                          py::return_value_policy<py::copy_const_reference>()))
			.add_property("universe", py::make_function(&ExecuteResult::getUniverse,
			                                            py::return_value_policy<py::copy_const_reference>()))
					// rst:		.. method:: list(*, withUniverse=False)
					// rst:
					// rst:			Output information from the execution of the strategy.
					// rst:
					// rst:			:param bool withUniverse: The universe lists can be rather long. As default, they are omitted when listing.
			.def("list", &ExecuteResult::list);
}

} // namespace mod::dg::Py