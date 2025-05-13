#ifndef MOD_LIB_HYPERFLOW_UTILS_HPP
#define MOD_LIB_HYPERFLOW_UTILS_HPP

#include <mod/lib/DG/GraphDecl.hpp>
#include <mod/lib/IO/Json.hpp>

#include <optional>

namespace mod::hyperflow {
struct Var;
struct LinExp;
struct LinConstraint;
} // namespace mod::hyperflow
namespace mod::lib::CombiOpt {
struct BoolVar;
struct Model;
} // namespace mod::lib::CombiOpt
namespace mod::lib::HyperFlow {
struct Specification;
struct Model;

void addDisableInternalIfTrue(Model &model, CombiOpt::Model &cModel, CombiOpt::BoolVar var, lib::DG::HyperVertex v);

nlohmann::json dump(const Specification &spec, const hyperflow::Var &var);
std::optional<hyperflow::Var>
loadVar(const Specification &spec, const nlohmann::json &j, std::ostream &err);

nlohmann::json dump(const Specification &spec, const hyperflow::LinExp &exp);
std::optional<hyperflow::LinExp>
loadExp(const Specification &spec, const nlohmann::json &j, std::ostream &err);

nlohmann::json dump(const Specification &spec, const hyperflow::LinConstraint &c);
std::optional<hyperflow::LinConstraint>
loadConstraint(const Specification &spec, const nlohmann::json &j, std::ostream &err);

} // namespace mod::lib::HyperFlow

#endif // MOD_LIB_HYPERFLOW_UTILS_HPP