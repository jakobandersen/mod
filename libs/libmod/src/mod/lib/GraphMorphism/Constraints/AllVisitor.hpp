#ifndef MOD_LIB_GRAPHMORPHISM_ALLVISITOR_HPP
#define MOD_LIB_GRAPHMORPHISM_ALLVISITOR_HPP

#include <mod/lib/GraphMorphism/Constraints/LabelAny.hpp>
#include <mod/lib/GraphMorphism/Constraints/ShortestPath.hpp>
#include <mod/lib/GraphMorphism/Constraints/VertexAdjacency.hpp>
#include <mod/lib/GraphMorphism/Constraints/Visitor.hpp>

namespace mod::lib::GraphMorphism::Constraints {
namespace detail {

template<typename Graph, template<typename> class ...Cs>
struct AllVisitor : BaseVisitor<Graph>, Visitor<const Cs<Graph> > ... {
};

template<typename Graph, template<typename> class ...Cs>
struct AllVisitorNonConst : BaseVisitorNonConst<Graph>, Visitor<Cs<Graph> > ... {
};

} // namespace detail

template<typename Graph>
struct AllVisitor : detail::AllVisitor<Graph,
/*   */ LabelAny,
/*   */ ShortestPath,
/*   */ VertexAdjacency
> {
};

template<typename Graph>
struct AllVisitorNonConst : detail::AllVisitorNonConst<Graph,
/*   */ LabelAny,
/*   */ ShortestPath,
/*   */ VertexAdjacency
> {
};

} // namespace mod::lib::GraphMorphism::Constraints

#endif // MOD_LIB_GRAPHMORPHISM_ALLVISITOR_HPP