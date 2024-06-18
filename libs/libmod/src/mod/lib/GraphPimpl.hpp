#ifndef MOD_LIB_GRAPHPIMPL_HPP
#define MOD_LIB_GRAPHPIMPL_HPP

#include <mod/Error.hpp>
#include <mod/GraphConcepts.hpp>

#include <cassert>
#include <optional>

namespace mod::lib {
namespace {

template<typename Graph>
std::shared_ptr<Graph> getGraphFromStore(std::shared_ptr<Graph> g) {
	return g;
}

template<typename Graph>
Graph getGraphFromStore(std::optional<Graph> g) {
	return *g;
}

} // namespace

#define MOD_GRAPHPIMPL_Define_Vertex(GraphClass, GraphName, getMacroGraph, g, VertexPrint)        \
    MOD_GRAPHPIMPL_Define_Vertex_noGraph(GraphClass, GraphName, getMacroGraph, g, VertexPrint)    \
	 MOD_GRAPHPIMPL_Define_Vertex_graph(GraphClass, g)

#define MOD_GRAPHPIMPL_Define_Vertex_graph(GraphClass, g)                            \
                                                                                     \
GraphHandle<GraphClass> GraphClass::Vertex::getGraph() const {                       \
    if(!*this) throw LogicError("Can not get graph on a null vertex.");              \
    return lib::getGraphFromStore(g);                                                \
}

#define MOD_GRAPHPIMPL_Define_Vertex_noGraph(GraphClass, GraphName, getMacroGraph, g, VertexPrint) \
	MOD_GRAPHPIMPL_Define_Vertex_noGraph_noId(GraphClass, GraphName, getMacroGraph, g, VertexPrint) \
	MOD_GRAPHPIMPL_Define_Vertex_id(GraphClass, getMacroGraph)

#define MOD_GRAPHPIMPL_Define_Vertex_noGraph_noId(GraphClass, GraphName, getMacroGraph, g, VertexPrint)\
                                                                                     \
GraphClass::Vertex::Vertex(GraphHandle<GraphClass> g, std::size_t vId) : g(g), vId(vId) { \
    using boost::vertices;                                                           \
    const auto &graph = getMacroGraph;                                               \
    const auto &vs = vertices(graph);                                                \
    if(vId >= std::distance(vs.first, vs.second)) {                                  \
        this->g = {};                                                                \
        this->vId = 0;                                                               \
    }                                                                                \
}                                                                                    \
                                                                                     \
GraphClass::Vertex::Vertex() : vId(0) { }                                            \
                                                                                     \
std::ostream &operator<<(std::ostream &s, const GraphClass::Vertex &v) {             \
    s << #GraphName "Vertex(";                                                       \
    if(!v) s << "null";                                                              \
    else s << *(v.g VertexPrint) << ", " << v.getId();                               \
    return s << ")";                                                                 \
}                                                                                    \
                                                                                     \
bool operator==(const GraphClass::Vertex &v1, const GraphClass::Vertex &v2) {        \
    return v1.g == v2.g && v1.vId == v2.vId;                                         \
}                                                                                    \
                                                                                     \
bool operator!=(const GraphClass::Vertex &v1, const GraphClass::Vertex &v2) {        \
    return !(v1 == v2);                                                              \
}                                                                                    \
                                                                                     \
bool operator<(const GraphClass::Vertex &v1, const GraphClass::Vertex &v2) {         \
    return std::tie(v1.g, v1.vId) < std::tie(v2.g, v2.vId);                          \
}                                                                                    \
                                                                                     \
std::size_t GraphClass::Vertex::hash() const {                                       \
    if(*this) return getId();                                                        \
    else return -1;                                                                  \
}                                                                                    \
                                                                                     \
GraphClass::Vertex::operator bool() const {                                          \
    return !isNull();                                                                \
}                                                                                    \
                                                                                     \
bool GraphClass::Vertex::isNull() const {                                            \
    return *this == GraphClass::Vertex();                                            \
}

#define MOD_GRAPHPIMPL_Define_Vertex_id(GraphClass, getMacroGraph)                   \
                                                                                     \
std::size_t GraphClass::Vertex::getId() const {                                      \
    if(!*this) throw LogicError("Can not get ID on a null vertex.");                 \
    const auto &graph = getMacroGraph;                                               \
    using boost::vertices;                                                           \
    auto v = *std::next(vertices(graph).first, vId);                                 \
    return get(boost::vertex_index_t(), graph, v);                                   \
}


#define MOD_GRAPHPIMPL_Define_Vertex_Undirected(GraphClass, getMacroGraph, g)            \
                                                                                         \
std::size_t GraphClass::Vertex::getDegree() const {                                      \
    if(!*this) throw LogicError("Can not get degree on a null vertex.");                 \
    const auto &graph = getMacroGraph;                                                   \
    using boost::vertices;                                                               \
    auto v = *std::next(vertices(graph).first, vId);                                     \
    return out_degree(v, graph);                                                         \
}                                                                                        \
                                                                                         \
GraphClass::IncidentEdgeRange GraphClass::Vertex::incidentEdges() const {                \
    if(!*this) throw LogicError("Can not get incident edges on a null vertex.");         \
    return IncidentEdgeRange(lib::getGraphFromStore(g), vId);                            \
}


#define MOD_GRAPHPIMPL_Define_Vertex_Directed(GraphClass, getMacroGraph, g)            \
                                                                                       \
std::size_t GraphClass::Vertex::inDegree() const {                                     \
    if(!*this) throw LogicError("Can not get in-degree on a null vertex.");            \
    const auto &graph = getMacroGraph;                                                 \
    using boost::vertices;                                                             \
    auto v = *std::next(vertices(graph).first, vId);                                   \
    return in_degree(v, graph);                                                        \
}                                                                                      \
                                                                                       \
std::size_t GraphClass::Vertex::outDegree() const {                                    \
    if(!*this) throw LogicError("Can not get out-degree on a null vertex.");           \
    const auto &graph = getMacroGraph;                                                 \
    using boost::vertices;                                                             \
    auto v = *std::next(vertices(graph).first, vId);                                   \
    return out_degree(v, graph);                                                       \
}


#define MOD_GRAPHPIMPL_Define_Indices(GraphClass, GraphName, getMacroGraph, g, EdgePrint)        \
                                                                                                 \
/*----------------------------------------------------------------------------*/                 \
/* Edge                                                                       */                 \
/*----------------------------------------------------------------------------*/                 \
                                                                                                 \
GraphClass::Edge::Edge(GraphHandle<GraphClass> g, std::size_t vId, std::size_t eId) : g(g), vId(vId), eId(eId) { \
    using boost::vertices;                                                                       \
    const auto &graph = getMacroGraph;                                                           \
    const auto &vs = vertices(graph);                                                            \
    if(vId >= std::distance(vs.first, vs.second)                                                 \
            || eId >= out_degree(*std::next(vertices(graph).first, vId), graph)) {               \
        this->g = {};                                                                            \
        this->vId = 0;                                                                           \
        this->eId = 0;                                                                           \
    }                                                                                            \
}                                                                                                \
                                                                                                 \
GraphClass::Edge::Edge() : vId(0), eId(0) { }                                                    \
                                                                                                 \
std::ostream &operator<<(std::ostream &s, const GraphClass::Edge &e) {                           \
    s << #GraphName "Edge(";                                                                     \
    if(e) s << *(e.g EdgePrint) << ", " << e.source().getId() << ", " << e.target().getId();     \
    else s << "null";                                                                            \
    return s << ")";                                                                             \
}                                                                                                \
                                                                                                 \
bool operator==(const GraphClass::Edge &e1, const GraphClass::Edge &e2) {                        \
    if(e1.g && e2.g)                                                                             \
        return (e1.source() == e2.source() && e1.target() == e2.target())                        \
                || (e1.source() == e2.target() && e1.target() == e2.source());                   \
    else if(!e1.g && !e2.g) return true;                                                         \
    else return false;                                                                           \
}                                                                                                \
                                                                                                 \
bool operator!=(const GraphClass::Edge &e1, const GraphClass::Edge &e2) {                        \
    return !(e1 == e2);                                                                          \
}                                                                                                \
                                                                                                 \
bool operator<(const GraphClass::Edge &e1, const GraphClass::Edge &e2) {                         \
    return std::tie(e1.g, e1.vId, e1.eId) < std::tie(e2.g, e2.vId, e2.eId);                      \
}                                                                                                \
                                                                                                 \
GraphClass::Edge::operator bool() const {                                                        \
    return !isNull();                                                                            \
}                                                                                                \
                                                                                                 \
bool GraphClass::Edge::isNull() const {                                                          \
    return *this == GraphClass::Edge();                                                          \
}                                                                                                \
                                                                                                 \
GraphHandle<GraphClass> GraphClass::Edge::getGraph() const {                                     \
    if(!g) throw LogicError("Can not get graph on a null edge.");                                \
    return lib::getGraphFromStore(g);                                                            \
}                                                                                                \
                                                                                                 \
GraphClass::Vertex GraphClass::Edge::source() const {                                            \
    if(!g) throw LogicError("Can not get source on a null edge.");                               \
    return Vertex(lib::getGraphFromStore(g), vId);                                               \
}                                                                                                \
                                                                                                 \
GraphClass::Vertex GraphClass::Edge::target() const {                                            \
    if(!g) throw LogicError("Can not get target on a null edge.");                               \
    const auto &graph = getMacroGraph;                                                           \
    using boost::vertices;                                                                       \
    auto v = *std::next(vertices(graph).first, vId);                                             \
    auto e = *std::next(out_edges(v, graph).first, eId);                                         \
    using boost::target;                                                                         \
    auto vTar = target(e, graph);                                                                \
    const auto &vs = vertices(graph);                                                            \
    auto vTarIter = std::find(vs.first, vs.second, vTar);                                        \
    return Vertex(lib::getGraphFromStore(g), std::distance(vs.first, vTarIter));                 \
}                                                                                                \
                                                                                                 \
/*----------------------------------------------------------------------------*/                 \
/* VertexIterator                                                             */                 \
/*----------------------------------------------------------------------------*/                 \
                                                                                                 \
GraphClass::VertexIterator::VertexIterator(GraphHandle<GraphClass> g) : g(g), vId(0) {           \
    using boost::vertices;                                                                       \
    const auto &graph = getMacroGraph;                                                           \
    const auto &vs = vertices(graph);                                                            \
    if(0 == std::distance(vs.first, vs.second))                                                  \
        this->g = {};                                                                            \
}                                                                                                \
                                                                                                 \
GraphClass::VertexIterator::VertexIterator() : g(), vId(0) { }                                   \
                                                                                                 \
GraphClass::Vertex GraphClass::VertexIterator::dereference() const {                             \
    return GraphClass::Vertex(lib::getGraphFromStore(g), vId);                                   \
}                                                                                                \
                                                                                                 \
bool GraphClass::VertexIterator::equal(const VertexIterator &iter) const {                       \
    if(g) return g == iter.g && vId == iter.vId;                                                 \
    else return g == iter.g;                                                                     \
}                                                                                                \
                                                                                                 \
void GraphClass::VertexIterator::increment() {                                                   \
    ++this->vId;                                                                                 \
    using boost::vertices;                                                                       \
    const auto &graph = getMacroGraph;                                                           \
    const auto &vs = vertices(graph);                                                            \
    if(vId >= std::distance(vs.first, vs.second)) {                                              \
        this->g = {};                                                                            \
        this->vId = 0;                                                                           \
    }                                                                                            \
}                                                                                                \
                                                                                                 \
/*----------------------------------------------------------------------------*/                 \
/* EdgeIterator												                           */                 \
/*----------------------------------------------------------------------------*/                 \
                                                                                                 \
GraphClass::EdgeIterator::EdgeIterator(GraphHandle<GraphClass> g) : g(g), vId(0), eId(0) {       \
    advanceToValid();                                                                            \
}                                                                                                \
                                                                                                 \
GraphClass::EdgeIterator::EdgeIterator() : g(), vId(0), eId(0) { }                               \
                                                                                                 \
GraphClass::Edge GraphClass::EdgeIterator::dereference() const {                                 \
    return Edge(lib::getGraphFromStore(g), vId, eId);                                            \
}                                                                                                \
                                                                                                 \
bool GraphClass::EdgeIterator::equal(const EdgeIterator &iter) const {                           \
    if(g) return g == iter.g && vId == iter.vId && eId == iter.eId;                              \
    else return g == iter.g;                                                                     \
}                                                                                                \
                                                                                                 \
void GraphClass::EdgeIterator::increment() {                                                     \
    eId++;                                                                                       \
    advanceToValid();                                                                            \
}                                                                                                \
                                                                                                 \
void GraphClass::EdgeIterator::advanceToValid() {                                                \
    assert(g);                                                                                   \
    using boost::vertices;                                                                       \
    const auto &graph = getMacroGraph;                                                           \
    const auto &vs = vertices(graph);                                                            \
    for(; vId < std::distance(vs.first, vs.second); vId++, eId = 0) {                            \
        using boost::vertices;                                                                   \
        for(auto v = *std::next(vertices(graph).first, vId); eId < out_degree(v, graph); eId++) {\
            auto e = *std::next(out_edges(v, graph).first, eId);                                 \
            auto vTar = target(e, graph);                                                        \
            if(get(boost::vertex_index_t(), graph, vTar) > get(boost::vertex_index_t(), graph, v)) \
                return;                                                                          \
        }                                                                                        \
    }                                                                                            \
    g = {};                                                                                      \
    vId = 0;                                                                                     \
    eId = 0;                                                                                     \
}                                                                                                \
                                                                                                 \
/*----------------------------------------------------------------------------*/                 \
/* IncidentEdgeIterator                                                       */                 \
/*----------------------------------------------------------------------------*/                 \
                                                                                                 \
GraphClass::IncidentEdgeIterator::IncidentEdgeIterator(GraphHandle<GraphClass> g, std::size_t vId) : g(g), vId(vId), eId(0) { \
    const auto &graph = getMacroGraph;                                                           \
    using boost::vertices;                                                                       \
    auto v = *std::next(vertices(graph).first, vId);                                             \
    if(out_degree(v, graph) == 0) {                                                              \
        this->g = {};                                                                            \
        this->vId = 0;                                                                           \
    }                                                                                            \
}                                                                                                \
                                                                                                 \
GraphClass::IncidentEdgeIterator::IncidentEdgeIterator() : g(), vId(0), eId(0) { }               \
                                                                                                 \
GraphClass::Edge GraphClass::IncidentEdgeIterator::dereference() const {                         \
    return GraphClass::Edge(lib::getGraphFromStore(g), vId, eId);                                \
}                                                                                                \
                                                                                                 \
bool GraphClass::IncidentEdgeIterator::equal(const IncidentEdgeIterator &iter) const {           \
    if(g) return g == iter.g && vId == iter.vId && eId == iter.eId;                              \
    else return g == iter.g;                                                                     \
}                                                                                                \
                                                                                                 \
void GraphClass::IncidentEdgeIterator::increment() {                                             \
    ++this->eId;                                                                                 \
    const auto &graph = getMacroGraph;                                                           \
    using boost::vertices;                                                                       \
    auto v = *std::next(vertices(graph).first, vId);                                             \
    if(eId >= out_degree(v, graph)) {                                                            \
        this->g = {};                                                                            \
        this->vId = 0;                                                                           \
    }                                                                                            \
}

} // namespace mod::lib

#endif // MOD_LIB_GRAPHPIMPL_HPP