#ifndef MOD_LIB_COMBIOPT_LOADEDSOLUTION_HPP
#define MOD_LIB_COMBIOPT_LOADEDSOLUTION_HPP

#include <mod/lib/CombiOpt/Value.hpp>
#include <mod/lib/CombiOpt/Variable.hpp>

#include <unordered_map>

namespace mod {
namespace lib {
namespace CombiOpt {

struct LoadedSolution {
	Value objVal;
	std::unordered_map<IntegralVar, Int> integralValues;
	std::unordered_map<FloatVar, Float> floatValues;
public:
	Int getVal(IntegralVar v) const {
		const auto iter = integralValues.find(v);
		if(iter == end(integralValues)) return 0;
		else return iter->second;
	}

	Float getVal(FloatVar v) const {
		const auto iter = floatValues.find(v);
		if(iter == end(floatValues)) return 0.0;
		else return iter->second;
	}
};

} // namespace CombiOpt
} // namespace lib
} // namespace mod

#endif //MOD_LIB_COMBIOPT_LOADEDSOLUTION_HPP