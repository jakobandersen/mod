#ifndef MOD_HYPERFLOW_LINEXPR_HPP
#define MOD_HYPERFLOW_LINEXPR_HPP

#include <mod/dg/GraphInterface.hpp>
#include <mod/hyperflow/ForwardDecl.hpp>
#include <mod/graph/ForwardDecl.hpp>

#include <iosfwd>
#include <memory>
#include <variant>

namespace mod::hyperflow {
// rst:
// rst: This header implements variable specifiers, linear expressions, and linear constraints.
// rst: See :ref:`flowCommon-varSpecLinExp` for a higher-level description of these entities.
// rst:
// rst: Linear expressions and constraints can be implicitly created from variable specifiers and operators,
// rst: meaning one rarely need to instantiate them explicitly.
// rst: However creating the zero-expression might sometimes be useful (e.g., in a sum expression)
// rst: and can be done simply with the expression :cpp:texpr:`hyperflow::LinExp()`.
// rst:
// rst: A variable specifier is a representation of either a single variable or a sum of variables.
// rst: The addition, multiplication and division operators are overloaded for variable specifiers and
// rst: linear expressions such that expressions can be written in the natural manner.
// rst:
// rst: The operators ``<=``, ``==`` and ``>=`` are overloaded on linear expressions to make the creation of
// rst: linear constraints easier. However, one argument to the operator must be a linear expression while
// rst: the other argument must be a number.
// rst:
// rst: Variable Specifier Classes
// rst: --------------------------
// rst:
// rst: The main variables in a flow model are associated with either vertices or hyperedges in the underlying
// rst: derivation graph.
// rst: A variable specifier represents either a set of variables or a single variable,
// rst: Each specifier has an associated ID that must be registered by a flow model module
// rst: at the time when the specifier is added to the model.
// rst: See also :ref:`flowCommon` for an overview of the specifiers.
// rst:
// rst:
// rst-class: hyperflow::VarSumVertex
// rst:
// rst: 	Represents the sum of all variables indicated by the given :cpp:texpr:`id`.
// rst: 	Can be indexed to retrieve a specifier for a single variable in the set.
// rst:
// rst-class-start:
struct MOD_DECL VarSumVertex {
	// rst: .. function:: explicit VarSumVertex(std::string id)
	// rst:
	// rst: 	Construct a variable specifier for a sum of variables associated with
	// rst: 	vertices in the underlying derivation graph.
	explicit VarSumVertex(std::string id);
	// rst: .. function:: friend std::ostream &operator<<(std::ostream &s, const VarSumVertex &v)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const VarSumVertex &v);
	// rst: .. function:: friend bool operator==(const VarSumVertex &a, const VarSumVertex &b)
	MOD_DECL friend bool operator==(const VarSumVertex &a, const VarSumVertex &b) {
		return a.id == b.id;
	}
	// rst: .. function:: VarVertex operator[](dg::DG::Vertex v) const
	// rst:
	// rst: 	Select the variable associated with the given derivation graph vertex.
	// rst: 	When the resulting specifier is given to a flow model the vertex must be from the
	// rst: 	underlying derivation graph.
	// rst:
	// rst: 	:returns: a variable specifier for a single variable in the set represented by this object.
	// rst: 	:throws: `LogicError` if `!v`.
	VarVertex operator[](dg::DG::Vertex v) const;
	// rst: .. function:: VarVertexGraph operator[](std::shared_ptr<graph::Graph> g) const
	// rst:
	// rst: 	Select the variable associated with the derivation graph vertex which represents the given graph.
	// rst: 	When the resulting specifier is given to a flow model such a vertex must then exist in the
	// rst: 	underlying derivation graph.
	// rst:
	// rst: 	:returns: a variable specifier for a single variable in the set represented by this object.
	// rst: 	:throws: `LogicError` if `!g`.
	VarVertexGraph operator[](std::shared_ptr<graph::Graph> g) const;
public:
	// rst: .. var:: std::string id
	// rst:
	// rst: 	The ID for this specifier.
	std::string id;
};
// rst-class-end:

// rst-class: hyperflow::VarVertex
// rst:
// rst: 	Represents a variable associated with a single vertex in a derivation graph.
// rst:
// rst-class-start:
struct MOD_DECL VarVertex {
	// rst: .. function:: explicit VarVertex(std::string id, dg::DG::Vertex v)
	// rst:
	// rst: 	:throws: `LogicError` if `!v`.
	explicit VarVertex(std::string id, dg::DG::Vertex v);
	// rst: .. function:: friend std::ostream &operator<<(std::ostream &s, const VarVertex &v)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const VarVertex &v);
	// rst: .. function:: friend bool operator==(const VarVertex &a, const VarVertex &b)
	MOD_DECL friend bool operator==(const VarVertex &a, const VarVertex &b) {
		return a.id == b.id && a.v == b.v;
	}
public:
	// rst: .. var:: std::string id
	std::string id;
	// rst: .. var:: dg::DG::Vertex v
	dg::DG::Vertex v;
};
// rst-class-end:

// rst-class: hyperflow::VarVertexGraph
// rst:
// rst: 	Represents a variable associated with a single vertex in a derivation graph.
// rst: 	This is done implicitly by a graph, and when this specifier is later given to a flow model
// rst: 	a vertex with the graph associated must exist in the underlying derivation graph.
// rst:
// rst-class-start:
struct MOD_DECL VarVertexGraph {
	// rst: .. function:: explicit VarVertexGraph(std::string id, std::shared_ptr<graph::Graph> g)
	// rst:
	// rst: 	:throws: `LogicError` if `!g`.
	explicit VarVertexGraph(std::string id, std::shared_ptr<graph::Graph> g);
	// rst: .. function:: friend std::ostream &operator<<(std::ostream &s, const VarVertexGraph &v)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const VarVertexGraph &v);
	// rst: .. function:: friend bool operator==(const VarVertexGraph &a, const VarVertexGraph &b)
	MOD_DECL friend bool operator==(const VarVertexGraph &a, const VarVertexGraph &b) {
		return a.id == b.id && a.g == b.g;
	}
public:
	// rst: .. var:: std::string id
	std::string id;
	// rst: .. var:: std::shared_ptr<graph::Graph> g
	std::shared_ptr<graph::Graph> g;
};
// rst-class-end:

// rst-class: hyperflow::VarSumEdge
// rst:
// rst: 	Represents the sum of all variables indicated by the given :cpp:texpr:`id`.
// rst: 	Can be indexed to retrieve a specifier for a single variable in the set.
// rst:
// rst-class-start:
struct MOD_DECL VarSumEdge {
	// rst: .. function:: explicit VarSumEdge(std::string id)
	// rst:
	// rst: 	Construct a variable specifier for a sum of variables associated with
	// rst: 	hyperedges in the underlying derivation graph.
	explicit VarSumEdge(std::string id);
public:
	// rst: .. function:: friend std::ostream &operator<<(std::ostream &s, const VarSumEdge &v)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const VarSumEdge &v);
	// rst: .. function:: friend bool operator==(const VarSumEdge &a, const VarSumEdge &b)
	MOD_DECL friend bool operator==(const VarSumEdge &a, const VarSumEdge &b) {
		return a.id == b.id;
	}
	// rst: .. function:: VarEdge operator[](dg::DG::HyperEdge e) const
	// rst:
	// rst: 	Select the variable associated with the given derivation graph hyperedge.
	// rst: 	When the resulting specifier is given to a flow model its underlying derivation graph
	// rst: 	must be the same as the derivation graph the given hyperedge belongs to.
	// rst:
	// rst: 	:returns: a variable specifier for a single variable in the set represented by this object.
	// rst: 	:throws: :cpp:type:`LogicError` if :cpp:texpr:`!e`.
	VarEdge operator[](dg::DG::HyperEdge e) const;
public:
	// rst: .. var:: std::string id
	std::string id;
};
// rst-class-end:

// rst-class: hyperflow::VarEdge
// rst:
// rst: 	Represents a variable associated with a single hyperedge in a derivation graph.
// rst: 	When this specifier is later given to a flow model its underlying derivation graph
// rst: 	must be the same as the derivation graph the hyperedge in this object belongs to.
// rst:
// rst-class-start:
struct MOD_DECL VarEdge {
	// rst: .. function:: explicit VarEdge(std::string id, dg::DG::HyperEdge e)
	// rst:
	// rst: 	:throws: :cpp:type:`LogicError` if :cpp:texpr:`!e`.
	explicit VarEdge(std::string id, dg::DG::HyperEdge e);
	// rst: .. function:: friend std::ostream &operator<<(std::ostream &s, const VarEdge &v)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const VarEdge &v);
	// rst: .. function:: friend bool operator==(const VarEdge &a, const VarEdge &b)
	MOD_DECL friend bool operator==(const VarEdge &a, const VarEdge &b) {
		return a.id == b.id && a.edge == b.edge;
	}
public:
	// rst: .. var:: std::string id
	std::string id;
	// rst: .. var:: dg::DG::HyperEdge edge
	dg::DG::HyperEdge edge;
};
// rst-class-end:


// rst-class: hyperflow::VarSumCustom
// rst:
// rst: 	Represents the sum of all variables indicated by the given :cpp:texpr:`id`.
// rst: 	Can be indexed to retrieve a specifier for a single variable in the set.
// rst:
// rst-class-start:
struct MOD_DECL VarSumCustom {
	// rst: .. function:: explicit VarSumCustom(std::string id)
	explicit VarSumCustom(std::string id);
	// rst: .. function:: friend std::ostream &operator<<(std::ostream &s, const VarSumCustom &v)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const VarSumCustom &v);
	// rst: .. function:: friend bool operator==(const VarSumCustom &a, const VarSumCustom &b)
	MOD_DECL friend bool operator==(const VarSumCustom &a, const VarSumCustom &b) {
		return a.id == b.id;
	}
	// rst: .. function:: VarCustom operator[](std::string name) const
	// rst:
	// rst: 	Select the variable associated with the given name.
	// rst: 	When the resulting specifier is given to a flow model that variable must have been created
	// rst: 	in the specification in some way.
	// rst:
	// rst: 	:returns: a variable specifier for a single variable in the set represented by this object.
	VarCustom operator[](std::string name) const;
public:
	// rst: .. var:: std::string id
	std::string id;
};
// rst-class-end:

// rst-class: hyperflow::VarCustom
// rst:
// rst: 	Represents a custom variable.
// rst:
// rst-class-start:
struct MOD_DECL VarCustom {
	// rst: .. function:: explicit VarCustom(std::string id, std::string name)
	// rst:
	explicit VarCustom(std::string id, std::string name);
	// rst: .. function:: friend std::ostream &operator<<(std::ostream &s, const VarCustom &v)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const VarCustom &v);
	// rst: .. function:: friend bool operator==(const VarCustom &a, const VarCustom &b)
	MOD_DECL friend bool operator==(const VarCustom &a, const VarCustom &b) {
		return a.id == b.id && a.name == b.name;
	}
public:
	// rst: .. var:: std::string id
	std::string id;
	// rst: .. var:: std::string name
	std::string name;
};
// rst-class-end:

// rst-class: hyperflow::Var
// rst:
// rst: 	Represents any of the variable specifier types, and can be implicitly converted to from any of them.
// rst:
// rst-class-start:
struct MOD_DECL Var {
	// rst: .. type:: VarImpl = std::variant<VarSumVertex, VarVertex, VarVertexGraph, VarSumEdge, VarEdge, VarSumCustom, VarCustom>
	using VarImpl = std::variant<VarSumVertex, VarVertex, VarVertexGraph, VarSumEdge, VarEdge, VarSumCustom, VarCustom>;
public:
	// rst: .. function:: template<typename T> \
	// rst:               Var(T v)
	// rst:
	// rst: 	Requires `T` being a variable specifier type.
	template<typename T, typename = typename std::enable_if<std::is_convertible<T, VarImpl>::value>::type>
	Var(T v) : v(std::move(v)) {}

	// rst: .. function:: template<typename Visitor> \
	// rst:               decltype(auto) applyVisitor(Visitor &&visitor)
	// rst:               template<typename Visitor> \
	// rst:               decltype(auto) applyVisitor(Visitor &&visitor) const
	// rst:
	// rst: 	:returns: `boost::apply_visitor(visitor, v)`
	template<typename Visitor>
	decltype(auto) applyVisitor(Visitor &&visitor) {
		return std::visit(visitor, v);
	}

	template<typename Visitor>
	decltype(auto) applyVisitor(Visitor &&visitor) const {
		return std::visit(visitor, v);
	}

	// rst: .. function:: template<typename T> \
	// rst:               const T *get() const
	// rst:
	// rst: 	:returns: `get<T>(&v)`
	template<typename T>
	const T *get() const {
		return get<T>(&v);
	}

	// rst: .. function:: friend std::ostream &operator<<(std::ostream &s, const Var &v)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const Var &v);
	// rst: .. function:: friend bool operator==(const Var &a, const Var &b)
	MOD_DECL friend bool operator==(const Var &a, const Var &b) {
		return a.v == b.v;
	}
private:
	// rst: .. var:: private VarImpl v
	VarImpl v;
};
// rst-class-end:

// rst-class: hyperflow::LinExp
// rst:
// rst: 	Represents a linear expression over variable specifiers.
// rst:
// rst-class-start:
struct MOD_DECL LinExp {
	// rst: .. type:: IntElement = std::pair<int, Var>
	// rst: .. type:: FloatElement = std::pair<double, Var>
	using IntElement = std::pair<int, Var>;
	using FloatElement = std::pair<double, Var>;
public:
	// rst: .. function:: LinExp() = default
	LinExp() = default;
	// rst: .. function:: LinExp(Var v)
	LinExp(Var v);
	// rst: .. function:: template<typename T> LinExp(T v)
	// rst:
	// rst: 	Requires `T` convertible to `Var`.
	template<typename T, typename = typename std::enable_if<std::is_convertible<T, Var>::value>::type>
	LinExp(T v) : LinExp(Var(std::move(v))) {}
	// rst: .. function:: friend std::ostream &operator<<(std::ostream &s, const LinExp &exp)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const LinExp &exp);
	// rst: .. function:: const std::vector<IntElement> &getIntElements() const
	// rst:
	// rst: 	:returns: all elements of the linear expression that have integer coefficients.
	const std::vector<IntElement> &getIntElements() const;
	// rst: .. function:: const std::vector<FloatElement> &getFloatElements() const
	// rst:
	// rst: 	:returns: all elements of the linear expression that have floating point coefficients.
	const std::vector<FloatElement> &getFloatElements() const;
	// rst: .. function:: LinExp &operator+=(const LinExp &e)
	// rst:               LinExp &operator-=(const LinExp &e)
	// rst:               LinExp &operator*=(int c)
	// rst:               LinExp &operator*=(double c)
	LinExp &operator+=(const LinExp &e);
	LinExp &operator-=(const LinExp &e);
	LinExp &operator*=(int c);
	LinExp &operator*=(double c);
private:
	std::vector<IntElement> intElements;
	std::vector<FloatElement> floatElements;
};
// rst-class-end:

#define MOD_MAKE_PM_OP(Op)                                        \
    inline LinExp operator Op(LinExp lhs, const LinExp &rhs) {    \
        lhs Op##= rhs;                                            \
        return lhs;                                               \
    }
MOD_MAKE_PM_OP(+)
MOD_MAKE_PM_OP(-)
#undef MOD_MAKE_PM_OP

// rst:
// rst: .. namespace-push:: hyperflow
// rst:

// rst: .. function:: LinExp operator+(const LinExp &e)
inline LinExp operator+(const LinExp &e) {
	return e;
}

// rst: .. function:: LinExp operator-(const LinExp &e)
inline LinExp operator-(const LinExp &e) {
	return LinExp() - e;
}

#define MOD_MAKE_M_OP(T)                                   \
    inline LinExp operator*(LinExp lhs, T t) {             \
        return lhs *= t;                                   \
    }                                                      \
    inline LinExp operator*(T t, LinExp rhs) {             \
        return std::move(rhs) * t;                         \
    }
// rst: .. function:: LinExp operator*(LinExp lhs, int t)
// rst:               LinExp operator*(int t, LinExp rhs)
MOD_MAKE_M_OP(int)
// rst: .. function:: LinExp operator*(LinExp lhs, double t)
// rst:               LinExp operator*(double t, LinExp rhs)
MOD_MAKE_M_OP(double)
#undef MOD_MAKE_M_OP

// rst:
// rst: .. namespace-pop::
// rst:

// rst-class: hyperflow::LinConstraint
// rst:
// rst: 	Represents a linear constraint on the form :cpp:texpr:`exp` :cpp:texpr:`relation` :cpp:texpr:`bound`,
// rst:
// rst-class-start:
struct MOD_DECL LinConstraint {
	// rst: .. enum-class:: Relation
	// rst:
	enum class Relation {
		// rst: 	.. enumerator: Leq
		// rst: 	               Eq
		// rst: 	               Geq
		Leq, Eq, Geq
	};
public:
	// rst: .. function:: explicit LinConstraint(LinExp exp, Relation relation, double bound)
	explicit LinConstraint(LinExp exp, Relation relation, double bound);
	// rst: .. function:: friend std::ostream &operator<<(std::ostream &s, const LinConstraint &c)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const LinConstraint &c);
public:
	// rst: .. var:: LinExp exp
	LinExp exp;
	// rst: .. var:: Relation relation
	Relation relation;
	// rst: .. var:: double bound
	double bound;
};
// rst-class-end:

// rst:
// rst: .. namespace-push:: hyperflow
// rst:
// rst: .. function:: LinConstraint operator<=(const LinExp &e, double c)
// rst:               LinConstraint operator==(const LinExp &e, double c)
// rst:               LinConstraint operator>=(const LinExp &e, double c)
// rst:               LinConstraint operator<=(double c, const LinExp &e)
// rst:               LinConstraint operator==(double c, const LinExp &e)
// rst:               LinConstraint operator>=(double c, const LinExp &e)
MOD_DECL LinConstraint operator<=(const LinExp &e, double c);
MOD_DECL LinConstraint operator==(const LinExp &e, double c);
MOD_DECL LinConstraint operator>=(const LinExp &e, double c);
MOD_DECL LinConstraint operator<=(double c, const LinExp &e);
MOD_DECL LinConstraint operator==(double c, const LinExp &e);
MOD_DECL LinConstraint operator>=(double c, const LinExp &e);
// rst:
// rst: .. namespace-pop::
// rst:

namespace vars {
// rst: Pre-defined Variable Specifiers
// rst: ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// rst:
// rst: Each of the variable specifiers are defined in a separate namespace,
// rst: so you can use ``using namespace mod::hyperflow::vars;`` in the scopes where you need them,
// rst: without having to fully qualify their names each time.
// rst:
// Note: these are simply shortcuts defined for convenience.
// rst: Base Model
// rst: """"""""""
// rst:
// rst: .. var:: const hyperflow::VarSumVertex hyperflow::vars::inFlow
// rst: .. var:: const hyperflow::VarSumVertex hyperflow::vars::outFlow
const VarSumVertex inFlow("inFlow"), outFlow("outFlow");
// rst: .. var:: const hyperflow::VarSumVertex hyperflow::vars::isInUsed
// rst: .. var:: const hyperflow::VarSumVertex hyperflow::vars::isOutUsed
const VarSumVertex isInUsed("isInUsed"), isOutUsed("isOutUsed");
// rst: .. var:: const hyperflow::VarSumVertex hyperflow::vars::isInLessOut
// rst: .. var:: const hyperflow::VarSumVertex hyperflow::vars::isInGreaterOut
const VarSumVertex isInLessOut("isInLessOut"), isInGreaterOut("isInGreaterOut");
// rst: .. var:: const hyperflow::VarSumVertex hyperflow::vars::isInOutZero
const VarSumVertex isInOutZero("isInOutZero");
// rst: .. var:: const hyperflow::VarSumVertex hyperflow::vars::vertexFlow
// rst: .. var:: const hyperflow::VarSumVertex hyperflow::vars::isVertexUsed
// rst: .. var:: const hyperflow::VarSumVertex hyperflow::vars::transitInternalFlow
const VarSumVertex vertexFlow("vertexFlow"), isVertexUsed("isVertexUsed"), transitInternalFlow("transitInternalFlow");
// rst: .. var:: const hyperflow::VarSumEdge hyperflow::vars::edgeFlow
// rst: .. var:: const hyperflow::VarSumEdge hyperflow::vars::isEdgeUsed
const VarSumEdge edgeFlow("edgeFlow"), isEdgeUsed("isEdgeUsed");
// rst: .. var:: const hyperflow::VarSumEdge hyperflow::vars::isBothReverseUsed
const VarSumEdge isBothReverseUsed("isBothReverseUsed");
// rst:
// rst: OverallCatalysis
// rst: """"""""""""""""
// rst:
// rst: .. var:: const hyperflow::VarSumVertex hyperflow::vars::isOverallCata
const VarSumVertex isOverallCata("isOverallCata");
// rst:
// rst: OverallAutocatalysis
// rst: """"""""""""""""""""
// rst:
// rst: .. var:: const hyperflow::VarSumVertex hyperflow::vars::isOverallAutocata
const VarSumVertex isOverallAutocata("isOverallAutocata");

} // namespace vars
} // namespace mod::hyperflow

#endif // MOD_HYPERFLOW_LINEXPR_HPP