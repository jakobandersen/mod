#ifndef MOD_LIB_COMBIOPT_BOOLEXPR_HPP
#define MOD_LIB_COMBIOPT_BOOLEXPR_HPP

#include <mod/lib/CombiOpt/Variable.hpp>

#include <vector>

namespace mod {
namespace lib {
namespace CombiOpt {

struct NegatedBoolVar {
	BoolVar v;
};

inline NegatedBoolVar operator!(BoolVar v) {
	return {v};
}

struct Conjunction {
	struct Term {
		BoolVar v;
		bool negated;
	public:
		friend bool operator==(Term a, Term b);
		friend bool operator<(Term a, Term b);
	};
public:
	Conjunction() = default;
	Conjunction(BoolVar v) : vars(1, {v, false}) {}
	Conjunction(NegatedBoolVar v) : vars(1, {v.v, true}) {}

	Term front() const {
		return vars.front();
	}

	auto size() const {
		return vars.size();
	}

	auto begin() const {
		return vars.begin();
	}

	auto end() const {
		return vars.end();
	}
public:
	friend bool operator==(const Conjunction &a, const Conjunction &b);
	friend bool operator<(const Conjunction &a, const Conjunction &b);
	friend Conjunction operator&&(Conjunction a, BoolVar v);
	friend Conjunction operator&&(Conjunction a, NegatedBoolVar v);
private:
	std::vector<Term> vars;
};

struct Disjunction {
	struct Term {
		BoolVar v;
		bool negated;
	public:
		friend bool operator==(Term a, Term b);
		friend bool operator<(Term a, Term b);
	};
public:
	Disjunction() = default;

	Term front() const {
		return vars.front();
	}

	auto size() const {
		return vars.size();
	}

	auto begin() const {
		return vars.begin();
	}

	auto end() const {
		return vars.end();
	}
public:
	friend bool operator==(const Disjunction &a, const Disjunction &b);
	friend bool operator<(const Disjunction &a, const Disjunction &b);
	friend Disjunction operator||(Disjunction a, BoolVar v);
	friend Disjunction operator||(Disjunction a, NegatedBoolVar v);
private:
	std::vector<Term> vars;
};

// ============================================================================
// Implementation
// ============================================================================

inline bool operator==(Conjunction::Term a, Conjunction::Term b) {
	return std::tie(a.v, a.negated) == std::tie(b.v, b.negated);
}

inline bool operator<(Conjunction::Term a, Conjunction::Term b) {
	return std::tie(a.v, a.negated) < std::tie(b.v, b.negated);
}

inline bool operator==(const Conjunction &a, const Conjunction &b) {
	return a.vars == b.vars;
}

inline bool operator<(const Conjunction &a, const Conjunction &b) {
	return a.vars < b.vars;
}

inline Conjunction operator&&(Conjunction a, BoolVar v) {
	a.vars.push_back({v, false});
	return a;
}

inline Conjunction operator&&(Conjunction a, NegatedBoolVar v) {
	a.vars.push_back({v.v, true});
	return a;
}

// ----------------------------------------------------------------------------

inline bool operator==(Disjunction::Term a, Disjunction::Term b) {
	return std::tie(a.v, a.negated) == std::tie(b.v, b.negated);
}

inline bool operator<(Disjunction::Term a, Disjunction::Term b) {
	return std::tie(a.v, a.negated) < std::tie(b.v, b.negated);
}

inline bool operator==(const Disjunction &a, const Disjunction &b) {
	return a.vars == b.vars;
}

inline bool operator<(const Disjunction &a, const Disjunction &b) {
	return a.vars < b.vars;
}

inline Disjunction operator||(Disjunction a, BoolVar v) {
	a.vars.push_back({v, false});
	return a;
}

inline Disjunction operator||(Disjunction a, NegatedBoolVar v) {
	a.vars.push_back({v.v, true});
	return a;
}

} // namespace CombiOpt
} // namespace lib
} // namespace mod

#endif // MOD_LIB_COMBIOPT_BOOLEXPR_HPP