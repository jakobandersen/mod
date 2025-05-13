#ifndef MOD_LIB_COMBIOPT_ILPIMPLEMENTATION_HPP
#define MOD_LIB_COMBIOPT_ILPIMPLEMENTATION_HPP

#include <mod/lib/ILP/ILP.hpp>

#include <memory>
#include <unordered_map>

namespace mod::lib::CombiOpt {
struct Int;
struct Model;
struct Var;

struct ILPImplementation {
	ILP::Var getVar(CombiOpt::Var var) const;
public:
	std::unique_ptr<ILP::Model> model;
	std::unordered_map<std::size_t, ILP::Var> vars;
};

ILPImplementation makeILPModel(const Model &model, Int bigM, const std::string &solver);

} // namespace mod::lib::CombiOpt

#endif // MOD_LIB_COMBIOPT_ILPIMPLEMENTATION_HPP
