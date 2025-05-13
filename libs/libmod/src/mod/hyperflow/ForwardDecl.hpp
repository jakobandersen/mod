#ifndef MOD_HYPERFLOW_FORWARDDECL_HPP
#define MOD_HYPERFLOW_FORWARDDECL_HPP

// rst: This header contains declarations for all classes in the ``mod::hyperflow`` namespace.

namespace mod::hyperflow {
struct Model;
struct ModelImplementationView;
struct LinConstraint;
struct LinExp;
struct Printer;
struct SolutionRange;
struct Solution;
struct Var;
struct VarCustom;
struct VarEdge;
struct VarVertex;
struct VarVertexGraph;
} // namespace mod::hyperflow
namespace mod::lib::HyperFlow {
struct Flow;
} // namespace mod::lib::HyperFlow
namespace mod::lib::HyperFlow::Write {
struct Printer;
} // namespace mod::lib::HyperFlow::Write

#endif // MOD_HYPERFLOW_FORWARDDECL_HPP