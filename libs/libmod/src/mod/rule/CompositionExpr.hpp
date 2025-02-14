#ifndef MOD_RULE_COMPOSITIONEXPR_HPP
#define MOD_RULE_COMPOSITIONEXPR_HPP

#include <mod/BuildConfig.hpp>
#include <mod/graph/ForwardDecl.hpp>
#include <mod/rule/ForwardDecl.hpp>

#include <boost/variant/recursive_wrapper.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/variant.hpp>

#include <memory>
#include <vector>

namespace mod::rule::RCExp {
// rst:
// rst: In the ``rule::RCExp`` namespace the data structures and operators for representing rule composition expressions are defined.
// rst: An expression can be evaluated through the method :func:`rule::Composer::eval`. The result of an expression
// rst: is a list of rules.
// rst:
// rst: See also [AFMS-RC]_, [AFMS-RC-AtomMap]_, and [AFMS-RC-Matrix]_ for details on how these expressions are computed internally,
// rst: and further examples of how they can be used to solve particular problems.
// rst:
// Don't use the rst-class stuff, as they are all so small.
//
// rst: .. class:: rule::RCExp::Union
// rst:
// rst:		When evaluated, the subexpressions are evaluated and the results are combined.
// rst:		The combination is either simple concatenation of lists, or taking the union of the subresults.
// rst:
struct MOD_DECL Union {
	// rst:		.. function:: Union(std::vector<Expression> exps)
	// rst:
	// rst:			:throws: :class:`LogicError` if `exps.empty()`.
	Union(std::vector<Expression> exps);
	// rst:		.. function::  friend std::ostream &operator<<(std::ostream &s, const Union &par)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const Union &par);
	// rst:		.. function::  const std::vector<Expression> &getExpressions() const
	const std::vector<Expression> &getExpressions() const;
private:
	std::vector<Expression> exps;
};

// rst: .. class:: rule::RCExp::Bind
// rst:
// rst:		When evaluated, returns a list with a single rule, :math:`\emptyset \leftarrow \emptyset \rightarrow G`,
// rst:		for the given graph :math:`G`.
// rst:
struct MOD_DECL Bind {
	// rst:		.. function:: Bind(std::shared_ptr<graph::Graph> g)
	// rst:
	// rst:			:throws: :class:`LogicError` if `!g`.
	Bind(std::shared_ptr<graph::Graph> g);
	// rst:		.. function:: friend std::ostream &operator<<(std::ostream &s, const Bind &b)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const Bind &b);
	// rst:		.. function:: std::shared_ptr<graph::Graph> getGraph() const
	std::shared_ptr<graph::Graph> getGraph() const;
private:
	std::shared_ptr<graph::Graph> g;
};

// rst: .. class:: rule::RCExp::Id
// rst:
// rst:		When evaluated, returns a list with a single rule, :math:`G \leftarrow G \rightarrow G`,
// rst:		for the given graph :math:`G`.
// rst:
struct MOD_DECL Id {
	// rst:		.. function:: Id(std::shared_ptr<graph::Graph> g)
	// rst:
	// rst:			:throws: :class:`LogicError` if `!g`.
	Id(std::shared_ptr<graph::Graph> g);
	// rst:		.. function:: friend std::ostream &operator<<(std::ostream &s, const Id &i)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const Id &i);
	// rst:		.. function:: std::shared_ptr<graph::Graph> getGraph() const
	std::shared_ptr<graph::Graph> getGraph() const;
private:
	std::shared_ptr<graph::Graph> g;
};

// rst: .. class:: rule::RCExp::Unbind
// rst:
// rst:		When evaluated, returns a list with a single rule, :math:`G \leftarrow \emptyset \rightarrow \emptyset`,
// rst:		for the given graph :math:`G`.
// rst:
struct MOD_DECL Unbind {
	// rst:		.. function:: Unbind(std::shared_ptr<graph::Graph> g)
	// rst:
	// rst:			:throws: :class:`LogicError` if `!g`.
	Unbind(std::shared_ptr<graph::Graph> g);
	// rst:		.. function:: friend std::ostream &operator<<(std::ostream &s, const Unbind &u)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const Unbind &u);
	// rst:		.. function:: std::shared_ptr<graph::Graph> getGraph() const
	std::shared_ptr<graph::Graph> getGraph() const;
private:
	std::shared_ptr<graph::Graph> g;
};

class ComposeCommon;
class ComposeParallel;
class ComposeSub;
class ComposeSuper;

// rst: .. class:: rule::RCExp::Expression
// rst:
// rst:		A class containing any type of the rule composition expression.
// rst:		That is, it acts as a variant.
// rst:
struct MOD_DECL Expression {
	// rst:		.. function:: Expression(std::shared_ptr<Rule> r)
	// rst:		              Expression(Union u)
	// rst:		              Expression(Bind bind)
	// rst:		              Expression(Id id)
	// rst:		              Expression(Unbind unbind)
	// rst:		              Expression(ComposeCommon compose)
	// rst:		              Expression(ComposeParallel compose)
	// rst:		              Expression(ComposeSub compose)
	// rst:		              Expression(ComposeSuper compose)
	// rst:
	// rst:			:throws: :class:`LogicError` if `!r`.
	Expression(std::shared_ptr<Rule> r);
	Expression(Union u);
	Expression(Bind bind);
	Expression(Id id);
	Expression(Unbind unbind);
	Expression(ComposeCommon compose);
	Expression(ComposeParallel compose);
	Expression(ComposeSub compose);
	Expression(ComposeSuper compose);

	template<typename Visitor>
	typename Visitor::result_type applyVisitor(Visitor visitor) const {
		return boost::apply_visitor(visitor, data);
	}

	// rst:		.. function:: friend std::ostream &operator<<(std::ostream &s, const Expression &exp)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const Expression &exp);
private:
	boost::variant<std::shared_ptr<Rule>, Union, Bind, Id, Unbind,
	               boost::recursive_wrapper<ComposeCommon>,
	               boost::recursive_wrapper<ComposeParallel>,
	               boost::recursive_wrapper<ComposeSub>,
	               boost::recursive_wrapper<ComposeSuper>
	> data;
};

// rst: .. class:: rule::RCExp::ComposeCommon
// rst:
// rst:		When evaluated, evaluate the `first` and `second` expressions to obtain list of rules :math:`P_1` and :math:`P_2`.
// rst:		For each pair of rules
// rst:		:math:`p_1 = (L_1\leftarrow K_1\rightarrow R_1) \in P_1` and
// rst:		:math:`p_2 = (L_2\leftarrow K_2\rightarrow R_2) \in P_2`,
// rst:		compose them by common subgraphs of :math:`R_1` and :math:`L_2`,
// rst:		possibly limited to connected subgraphs or to the subgraphs of maximum size.
// rst:		By default the empty subgraph is not considered, but can be enabled to be.
// rst:
struct MOD_DECL ComposeCommon {
	// rst:		.. function:: ComposeCommon(Expression first, Expression second, bool maximum, bool connected, bool includeEmpty)
	ComposeCommon(Expression first, Expression second, bool maximum, bool connected, bool includeEmpty);
	// rst:		.. function:: friend std::ostream &operator<<(std::ostream &s, const ComposeCommon &c)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const ComposeCommon &c);
public:
	// rst:		.. var:: Expression first
	// rst:		         Expression second
	Expression first, second;
	// rst:		.. var:: bool maximum
	// rst:		         bool connected
	// rst:		         bool includeEmpty
	bool maximum, connected, includeEmpty;
};

// rst: .. class:: rule::RCExp::ComposeParallel
// rst:
// rst:		When evaluated, evaluate the `first` and `second` expressions to obtain list of rules :math:`P_1` and :math:`P_2`.
// rst:		For each pair of rules
// rst:		:math:`p_1 = (L_1\leftarrow K_1\rightarrow R_1) \in P_1` and
// rst:		:math:`p_2 = (L_2\leftarrow K_2\rightarrow R_2) \in P_2`,
// rst:		compose them by the empty overlap of :math:`R_1` and :math:`L_2`.
// rst:
struct MOD_DECL ComposeParallel {
	// rst:		.. function:: ComposeParallel(Expression first, Expression second)
	ComposeParallel(Expression first, Expression second);
	// rst:		.. function:: friend std::ostream &operator<<(std::ostream &s, const ComposeParallel &c)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const ComposeParallel &c);
public:
	// rst:		.. var:: Expression first
	// rst:		         Expression second
	Expression first, second;
};

// rst: .. class:: rule::RCExp::ComposeSub
// rst:
// rst:		When evaluated, evaluate the `first` and `second` expressions to obtain list of rules :math:`P_1` and :math:`P_2`.
// rst:		For each pair of rules
// rst:		:math:`p_1 = (L_1\leftarrow K_1\rightarrow R_1) \in P_1` and
// rst:		:math:`p_2 = (L_2\leftarrow K_2\rightarrow R_2) \in P_2`,
// rst:		compose them by common subgraphs of :math:`R_1` and :math:`L_2` such that :math:`R_1` is the common subgraph,
// rst:		i.e., that :math:`R_1` is matched as a subgraph of :math:`L_2`.
// rst:
// rst:		If ``allowPartial`` is set `true`, then each non-empty subset of connected components of :math:`R_1` are matched
// rst:		as a subgraph of :math:`L_2`.
// rst:
struct MOD_DECL ComposeSub {
	// rst:		.. function:: ComposeSub(Expression first, Expression second, bool allowPartial)
	ComposeSub(Expression first, Expression second, bool allowPartial);
	// rst:		.. function:: friend std::ostream &operator<<(std::ostream &s, const ComposeSub &c)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const ComposeSub &c);
public:
	// rst:		.. var:: Expression first
	// rst:		         Expression second
	Expression first, second;
	// rst:		.. var:: bool allowPartial
	bool allowPartial;
};

// rst: .. class:: rule::RCExp::ComposeSuper
// rst:
// rst:		When evaluated, evaluate the `first` and `second` expressions to obtain list of rules :math:`P_1` and :math:`P_2`.
// rst:		For each pair of rules
// rst:		:math:`p_1 = (L_1\leftarrow K_1\rightarrow R_1) \in P_1` and
// rst:		:math:`p_2 = (L_2\leftarrow K_2\rightarrow R_2) \in P_2`,
// rst:		compose them by common subgraphs of :math:`R_1` and :math:`L_2` such that :math:`L_2` is the common subgraph,
// rst:		i.e., that :math:`L_2` is matched as a subgraph of :math:`R_1`.
// rst:
// rst:		If ``allowPartial`` is set `true`, then each non-empty subset of connected components of :math:`L_2` are matched
// rst:		as a subgraph of :math:`R_1`.
// rst:
// TODO: enforceConstraints should really be deleted when the constraint system is revamped
struct MOD_DECL ComposeSuper {
	// rst:		.. function:: ComposeSuper(Expression first, Expression second, bool allowPartial, bool enforceConstraints)
	ComposeSuper(Expression first, Expression second, bool allowPartial, bool enforceConstraints);
	// rst:		.. function:: friend std::ostream &operator<<(std::ostream &s, const ComposeSuper &c)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const ComposeSuper &c);
public:
	// rst:		.. var:: Expression first
	// rst:		         Expression second
	Expression first, second;
	// rst:		.. var:: bool allowPartial
	// rst:		         bool enforceConstraints
	bool allowPartial, enforceConstraints;
};

} // namespace mod::rule::RCExp

#endif // MOD_RULE_COMPOSITIONEXPR_HPP