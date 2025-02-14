#include "CompositionExpr.hpp"

#include <mod/Error.hpp>
#include <mod/graph/Graph.hpp>
#include <mod/rule/Rule.hpp>

#include <ostream>

namespace mod::rule::RCExp {

// Parallel
//------------------------------------------------------------------------------

Union::Union(std::vector<Expression> exps) : exps(exps) {
	if(exps.empty())
		throw LogicError("An RC Union expression must be instantiated with a non-empty list of subexpressions.");
}

std::ostream &operator<<(std::ostream &s, const Union &par) {
	s << "[" << par.exps.front();
	for(unsigned int i = 1; i < par.exps.size(); i++)
		s << ", " << par.exps[i];
	return s << "]";
}

const std::vector<Expression> &Union::getExpressions() const {
	return exps;
}

// Bind
//------------------------------------------------------------------------------

Bind::Bind(std::shared_ptr<graph::Graph> g) : g(g) {
	if(!g) throw LogicError("The graph is null.");
}

std::ostream &operator<<(std::ostream &s, const Bind &b) {
	return s << "bind(" << *b.g << ")";
}

std::shared_ptr<graph::Graph> Bind::getGraph() const {
	return g;
}

// Id
//------------------------------------------------------------------------------

Id::Id(std::shared_ptr<graph::Graph> g) : g(g) {
	if(!g) throw LogicError("The graph is null.");
}

std::ostream &operator<<(std::ostream &s, const Id &i) {
	return s << "id(" << *i.g << ")";
}

std::shared_ptr<graph::Graph> Id::getGraph() const {
	return g;
}

// Unbind
//------------------------------------------------------------------------------

Unbind::Unbind(std::shared_ptr<graph::Graph> g) : g(g) {
	if(!g) throw LogicError("The graph is null.");
}

std::ostream &operator<<(std::ostream &s, const Unbind &u) {
	return s << "bind(" << *u.g << ")";
}

std::shared_ptr<graph::Graph> Unbind::getGraph() const {
	return g;
}

// Expression
//------------------------------------------------------------------------------

Expression::Expression(std::shared_ptr<rule::Rule> r) : data(r) {
	if(!r) throw LogicError("The rule is null.");
}

Expression::Expression(Union u) : data(u) {}

Expression::Expression(Bind bind) : data(bind) {}

Expression::Expression(Id id) : data(id) {}

Expression::Expression(Unbind unbind) : data(unbind) {}

Expression::Expression(ComposeCommon compose) : data(compose) {}

Expression::Expression(ComposeParallel compose) : data(compose) {}

Expression::Expression(ComposeSub compose) : data(compose) {}

Expression::Expression(ComposeSuper compose) : data(compose) {}

namespace {

struct Visitor : boost::static_visitor<std::ostream&> {

	Visitor(std::ostream &s) : s(s) {}

	template<typename T>
	std::ostream &operator()(const T &v) {
		return s << v;
	}

	template<typename T>
	std::ostream &operator()(const std::shared_ptr<T> &p) {
		return s << *p;
	}
public:
	std::ostream &s;
};

} // namespace 

std::ostream &operator<<(std::ostream &s, const Expression &exp) {
	return exp.applyVisitor(Visitor(s));
}

// ComposeCommon
//------------------------------------------------------------------------------

ComposeCommon::ComposeCommon(Expression first, Expression second, bool maximum, bool connected, bool includeEmpty)
	: first(first), second(second), maximum(maximum), connected(connected), includeEmpty(includeEmpty) {}

std::ostream &operator<<(std::ostream &s, const ComposeCommon &c) {
	return s << c.first << std::boolalpha << " *rcCommon(maximum=" << c.maximum
			<< ", connected=" << c.connected
			<< ", includeEmpty=" << c.includeEmpty << ")* " << c.second;
}

// ComposeParallel
//------------------------------------------------------------------------------

ComposeParallel::ComposeParallel(Expression first, Expression second) : first(first), second(second) {}

std::ostream &operator<<(std::ostream &s, const ComposeParallel &c) {
	return s << c.first << " *rcParallel* " << c.second;
}

// ComposeSub
//------------------------------------------------------------------------------

ComposeSub::ComposeSub(Expression first, Expression second, bool allowPartial)
	: first(first), second(second), allowPartial(allowPartial) {}

std::ostream &operator<<(std::ostream &s, const ComposeSub &c) {
	return s << c.first << " *rcSub(allowPartial=" << std::boolalpha << c.allowPartial << ")* " << c.second;
}

// ComposeSuper
//------------------------------------------------------------------------------

ComposeSuper::ComposeSuper(Expression first, Expression second, bool allowPartial, bool enforceConstraints)
	: first(first), second(second), allowPartial(allowPartial),
	  enforceConstraints(enforceConstraints) {}

std::ostream &operator<<(std::ostream &s, const ComposeSuper &c) {
	return s << c.first << " *rcSuper(allowPartial=" << std::boolalpha << c.allowPartial << ", enforceConstraints="
			<< c.enforceConstraints << ")* " << c.second;
}

} // namespace mod::rule::RCExp