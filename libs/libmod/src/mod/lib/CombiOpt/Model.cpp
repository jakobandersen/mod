#include "Model.hpp"

#include <mod/Error.hpp>
#include <mod/Post.hpp>
#include <mod/lib/IO/IO.hpp>

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/stringize.hpp>

#include <algorithm>
#include <iostream>

namespace mod::lib::CombiOpt {

BoolVar Model::addBoolVar(std::string name) {
	const auto id = vars.size();
	vars.push_back(VariableInfo{VarType::Bool, std::move(name), boolVars.size()});
	boolVars.push_back(BoolVarInfo{id});
	return BoolVar(id);
}

IntVar Model::addIntVariable(std::string name) {
	const auto id = vars.size();
	vars.push_back(VariableInfo{VarType::Int, std::move(name), intVars.size()});
	intVars.push_back(IntVarInfo{id});
	return IntVar(id);
}

FloatVar Model::addFloatVariable(std::string name) {
	const auto id = vars.size();
	vars.push_back(VariableInfo{VarType::Float, std::move(name), floatVars.size()});
	floatVars.push_back(FloatVarInfo{id});
	return FloatVar(id);
}

void Model::addConstraint(Conjunction c) {
	conjunctions.push_back(std::move(c));
}

void Model::addConstraint(Disjunction c) {
	disjunctions.push_back(std::move(c));
}

void Model::addConstraint(LinConstraintInt c) {
	linConstraintsInt.push_back(std::move(c));
}

void Model::addConstraint(LinConstraintFloat c) {
	linConstraintsFloat.push_back(std::move(c));
}

void Model::addConstraint(LinConstraintAny c) {
	linConstraintsAny.push_back(std::move(c));
}

void Model::addImplication(Conjunction v, LinConstraintInt c) {
	implicationsInt.push_back({v, c});
}

void Model::addImplication(Conjunction v, LinConstraintFloat c) {
	implicationsFloat.push_back({v, c});
}

void Model::addImplication(Conjunction v, BoolVar c) {
	implicationsBool.push_back({v, c, false});
}

void Model::addImplication(Conjunction v, NegatedBoolVar c) {
	implicationsBool.push_back({v, c.v, true});
}

void Model::addEquivalence(LinConstraintInt c, Conjunction v) {
	equivalences.push_back({c, v});
}

void Model::setLB(BoolVar var, Int value) {
	boolVars[vars[var.getData()].offset].lb = value;
}

void Model::setUB(BoolVar var, Int value) {
	boolVars[vars[var.getData()].offset].ub = value;
}

void Model::setLB(IntVar var, Int value) {
	intVars[vars[var.getData()].offset].lb = value;
}

void Model::setUB(IntVar var, Int value) {
	intVars[vars[var.getData()].offset].ub = value;
}

void Model::setLB(FloatVar var, Float value) {
	floatVars[vars[var.getData()].offset].lb = value;
}

void Model::setUB(FloatVar var, Float value) {
	floatVars[vars[var.getData()].offset].ub = value;
}

void Model::setObjectiveFunction(LinExpAny e) {
	objectiveFunction = std::move(e);
}

const LinExpAny &Model::getObjectiveFunction() const {
	return objectiveFunction;
}

Model::VariableRange Model::getVariables() const {
	return {this, 0, static_cast<int>(vars.size())};
}

const std::string &Model::getVarName(Var var) const {
	return vars[var.getData()].name;
}

Int Model::getLB(BoolVar var) const {
	return boolVars[vars[var.getData()].offset].lb;
}

Int Model::getUB(BoolVar var) const {
	return boolVars[vars[var.getData()].offset].ub;
}

Int Model::getLB(IntVar var) const {
	return intVars[vars[var.getData()].offset].lb;
}

Int Model::getUB(IntVar var) const {
	return intVars[vars[var.getData()].offset].ub;
}

Float Model::getLB(FloatVar var) const {
	return floatVars[vars[var.getData()].offset].lb;
}

Float Model::getUB(FloatVar var) const {
	return floatVars[vars[var.getData()].offset].ub;
}

Int Model::getLB(IntegralVar var) const {
	switch(var.getType()) {
	case VarType::Bool:
		return getLB(BoolVar(var.getData()));
	case VarType::Int:
		return getLB(IntVar(var.getData()));
	case VarType::Float:
		break;
	}
	__builtin_unreachable();
}

Int Model::getUB(IntegralVar var) const {
	switch(var.getType()) {
	case VarType::Bool:
		return getUB(BoolVar(var.getData()));
	case VarType::Int:
		return getUB(IntVar(var.getData()));
	case VarType::Float:
		break;
	}
	__builtin_unreachable();
}

Float Model::getLB(Var var) const {
	switch(var.getType()) {
	case VarType::Bool:
		return Float(getLB(BoolVar(var.getData())));
	case VarType::Int:
		return Float(getLB(IntVar(var.getData())));
	case VarType::Float:
		return getLB(FloatVar(var.getData()));
	}
	__builtin_unreachable();
}

Float Model::getUB(Var var) const {
	switch(var.getType()) {
	case VarType::Bool:
		return Float(getUB(BoolVar(var.getData())));
	case VarType::Int:
		return Float(getUB(IntVar(var.getData())));
	case VarType::Float:
		return getUB(FloatVar(var.getData()));
	}
	__builtin_unreachable();
}

Int Model::getLB(const LinExpInt &e) const {
	Int b(0);
	for(const auto&[coef, v] : e.getTerms()) {
		if(coef > 0) b += coef * getLB(v);
		else b += coef * Int::INF_POS; //data.ub; // TODO: hax
	}
	return b;
}

Int Model::getUB(const LinExpInt &e) const {
	Int b(0);
	for(const auto&[coef, v] : e.getTerms()) {
		if(coef > 0) b += coef * Int::INF_POS; //data.ub; // TODO: hax
		else b += coef * getLB(v);
	}
	return b;
}

Float Model::getLB(const LinExpFloat &e) const {
	auto b = Float(getLB(e.getIntExp()));
	for(const auto&[coef, v] : e.getFloatTerms()) {
		if(coef > 0.0) b += coef * getLB(v);
		else b += coef * Float(Int::INF_POS); //data.ub; // TODO: hax
	}
	return b;
}

Float Model::getUB(const LinExpFloat &e) const {
	auto b = Float(getUB(e.getIntExp()));
	for(const auto&[coef, v] : e.getFloatTerms()) {
		if(coef > 0.0) b += coef * Float(Int::INF_POS); //data.ub; // TODO: hax
		else b += coef * getLB(v);
	}
	return b;
}

namespace {

std::ostream &operator<<(std::ostream &s, VarType v) {
	switch(v) {
	case VarType::Bool:
		return s << "Bool";
	case VarType::Int:
		return s << "Int";
	case VarType::Float:
		return s << "Float";
	}
	return s;
}

} // namespace

std::ostream &Model::print(std::ostream &s, const Model::VariableInfo &v) const {
	s << "name=" << v.name << ", type=" << v.type;
	switch(v.type) {
	case VarType::Bool:
		return s << ", lb=" << boolVars[v.offset].lb << ", ub=" << boolVars[v.offset].ub;
	case VarType::Int:
		return s << ", lb=" << intVars[v.offset].lb << ", ub=" << intVars[v.offset].ub;
	case VarType::Float:
		return s << ", lb=" << floatVars[v.offset].lb << ", ub=" << floatVars[v.offset].ub;
	}
	__builtin_unreachable();
}

std::ostream &Model::print(std::ostream &s, const BoolVar &v) const {
	return print(s, Var(v));
}

std::ostream &Model::print(std::ostream &s, const IntVar &v) const {
	return print(s, Var(v));
}

std::ostream &Model::print(std::ostream &s, const IntegralVar &v) const {
	return print(s, Var(v));
}

std::ostream &Model::print(std::ostream &s, const FloatVar &v) const {
	return print(s, Var(v));
}

std::ostream &Model::print(std::ostream &s, const Var &v) const {
	return s << vars[v.getData()].name;
}

std::ostream &Model::print(std::ostream &s, const Conjunction &v) const {
	bool first = true;
	for(const auto &t : v) {
		if(!first) s << " && ";
		first = false;
		if(t.negated) s << "!";
		print(s, t.v);
	}
	return s;
}

std::ostream &Model::print(std::ostream &s, const Disjunction &v) const {
	bool first = true;
	for(const auto &t : v) {
		if(!first) s << " || ";
		first = false;
		if(t.negated) s << "!";
		print(s, t.v);
	}
	return s;
}

std::ostream &Model::print(std::ostream &s, const LinConstraintInt &e) const {
	return print(s, e.getExpr()) << " " << e.getRelation() << " " << e.getBound();
}

std::ostream &Model::print(std::ostream &s, const LinConstraintFloat &e) const {
	return print(s, e.getExpr()) << " " << e.getRelation() << " " << e.getBound();
}

std::ostream &Model::print(std::ostream &s, const LinConstraintAny &e) const {
	return print(s, e.getExpr()) << " " << e.getRelation() << " " << e.getBound();
}

std::ostream &Model::print(std::ostream &s, const LinExpInt &e) const {
	if(e.getTerms().empty()) return s << "0";
	bool first = true;
	for(const auto &t : e.getTerms()) {
		if(!first) {
			if(t.first < 0) s << " - ";
			else s << " + ";
		} else if(t.first < 0) s << "-";
		const auto coef = std::abs(t.first.getValue());
		if(coef != 1) s << coef << "*";
		print(s, t.second);
		first = false;
	}
	return s;
}

std::ostream &Model::print(std::ostream &s, const LinExpFloat &e) const {
	if(e.getIntTerms().empty() && e.getFloatTerms().empty()) return s << "0";
	bool first = true;
	for(const auto &t : e.getIntTerms()) {
		if(!first) {
			if(t.first < 0) s << " - ";
			else s << " + ";
		} else if(t.first < 0) s << "-";
		const auto coef = std::abs(t.first.getValue());
		if(coef != 1) s << coef << "*";
		print(s, t.second);
		first = false;
	}
	for(const auto &t : e.getFloatTerms()) {
		if(!first) {
			if(t.first < 0.0) s << " - ";
			else s << " + ";
		} else if(t.first < 0.0) s << "-";
		const auto coef = std::abs(t.first.getValue());
		if(coef != 1) s << coef << "*";
		print(s, t.second);
		first = false;
	}
	return s;
}

std::ostream &Model::print(std::ostream &s, const LinExpAny &e) const {
	bool first = true;
	e.forEach([&first, &s, this](auto coef, const auto var) {
		using Val = decltype(coef);
		if(!first) {
			if(coef < Val()) s << " - ";
			else s << " + ";
		} else if(coef < Val()) s << "-";
		coef = std::abs(coef.getValue());
		if(coef != Val(1)) s << coef << "*";
		print(s, var);
		first = false;
	});
	if(first) return s << "0";
	return s;
}

std::ostream &Model::print(std::ostream &s, const ImplicationInt &e) const {
	print(s, e.left) << " => ";
	print(s, e.right);
	return s;
}

std::ostream &Model::print(std::ostream &s, const ImplicationFloat &e) const {
	print(s, e.left) << " => ";
	print(s, e.right);
	return s;
}

std::ostream &Model::print(std::ostream &s, const ImplicationBool &e) const {
	print(s, e.left) << " => ";
	if(e.rightNegated) s << "!";
	print(s, e.right);
	return s;
}

std::ostream &Model::print(std::ostream &s, const Equivalence &e) const {
	print(s, e.left) << " <=> ";
	print(s, e.right);
	return s;
}

void Model::compare(const Model &ma, const Model &mb) {
#define MEMBERS()                                                \
   (vars)                                                        \
   (conjunctions)(disjunctions)                                  \
   (linConstraintsInt)(linConstraintsFloat)(linConstraintsAny)   \
   (implicationsInt)(implicationsFloat)(implicationsBool)        \
   (equivalences)

	const auto printModels = [&ma, &mb]() {
		post::FileHandle fa(IO::makeUniqueFilePrefix() + "ma.txt");
		post::FileHandle fb(IO::makeUniqueFilePrefix() + "mb.txt");
#define PRINT(r, mName, x)                               \
         for(const auto &e : m ## mName .x) {            \
            f ## mName << BOOST_PP_STRINGIZE(x) << ": "; \
            m ## mName.print(f ## mName, e);             \
            f ## mName << '\n';                          \
         }
		BOOST_PP_SEQ_FOR_EACH(PRINT, a, MEMBERS())
		BOOST_PP_SEQ_FOR_EACH(PRINT, b, MEMBERS())
		ma.print(fa << "objectiveFunction: ", ma.objectiveFunction) << '\n';
		ma.print(fb << "objectiveFunction: ", mb.objectiveFunction) << '\n';
	};

	const auto comp = [&ma, &mb, printModels](auto a, auto b, std::string thing) {
		auto &s = std::cout;
		std::sort(a.begin(), a.end());
		std::sort(b.begin(), b.end());
		const auto p = std::mismatch(a.begin(), a.end(), b.begin(), b.end());
		if(p.first != a.end() || p.second != b.end()) {
			s << thing << " mismatch:\n";
			s << "A: ";
			if(p.first == a.end())
				s << "at end";
			else
				ma.print(s, *p.first);
			s << "\nB: ";
			if(p.second == b.end())
				s << "at end";
			else
				mb.print(s, *p.second);
			s << "\n";
			printModels();
			s << std::flush;
			MOD_ABORT;
		}
	};

#define COMP(r, data, x) comp(ma.x, mb.x, BOOST_PP_STRINGIZE(x));
	BOOST_PP_SEQ_FOR_EACH(COMP, ~, MEMBERS())
	if(!(ma.objectiveFunction == mb.objectiveFunction)) {
		std::cout << "objectiveFunction mismatch\n";
		printModels();
		std::cout << std::flush;
		MOD_ABORT;
	}
}

} // namespace mod::lib::CombiOpt