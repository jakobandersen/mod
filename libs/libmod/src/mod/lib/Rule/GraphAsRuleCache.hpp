#ifndef MOD_LIB_RULES_GRAPHASRULECACHE_HPP
#define MOD_LIB_RULES_GRAPHASRULECACHE_HPP

#include <mod/rule/ForwardDecl.hpp>
#include <mod/lib/DPO/Membership.hpp>

#include <map>
#include <memory>

namespace mod::lib::graph {
struct Graph;
} // namespace mod::lib::graph
namespace mod::lib::rule {

struct GraphAsRuleCache {
	std::shared_ptr<mod::rule::Rule> getBindRule(const lib::graph::Graph *g);
	std::shared_ptr<mod::rule::Rule> getIdRule(const lib::graph::Graph *g);
	std::shared_ptr<mod::rule::Rule> getUnbindRule(const lib::graph::Graph *g);
private:
	std::shared_ptr<mod::rule::Rule> getRule(const lib::graph::Graph *g, lib::DPO::Membership m);
private:
	std::map<std::pair<const lib::graph::Graph *, lib::DPO::Membership>, std::shared_ptr<mod::rule::Rule>> storage;
};

} // namespace mod::lib::rule

#endif // MOD_LIB_RULES_GRAPHASRULECACHE_HPP
