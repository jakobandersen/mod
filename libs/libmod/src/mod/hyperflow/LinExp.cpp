#include "LinExp.hpp"

#include <mod/Error.hpp>
#include <mod/graph/Graph.hpp>

#include <ostream>

namespace mod::hyperflow {

// VarSumVertex
//------------------------------------------------------------------------------

VarSumVertex::VarSumVertex(std::string id) : id(std::move(id)) {}

std::ostream &operator<<(std::ostream &s, const VarSumVertex &v) {
	return s << v.id;
}

VarVertex VarSumVertex::operator[](dg::DG::Vertex v) const {
	return VarVertex(id, v);
}

VarVertexGraph VarSumVertex::operator[](std::shared_ptr<graph::Graph> g) const {
	return VarVertexGraph(id, g);
}

// VarVertex
//------------------------------------------------------------------------------

VarVertex::VarVertex(std::string id, dg::DG::Vertex v) : id(id), v(v) {
	if(!v) throw LogicError("Can not create vertex indexed variable specifier from null-vertex.");
}

std::ostream &operator<<(std::ostream &s, const VarVertex &v) {
	return s << v.id << "[" << v.v << "]";
}

// VarVertexGraph
//------------------------------------------------------------------------------

VarVertexGraph::VarVertexGraph(std::string id, std::shared_ptr<graph::Graph> g) : id(id), g(g) {}

std::ostream &operator<<(std::ostream &s, const VarVertexGraph &v) {
	return s << v.id << "[" << *v.g << "]";
}

// VarSumEdge
//------------------------------------------------------------------------------

VarSumEdge::VarSumEdge(std::string id) : id(id) {}

std::ostream &operator<<(std::ostream &s, const VarSumEdge &v) {
	return s << v.id;
}

VarEdge VarSumEdge::operator[](dg::DG::HyperEdge e) const {
	return VarEdge(id, e);
}

// VarEdge
//------------------------------------------------------------------------------

VarEdge::VarEdge(std::string id, dg::DG::HyperEdge e) : id(id), edge(e) {
	if(!e) throw LogicError("Can not create edge indexed variable specifier from null-edge.");
}

std::ostream &operator<<(std::ostream &s, const VarEdge &v) {
	return s << v.id << "[" << v.edge << "]";
}

// VarSumCustom
//------------------------------------------------------------------------------

VarSumCustom::VarSumCustom(std::string id) : id(std::move(id)) {}

std::ostream &operator<<(std::ostream &s, const VarSumCustom &v) {
	return s << v.id;
}

VarCustom VarSumCustom::operator[](std::string name) const {
	return VarCustom(id, std::move(name));
}

// VarCustom
//------------------------------------------------------------------------------

VarCustom::VarCustom(std::string id, std::string name)
		: id(std::move(id)), name(std::move(name)) {}

std::ostream &operator<<(std::ostream &s, const VarCustom &v) {
	return s << v.id << "[" << v.name << "]";
}

// Var
//------------------------------------------------------------------------------

std::ostream &operator<<(std::ostream &s, const Var &v) {
	v.applyVisitor([&s](const auto &v) { s << v; });
	return s;
}

// LinExp
//------------------------------------------------------------------------------

LinExp::LinExp(Var v) {
	intElements.emplace_back(1, v);
}

std::ostream &operator<<(std::ostream &s, const LinExp &exp) {
	if(exp.intElements.empty() && exp.floatElements.empty()) return s << 0;
	bool isFirst = true;
	for(const auto &e : exp.getIntElements()) {
		auto coef = e.first;
		if(coef < 0) {
			if(isFirst) s << ' ';
			s << '-';
			coef *= -1;
			if(coef > 1) s << ' ';
		} else if(!isFirst) s << " + ";
		if(coef != 1) s << coef << " * ";
		s << e.second;
		isFirst = false;
	}
	for(const auto &e : exp.getFloatElements()) {
		auto coef = e.first;
		if(coef < 0) {
			if(isFirst) s << ' ';
			s << '-';
			coef *= -1;
			if(coef > 1) s << ' ';
		} else if(!isFirst) s << " + ";
		if(coef != 1) s << std::showpoint << coef << " * ";
		s << e.second;
		isFirst = false;
	}
	return s;
}

const std::vector<LinExp::IntElement> &LinExp::getIntElements() const {
	return intElements;
}

const std::vector<LinExp::FloatElement> &LinExp::getFloatElements() const {
	return floatElements;
}

LinExp &LinExp::operator+=(const LinExp &e) {
	for(const auto &elem : e.getIntElements()) intElements.push_back(elem);
	for(const auto &elem : e.getFloatElements()) floatElements.push_back(elem);
	return *this;
}

LinExp &LinExp::operator-=(const LinExp &e) {
	for(const auto &elem : e.getIntElements()) intElements.emplace_back(-elem.first, elem.second);
	for(const auto &elem : e.getFloatElements()) floatElements.emplace_back(-elem.first, elem.second);
	return *this;
}

LinExp &LinExp::operator*=(int c) {
	for(auto &elem : intElements) elem.first *= c;
	for(auto &elem : floatElements) elem.first *= c;
	return *this;
}

LinExp &LinExp::operator*=(double c) {
	for(auto &elem : intElements) floatElements.emplace_back(elem.first, elem.second);
	intElements.clear();
	for(auto &elem : floatElements) elem.first *= c;
	return *this;
}

LinConstraint operator<=(const LinExp &e, double c) {
	return LinConstraint(e, LinConstraint::Relation::Leq, c);
}

LinConstraint operator==(const LinExp &e, double c) {
	return LinConstraint(e, LinConstraint::Relation::Eq, c);
}

LinConstraint operator>=(const LinExp &e, double c) {
	return LinConstraint(e, LinConstraint::Relation::Geq, c);
}

LinConstraint operator<=(double c, const LinExp &e) {
	return e >= c;
}

LinConstraint operator==(double c, const LinExp &e) {
	return e == c;
}

LinConstraint operator>=(double c, const LinExp &e) {
	return e <= c;
}

// LinConstraint
//------------------------------------------------------------------------------

LinConstraint::LinConstraint(LinExp exp, Relation relation, double bound)
		: exp(exp), relation(relation), bound(bound) {}

std::ostream &operator<<(std::ostream &s, const LinConstraint &c) {
	s << c.bound << " ";
	switch(c.relation) {
	case LinConstraint::Relation::Leq:
		s << ">=";
		break;
	case LinConstraint::Relation::Eq:
		s << "==";
		break;
	case LinConstraint::Relation::Geq:
		s << "<=";
		break;
	}
	return s << " " << c.exp;
}

} // namespace mod::hyperflow