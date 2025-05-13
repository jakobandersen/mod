#ifndef MOD_LIB_COMBIOPT_LINCONSTRAINTINT_HPP
#define MOD_LIB_COMBIOPT_LINCONSTRAINTINT_HPP

#include <mod/lib/CombiOpt/LinExpInt.hpp>

namespace mod::lib::CombiOpt {

struct LinConstraintInt {
	// represents: exp relation bound

	LinConstraintInt(LinExpInt exp, Relation relation, Int bound)
			: exp(exp), relation(relation), bound(bound) {}

	friend bool operator==(const LinConstraintInt &a, const LinConstraintInt &b) {
		return std::tie(a.exp, a.relation, a.bound) == std::tie(b.exp, b.relation, b.bound);
	}

	friend bool operator<(const LinConstraintInt &a, const LinConstraintInt &b) {
		return std::tie(a.exp, a.relation, a.bound) < std::tie(b.exp, b.relation, b.bound);
	}

	const LinExpInt &getExpr() const {
		return exp;
	}

	const Relation &getRelation() const {
		return relation;
	}

	Int getBound() const {
		return bound;
	}
private:
	LinExpInt exp;
	Relation relation;
	Int bound;
};

#define MOD_MAKE_OP(Op1, Op2, Rel, Type)                                        \
    inline LinConstraintInt operator Op1(LinExpInt e, Type c) {                 \
        return LinConstraintInt(e, Relation::Rel, Int(c));                      \
    }                                                                           \
    inline LinConstraintInt operator Op2(Type c, LinExpInt e) {                 \
        return e Op1 c;                                                         \
    }
MOD_MAKE_OP(<=, >=, Leq, int)
MOD_MAKE_OP(<=, >=, Leq, Int)
MOD_MAKE_OP(>=, <=, Geq, int)
MOD_MAKE_OP(>=, <=, Geq, Int)
MOD_MAKE_OP(==, ==, Eq, int)
MOD_MAKE_OP(==, ==, Eq, Int)
#undef MOD_MAKE_OP

} // namespace mod::lib::CombiOpt

#endif // MOD_LIB_COMBIOPT_LINCONSTRAINTINT_HPP