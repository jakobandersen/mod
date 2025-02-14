#ifndef MOD_LIB_RULE_GRAPHDECL_HPP
#define MOD_LIB_RULE_GRAPHDECL_HPP

#include <mod/lib/DPO/CombinedRule.hpp>
#include <mod/lib/DPO/FilteredGraphProjection.hpp>

namespace mod::lib::rule {
using lib::DPO::Membership;

using GraphType = lib::DPO::CombinedRule::CombinedGraphType;
using Vertex = boost::graph_traits<GraphType>::vertex_descriptor;
using Edge = boost::graph_traits<GraphType>::edge_descriptor;

} // namespace mod::lib::rule

#endif // MOD_LIB_RULE_GRAPHDECL_HPP