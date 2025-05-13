#ifndef MOD_LIB_COMBIOPT_LINEXPFLOAT_HPP
#define MOD_LIB_COMBIOPT_LINEXPFLOAT_HPP

#include <mod/lib/CombiOpt/LinExpInt.hpp>
#include <mod/lib/CombiOpt/Value.hpp>
#include <mod/lib/CombiOpt/Variable.hpp>

#include <vector>

namespace mod::lib::CombiOpt {

struct LinExpFloat :
		boost::additive<LinExpFloat,
				boost::additive<LinExpFloat, FloatVar,
						boost::additive<LinExpFloat, Var,
								boost::multipliable<LinExpFloat, Int,
										boost::multipliable<LinExpFloat, Float>>>>> {
	using IntTerm = LinExpInt::IntTerm;
	using FloatTerm = std::pair<Float, Var>;
public:
	LinExpFloat() = default;
	LinExpFloat(IntVar var) : intExp(var) {}
	LinExpFloat(FloatVar var) {
		floatTerms.emplace_back(1, var);
	}

	friend bool operator==(const LinExpFloat &a, const LinExpFloat &b) {
		return std::tie(a.intExp, a.floatTerms) == std::tie(b.intExp, b.floatTerms);
	}

	friend bool operator<(const LinExpFloat &a, const LinExpFloat &b) {
		return std::tie(a.intExp, a.floatTerms) < std::tie(b.intExp, b.floatTerms);
	}

	const LinExpInt &getIntExp() const {
		return intExp;
	}

	const std::vector<IntTerm> &getIntTerms() const {
		return intExp.getTerms();
	}

	const std::vector<FloatTerm> &getFloatTerms() const {
		return floatTerms;
	}

	LinExpFloat &operator+=(const LinExpFloat &e) {
		intExp += e.intExp;
		const auto &is = e.getFloatTerms();
		floatTerms.insert(end(floatTerms), begin(is), end(is));
		return *this;
	}

	LinExpFloat &operator-=(const LinExpFloat &e) {
		intExp -= e.intExp;
		for(const auto &el : e.getFloatTerms())
			floatTerms.emplace_back(-el.first, el.second);
		return *this;
	}

	LinExpFloat &operator*=(Int c) {
		intExp *= c;
		for(auto &el : floatTerms) el.first *= Float(c);
		return *this;
	}

	LinExpFloat &operator*=(Float c) {
		for(auto &el : intExp.getTerms())
			floatTerms.emplace_back(Float(el.first), el.second);
		intExp = LinExpInt();
		for(auto &el : floatTerms)
			el.first *= c;
		return *this;
	}

	bool isIntegral() const {
		return floatTerms.empty();
	}
private:
	LinExpInt intExp;
	std::vector<FloatTerm> floatTerms;
};

inline LinExpFloat operator-(LinExpFloat e) {
	return LinExpFloat() - e;
}

#define MOD_MAKE_OP(Op, VL, VR)                                                 \
    inline LinExpFloat operator Op(VL lhs, VR rhs) {                            \
        LinExpFloat res(lhs);                                                   \
        return res Op##= rhs;                                                   \
    }
//MOD_MAKE_OP(+);
//MOD_MAKE_OP(-);
#undef MOD_MAKE_OP

inline LinExpFloat operator*(Float lhs, FloatVar rhs) {
	LinExpFloat res(rhs);
	return res * lhs;
}

} // namespace mod::lib::CombiOpt

#endif // MOD_LIB_COMBIOPT_LINEXPFLOAT_HPP