#ifndef MOD_LIB_RULES_PROP_TERM_HPP
#define MOD_LIB_RULES_PROP_TERM_HPP

#include <mod/lib/GraphMorphism/Constraints/Constraint.hpp>
#include <mod/lib/Rule/GraphDecl.hpp>
#include <mod/lib/Rule/Properties/Property.hpp>
#include <mod/lib/Term/WAM.hpp>

#include <optional>
#include <string>

namespace mod::lib::rule {
struct PropString;

struct PropTerm : PropBase<PropTerm, std::size_t, std::size_t> {
	using Base = PropBase<PropTerm, std::size_t, std::size_t>;
	using ConstraintPtr = std::unique_ptr<GraphMorphism::Constraints::Constraint<lib::DPO::CombinedRule::SideGraphType>>;
public:
	PropTerm(const RuleType &rule,
	         const std::vector<ConstraintPtr> &leftMatchConstraints,
	         const std::vector<ConstraintPtr> &rightMatchConstraints,
	         const PropString &pString, const StringStore &stringStore); // parse-construct
	PropTerm(const RuleType &rule, lib::Term::Wam machine); // import a machine
	friend bool isValid(const PropTerm &core);
	const std::string &getParsingError() const;
	friend lib::Term::Wam &getMachine(PropTerm &core);
	friend const lib::Term::Wam &getMachine(const PropTerm &core);
private:
	std::optional<std::string> parsingError;
	lib::Term::Wam machine;
};

inline const lib::Term::Wam &getMachine(const PropTerm::Side &p) {
	return getMachine(p.p.getDerived());
}

inline bool isValid(const PropTerm::Side &p) {
	return isValid(p.p.getDerived());
}

} // namespace mod::lib::rule

#endif // MOD_LIB_RULES_PROP_TERM_HPP