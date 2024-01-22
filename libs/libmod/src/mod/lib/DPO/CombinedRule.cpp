#include "CombinedRule.hpp"

#include <mod/lib/DPO/Concepts.hpp>

namespace mod::lib::DPO {

BOOST_CONCEPT_ASSERT((RuleConcept<CombinedRule>));

CombinedRule::CombinedRule() : gProjectedK(gCombined, Membership::K),
                               gProjectedL(gCombined, Membership::L),
                               gProjectedR(gCombined, Membership::R),
                               mL(getK(*this), getL(*this)),
                               mR(getK(*this), getR(*this)),
                               mKtoCG(getK(*this), gCombined),
                               mLtoCG(getL(*this), gCombined),
                               mRtoCG(getR(*this), gCombined) {}

const CombinedRule::SideGraphType &getL(const CombinedRule &r) {
	return r.gProjectedL;
}

const CombinedRule::KGraphType &getK(const CombinedRule &r) { return r.gProjectedK; }

const CombinedRule::SideGraphType &getR(const CombinedRule &r) {
	return r.gProjectedR;
}

const CombinedRule::MorphismType &getMorL(const CombinedRule &r) { return r.mL; }
const CombinedRule::MorphismType &getMorR(const CombinedRule &r) { return r.mR; }

// ======================================================================================

void invert(CombinedRule &r) {
	auto &g = r.gCombined;
	for(const auto v: asRange(vertices(g)))
		g[v].membership = lib::DPO::invert(g[v].membership);
	for(const auto e: asRange(edges(g)))
		g[e].membership = lib::DPO::invert(g[e].membership);
	using std::swap;
	// don't swap projected, it acts as a reference type
	swap(r.mL, r.mR);
	swap(r.mLtoCG, r.mRtoCG);
}

CombinedRule::SideVertex addVertexL(CombinedRule &r) {
	const auto vCG = add_vertex(r.gCombined);
	r.gCombined[vCG].membership = Membership::L;
	return vCG;
}

CombinedRule::KVertex addVertexK(CombinedRule &r) {
	const auto vCG = add_vertex(r.gCombined);
	r.gCombined[vCG].membership = Membership::K;
	return vCG;
}

CombinedRule::SideVertex addVertexR(CombinedRule &r) {
	const auto vCG = add_vertex(r.gCombined);
	r.gCombined[vCG].membership = Membership::R;
	return vCG;
}

CombinedRule::SideVertex promoteVertexL(CombinedRule &r, CombinedRule::SideVertex vL) {
	const auto vCG = get(r.getLtoCG(), getL(r), r.getCombinedGraph(), vL);
	assert(r.gCombined[vCG].membership == Membership::L);
	r.gCombined[vCG].membership = Membership::K;
	return vCG;
}

CombinedRule::SideEdge addEdgeL(CombinedRule &r, CombinedRule::SideVertex v1, CombinedRule::SideVertex v2) {
	const auto vc1 = get(r.getLtoCG(), getL(r), r.getCombinedGraph(), v1);
	const auto vc2 = get(r.getLtoCG(), getL(r), r.getCombinedGraph(), v2);
	const auto[eCG, addedCG] = add_edge(vc1, vc2, {Membership::L}, r.gCombined);
	assert(addedCG);
	return eCG;
}

CombinedRule::KEdge addEdgeK(CombinedRule &r, CombinedRule::KVertex v1, CombinedRule::KVertex v2) {
	const auto[eCG, added] = add_edge(v1, v2, {Membership::K}, r.gCombined);
	assert(added);
	return eCG;
}

CombinedRule::SideEdge addEdgeR(CombinedRule &r, CombinedRule::SideVertex v1, CombinedRule::SideVertex v2) {
	const auto vc1 = get(r.getRtoCG(), getR(r), r.getCombinedGraph(), v1);
	const auto vc2 = get(r.getRtoCG(), getR(r), r.getCombinedGraph(), v2);
	const auto[eCG, addedCG] = add_edge(vc1, vc2, {Membership::R}, r.gCombined);
	assert(addedCG);
	return eCG;
}

// ======================================================================================

CombinedRule::CombinedGraphType &CombinedRule::getCombinedGraph() {
	return gCombined;
}

const CombinedRule::CombinedGraphType &CombinedRule::getCombinedGraph() const {
	return gCombined;
}

CombinedRule::MembershipPropertyMap CombinedRule::makeMembershipPropertyMap() const {
	return {*this};
}

const CombinedRule::ToCombinedMorphismSide &CombinedRule::getLtoCG() const { return mLtoCG; }
const CombinedRule::ToCombinedMorphismK &CombinedRule::getKtoCG() const { return mKtoCG; }
const CombinedRule::ToCombinedMorphismSide &CombinedRule::getRtoCG() const { return mRtoCG; }

// =========================================================================================

const CombinedRule::SideProjectedGraphType &CombinedRule::getLProjected() const {
	return gProjectedL;
}

const CombinedRule::KProjectedGraphType &CombinedRule::getKProjected() const {
	return gProjectedK;
}

const CombinedRule::SideProjectedGraphType &CombinedRule::getRProjected() const {
	return gProjectedR;
}

} // namespace mod::lib::DPO

BOOST_CONCEPT_ASSERT((mod::lib::DPO::WritableRuleConcept<mod::lib::DPO::CombinedRule>));