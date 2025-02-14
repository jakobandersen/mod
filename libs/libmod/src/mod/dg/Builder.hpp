#ifndef MOD_DG_BUILDER_HPP
#define MOD_DG_BUILDER_HPP

#include <mod/BuildConfig.hpp>
#include <mod/dg/ForwardDecl.hpp>
#include <mod/dg/GraphInterface.hpp>

#include <memory>

namespace mod {
struct Derivations;
template<typename T> struct Function;
} // namespace mod
namespace mod::lib::DG {
struct ExecuteResult;
struct AddAbstractResult;
} // namespace mod::lib::DG
namespace mod::dg {

// rst-class: dg::Builder
// rst:
// rst:		An RAII-style object obtained from :cpp:func:`DG::build`.
// rst:		On destruction of an active builder object the owning :cpp:class:`DG` will be locked
// rst:		for further modifications.
// rst:		Builder objects are move-only types, and a moved from object becomes an inactive builder,
// rst:		not associated with any :cpp:class:`DG`.
// rst:
// rst-class-start:
class MOD_DECL Builder {
	friend class DG;
	explicit Builder(lib::DG::NonHyperBuilder &dg_,
					 std::shared_ptr<Function<void(dg::DG::Vertex)>> onNewVertex,
					 std::shared_ptr<Function<void(dg::DG::HyperEdge)>> onNewHyperEdge);
public:
	Builder(Builder &&other);
	Builder &operator=(Builder &&other);
	~Builder();
	// rst: .. function:: std::shared_ptr<DG> getDG() const
	// rst:
	// rst:		:returns: the derivation graph this builder can modify.
	std::shared_ptr<DG> getDG() const;
	// rst: .. function:: bool isActive() const
	// rst:
	// rst:		:returns: whether this object is associated with a :cpp:class:`DG`.
	bool isActive() const;
public:
	// rst: .. function:: DG::HyperEdge addDerivation(const Derivations &d)
	// rst:               DG::HyperEdge addDerivation(const Derivations &d, IsomorphismPolicy graphPolicy)
	// rst:
	// rst:		Adds a hyperedge corresponding to the given derivation to the associated :class:`DG`.
	// rst:		If it already exists, only add the given rules to the edge.
	// rst:		The given :var:`graphPolicy` refers to adding the graphs in :var:`d`,
	// rst:		and it defaults to :enumerator:`IsomorphismPolicy::Check`.
	// rst:
	// rst:		:returns: the hyperedge corresponding to the given derivation.
	// rst:		:throws: :class:`LogicError` if `!isActive()`.
	// rst:		:throws: :class:`LogicError` if `d.left.empty()`.
	// rst:		:throws: :class:`LogicError` if `d.right.empty()`.
	// rst:		:throws: :class:`LogicError` if a `nullptr` is in `d.left`, `d.right`, or `d.rules`.
	// rst:		:throws: :class:`LogicError` if `graphPolicy == IsomorphismPolicy::Check` and a given graph object
	// rst:			is different but isomorphic to another given graph object or to a graph object already
	// rst:			in the internal graph database in the associated derivation graph.
	DG::HyperEdge addDerivation(const Derivations &d);
	DG::HyperEdge addDerivation(const Derivations &d, IsomorphismPolicy graphPolicy);
	// rst: .. function:: DG::HyperEdge addHyperEdge(const DG::HyperEdge &e)
	// rst:               DG::HyperEdge addHyperEdge(const DG::HyperEdge &e, IsomorphismPolicy graphPolicy)
	// rst:
	// rst:		Adds a hyperedge to the associated :class:`DG` from a copy of the given hyperedge
	// rst:		(from a different :class:`DG`).
	// rst:		If it already exists, only add the rules to the edge.
	// rst:		The given :var:`graphPolicy` refers to adding the graphs associated with :var:`e`,
	// rst:		and it defaults to :enumerator:`IsomorphismPolicy::Check`.
	// rst:
	// rst:		:returns: the hyperedge corresponding to the given derivation.
	// rst:		:throws: :class:`LogicError` if `!isActive()`.
	// rst:		:throws: :class:`LogicError` if `!e`.
	// rst:		:throws: :class:`LogicError` if `graphPolicy == IsomorphismPolicy::Check` and a given graph object
	// rst:			is different but isomorphic to another given graph object or to a graph object already
	// rst:			in the internal graph database in the associated derivation graph.
	DG::HyperEdge addHyperEdge(const DG::HyperEdge &e);
	DG::HyperEdge addHyperEdge(const DG::HyperEdge &e, IsomorphismPolicy graphPolicy);
	// rst: .. function:: ExecuteResult execute(std::shared_ptr<Strategy> strategy)
	// rst:               ExecuteResult execute(std::shared_ptr<Strategy> strategy, int verbosity)
	// rst:               ExecuteResult execute(std::shared_ptr<Strategy> strategy, int verbosity, bool ignoreRuleLabelTypes)
	// rst:
	// rst:		Execute the given strategy (:ref:`dgStrat`) and as a side effect add
	// rst:		vertices and hyperedges to the underlying derivation graph.
	// rst:
	// rst:		The :cpp:var:`verbosity` defaults to level 2.
	// rst:		The levels have the following meaning:
	// rst:
	// rst:		- 0 (or less): no information is printed.
	// rst:		- 2: Repetition strategies print information for each round.
	// rst:		- 4: All strategies print minimal information.
	// rst:		- 6: Derivation predicate strategies and filtering strategies also print their predicates.
	// rst:		- 8: Rule strategies print minimal information about graph binding.
	// rst:		- 10: Rule strategies print more information about graph binding, including failure due to derivation predicates.
	// rst:		- 50: Print information about morphism generation for rule composition.
	// rst:		- 60: Print rule composition information.
	// rst:
	// rst:		:throws: :class:`LogicError` if a static "add" strategy has `IsomorphismPolicy::Check` as graph policy,
	// rst:			and it tries to add a graph object isomorphic to an already known, but different, graph object in the database.
	// rst:			This is checked before execution, so there is strong exception guarantee.
	// rst:		:throws: :class:`LogicError` if a dynamic "add" strategy has `IsomorphismPolicy::Check` as graph policy,
	// rst:			and it tries to add a graph object isomorphic to an already known, but different, graph object in the database.
	// rst:
	// rst:			.. warning:: This is checked during execution, so while the basic exception guarantee is provided,
	// rst:				there may be modifications to the underlying derivation graph.
	// rst:		:throws: :class:`LogicError` if a dynamic "add" strategy is executed where a returned graph is a `nullptr`.
	// rst:
	// rst:			.. warning:: This is checked during execution, so while the basic exception guarantee is provided,
	// rst:				there may be modifications to the underlying derivation graph.
	// rst:		:throws: :class:`LogicError`: if `ignoreRuleLabelTypes` is `false`, which is the default,
	// rst:			and a rule in the given strategy has an associated :enum:`LabelType` which is different from the one
	// rst:			in the derivation graph.
	ExecuteResult execute(std::shared_ptr<Strategy> strategy);
	ExecuteResult execute(std::shared_ptr<Strategy> strategy, int verbosity);
	ExecuteResult execute(std::shared_ptr<Strategy> strategy, int verbosity, bool ignoreRuleLabelTypes);
	// rst: .. function:: std::vector<DG::HyperEdge> apply(const std::vector<std::shared_ptr<graph::Graph> > &graphs, \
	// rst:                                                std::shared_ptr<rule::Rule> r)
	// rst:               std::vector<DG::HyperEdge> apply(const std::vector<std::shared_ptr<graph::Graph> > &graphs, \
	// rst:                                                std::shared_ptr<rule::Rule> r, bool onlyProper, \
	// rst:                                                int verbosity)
	// rst:               std::vector<DG::HyperEdge> apply(const std::vector<std::shared_ptr<graph::Graph> > &graphs, \
	// rst:                                                std::shared_ptr<rule::Rule> r, bool onlyProper, \
	// rst:                                                int verbosity, IsomorphismPolicy graphPolicy)
	// rst:
	// rst:		Compute direct derivations using `graphs` for the left-hand side and `r` as the rule.
	// rst:
	// rst:		When `onlyProper` is `true`, then all of `graphs` must be used in each direct derivation.
	// rst:		The default is `true`.
	// rst:
	// rst:		The given :var:`graphPolicy` refers to adding the graphs in :var:`graphs`,
	// rst:		and it defaults to :enumerator:`IsomorphismPolicy::Check`.
	// rst:
	// rst:		The :cpp:var:`verbosity` defaults to level 0.
	// rst:		The levels have the following meaning:
	// rst:
	// rst:		- 0 (or less): no information is printed.
	// rst:		- 2: Print minimal information about graph binding.
	// rst:		- 10: Print information about morphism generation for rule composition.
	// rst:		- 20: Print rule composition information.
	// rst:
	// rst:		:returns: a list of hyper edges representing the found direct derivations.
	// rst:			The list may contain duplicates if there are multiple ways of constructing
	// rst:			the same direct derivation when ignoring the specific match morphism.
	// rst:		:throws: :class:`LogicError` if there is a `nullptr` in `graphs`.
	// rst:		:throws: :class:`LogicError` if `r == nullptr`.
	// rst:		:throws: :class:`LogicError` if `graphPolicy == IsomorphismPolicy::Check` and a given graph object
	// rst:			is different but isomorphic to another given graph object or to a graph object already
	// rst:			in the internal graph database in the associated derivation graph.
	std::vector<DG::HyperEdge> apply(const std::vector<std::shared_ptr<graph::Graph> > &graphs,
	                                 std::shared_ptr<rule::Rule> r);
	std::vector<DG::HyperEdge> apply(const std::vector<std::shared_ptr<graph::Graph> > &graphs,
	                                 std::shared_ptr<rule::Rule> r, bool onlyProper,
	                                 int verbosity);
	std::vector<DG::HyperEdge> apply(const std::vector<std::shared_ptr<graph::Graph> > &graphs,
	                                 std::shared_ptr<rule::Rule> r, bool onlyProper,
	                                 int verbosity, IsomorphismPolicy graphPolicy);
	// rst: .. function:: AddAbstractResult addAbstract(const std::string &description)
	// rst:
	// rst:		Add vertices and hyperedges based on the given abstract description.
	// rst:		The description must adhere to the grammar described at :ref:`dg_abstract-desc`.
	// rst:
	// rst:		For each vertex named in the description a graph object with a single vertex will be created.
	// rst:		The label of that vertex and the name of the graph is set to the given identifier.
	// rst:
	// rst:		:throws: :class:`InputError` if the description could not be parsed.
	AddAbstractResult addAbstract(const std::string &description);
	// rst: .. function:: void load(const std::vector<std::shared_ptr<rule::Rule>> &ruleDatabase, \
	// rst:                         const std::string &file, int verbosity)
	// rst:
	// rst:		Load and add a derivation graph dump.
	// rst:		Use :cpp:func:`DG::load` to load a dump as a locked derivation graph.
	// rst:
	// rst:		The label settings of this DG and the ones retrieved from the dump file must match.
	// rst:		Vertices with graphs and hyperedges with rules are then added from the dump.
	// rst:		Any graph in the dump which is isomorphic to a graph in the internal graph database of the DG
	// rst:		is replaced by the given graph.
	// rst:		The same procedure is done for the rules, but compared against the given rules.
	// rst:		If a graph/rule is not found in the given lists, a new object is instantiated and used.
	// rst:
	// rst:		See :cpp:func:`DG::load` for an explanation of the verbosity levels.
	// rst:
	// rst:		:throws: :class:`LogicError` if there is a `nullptr` in `ruleDatabase`.
	// rst:		:throws: :class:`LogicError` if the label settings of the dump does not match those of this DG.
	// rst: 		:throws: :class:`InputError` if the file can not be opened or its content is bad.
	void load(const std::vector<std::shared_ptr<rule::Rule>> &ruleDatabase, const std::string &file, int verbosity);
private:
	struct Pimpl;
	std::unique_ptr<Pimpl> p;
};
// rst-class-end:

// rst-class: dg::ExecuteResult
// rst:
// rst:		The result from calling :func:`Builder::execute`.
// rst:
// rst-class-start:
class MOD_DECL ExecuteResult {
	friend class Builder;
	explicit ExecuteResult(std::shared_ptr<DG> dg_, lib::DG::ExecuteResult innerRes);
public:
	ExecuteResult(ExecuteResult &&other);
	ExecuteResult &operator=(ExecuteResult &&other);
	~ExecuteResult();
	// rst: .. function:: const std::vector<std::shared_ptr<graph::Graph>> &getSubset() const
	// rst:               const std::vector<std::shared_ptr<graph::Graph>> &getUniverse() const
	// rst:
	// rst:		:returns: respectively the subset and the universe computed by the strategy execution (see also :ref:`dgStrat`).
	const std::vector<std::shared_ptr<graph::Graph>> &getSubset() const;
	const std::vector<std::shared_ptr<graph::Graph>> &getUniverse() const;
	// rst: .. function:: void list(bool withUniverse) const
	// rst:
	// rst:		Output information from the execution of the strategy.
	// rst:		The universe lists can be rather long so with `withUniverse == false` they are omitted.
	void list(bool withUniverse) const;
private:
	struct Pimpl;
	std::unique_ptr<Pimpl> p;
};
// rst-class-end:

// rst-class: dg::AddAbstractResult
// rst:
// rst:		The result from calling :func:`Builder::addAbstract`.
// rst:
// rst-class-start:
class MOD_DECL AddAbstractResult {
	friend class Builder;
	explicit AddAbstractResult(std::shared_ptr<DG> dg_, lib::DG::AddAbstractResult innerRes);
public:
	AddAbstractResult(AddAbstractResult &&other);
	AddAbstractResult &operator=(AddAbstractResult &&other);
	~AddAbstractResult();
	// rst: .. function:: std::shared_ptr<graph::Graph> getGraph(const std::string &name)
	// rst:
	// rst:		:returns: the graph associated with the given name from the initial description,
	// rst:			or a null pointer if no such graph exists.
	std::shared_ptr<graph::Graph> getGraph(const std::string &name);
	// rst: .. function:: DG::HyperEdge getEdge(const std::string &name)
	// rst:
	// rst:		:returns: the hyperedge associated with the given name from the initial description,
	// rst:			or a null edge if such a hyperedge doesn't exist.
	// rst:			If the name refers to a bidirectional derivation, then the returned hyperedge is
	// rst:			for the left-to-right derivation. The other can be accessed as the inverse on that returned edge.
	DG::HyperEdge getEdge(const std::string &name);
private:
	struct Pimpl;
	std::unique_ptr<Pimpl> p;
};
// rst-class-end:


} // namespace mod::dg

#endif // MOD_DG_BUILDER_HPP