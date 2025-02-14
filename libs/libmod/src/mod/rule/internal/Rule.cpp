#include "Rule.hpp"

#include <mod/lib/Rule/LabelledRule.hpp>
#include <mod/lib/Rule/Rule.hpp>

namespace mod::rule::internal {

lib::rule::LabelledRule makeLabelledRule() {
	return lib::rule::LabelledRule();
}

lib::rule::LabelledRule::GraphType &getGraph(lib::rule::LabelledRule &r) {
	return get_graph(r);
}

std::unique_ptr<lib::rule::PropString> makePropStringCore(const lib::rule::LabelledRule &rule) {
	return std::make_unique<lib::rule::PropString>(rule.getRule());
}

void add(lib::rule::PropString &pString, lib::DPO::CombinedRule::CombinedVertex v,
         const std::string &valueLeft, const std::string &valueRight) {
	pString.add(v, valueLeft, valueRight);
}

void add(lib::rule::PropString &pString, lib::DPO::CombinedRule::CombinedEdge e,
         const std::string &valueLeft, const std::string &valueRight) {
	pString.add(e, valueLeft, valueRight);
}

void setRight(lib::rule::PropString &pString, lib::DPO::CombinedRule::CombinedEdge e, const std::string &value) {
	pString.setRight(e, value);
}

lib::rule::PropMolecule
makePropMoleculeCore(const lib::rule::LabelledRule &rule, const lib::rule::PropString &str) {
	return lib::rule::PropMolecule(rule.getRule(), str);
}

std::shared_ptr<Rule> makeRule(lib::rule::LabelledRule &&r) {
	auto rLib = std::make_unique<mod::lib::rule::Rule>(std::move(r), std::nullopt);
	return mod::rule::Rule::makeRule(std::move(rLib));
}

const std::string &getStringLeft(lib::DPO::CombinedRule::CombinedVertex v,
                                 const lib::rule::PropString &str) {
	return str.getLeft()[v];
}

const std::string &getStringRight(lib::DPO::CombinedRule::CombinedVertex v,
                                  const lib::rule::PropString &str) {
	return str.getRight()[v];
}

BondType getMoleculeLeft(lib::DPO::CombinedRule::CombinedEdge e,
                         const lib::rule::PropMolecule &mol) {
	return mol.getLeft()[e];
}

BondType getMoleculeRight(lib::DPO::CombinedRule::CombinedEdge e,
                          const lib::rule::PropMolecule &mol) {
	return mol.getRight()[e];
}

} // namespace mod::rule::internal