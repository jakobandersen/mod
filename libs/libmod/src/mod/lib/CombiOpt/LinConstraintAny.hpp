#ifndef MOD_LIB_COMBIOPT_LINCONSTRAINTANY_HPP
#define MOD_LIB_COMBIOPT_LINCONSTRAINTANY_HPP

#include <mod/lib/CombiOpt/LinExpAny.hpp>

namespace mod {
namespace lib {
namespace CombiOpt {

struct LinConstraintAny {
	// represents: exp relation bound

	LinConstraintAny(LinExpAny exp, Relation relation, Float bound)
			: exp(exp), relation(relation), bound(bound) {}

	friend bool operator==(const LinConstraintAny &a, const LinConstraintAny &b) {
		return std::tie(a.exp, a.relation, a.bound) == std::tie(b.exp, b.relation, b.bound);
	}

	friend bool operator<(const LinConstraintAny &a, const LinConstraintAny &b) {
		return std::tie(a.exp, a.relation, a.bound) < std::tie(b.exp, b.relation, b.bound);
	}

	const LinExpAny &getExpr() const {
		return exp;
	}

	const Relation &getRelation() const {
		return relation;
	}

	Float getBound() const {
		return bound;
	}
private:
	LinExpAny exp;
	Relation relation;
	Float bound;
};

} // namespace CombiOpt
} // namespace lib
} // namespace mod

#endif //MOD_LIB_COMBIOPT_LINCONSTRAINTANY_HPP