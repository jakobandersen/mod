#include "Real.hpp"

#include <mod/rule/Rule.hpp>
#include <mod/lib/Chem/MoleculeUtil.hpp>
#include <mod/lib/Graph/Single.hpp>
#include <mod/lib/Graph/Properties/Stereo.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/GraphMorphism/LabelledMorphism.hpp>
#include <mod/lib/GraphMorphism/VF2Finder.hpp>
#include <mod/lib/LabelledGraph.hpp>
#include <mod/lib/LabelledFilteredGraph.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/Rules/IO/DepictionData.hpp>
#include <mod/lib/Rules/Properties/Molecule.hpp>
#include <mod/lib/Rules/Properties/Stereo.hpp>
#include <mod/lib/Rules/Properties/String.hpp>
#include <mod/lib/Rules/Properties/Term.hpp>
#include <mod/lib/Stereo/Inference.hpp>

#include <jla_boost/graph/morphism/callbacks/Limit.hpp>

#include <boost/lexical_cast.hpp>

namespace mod::lib::Rules {
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

bool Real::sanityChecks(const lib::DPO::CombinedRule::CombinedGraphType &gCombined,
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
			s << "Rule::sanityCheck\tcan't handle parallel edges in lib::Rules::GraphType" << std::endl;
			printEdge(eCG, gCombined, pString, s);
			return false;
		}
	}
	return true;
}

namespace {
std::size_t nextRuleNum = 0;
} // namespace

Real::Real(LabelledRule &&rule, std::optional<LabelType> labelType)
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

Real::~Real() = default;

std::size_t Real::getId() const {
	return id;
}

std::shared_ptr<rule::Rule> Real::getAPIReference() const {
	if(apiReference.use_count() > 0) return std::shared_ptr<rule::Rule>(apiReference);
	else std::abort();
}

void Real::setAPIReference(std::shared_ptr<rule::Rule> r) {
	assert(apiReference.use_count() == 0);
	apiReference = r;
#ifndef NDEBUG
	assert(&r->getRule() == this);
#endif
}

const std::string &Real::getName() const {
	return name;
}

void Real::setName(std::string name) {
	this->name = name;
}

std::optional<LabelType> Real::getLabelType() const {
	return labelType;
}

const LabelledRule &Real::getDPORule() const {
	return dpoRule;
}

const lib::Rules::GraphType &Real::getGraph() const {
	return get_graph(dpoRule);
}

Write::DepictionData &Real::getDepictionData() {
	if(!depictionData) depictionData.reset(new Write::DepictionData(getDPORule()));
	return *depictionData;
}

const Write::DepictionData &Real::getDepictionData() const {
	if(!depictionData) depictionData.reset(new Write::DepictionData(getDPORule()));
	return *depictionData;
}

bool Real::isChemical() const {
	// TODO: actually implement it
	return true;
}

bool Real::isOnlySide(Membership membership) const {
	const auto &gCombined = getDPORule().getRule().getCombinedGraph();
	for(const auto v: asRange(vertices(gCombined)))
		if(gCombined[v].membership != membership) return false;
	for(const auto e: asRange(edges(gCombined)))
		if(gCombined[e].membership != membership) return false;
	return true;
}

bool Real::isOnlyRightSide() const {
	return isOnlySide(Membership::R);
}

namespace {

template<typename Finder>
std::size_t morphism(const Real &gDomain,
                     const Real &gCodomain,
                     std::size_t maxNumMatches,
                     LabelSettings labelSettings,
                     Finder finder) {
	auto mr = jla_boost::GraphMorphism::makeLimit(maxNumMatches);
	lib::GraphMorphism::morphismSelectByLabelSettings(gDomain.getDPORule(), gCodomain.getDPORule(), labelSettings,
	                                                  finder, std::ref(mr), MembershipPredWrapper());
	return mr.getNumHits();
}

} // namespace

std::size_t Real::isomorphism(const Real &rDom,
                              const Real &rCodom,
                              std::size_t maxNumMatches,
                              LabelSettings labelSettings) {
	return morphism(rDom, rCodom, maxNumMatches, labelSettings, lib::GraphMorphism::VF2Isomorphism());
}

std::size_t Real::monomorphism(const Real &rDom,
                               const Real &rCodom,
                               std::size_t maxNumMatches,
                               LabelSettings labelSettings) {
	return morphism(rDom, rCodom, maxNumMatches, labelSettings, lib::GraphMorphism::VF2Monomorphism());
}

bool Real::isomorphicLeftRight(const Real &rDom, const Real &rCodom, LabelSettings labelSettings) {
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

} // namespace mod::lib::Rules