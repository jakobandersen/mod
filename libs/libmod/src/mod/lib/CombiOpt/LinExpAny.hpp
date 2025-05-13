#ifndef MOD_LIB_COMBIOPT_LINEXPANY_HPP
#define MOD_LIB_COMBIOPT_LINEXPANY_HPP

#include <mod/lib/CombiOpt/LinExpFloat.hpp>

#include <vector>

namespace mod::lib::CombiOpt {

struct LinExpAny {
	template<typename F>
	void forEachIntegral(F f) const {
		for(const auto&[coef, var] : boolVarsInt)
			f(coef, var);
		for(const auto&[coef, var] : rest.getIntTerms())
			f(coef, var);
	}

	template<typename F>
	void forEachFloat(F f) const {
		for(const auto&[coef, var] : boolVarsFloat)
			f(coef, var);
		for(const auto&[coef, var] : rest.getFloatTerms())
			f(coef, var);
	}

	template<typename F>
	void forEach(F f) const {
		forEachIntegral(f);
		forEachFloat(f);
	}

	friend LinExpAny operator*(Int i, LinExpAny e) {
		for(auto &p : e.boolVarsInt)
			p.first *= i;
		for(auto &p : e.boolVarsFloat)
			p.first *= Float(i);
		e.rest *= i;
		return e;
	}

	friend LinExpAny operator*(Float f, LinExpAny e) {
		for(auto &p : e.boolVarsFloat)
			p.first *= f;
		for(const auto &p : e.boolVarsInt)
			e.boolVarsFloat.emplace_back(Float(p.first) * f, p.second);
		e.boolVarsInt.clear();
		e.rest *= f;
		return e;
	}

	LinExpAny &operator+=(BoolVar v) {
		boolVarsInt.emplace_back(1, v);
		return *this;
	}

	LinExpAny &operator+=(IntVar v) {
		rest += v;
		return *this;
	}

	LinExpAny &operator+=(FloatVar v) {
		rest += v;
		return *this;
	}

	LinExpAny &operator+=(const LinExpAny &e) {
		boolVarsInt.insert(boolVarsInt.end(), e.boolVarsInt.begin(), e.boolVarsInt.end());
		boolVarsFloat.insert(boolVarsFloat.end(), e.boolVarsFloat.begin(), e.boolVarsFloat.end());
		rest += e.rest;
		return *this;
	}

	bool isIntegral() const {
		return boolVarsFloat.empty() && rest.isIntegral();
	}

	friend bool operator==(const LinExpAny &a, const LinExpAny &b) {
		return std::tie(a.boolVarsInt, a.boolVarsFloat, a.rest)
		       == std::tie(b.boolVarsInt, b.boolVarsFloat, b.rest);
	}

	friend bool operator<(const LinExpAny &a, const LinExpAny &b) {
		return std::tie(a.boolVarsInt, a.boolVarsFloat, a.rest)
		       < std::tie(b.boolVarsInt, b.boolVarsFloat, b.rest);
	}
public:
	std::vector<std::pair<Int, BoolVar>> boolVarsInt;
	std::vector<std::pair<Float, BoolVar>> boolVarsFloat;
	LinExpFloat rest;
};

} // namespace mod::lib::CombiOpt

#endif // MOD_LIB_COMBIOPT_LINEXPANY_HPP