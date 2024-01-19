#include <mod/py/Common.hpp>

#include <mod/rule/Composer.hpp>
#include <mod/rule/CompositionExpr.hpp>

#include <vector>

// rst: This page contains the core functionality for composing rules and creating rules from graphs.
// rst: Note that there is both a low-level interface for constructing expressions,
// rst: and an embedded mini-language for defining the expressions using normal Python operators.
// rst:
// rst: The Embedded Rule Composition Expression Language
// rst: #################################################
// rst:
// rst: The embedded language is really a collection of proxy classes with a lot of operator overloading, thus
// rst: the normal syntax and semantics of Python applies.
// rst:
// rst: A rule composition expression always returns a list of rules when evaluated.
// rst: The following is the grammar for the expressions.
// rst:
// rst: .. productionlist:: rcExp
// rst:    rcExp: `rcExp` `op` `rcExp`
// rst:         : "rcBind(" graphs ")"
// rst:         : "rcUnbind(" graphs ")"
// rst:         : "rcId(" graphs ")"
// rst:         : rules
// rst:    op: "*" `opObject` "*"
// rst:    opObject: "rcParallel"
// rst:            : "rcSuper(allowPartial=False)"
// rst:            : "rcSuper"
// rst:            : "rcSub(allowPartial=False)"
// rst:            : "rcSub"
// rst:            : "rcCommon"
// rst:
// rst: Here a ``graphs`` is any Python expression that is either a single :class:`Graph` or an iterable of graphs.
// rst: Similarly, ``rules`` must be either a :class:`Rule` or an iterable of rules.
// rst: An :token:`~rcExp:rcExp` may additionally be an iterable of expressions.
// rst: See the API below for more details on the semantics of each expression,
// rst: and the corresponding :ref:`C++ page<cpp-rule/CompositionExpr>`.
// rst:

namespace mod::rule::Py {
namespace {

std::vector<std::shared_ptr<Rule>> getRuleDatabase(std::shared_ptr<Composer> rc) {
	return std::vector<std::shared_ptr<Rule>>(begin(rc->getRuleDatabase()), end(rc->getRuleDatabase()));
}

std::vector<std::shared_ptr<Rule>> getProducts(std::shared_ptr<Composer> rc) {
	return std::vector<std::shared_ptr<Rule>>(begin(rc->getProducts()), end(rc->getProducts()));
}

std::shared_ptr<Composer> create(const std::vector<std::shared_ptr<Rule>> dVec, LabelSettings labelSettings) {
	std::unordered_set<std::shared_ptr<Rule>> dUSet(begin(dVec), end(dVec));
	return Composer::create(dUSet, labelSettings);
}

} // namespace 

void Composition_doExport() {
	// rst: Expression Evaluator
	// rst: #################################################
	// rst:
	// rst: .. class:: RCEvaluator
	// rst:
	// rst:		This class can evaluate rule composition expressions. During evaluation an expression graph is recorded.
	// rst: 
	// rst:		The expression graph is a directed hypergraph :math:`(V, E)`, with each vertex representing a rule.
	// rst:		Each edge represent all compositions calculated for a unique input. That is every edge :math:`e\in E` is on the form
	// rst:		:math:`((u, v), R_e)` with :math:`(u, v)\in V\times V` as an *ordered* pair of rules and :math:`R_e\subseteq V` is the set
	// rst:		of all resulting rules found.
	// rst:
	// rst:		The graph is visualised as a bipartite graph with point-shaped vertices representing the hyperedges. The in-edges to these
	// rst:		hyperedge vertices are labelled with 1 and 2.
	// rst:
	py::class_<Composer, std::shared_ptr<Composer>, boost::noncopyable>("RCEvaluator", py::no_init)
			// rst:		.. attribute:: ruleDatabase
			// rst:
			// rst:			(Read-only) The list of unique rules known by the evaluator.
			// rst:
			// rst:			:type: list[Rule]
			.add_property("_ruleDatabase", &getRuleDatabase)
					// rst:		.. attribute:: products
					// rst:
					// rst:			(Read-only) The list of unique rules this evaluator has constructed.
					// rst:
					// rst:			:type: list[Rule]
			.add_property("_products", &getProducts)
					// rst:		.. method:: eval(exp, *, onlyUnique=True, verbosity=0)
					// rst:
					// rst:			Evaluates a rule composition expression.
					// rst:			Any created rule is replaced by a rule in the database if they are isomorphic.
					// rst:			A rule may appear multiple times in the result if multiple overlaps resulted in the same composed rule.
					// rst:
					// rst:			:param RCExpExp exp: the expression to evaluate.
					// rst:			:param bool onlyUnique: whether each composition (sub-)result may contain duplicates or not.
					// rst:			:param int verbosity: the level of information being printed about the evaluation.
					// rst:				See :cpp:func:`rule::Composer::eval` for details.
					// rst:			:returns: the resulting list of rules of the expression.
					// rst:			:rtype: list[Rule]
			.def("eval", &Composer::eval)
					// rst:		.. method:: print()
					// rst:
					// rst:			Print the graph representing all expressions evaluated so far.
			.def("print", &Composer::print);

	// rst: .. method:: rcEvaluator(database, labelSettings)
	// rst:
	// rst:		:param database: a list of isomorphic rules the evaluator will compare against.
	// rst:		:type database: list[Rule]
	// rst:		:param LabelSettings labelSettings: the settings to use for morphisms.
	// rst:		:returns: a rule composition expression evaluator.
	// rst:		:rtype: RCEvaluator
	// rst:
	// rst:		.. note:: The caller is responsible for ensuring the given rules are unique.
	// rst:
	py::def("_rcEvaluator", create);

	// rst: Rule Composition Expressions
	// rst: #################################################
	// rst:
	// rst: An expression, ``RCExpExp``, can be evaluated through the method :meth:`RCEvaluator.eval`.
	// rst: The result of an expression is a list of rules.
	// rst:

	// Nullary/unary
	//--------------------------------------------------------------------------
	// rst: .. class:: RCExpUnion
	// rst:
	// rst:		Return the union of the subexpressions. I.e., flatten the subresult lists into a single list.
	// rst:
	py::class_<RCExp::Union>("RCExpUnion", py::no_init)
			.def(py::init<std::vector<RCExp::Expression>>())
			.def(str(py::self));
	// rst: .. class:: RCExpBind
	// rst:
	// rst:		Return the singleton list with the rule :math:`(\emptyset, \emptyset, G)` for the given graph :math:`G`.
	// rst:
	py::class_<RCExp::Bind>("RCExpBind", py::no_init)
			.def(py::init<std::shared_ptr<graph::Graph>>())
			.def(str(py::self));
	// rst: .. class:: RCExpId
	// rst:
	// rst:		Return the singleton list with the rule :math:`(G, G, G)` for the given graph :math:`G`.
	// rst:
	py::class_<RCExp::Id>("RCExpId", py::no_init)
			.def(py::init<std::shared_ptr<graph::Graph>>())
			.def(str(py::self));
	// rst: .. class:: RCExpUnbind
	// rst:
	// rst:		Return the singleton list with the rule :math:`(G, \emptyset, \emptyset)` for the given graph :math:`G`.
	// rst:
	py::class_<RCExp::Unbind>("RCExpUnbind", py::no_init)
			.def(py::init<std::shared_ptr<graph::Graph>>())
			.def(str(py::self));

	// Expression
	//--------------------------------------------------------------------------
	// rst: .. class:: RCExpExp
	// rst:
	// rst:		The base class for the composition of two rule :math:`(L_1, K_1, R_1)` and :math:`(L_2, K_2, R_2)`.
	// rst:
	py::class_<RCExp::Expression>("RCExpExp", py::no_init)
			.def(str(py::self));

	// Binary
	//--------------------------------------------------------------------------
	// rst: .. class:: RCExpComposeCommon
	// rst:
	// rst:		Compose the rules by all common subgraphs of :math:`R_1` and :math:`L_2`,
	// rst:		possibly limited to connected subgraphs or to the subgraphs of maximum size.
	// rst:		By default the empty overlap is not considered, but can be enabled to be.
	// rst:
	py::class_<RCExp::ComposeCommon>("RCExpComposeCommon", py::no_init)
			.def(py::init<RCExp::Expression, RCExp::Expression, bool, bool, bool, bool>())
			.def(str(py::self));
	// rst: .. class:: RCExpComposeParallel
	// rst:
	// rst:		Compose the rules by all common subgraphs of :math:`R_1` and :math:`L_2`,
	// rst:		possibly limited to connected subgraphs or to the subgraphs of maximum size.
	// rst:
	py::class_<RCExp::ComposeParallel>("RCExpComposeParallel", py::no_init)
			.def(py::init<RCExp::Expression, RCExp::Expression, bool>())
			.def(str(py::self));
	// rst: .. class:: RCExpComposeSub
	// rst:
	// rst:		Compose the rules by the empty graph, i.e., create a rule representing the parallel application of two input rules.
	// rst:
	py::class_<RCExp::ComposeSub>("RCExpComposeSub", py::no_init)
			.def(py::init<RCExp::Expression, RCExp::Expression, bool, bool>())
			.def(str(py::self));
	// rst: .. class:: RCExpComposeSuper
	// rst:
	// rst:		Compose the rules such that overlapping connected components of :math:`R_1` and :math:`L_2` have the :math:`L_2` component as a subgraph of :math:`R_1`.
	// rst:		The overlap is `partial` if not every connected component of :math:`L_2` is participating in the common subgraph.
	// rst:
	py::class_<RCExp::ComposeSuper>("RCExpComposeSuper", py::no_init)
			.def(py::init<RCExp::Expression, RCExp::Expression, bool, bool, bool>())
			.def(str(py::self));

	py::implicitly_convertible<std::shared_ptr<Rule>, RCExp::Expression>();
	py::implicitly_convertible<RCExp::Union, RCExp::Expression>();
	py::implicitly_convertible<RCExp::Bind, RCExp::Expression>();
	py::implicitly_convertible<RCExp::Id, RCExp::Expression>();
	py::implicitly_convertible<RCExp::Unbind, RCExp::Expression>();
	py::implicitly_convertible<RCExp::ComposeCommon, RCExp::Expression>();
	py::implicitly_convertible<RCExp::ComposeParallel, RCExp::Expression>();
	py::implicitly_convertible<RCExp::ComposeSub, RCExp::Expression>();
	py::implicitly_convertible<RCExp::ComposeSuper, RCExp::Expression>();
}

} // namespace mod::rule::Py