#ifndef MOD_LIB_GRAPH_SINGLE_H
#define MOD_LIB_GRAPH_SINGLE_H

#include <mod/Config.hpp>
#include <mod/graph/ForwardDecl.hpp>
#include <mod/rule/ForwardDecl.hpp>
#include <mod/lib/Graph/GraphDecl.hpp>
#include <mod/lib/Graph/LabelledGraph.hpp>

#include <graph_canon/ordered_graph.hpp>

#include <perm_group/allocator/raw_ptr.hpp>
#include <perm_group/group/generated.hpp>
#include <perm_group/permutation/built_in.hpp>

#include <boost/optional/optional.hpp>

#include <iosfwd>
#include <string>

namespace mod {
namespace lib {
namespace Graph {
struct PropMolecule;
struct DepictionData;

struct Single {
	using CanonIdxMap = boost::iterator_property_map<std::vector<int>::const_iterator,
					decltype(get(boost::vertex_index_t(), GraphType()))>;
	using CanonForm = graph_canon::ordered_graph<GraphType, CanonIdxMap>;
	using AutGroup = perm_group::generated_group<perm_group::raw_ptr_allocator<std::vector<int> > >;
public:
	// requires g != nullptr, pString != nullptr
	// pStereo may be null
	Single(std::unique_ptr<GraphType> g, std::unique_ptr<PropString> pString, std::unique_ptr<PropStereo> pStereo);
public:
	Single(Single &&) = default;
	~Single();
	const LabelledGraph &getLabelledGraph() const;
	std::size_t getId() const;
	std::shared_ptr<graph::Graph> getAPIReference() const;
	void setAPIReference(std::shared_ptr<graph::Graph> g);
	const std::string &getName() const;
	void setName(std::string name);
	const std::pair<const std::string&, bool> getGraphDFS() const;
	const std::string &getGraphDFSWithIds() const;
	const std::string &getSmiles() const;
	const std::string &getSmilesWithIds() const;
	std::shared_ptr<rule::Rule> getBindRule() const;
	std::shared_ptr<rule::Rule> getIdRule() const;
	std::shared_ptr<rule::Rule> getUnbindRule() const;
	unsigned int getVertexLabelCount(const std::string &label) const;
	unsigned int getEdgeLabelCount(const std::string &label) const;
	DepictionData &getDepictionData();
	const DepictionData &getDepictionData() const;
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
	std::weak_ptr<graph::Graph> apiReference;
	std::string name;
	mutable boost::optional<std::string> dfs, dfsWithIds;
	mutable bool dfsHasNonSmilesRingClosure;
	mutable boost::optional<std::string> smiles, smilesWithIds;
	mutable std::shared_ptr<rule::Rule> bindRule, idRule, unbindRule;
	mutable std::unique_ptr<std::vector<Vertex> > vertexOrder;
	mutable std::vector<int> canon_perm_string;
	mutable std::unique_ptr<const CanonForm> canon_form_string;
	mutable std::unique_ptr<const AutGroup> aut_group_string;
	mutable std::unique_ptr<DepictionData> depictionData;
public:
	static std::size_t isomorphismVF2(const Single &gDom, const Single &gCodom, std::size_t maxNumMatches, LabelSettings labelSettings);
	static bool isomorphic(const Single &gDom, const Single &gCodom, LabelSettings labelSettings);
	static std::size_t isomorphism(const Single &gDom, const Single &gCodom, std::size_t maxNumMatches, LabelSettings labelSettings);
	static std::size_t monomorphism(const Single &gDom, const Single &gCodom, std::size_t maxNumMatches, LabelSettings labelSettings);
	static bool nameLess(const Single *g1, const Single *g2);
	static bool canonicalCompare(const Single &g1, const Single &g2, LabelType labelType, bool withStereo);
public:
	struct IdLess {
		bool operator()(const Single *g1, const Single *g2) const {
			return g1->getId() < g2->getId();
		}
	};
};

Single makePermutation(const Single &g);

namespace detail {

struct IsomorphismPredicate {
	IsomorphismPredicate(LabelType labelType, bool withStereo)
	: settings(labelType, LabelRelation::Isomorphism, withStereo, LabelRelation::Isomorphism) { }

	bool operator()(const Single *gDom, const Single *gCodom) const {
		return Single::isomorphic(*gDom, *gCodom, settings);
	}
private:
	LabelSettings settings;
};

} // namespace detail

inline detail::IsomorphismPredicate makeIsomorphismPredicate(LabelType labelType, bool withStereo) {
	return detail::IsomorphismPredicate(labelType, withStereo);
}

} // namespace Graph
} // namespace lib
} // namespace mod

#endif /* MOD_LIB_GRAPH_SINGLE_H */
