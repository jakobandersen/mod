#ifndef MOD_PY_ERROR_HPP
#define MOD_PY_ERROR_HPP

#include <mod/py/Common.hpp>

#include <mod/Error.hpp>

namespace mod::Py {

struct BOOST_SYMBOL_VISIBLE MethodOverrideError : public Exception {
	MethodOverrideError(std::string &&method, std::string &&baseClass, std::string &&error);
	std::string getName() const { return "MØD MethodOverrideError"; }
};

} // namespace mod::Py

#endif // MOD_PY_ERROR_HPP