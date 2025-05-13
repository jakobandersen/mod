#include "ILPImplementation.hpp"

#include <mod/lib/CombiOpt/Model.hpp>
#include <mod/lib/IO/IO.hpp>

#include <iostream>

namespace mod::lib::CombiOpt {
namespace {
constexpr bool ILP_DEBUG = false;

struct Helper {
	ILP::LinConstraint::Relation convertRelation(Relation r) const {
		switch(r) {
		case Relation::Eq:
			return ILP::LinConstraint::Relation::Eq;
		case Relation::Leq:
			return ILP::LinConstraint::Relation::Leq;
		case Relation::Geq:
			return ILP::LinConstraint::Relation::Geq;
		}
		__builtin_unreachable();
	};

	ILP::LinExp convertExpr(const LinExpInt &e) const {
		ILP::LinExp expr;
		for(const auto &t: e.getTerms())
			expr += t.first * impl.getVar(t.second);
		return expr;
	}

	ILP::LinExp convertExpr(const LinExpFloat &e) const {
		ILP::LinExp expr = convertExpr(e.getIntExp());
		for(const auto &t: e.getFloatTerms())
			expr += t.first * ILP::LinExp(impl.getVar(t.second));
		return expr;
	}

	ILP::LinExp convertExpr(const LinExpAny &e) const {
		ILP::LinExp expr = convertExpr(e.rest);
		for(const auto &t: e.boolVarsInt)
			expr += t.first * ILP::LinExp(impl.getVar(t.second));
		for(const auto &t: e.boolVarsFloat)
			expr += t.first * ILP::LinExp(impl.getVar(t.second));
		return expr;
	}

	template<typename LinConstraint>
	void addImplication(Conjunction v, const LinConstraint &c) {
		// v => c
		if(ILP_DEBUG)
			model.print(model.print(std::cout << "Impl: ", v) << " => ", c) << std::endl;

		const auto &Ax = c.getExpr();
		const auto b = c.getBound();

		const auto ilp_Ax = convertExpr(Ax);

		switch(c.getRelation()) {
		case Relation::Eq:
			MOD_ABORT; // TODO
		case Relation::Leq: {
			// Ax <= b + M*(1 - v)                Ax + M*v <= b + M
			// ub <= b + MU                       ub - b <= MU
			const auto ub = model.getUB(Ax);
			using Val = std::remove_const_t<decltype(ub)>;
			const auto MU = ub == Val(Int::INF_POS)
							? Val(bigM) : ub.getValue() - b.getValue();

			ILP::LinExp exp;
			Val Mval;
			for(const auto term: v) {
				// if any of these are false, then disable the constraint
				if(term.negated) { // false is when var == 1
					// Ax       <= b + M*(1 - (1 - v))
					// Ax       <= b + M*v
					// Ax - M*v <= b
					exp -= MU * impl.getVar(term.v);
				} else { // false is when var == 0
					exp += MU * impl.getVar(term.v);
					Mval += MU;
				}
			}
			if(ILP_DEBUG)
				std::cout << "   Leq: ub = " << ub << ", MU = " << MU << std::endl;
			impl.model->addConstraint(ILP::LinConstraint(ilp_Ax + exp,
														 ILP::LinConstraint::Relation::Leq,
														 ILP::Float(b.getValue() + Mval)));
			break;
		}
		case Relation::Geq: {
			// Ax >= b - M*(1 - v)                Ax - M*v >= b - M
			// lb >= b - ML                       b - lb <= ML
			const auto lb = model.getLB(Ax);
			using Val = std::remove_const_t<decltype(lb)>;
			const auto ML = lb == Val(Int::INF_NEG) ? Val(bigM) : b.getValue() - lb.getValue();

			ILP::LinExp exp;
			Val Mval;
			for(const auto term: v) {
				// if any of these are false, then disable the constraint
				if(term.negated) { // false is when var == 1
					// Ax       >= b - M*(1 - (1 - v))
					// Ax       >= b - M*v
					// Ax + M*v >= b
					exp += ML * impl.getVar(term.v);
				} else { // false is when var == 0
					exp -= ML * impl.getVar(term.v);
					Mval -= ML;
				}
			}
			if(ILP_DEBUG)
				std::cout << "   Geq: lb = " << lb << ", ML = " << ML << std::endl;
			impl.model->addConstraint(ILP::LinConstraint(ilp_Ax + exp,
														 ILP::LinConstraint::Relation::Geq,
														 ILP::Float(b.getValue() + Mval)));
			break;
		}
		}
	}

	void addImplicationNegated(Conjunction v, const LinConstraintInt &c) {
		if(v.size() > 1 || v.front().negated) MOD_ABORT;
		const auto y = v.front().v;
		// NOT v => NOT c
		if(ILP_DEBUG)
			model.print(model.print(std::cout << "NegImpl: not ", v) << " => not ", c) << std::endl;

		const auto &Ax = c.getExpr();
		const auto b = c.getBound();

		const auto ilp_Ax = convertExpr(Ax);
		const auto ilp_y = impl.getVar(y);

		switch(c.getRelation()) {
		case Relation::Eq:
			MOD_ABORT; // TODO
		case Relation::Leq: {
			// Ax >= b + 1 - M*y                  Ax + M*y >= b + 1
			// lb >= b + 1 - ML                   ML >= b + 1 - lb
			const auto lb = model.getLB(Ax);
			const auto ML = lb == Int::INF_NEG ? bigM : b.getValue() + 1 - lb.getValue();
			if(ILP_DEBUG)
				std::cout << "   Leq: lb = " << lb << ", ML = " << ML << std::endl;
			impl.model->addConstraint(ILP::LinConstraint(ilp_Ax + ML * ilp_y,
														 ILP::LinConstraint::Relation::Geq,
														 ILP::Float(b.getValue() + 1)));
			break;
		}
		case Relation::Geq: {
			// Ax <= b - 1 + M*y                  Ax - M*y <= b - 1
			// ub <= b - 1 + MU                   ub - b + 1 <= MU
			const auto ub = model.getUB(Ax);
			const auto MU = ub == Int::INF_POS ? bigM : ub.getValue() - b.getValue() + 1;
			if(ILP_DEBUG)
				std::cout << "   Geq: ub = " << ub << ", MU = " << MU << std::endl;
			impl.model->addConstraint(ILP::LinConstraint(ilp_Ax - MU * ilp_y,
														 ILP::LinConstraint::Relation::Leq,
														 ILP::Float(b.getValue() - 1)));
			break;
		}
		}
	}
public:
	const Model &model;
	const Int bigM;
	ILPImplementation &impl;
};

} // namespace

ILP::Var ILPImplementation::getVar(CombiOpt::Var var) const {
	const auto iter = vars.find(var.getData());
	assert(iter != end(vars));
	return iter->second;
}

ILPImplementation makeILPModel(const Model &model, Int bigM, const std::string &solver) {
	// see also https://link.springer.com/chapter/10.1007/3-540-45349-0_27
	ILPImplementation impl;
	impl.model = ILP::Model::createModel(solver);
	impl.vars.reserve(model.vars.size());
	ILP::Model &m = *impl.model;
	for(const auto &v: model.boolVars) {
		if(ILP_DEBUG)
			std::cout << "BoolVar: " << model.vars[v.id].name << ": [" << v.lb.getValue() << ", " << v.ub.getValue()
					  << "]" << std::endl;
		const auto var = m.addVariableBinary(model.vars[v.id].name);
		impl.vars.emplace(v.id, var);
		m.setLB(var, v.lb.getValue());
		if(v.ub != Int::INF_POS) m.setUB(var, v.ub.getValue());
	}
	for(const auto &v: model.intVars) {
		if(ILP_DEBUG)
			std::cout << "IntVar: " << model.vars[v.id].name << ": [" << v.lb.getValue() << ", " << v.ub.getValue()
					  << "]" << std::endl;
		const auto var = m.addVariableInteger(model.vars[v.id].name);
		impl.vars.emplace(v.id, var);
		m.setLB(var, v.lb.getValue());
		if(v.ub != Int::INF_POS) m.setUB(var, v.ub.getValue());
	}
	for(const auto &v: model.floatVars) {
		if(ILP_DEBUG)
			std::cout << "FloatVar: " << model.vars[v.id].name << ": [" << v.lb.getValue() << ", " << v.ub.getValue()
					  << "]" << std::endl;
		const auto var = m.addVariableFloat(model.vars[v.id].name);
		impl.vars.emplace(v.id, var);
		m.setLB(var, v.lb.getValue());
		if(v.ub != Float(Int::INF_POS)) m.setUB(var, v.ub.getValue());
	}
	Helper helper{model, bigM, impl};
	for(const auto &c: model.conjunctions) {
		ILP::LinExp expr;
		int val = 0;
		for(const auto &t: c) {
			if(t.negated) {
				// !b <=> (1 - b) == 1
				//    <=> -b == 1 - 1
				expr -= impl.getVar(t.v);
			} else {
				// b <=> b == 1
				expr += impl.getVar(t.v);
				++val;
			}
		}
		m.addConstraint(std::move(expr) == val);
	}
	for(const auto &c: model.disjunctions) {
		ILP::LinExp expr;
		int val = 1;
		for(const auto &t: c) {
			if(t.negated) {
				// !b <=> (1 - b) >= 1
				//    <=> -b >= 1 - 1
				expr -= impl.getVar(t.v);
				--val;
			} else {
				// b <=> b >= 1
				expr += impl.getVar(t.v);
			}
		}
		m.addConstraint(std::move(expr) >= val);
	}
	for(const auto &c: model.linConstraintsInt) {
		if(ILP_DEBUG)
			model.print(std::cout << "LinConstraintInt: ", c) << std::endl;
		ILP::LinExp expr = helper.convertExpr(c.getExpr());
		auto constr = ILP::LinConstraint(std::move(expr), helper.convertRelation(c.getRelation()),
										 ILP::Float(c.getBound()));
		m.addConstraint(std::move(constr));
	}
	for(const auto &c: model.linConstraintsFloat) {
		if(ILP_DEBUG)
			model.print(std::cout << "LinConstraintFloat: ", c) << std::endl;
		ILP::LinExp expr = helper.convertExpr(c.getExpr());
		auto constr = ILP::LinConstraint(std::move(expr), helper.convertRelation(c.getRelation()),
										 ILP::Float(c.getBound()));
		m.addConstraint(std::move(constr));
	}
	for(const auto &c: model.linConstraintsAny) {
		if(ILP_DEBUG)
			model.print(std::cout << "LinConstraintAny: ", c) << std::endl;
		ILP::LinExp expr = helper.convertExpr(c.getExpr());
		auto constr = ILP::LinConstraint(std::move(expr), helper.convertRelation(c.getRelation()),
										 ILP::Float(c.getBound()));
		m.addConstraint(std::move(constr));
	}
	for(const auto &p: model.implicationsInt) {
		const auto &c = p.right;
		const auto y = p.left;
		helper.addImplication(y, c);
	}
	for(const auto &p: model.implicationsFloat) {
		const auto &c = p.right;
		const auto y = p.left;
		helper.addImplication(y, c);
	}
	for(const auto &p: model.implicationsBool) {
		const BoolVar c = p.right;
		const bool negated = p.rightNegated;
		const Conjunction &v = p.left;
		//  v => c     <=>      v => c >= 1
		// c is forced true when v is true
		// c  >= 1 - M*(1 - v)                c - M*v >= 1 - M
		//
		// v => !c     <=>      v => c <= 0
		// c is forced false when v is true
		// c  <= 0 + M*(1 - v)                c + M*v <= M
		ILP::LinExp exp;
		Int Mval = 0;
		for(const auto term: v) {
			// if any of these are false, then disable the constraint
			if(term.negated) { // false is when var == 1
				// normal:  c - M*v >= 1 - M
				// negated: c + M*v <= M
				// and with the term negated:
				// normal:  c - M*(1 - v) >= 1 - M
				// negated: c + M*(1 - v) <= M
				exp += impl.getVar(term.v);
			} else { // false is when var == 0
				exp -= impl.getVar(term.v);
				Mval -= 1;
			}
		}
		//			print(std::cout << "Equiv: ", c) << " <=> " << vars[y.getData()].name << std::endl;
		//			std::cout << "lb = " << lb << ", ML = " << ML << ", ub = " << ub << ", MU = " << MU << std::endl;
		if(!negated) {
			m.addConstraint(ILP::LinConstraint(impl.getVar(c) + exp,
											   ILP::LinConstraint::Relation::Geq,
											   ILP::Float(1 + Mval)));
		} else {
			m.addConstraint(ILP::LinConstraint(impl.getVar(c) - exp,
											   ILP::LinConstraint::Relation::Leq,
											   ILP::Float(-Mval)));
		}
	}
	for(const auto &p: model.equivalences) {
		const auto &c = p.left;
		const auto y = p.right;
		helper.addImplication(y, c);
		helper.addImplicationNegated(y, c);
	}

	ILP::LinExp expr;
	model.objectiveFunction.forEach([&](const auto coef, const auto var) {
		expr += coef * ILP::LinExp(impl.getVar(var));
	});
	m.setObjectiveFunction(std::move(expr));

	return impl;
}


} // namespace mod::lib::CombiOpt