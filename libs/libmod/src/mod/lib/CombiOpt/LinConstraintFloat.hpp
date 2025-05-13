#ifndef MOD_LIB_COMBIOPT_LINCONSTRAINTFLOAT_HPP
#define MOD_LIB_COMBIOPT_LINCONSTRAINTFLOAT_HPP

#include <mod/lib/CombiOpt/LinExpFloat.hpp>

namespace mod::lib::CombiOpt {

struct LinConstraintFloat {
	// represents: exp relation bound

	LinConstraintFloat(LinExpFloat exp, Relation relation, Float bound)
			: exp(exp), relation(relation), bound(bound) {}

	friend bool operator==(const LinConstraintFloat &a, const LinConstraintFloat &b) {
		return std::tie(a.exp, a.relation, a.bound) == std::tie(b.exp, b.relation, b.bound);
	}

	friend bool operator<(const LinConstraintFloat &a, const LinConstraintFloat &b) {
		return std::tie(a.exp, a.relation, a.bound) < std::tie(b.exp, b.relation, b.bound);
	}

	const LinExpFloat &getExpr() const {
		return exp;
	}

	const Relation &getRelation() const {
		return relation;
	}

	Float getBound() const {
		return bound;
	}

private:
	LinExpFloat exp;
	Relation relation;
	Float bound;
};

#define MOD_MAKE_OP(Op1, Op2, Rel, Type)                                        \
    inline LinConstraintFloat operator Op1(LinExpFloat e, Type c) {             \
        return LinConstraintFloat(e, Relation::Rel, Float(c));                  \
    }                                                                           \
    inline LinConstraintFloat operator Op2(Type c, LinExpFloat e) {             \
        return e Op1 c;                                                         \
    }

MOD_MAKE_OP(<=, >=, Leq, double)
MOD_MAKE_OP(<=, >=, Leq, Float)
MOD_MAKE_OP(>=, <=, Geq, double)
MOD_MAKE_OP(>=, <=, Geq, Float)
MOD_MAKE_OP(==, ==, Eq, double)
MOD_MAKE_OP(==, ==, Eq, Float)
#undef MOD_MAKE_OP

} // namespace mod::lib::CombiOpt

#endif // MOD_LIB_COMBIOPT_LINCONSTRAINTFLOAT_HPP