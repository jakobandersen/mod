#include "GraphAsRuleCache.hpp"

#include <mod/rule/Rule.hpp>
#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/Graph/Properties/Stereo.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/Rule/GraphToRule.hpp>

namespace mod::lib::rule {

std::shared_ptr<mod::rule::Rule> GraphAsRuleCache::getBindRule(const lib::graph::Graph *g) {
	return getRule(g, Membership::R);
}

std::shared_ptr<mod::rule::Rule> GraphAsRuleCache::getIdRule(const lib::graph::Graph *g) {
	return getRule(g, Membership::K);
}

std::shared_ptr<mod::rule::Rule> GraphAsRuleCache::getUnbindRule(const lib::graph::Graph *g) {
	return getRule(g, Membership::L);
}

std::shared_ptr<mod::rule::Rule> GraphAsRuleCache::getRule(const lib::graph::Graph *g, lib::DPO::Membership m) {
	const auto iter = storage.find({g, m});
	if(iter != end(storage)) return iter->second;
	auto r = mod::rule::Rule::makeRule(graphToRule(g->getLabelledGraph(), m, g->getName()));
	storage.emplace(std::make_pair(g, m), r);
	return r;
}

} // namespace mod::lib::rule