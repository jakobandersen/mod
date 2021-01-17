#include "GraphAsRuleCache.hpp"

#include <mod/rule/Rule.hpp>
#include <mod/lib/Graph/Single.hpp>
#include <mod/lib/Graph/Properties/Stereo.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/Rules/GraphToRule.hpp>

namespace mod::lib::Rules {

std::shared_ptr<rule::Rule> GraphAsRuleCache::getBindRule(const lib::Graph::Single *g) {
	return getRule(g, Membership::Right);
}

std::shared_ptr<rule::Rule> GraphAsRuleCache::getIdRule(const lib::Graph::Single *g) {
	return getRule(g, Membership::Context);
}

std::shared_ptr<rule::Rule> GraphAsRuleCache::getUnbindRule(const lib::Graph::Single *g) {
	return getRule(g, Membership::Left);
}

std::shared_ptr<rule::Rule> GraphAsRuleCache::getRule(const lib::Graph::Single *g, jla_boost::GraphDPO::Membership m) {
	const auto iter = storage.find({g, m});
	if(iter != end(storage)) return iter->second;
	auto r = rule::Rule::makeRule(graphToRule(g->getLabelledGraph(), m, g->getName()));
	storage.emplace(std::make_pair(g, m), r);
	return r;
}

} // namespace mod::lib::Rules