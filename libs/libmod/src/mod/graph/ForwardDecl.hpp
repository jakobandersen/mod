#ifndef MOD_GRAPH_FORWARDDECL_HPP
#define MOD_GRAPH_FORWARDDECL_HPP

// rst: This header contains declarations for all classes in the ``mod::graph`` namespace.

namespace mod::graph {
struct Graph;
struct GraphLess;
struct Printer;
} // namespace mod::graph
namespace mod::lib {
template<typename LGraph>
struct LabelledUnionGraph;
} // namespace mod::lib
namespace mod::lib::graph {
struct LabelledGraph;
struct Graph;
} // namespace mod::lib::graph
namespace mod::lib::IO::Graph::Write {
struct Options;
} // namespace mod::lib::IO::Graph::Write

#endif // MOD_GRAPH_FORWARDDECL_HPP