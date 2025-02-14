#ifndef MOD_LIB_GRAPH_MULTISETIO_HPP
#define MOD_LIB_GRAPH_MULTISETIO_HPP

#include <mod/lib/Graph/Multiset.hpp>

#include <ostream>

namespace mod::lib::graph {

template<typename GraphType>
std::ostream &operator<<(std::ostream &s, const Multiset<GraphType> &ms) {
	if(ms.size() == 1) return s << (*ms.begin())->getName();
	s << "{ ";
	for(const auto *g : ms) s << "'" << g->getName() << "' ";
	return s << "}";
}

} // namespace mod::lib::graph

#endif // MOD_LIB_GRAPH_MULTISETIO_HPP