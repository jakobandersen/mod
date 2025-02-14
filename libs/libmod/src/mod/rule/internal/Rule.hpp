#ifndef MOD_RULE_INTERNAL_RULE_HPP
#define MOD_RULE_INTERNAL_RULE_HPP

#include <mod/BuildConfig.hpp>
#include <mod/rule/Rule.hpp>
#include <mod/lib/Rule/LabelledRule.hpp>

namespace mod::rule::internal {

MOD_DECL lib::rule::LabelledRule makeLabelledRule();
MOD_DECL lib::rule::GraphType &getGraph(lib::rule::LabelledRule &r);

MOD_DECL std::unique_ptr<lib::rule::PropString> makePropStringCore(const lib::rule::LabelledRule &rule);
MOD_DECL void add(lib::rule::PropString &pString, lib::DPO::CombinedRule::CombinedVertex v,
                  const std::string &valueLeft, const std::string &valueRight);
MOD_DECL void add(lib::rule::PropString &pString, lib::DPO::CombinedRule::CombinedEdge e,
                  const std::string &valueLeft, const std::string &valueRight);
MOD_DECL void setRight(lib::rule::PropString &pString, lib::DPO::CombinedRule::CombinedEdge e,
                       const std::string &value);
MOD_DECL lib::rule::PropMolecule makePropMoleculeCore(const lib::rule::LabelledRule &rule,
                                                       const lib::rule::PropString &str);

MOD_DECL std::shared_ptr<Rule> makeRule(lib::rule::LabelledRule &&r);

MOD_DECL const std::string &getStringLeft(lib::DPO::CombinedRule::CombinedVertex v,
                                          const lib::rule::PropString &str);
MOD_DECL const std::string &getStringRight(lib::DPO::CombinedRule::CombinedVertex v,
                                           const lib::rule::PropString &str);
MOD_DECL BondType getMoleculeLeft(lib::DPO::CombinedRule::CombinedEdge e,
                                  const lib::rule::PropMolecule &mol);
MOD_DECL BondType getMoleculeRight(lib::DPO::CombinedRule::CombinedEdge e,
                                   const lib::rule::PropMolecule &mol);

} // namespace mod::rule::internal

#endif // MOD_RULE_INTERNAL_RULE_HPP
