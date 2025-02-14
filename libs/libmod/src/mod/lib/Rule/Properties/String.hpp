#ifndef MOD_LIB_RULES_PROP_STRING_HPP
#define MOD_LIB_RULES_PROP_STRING_HPP

#include <mod/lib/GraphMorphism/Constraints/Constraint.hpp>
#include <mod/lib/Rule/GraphDecl.hpp>
#include <mod/lib/Rule/Properties/Property.hpp>

namespace mod::lib {
struct StringStore;
} // namespace mod::lib
namespace mod::lib::rule {
struct PropTerm;

struct PropString : PropBase<PropString, std::string, std::string> {
	using ConstraintPtr = std::unique_ptr<GraphMorphism::Constraints::Constraint<lib::DPO::CombinedRule::SideGraphType>>;
public:
	explicit PropString(const RuleType &rule);
	PropString(const RuleType &rule,
	           const std::vector<ConstraintPtr> &leftMatchConstraints,
	           const std::vector<ConstraintPtr> &rightMatchConstraints,
	           const PropTerm &term, const StringStore &strings);
};

} // namespace mod::lib::rule

#endif // MOD_LIB_RULES_PROP_STRING_HPP