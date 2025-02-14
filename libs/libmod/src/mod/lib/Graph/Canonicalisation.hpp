#ifndef MOD_LIB_GRAPH_CANONICALISATION_HPP
#define MOD_LIB_GRAPH_CANONICALISATION_HPP

#include <mod/lib/Graph/Graph.hpp>

namespace mod::lib::graph {

std::tuple<std::vector<int>, std::unique_ptr<Graph::CanonForm>, std::unique_ptr<Graph::AutGroup>>
getCanonForm(const Graph &g, LabelType labelType, bool withStereo);

bool canonicalCompare(const Graph &g1, const Graph &g2, LabelType labelType, bool withStereo);

} // namespace mod::lib::graph

#endif // MOD_LIB_GRAPH_CANONICALISATION_HPP