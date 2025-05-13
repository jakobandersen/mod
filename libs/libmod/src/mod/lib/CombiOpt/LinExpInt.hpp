#ifndef MOD_LIB_COMBIOPT_LINEXPINT_HPP
#define MOD_LIB_COMBIOPT_LINEXPINT_HPP

#include <mod/lib/CombiOpt/Value.hpp>
#include <mod/lib/CombiOpt/Variable.hpp>

#include <vector>

namespace mod::lib::CombiOpt {

struct LinExpInt :
		boost::additive<LinExpInt,
				boost::additive<LinExpInt, IntVar,
						boost::multipliable<LinExpInt, Int>>> {
	using IntTerm = std::pair<Int, IntVar>;
public:
	LinExpInt() = default;

	LinExpInt(IntVar var) {
		terms.emplace_back(1, var);
	}

	friend bool operator==(const LinExpInt &a, const LinExpInt &b) {
		return a.terms == b.terms;
	}

	friend bool operator<(const LinExpInt &a, const LinExpInt &b) {
		return a.terms < b.terms;
	}

	const std::vector<IntTerm> &getTerms() const {
		return terms;
	}

	LinExpInt &operator+=(const LinExpInt &e) {
		const auto &is = e.getTerms();
		terms.insert(end(terms), begin(is), end(is));
		return *this;
	}

	LinExpInt &operator-=(const LinExpInt &e) {
		for(const auto &el : e.getTerms())
			terms.emplace_back(-el.first, el.second);
		return *this;
	}

	LinExpInt &operator*=(Int c) {
		for(auto &el : terms) el.first *= c;
		return *this;
	}
private:
	std::vector<IntTerm> terms;
};

inline LinExpInt operator-(LinExpInt e) {
	return LinExpInt() - e;
}

#define MOD_MAKE_OP(Op)                                                         \
    inline LinExpInt operator Op(IntVar lhs, IntVar rhs) {                      \
        LinExpInt res(lhs);                                                     \
        return res Op##= rhs;                                                   \
    }
MOD_MAKE_OP(+)
MOD_MAKE_OP(-)
#undef MOD_MAKE_OP

inline LinExpInt operator*(Int lhs, IntVar rhs) {
	LinExpInt res(rhs);
	return res * lhs;
}

} // namespace mod::lib::CombiOpt

#endif // MOD_LIB_COMBIOPT_LINEXPINT_HPP