#ifndef MOD_LIB_GRAPH_SINGLE_HPP
#define MOD_LIB_GRAPH_SINGLE_HPP

#include <mod/Config.hpp>
#include <mod/graph/ForwardDecl.hpp>
#include <mod/lib/Graph/GraphDecl.hpp>
#include <mod/lib/Graph/LabelledGraph.hpp>

#include <graph_canon/ordered_graph.hpp>

#include <perm_group/allocator/raw_ptr.hpp>
#include <perm_group/group/generated.hpp>
#include <perm_group/permutation/built_in.hpp>

#include <iosfwd>
#include <optional>
#include <string>

namespace mod {
template<typename Domain, typename Codomain>
struct VertexMap;
} // namespace mod
namespace mod::lib::graph {
struct PropMolecule;
namespace Write {
struct DepictionData;
} // namespace Write

struct Graph {
	using CanonIdxMap = boost::iterator_property_map<std::vector<int>::const_iterator,
			decltype(get(boost::vertex_index_t(), GraphType()))>;
	using CanonForm = graph_canon::ordered_graph<GraphType, CanonIdxMap>;
	using AutGroup = perm_group::generated_group<perm_group::raw_ptr_allocator<std::vector<int> > >;
public:
	// requires g != nullptr, pString != nullptr
	// pStereo may be null
	Graph(std::unique_ptr<GraphType> g, std::unique_ptr<PropString> pString, std::unique_ptr<PropStereo> pStereo);
public:
	Graph(Graph &&) = default;
	~Graph();
	const LabelledGraph &getLabelledGraph() const;
	std::size_t getId() const;
	std::shared_ptr<mod::graph::Graph> getAPIReference() const;
	void setAPIReference(std::shared_ptr<mod::graph::Graph> g);
	const std::string &getName() const;
	void setName(std::string name);
	const std::pair<const std::string &, bool> getGraphDFS() const;
	const std::string &getGraphDFSWithIds() const;
	const std::string &getSmiles(bool withIds) const;
	unsigned int getVertexLabelCount(const std::string &label) const;
	unsigned int getEdgeLabelCount(const std::string &label) const;
	Write::DepictionData &getDepictionData();
	const Write::DepictionData &getDepictionData() const;
public: // deprecated interface
	const GraphType &getGraph() const;
	const PropString &getStringState() const;
	const PropMolecule &getMoleculeState() const;
public:
	const CanonForm &getCanonForm(LabelType labelType, bool withStereo) const;
	const AutGroup &getAutGroup(LabelType labelType, bool withStereo) const;
private:
	LabelledGraph g;
	const std::size_t id;
	std::weak_ptr<mod::graph::Graph> apiReference;
	std::string name;
	mutable std::optional<std::string> dfs, dfsWithIds;
	mutable bool dfsHasNonSmilesRingClosure;
	mutable std::optional<std::pair<std::string, bool>> smiles, smilesWithIds; // str x isAbstract
	mutable std::unique_ptr<std::vector<Vertex>> vertexOrder;
	mutable std::vector<int> canon_perm_string;
	mutable std::unique_ptr<const CanonForm> canon_form_string;
	mutable std::unique_ptr<const AutGroup> aut_group_string;
	mutable std::unique_ptr<Write::DepictionData> depictionData;
public:
	static std::size_t
	isomorphismVF2(const Graph &gDom, const Graph &gCodom, std::size_t maxNumMatches, LabelSettings labelSettings);
	static bool isomorphic(const Graph &gDom, const Graph &gCodom, LabelSettings labelSettings);
	static std::size_t
	isomorphism(const Graph &gDom, const Graph &gCodom, std::size_t maxNumMatches, LabelSettings labelSettings);
	static std::size_t
	monomorphism(const Graph &gDom, const Graph &gCodom, std::size_t maxNumMatches, LabelSettings labelSettings);
	static void enumerateIsomorphisms(const Graph &gDom, const Graph &gCodom,
	                                  std::function<bool(VertexMap<mod::graph::Graph, mod::graph::Graph>)> callback,
	                                  LabelSettings labelSettings);
	static void enumerateMonomorphisms(const Graph &gDom, const Graph &gCodom,
	                                   std::function<bool(VertexMap<mod::graph::Graph, mod::graph::Graph>)> callback,
	                                   LabelSettings labelSettings);
	static bool nameLess(const Graph *g1, const Graph *g2);
	static bool canonicalCompare(const Graph &g1, const Graph &g2, LabelType labelType, bool withStereo);
public:
	struct IdLess {
		bool operator()(const Graph *g1, const Graph *g2) const {
			return g1->getId() < g2->getId();
		}
	};
};

Graph makePermutation(const Graph &g);

namespace detail {

struct IsomorphismPredicate {
	IsomorphismPredicate(LabelType labelType, bool withStereo)
			: settings(labelType, LabelRelation::Isomorphism, withStereo, LabelRelation::Isomorphism) {}

	bool operator()(const Graph *gDom, const Graph *gCodom) const {
		return Graph::isomorphic(*gDom, *gCodom, settings);
	}
private:
	LabelSettings settings;
};

} // namespace detail

inline detail::IsomorphismPredicate makeIsomorphismPredicate(LabelType labelType, bool withStereo) {
	return detail::IsomorphismPredicate(labelType, withStereo);
}

} // namespace mod::lib::graph

#endif // MOD_LIB_GRAPH_SINGLE_HPP