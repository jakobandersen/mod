#ifndef MOD_LIB_DG_HYPER_HPP
#define MOD_LIB_DG_HYPER_HPP

#include <mod/Derivation.hpp>
#include <mod/dg/GraphInterface.hpp>
#include <mod/lib/DG/NonHyper.hpp>

namespace mod {
template<typename>
struct Function;
} // namespace mod
namespace mod::lib::graph {
struct Graph;
} // namespace mod::lib::graph
namespace mod::lib::DG {
struct Expanded;

struct HyperCreator {
	HyperCreator(const HyperCreator &) = delete;
	HyperCreator &operator=(const HyperCreator &) = delete;
public:
	friend struct Hyper;
private:
	HyperCreator() = default;
	explicit HyperCreator(Hyper &hyper,
	                      std::shared_ptr<Function<void(dg::DG::Vertex)>> onNewVertex,
						  std::shared_ptr<Function<void(dg::DG::HyperEdge)>> onNewHyperEdge);
public:
	HyperCreator(HyperCreator &&other);
	HyperCreator &operator=(HyperCreator &&other);
	~HyperCreator();
	void addVertex(const lib::graph::Graph *g);
	HyperVertex addEdge(NonHyper::Edge eNon);
private:
	Hyper *owner = nullptr;
	std::shared_ptr<Function<void(dg::DG::Vertex)>> onNewVertex;
	std::shared_ptr<Function<void(dg::DG::HyperEdge)>> onNewHyperEdge;
};

struct Hyper {
	Hyper(const Hyper &) = delete;
	Hyper &operator=(const Hyper &) = delete;
public:
	// bipartite representation
	using GraphType = HyperGraphType;
	using Vertex = HyperVertex;
	using Edge = HyperEdge;
private:
	friend struct HyperCreator;
	Hyper(const NonHyper &dg);
public:
	static std::pair<std::unique_ptr<Hyper>, HyperCreator> makeHyper(
			const NonHyper &dg,
			std::shared_ptr<Function<void(dg::DG::Vertex)>> onNewVertex,
			std::shared_ptr<Function<void(dg::DG::HyperEdge)>> onNewHyperEdge);
	//	Hyper(const NonHyper &dg, int dummy);
private:
	// the vertex, isNew
	std::pair<Vertex, bool> addVertex(const lib::graph::Graph *g);
public:
	~Hyper();
	const NonHyper &getNonHyper() const;
	const GraphType &getGraph() const;
	void printStats(std::ostream &s) const;
	bool isVertexGraph(const lib::graph::Graph *g) const;
	Vertex getVertexOrNullFromGraph(const lib::graph::Graph *g) const;
	// requires: isVertexGraph(g)
	Vertex getVertexFromGraph(const lib::graph::Graph *g) const;
	// requires: getGraph()[e].kind == HyperVertexKind::Edge
	// returns null_vertex is no reverse exists, or the data has not been initialized yet
	Vertex getReverseEdge(Vertex e) const;
	// requires: getGraph()[e].kind == HyperVertexKind::Edge
	const std::vector<const lib::rule::Rule *> &getRulesFromEdge(Vertex e) const;
public:
	dg::DG::Vertex getInterfaceVertex(Vertex v) const;
	dg::DG::HyperEdge getInterfaceEdge(Vertex e) const;
	Vertex getInternalVertex(const dg::DG::Vertex &v) const;
	Vertex getInternalVertex(const dg::DG::HyperEdge &e) const;
public:
	Derivation getDerivation(Vertex v) const;
private:
	bool hasCalculated;
	const NonHyper &nonHyper;
	GraphType hyper;
private:
	std::map<const lib::graph::Graph *, Vertex> graphToHyperVertex;
};

} // namespace mod::lib::DG

#endif // MOD_LIB_DG_HYPER_HPP