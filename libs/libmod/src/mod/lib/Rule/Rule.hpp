#ifndef MOD_LIB_RULES_RULE_HPP
#define MOD_LIB_RULES_RULE_HPP

#include <mod/BuildConfig.hpp>
#include <mod/Config.hpp>
#include <mod/rule/ForwardDecl.hpp>
#include <mod/rule/GraphInterface.hpp>
#include <mod/lib/Graph/GraphDecl.hpp>
#include <mod/lib/Rule/LabelledRule.hpp>

#include <jla_boost/graph/morphism/Predicates.hpp>

#include <optional>

namespace mod::lib::graph {
struct PropStereo;
struct PropString;
struct Graph;
} // namespace mod::lib::graph
namespace mod::lib::rule {
struct PropString;
struct PropMolecule;

namespace Write {
struct DepictionData;
} // namespace Write

struct Rule {
	static bool sanityChecks(const lib::DPO::CombinedRule::CombinedGraphType &gCombined,
	                         const PropString &pString, std::ostream &s);
public:
	Rule(LabelledRule &&rule, std::optional<LabelType> labelType);
	~Rule();
	std::size_t getId() const;
	std::shared_ptr<mod::rule::Rule> getAPIReference() const;
	void setAPIReference(std::shared_ptr<mod::rule::Rule> r);
	const std::string &getName() const;
	void setName(std::string name);
	std::optional<LabelType> getLabelType() const;
	const LabelledRule &getDPORule() const;
public: // shorthands, deprecated
	const GraphType &getGraph() const;
	Write::DepictionData &getDepictionData(); // TODO: should not be available as non-const
public:
	const Write::DepictionData &getDepictionData() const;
	bool isOnlySide(Membership membership) const;
	bool isOnlyRightSide() const; // shortcut of above
public:
	using ContextGraphVertex = lib::DPO::CombinedRule::KVertex;
	using ContextGraphEdge = lib::DPO::CombinedRule::KEdge;
	using SideGraphVertex = lib::DPO::CombinedRule::SideVertex;
	using SideGraphEdge = lib::DPO::CombinedRule::SideEdge;
	mod::rule::Rule::LeftGraph::Vertex getLeftInterfaceVertex(SideGraphVertex vL) const;
	mod::rule::Rule::ContextGraph::Vertex getContextInterfaceVertex(ContextGraphVertex vK) const;
	mod::rule::Rule::RightGraph::Vertex getRightInterfaceVertex(SideGraphVertex vR) const;
	mod::rule::Rule::LeftGraph::Edge getLeftInterfaceEdge(SideGraphVertex vL, SideGraphEdge eL) const;
	mod::rule::Rule::ContextGraph::Edge getContextInterfaceEdge(ContextGraphVertex vK, ContextGraphEdge eK) const;
	mod::rule::Rule::RightGraph::Edge getRightInterfaceEdge(SideGraphVertex vR, SideGraphEdge eR) const;
	SideGraphVertex getLeftInternalVertex(mod::rule::Rule::LeftGraph::Vertex vL) const;
	SideGraphVertex getRightInternalVertex(mod::rule::Rule::RightGraph::Vertex vR) const;
public:
	static std::size_t isomorphism(const Rule &rDom,
	                               const Rule &rCodom,
	                               std::size_t maxNumMatches,
	                               LabelSettings labelSettings);
	static std::size_t monomorphism(const Rule &rDom,
	                                const Rule &rCodom,
	                                std::size_t maxNumMatches,
	                                LabelSettings labelSettings);
	static bool isomorphicLeftRight(const Rule &rDom,
	                                const Rule &rCodom,
	                                LabelSettings labelSettings);
private:
	const std::size_t id;
	std::weak_ptr<mod::rule::Rule> apiReference;
	std::string name;
	const std::optional<LabelType> labelType;
private:
	LabelledRule dpoRule;
	mutable std::unique_ptr<Write::DepictionData> depictionData;
};

struct LessById {
	bool operator()(const Rule *r1, const Rule *r2) const {
		return r1->getId() < r2->getId();
	}
};

namespace detail {

struct IsomorphismPredicate {
	IsomorphismPredicate(LabelType labelType, bool withStereo)
		: settings(labelType, LabelRelation::Isomorphism, withStereo, LabelRelation::Isomorphism) {}

	bool operator()(const Rule *rDom, const Rule *rCodom) const {
		return 1 == Rule::isomorphism(*rDom, *rCodom, 1, settings);
	}

	bool operator()(const std::unique_ptr<Rule> &rDom, const std::unique_ptr<Rule> &rCodom) const {
		return 1 == Rule::isomorphism(*rDom, *rCodom, 1, settings);
	}
private:
	LabelSettings settings;
};

} // namespace detail

inline detail::IsomorphismPredicate makeIsomorphismPredicate(LabelType labelType, bool withStereo) {
	return detail::IsomorphismPredicate(labelType, withStereo);
}

struct MembershipPredWrapper {
	template<typename Pred>
	auto operator()(const LabelledRule &gLabDomain, const LabelledRule &gLabCodomain, Pred pred) const {
		return jla_boost::GraphMorphism::makePropertyPredicateEq(
				gLabDomain.getRule().makeMembershipPropertyMap(),
				gLabCodomain.getRule().makeMembershipPropertyMap(), pred);
	}
};

} // namespace mod::lib::rule

#endif // MOD_LIB_RULES_RULE_HPP