#include "Rule.hpp"

#include <mod/rule/Rule.hpp>
#include <mod/lib/Chem/MoleculeUtil.hpp>
#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/Graph/Properties/Stereo.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/GraphMorphism/LabelledMorphism.hpp>
#include <mod/lib/GraphMorphism/VF2Finder.hpp>
#include <mod/lib/LabelledGraph.hpp>
#include <mod/lib/LabelledFilteredGraph.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/Rule/IO/DepictionData.hpp>
#include <mod/lib/Rule/Properties/Molecule.hpp>
#include <mod/lib/Rule/Properties/Stereo.hpp>
#include <mod/lib/Rule/Properties/String.hpp>
#include <mod/lib/Rule/Properties/Term.hpp>
#include <mod/lib/Stereo/Inference.hpp>

#include <jla_boost/graph/morphism/callbacks/Limit.hpp>

#include <boost/lexical_cast.hpp>

namespace mod::lib::rule {
BOOST_CONCEPT_ASSERT((LabelledGraphConcept<LabelledRule>));
BOOST_CONCEPT_ASSERT((LabelledGraphConcept<LabelledRule::Side>));

//------------------------------------------------------------------------------
// Main class
//------------------------------------------------------------------------------

void printEdge(lib::DPO::CombinedRule::CombinedEdge eCG, const lib::DPO::CombinedRule::CombinedGraphType &gCombined,
               const PropString &pString, std::ostream &s) {
	const auto vSrc = source(eCG, gCombined);
	const auto vTar = target(eCG, gCombined);
	s << "  vSrc (" << vSrc << "): ";
	pString.print(s, vSrc);
	s << "\n  e: ";
	pString.print(s, eCG);
	s << "\n  vTar (" << vTar << "): ";
	pString.print(s, vTar);
	s << std::endl;
}

bool Rule::sanityChecks(const lib::DPO::CombinedRule::CombinedGraphType &gCombined,
                        const PropString &pString, std::ostream &s) {
	// hmm, boost::edge_range is not supported for vecS, right? so we count in a rather stupid way

	for(const auto eCG: asRange(edges(gCombined))) {
		const auto vSrc = source(eCG, gCombined);
		const auto vTar = target(eCG, gCombined);
		const auto eContext = gCombined[eCG].membership,
		           srcContext = gCombined[vSrc].membership,
		           tarContext = gCombined[vTar].membership;
		// check danglingness
		if(eContext != srcContext && srcContext != Membership::K) {
			s << "Rule::sanityCheck\tdangling edge at source: " << std::endl;
			printEdge(eCG, gCombined, pString, s);
			return false;
		}
		if(eContext != tarContext && tarContext != Membership::K) {
			s << "Rule::sanityCheck\tdangling edge at target: " << std::endl;
			printEdge(eCG, gCombined, pString, s);
			return false;
		}
		// check parallelness
		int count = 0;
		for(const auto eOut: asRange(out_edges(vSrc, gCombined)))
			if(target(eOut, gCombined) == vTar)
				++count;
		if(count > 1) {
			s << "Rule::sanityCheck\tcan't handle parallel edges in lib::rule::GraphType" << std::endl;
			printEdge(eCG, gCombined, pString, s);
			return false;
		}
	}
	return true;
}

namespace {
std::size_t nextRuleNum = 0;
} // namespace

Rule::Rule(LabelledRule &&rule, std::optional<LabelType> labelType)
	: id(nextRuleNum++), name("r_{" + boost::lexical_cast<std::string>(id) + "}"), labelType(labelType),
	  dpoRule(std::move(rule)) {
	if(get_num_connected_components(get_labelled_left(dpoRule)) == -1)
		dpoRule.initComponents();
	// only one of propString and propTerm should be defined
	assert(this->dpoRule.pString || this->dpoRule.pTerm);
	assert(!this->dpoRule.pString || !this->dpoRule.pTerm);
	if(!sanityChecks(getDPORule().getRule().getCombinedGraph(), get_string(getDPORule()), std::cout)) {
		std::cout << "Rule::sanityCheck\tfailed in rule '" << getName() << "'" << std::endl;
		MOD_ABORT;
	}
}

Rule::~Rule() = default;

std::size_t Rule::getId() const {
	return id;
}

std::shared_ptr<mod::rule::Rule> Rule::getAPIReference() const {
	if(apiReference.use_count() > 0) return std::shared_ptr<mod::rule::Rule>(apiReference);
	else std::abort();
}

void Rule::setAPIReference(std::shared_ptr<mod::rule::Rule> r) {
	assert(apiReference.use_count() == 0);
	apiReference = r;
#ifndef NDEBUG
	assert(&r->getRule() == this);
#endif
}

const std::string &Rule::getName() const {
	return name;
}

void Rule::setName(std::string name) {
	this->name = name;
}

std::optional<LabelType> Rule::getLabelType() const {
	return labelType;
}

const LabelledRule &Rule::getDPORule() const {
	return dpoRule;
}

const lib::rule::GraphType &Rule::getGraph() const {
	return get_graph(dpoRule);
}

Write::DepictionData &Rule::getDepictionData() {
	if(!depictionData) depictionData.reset(new Write::DepictionData(getDPORule()));
	return *depictionData;
}

const Write::DepictionData &Rule::getDepictionData() const {
	if(!depictionData) depictionData.reset(new Write::DepictionData(getDPORule()));
	return *depictionData;
}

bool Rule::isOnlySide(Membership membership) const {
	const auto &gCombined = getDPORule().getRule().getCombinedGraph();
	for(const auto v: asRange(vertices(gCombined)))
		if(gCombined[v].membership != membership) return false;
	for(const auto e: asRange(edges(gCombined)))
		if(gCombined[e].membership != membership) return false;
	return true;
}

bool Rule::isOnlyRightSide() const {
	return isOnlySide(Membership::R);
}

namespace {

template<typename Graph>
std::size_t getVertexOffset(const Graph &g, const typename boost::graph_traits<Graph>::vertex_descriptor v) {
	using boost::vertices;
	const auto &vs = vertices(g);
	const auto vIter = std::find(vs.first, vs.second, v);
	assert(vIter != vs.second);
	return std::distance(vs.first, vIter);
}

} // namespace

mod::rule::Rule::LeftGraph::Vertex Rule::getLeftInterfaceVertex(SideGraphVertex vL) const {
	return mod::rule::Rule::LeftGraph::Vertex(getAPIReference(),
	                                          getVertexOffset(getL(getDPORule().getRule()), vL));
}

mod::rule::Rule::ContextGraph::Vertex Rule::getContextInterfaceVertex(ContextGraphVertex vK) const {
	return mod::rule::Rule::ContextGraph::Vertex(getAPIReference(),
	                                             getVertexOffset(getK(getDPORule().getRule()), vK));
}

mod::rule::Rule::RightGraph::Vertex Rule::getRightInterfaceVertex(SideGraphVertex vR) const {
	return mod::rule::Rule::RightGraph::Vertex(getAPIReference(),
	                                           getVertexOffset(getR(getDPORule().getRule()), vR));
}

namespace {

template<typename Graph>
std::pair<std::size_t, std::size_t> getEdgeOffsets(const Graph &g,
                                                   const typename boost::graph_traits<Graph>::vertex_descriptor v,
                                                   const typename boost::graph_traits<Graph>::edge_descriptor e) {
	using boost::vertices;
	const auto &vs = vertices(g);
	const auto vIter = std::find(vs.first, vs.second, v);
	assert(vIter != vs.second);
	const auto vId = std::distance(vs.first, vIter);
	const auto &es = out_edges(v, g);
	const auto eIter = std::find(es.first, es.second, e);
	assert(eIter != es.second);
	const auto eId = std::distance(es.first, eIter);
	return {vId, eId};
}

} // namespace

mod::rule::Rule::LeftGraph::Edge Rule::getLeftInterfaceEdge(SideGraphVertex vL, SideGraphEdge eL) const {
	auto [vId, eId] = getEdgeOffsets(getL(getDPORule().getRule()), vL, eL);
	return mod::rule::Rule::LeftGraph::Edge(getAPIReference(), vId, eId);
}


mod::rule::Rule::ContextGraph::Edge Rule::getContextInterfaceEdge(ContextGraphVertex vK, ContextGraphEdge eK) const {
	auto [vId, eId] = getEdgeOffsets(getK(getDPORule().getRule()), vK, eK);
	return mod::rule::Rule::ContextGraph::Edge(getAPIReference(), vId, eId);
}

mod::rule::Rule::RightGraph::Edge Rule::getRightInterfaceEdge(SideGraphVertex vR, SideGraphEdge eR) const {
	auto [vId, eId] = getEdgeOffsets(getR(getDPORule().getRule()), vR, eR);
	return mod::rule::Rule::RightGraph::Edge(getAPIReference(), vId, eId);
}

Rule::SideGraphVertex Rule::getLeftInternalVertex(mod::rule::Rule::LeftGraph::Vertex vL) const {
	return *std::next(vertices(getL(getDPORule().getRule())).first, vL.vId);
}

Rule::SideGraphVertex Rule::getRightInternalVertex(mod::rule::Rule::RightGraph::Vertex vR) const {
	return *std::next(vertices(getR(getDPORule().getRule())).first, vR.vId);
}

namespace {

template<typename Finder>
std::size_t morphism(const Rule &gDomain,
                     const Rule &gCodomain,
                     std::size_t maxNumMatches,
                     LabelSettings labelSettings,
                     Finder finder) {
	auto mr = jla_boost::GraphMorphism::makeLimit(maxNumMatches);
	lib::GraphMorphism::morphismSelectByLabelSettings(gDomain.getDPORule(), gCodomain.getDPORule(), labelSettings,
	                                                  finder, std::ref(mr), MembershipPredWrapper());
	return mr.getNumHits();
}

} // namespace

std::size_t Rule::isomorphism(const Rule &rDom,
                              const Rule &rCodom,
                              std::size_t maxNumMatches,
                              LabelSettings labelSettings) {
	return morphism(rDom, rCodom, maxNumMatches, labelSettings, lib::GraphMorphism::VF2Isomorphism());
}

std::size_t Rule::monomorphism(const Rule &rDom,
                               const Rule &rCodom,
                               std::size_t maxNumMatches,
                               LabelSettings labelSettings) {
	return morphism(rDom, rCodom, maxNumMatches, labelSettings, lib::GraphMorphism::VF2Monomorphism());
}

bool Rule::isomorphicLeftRight(const Rule &rDom, const Rule &rCodom, LabelSettings labelSettings) {
	auto mrLeft = jla_boost::GraphMorphism::makeLimit(1);
	lib::GraphMorphism::morphismSelectByLabelSettings(
			LabelledFilteredGraph(get_labelled_left(rDom.getDPORule())),
			LabelledFilteredGraph(get_labelled_left(rCodom.getDPORule())),
			labelSettings,
			lib::GraphMorphism::VF2Isomorphism(), std::ref(mrLeft));
	if(mrLeft.getNumHits() == 0) return false;
	auto mrRight = jla_boost::GraphMorphism::makeLimit(1);
	lib::GraphMorphism::morphismSelectByLabelSettings(
			LabelledFilteredGraph(get_labelled_right(rDom.getDPORule())),
			LabelledFilteredGraph(get_labelled_right(rCodom.getDPORule())),
			labelSettings,
			lib::GraphMorphism::VF2Isomorphism(), std::ref(mrRight));
	return mrRight.getNumHits() == 1;
}

} // namespace mod::lib::rule