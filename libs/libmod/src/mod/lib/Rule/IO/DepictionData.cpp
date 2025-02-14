#include "DepictionData.hpp"

#include <mod/Error.hpp>
#include <mod/lib/Chem/MoleculeUtil.hpp>
#include <mod/lib/Chem/OBabel.hpp>
#include <mod/lib/Graph/IO/Write.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/Rule/Properties/String.hpp>
#include <mod/lib/Rule/Properties/Molecule.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <iostream>
#include <map>

namespace mod::lib::rule::Write {
namespace {
constexpr bool VERBOSE = false;
} // namespace

DepictionData::Side::Side(const DepictionData &depict, const SideData &data,
                          const Chem::OBMolHandle CoordData::*obSide,
                          const lib::DPO::CombinedRule::SideGraphType &g,
                          SideToCG mToCG,
                          PropMolecule::Side pMol,
                          std::function<PropStereo::Side()> fStereo)
		: depict(depict), data(data), obSide(obSide), g(g), mToCG(mToCG), pMol(pMol), fStereo(fStereo) {}

AtomData DepictionData::Side::getAtomData(SideVertex vS) const {
	return pMol[vS];
}

AtomId DepictionData::Side::getAtomId(SideVertex vS) const {
	return pMol[vS].getAtomId();
}

Isotope DepictionData::Side::getIsotope(SideVertex vS) const {
	return pMol[vS].getIsotope();
}

Charge DepictionData::Side::getCharge(SideVertex vS) const {
	return pMol[vS].getCharge();
}

bool DepictionData::Side::getRadical(SideVertex vS) const {
	return pMol[vS].getRadical();
}

std::string DepictionData::Side::getVertexLabelNoIsotopeChargeRadical(SideVertex vS) const {
	const auto atomId = getAtomId(vS);
	if(atomId != AtomIds::Invalid)
		return Chem::symbolFromAtomId(atomId);
	const auto &nonAtomToPhonyAtom = data.nonAtomToPhonyAtom;
	const auto nonAtomIter = nonAtomToPhonyAtom.find(vS);
	assert(nonAtomIter != end(nonAtomToPhonyAtom));
	const auto labelIter = depict.phonyAtomToString.find(nonAtomIter->second.getAtomId());
	assert(labelIter != end(depict.phonyAtomToString));
	return labelIter->second;
}

BondType DepictionData::Side::getBondData(SideEdge eS) const {
	return pMol[eS];
}

std::string DepictionData::Side::getEdgeLabel(SideEdge eS) const {
	const auto bt = getBondData(eS);
	if(bt != BondType::Invalid)
		return std::string(1, Chem::bondToChar(bt));
	const auto &nonBondEdges = data.nonBondEdges;
	const auto iter = nonBondEdges.find(eS);
	assert(iter != end(nonBondEdges));
	return iter->second;
}

bool DepictionData::Side::hasImportantStereo(SideVertex vS) const {
	if(!has_stereo(depict.lr)) return false;
	return !fStereo()[vS]->morphismDynamicOk();
}

lib::IO::Graph::Write::EdgeFake3DType
DepictionData::Side::getEdgeFake3DType(SideEdge eS, bool withHydrogen) const {
	const auto vSrc = source(eS, g);
	const auto vTar = target(eS, g);
	if(!hasImportantStereo(vSrc) && !hasImportantStereo(vTar))
		return lib::IO::Graph::Write::EdgeFake3DType::None;
#ifndef MOD_HAVE_OPENBABEL
		throw FatalError(MOD_NO_OPENBABEL_ERROR_STR);
#else
	assert(depict.hasMoleculeEncoding);
	const auto idSrc = get(boost::vertex_index_t(), g, vSrc);
	const auto idTar = get(boost::vertex_index_t(), g, vTar);
	const CoordData &cData = withHydrogen ? depict.cDataAll : depict.cDataNoHydrogen;
	return (cData.*obSide).getBondFake3D(idSrc, idTar);
#endif
}

bool DepictionData::Side::getHasCoordinates() const {
	return depict.getHasCoordinates();
}

double DepictionData::Side::getX(SideVertex vS, bool withHydrogen) const {
	const auto v = get(mToCG, g, depict.lr.getRule().getCombinedGraph(), vS);
	return depict.getX(v, withHydrogen);
}

double DepictionData::Side::getY(SideVertex vS, bool withHydrogen) const {
	const auto v = get(mToCG, g, depict.lr.getRule().getCombinedGraph(), vS);
	return depict.getY(v, withHydrogen);
}

const AtomData &DepictionData::Side::operator()(SideVertex vS) const {
	if(getAtomId(vS) != AtomIds::Invalid)
		return pMol[vS];
	const auto iter = data.nonAtomToPhonyAtom.find(vS);
	assert(iter != end(data.nonAtomToPhonyAtom));
	return iter->second;
}

BondType DepictionData::Side::operator()(SideEdge eS) const {
	const auto bt = getBondData(eS);
	return bt == BondType::Invalid ? BondType::Single : bt;
}

//bool DepictionDataCore::Side::isAtomIdInvalidContext(Vertex v) const {
//	switch(membership) {
//	case Membership::Left:
//	case Membership::Right
//		return true;
//	}
//	const auto &molState = depict.moleculeState;
//	if(membership == Membership::Left) return true;
//	else if(membership == Membership::Right) return true;
//	else {
//		auto m = depict.g[v].membership;
//		//		if(m != Membership::StateChange) return true;
//		auto leftId = molState.getNormal(v).getAtomId();
//		auto rightId = molState.getChange(v).getAtomId();
//		if(leftId == rightId) return true;
//		else return false;
//	}
//}

//------------------------------------------------------------------------------
// K
//------------------------------------------------------------------------------

DepictionData::K::K(const DepictionData &depict) : depict(depict) {}

AtomId DepictionData::K::getAtomId(KVertex v) const {
	const auto &rDPO = depict.lr.getRule();
	const auto vL = get(getMorL(rDPO), getK(rDPO), getL(rDPO), v);
	const auto vR = get(getMorR(rDPO), getK(rDPO), getR(rDPO), v);
	const auto &pMol = get_molecule(depict.lr);
	const auto l = pMol.getLeft()[vL].getAtomId();
	const auto r = pMol.getRight()[vR].getAtomId();
	if(l == r) return l;
	else return AtomIds::Invalid;
}

Isotope DepictionData::K::getIsotope(KVertex v) const {
	const auto &rDPO = depict.lr.getRule();
	const auto vL = get(getMorL(rDPO), getK(rDPO), getL(rDPO), v);
	const auto vR = get(getMorR(rDPO), getK(rDPO), getR(rDPO), v);
	const auto &pMol = get_molecule(depict.lr);
	const auto l = pMol.getLeft()[vL].getIsotope();
	const auto r = pMol.getRight()[vR].getIsotope();
	if(l == r) return l;
	else return Isotope();
}

Charge DepictionData::K::getCharge(KVertex v) const {
	const auto &rDPO = depict.lr.getRule();
	const auto vL = get(getMorL(rDPO), getK(rDPO), getL(rDPO), v);
	const auto vR = get(getMorR(rDPO), getK(rDPO), getR(rDPO), v);
	const auto &pMol = get_molecule(depict.lr);
	const auto l = pMol.getLeft()[vL].getCharge();
	const auto r = pMol.getRight()[vR].getCharge();
	if(l == r) return l;
	else return Charge(0);
}

bool DepictionData::K::getRadical(KVertex v) const {
	const auto &rDPO = depict.lr.getRule();
	const auto vL = get(getMorL(rDPO), getK(rDPO), getL(rDPO), v);
	const auto vR = get(getMorR(rDPO), getK(rDPO), getR(rDPO), v);
	const auto &pMol = get_molecule(depict.lr);
	const auto l = pMol.getLeft()[vL].getRadical();
	const auto r = pMol.getRight()[vR].getRadical();
	if(l == r) return l;
	else return false;
}

std::string DepictionData::K::getVertexLabelNoIsotopeChargeRadical(KVertex v) const {
	const auto &rDPO = depict.lr.getRule();
	const auto vL = get(getMorL(rDPO), getK(rDPO), getL(rDPO), v);
	const auto vR = get(getMorR(rDPO), getK(rDPO), getR(rDPO), v);
	const auto atomId = getAtomId(v);
	if(atomId != AtomIds::Invalid)
		return Chem::symbolFromAtomId(atomId);
	const auto &pMol = get_molecule(depict.lr);
	const auto nonAtomIterLeft = depict.leftData.nonAtomToPhonyAtom.find(vL);
	const auto nonAtomIterRight = depict.rightData.nonAtomToPhonyAtom.find(vR);
	std::string left, right;
	if(nonAtomIterLeft == end(depict.leftData.nonAtomToPhonyAtom)) {
		const auto a = pMol.getLeft()[vL].getAtomId();
		left = Chem::symbolFromAtomId(a);
	} else {
		auto labelIterLeft = depict.phonyAtomToString.find(nonAtomIterLeft->second.getAtomId());
		assert(labelIterLeft != end(depict.phonyAtomToString));
		left = labelIterLeft->second;
	}
	if(nonAtomIterRight == end(depict.rightData.nonAtomToPhonyAtom)) {
		const auto a = pMol.getRight()[vR].getAtomId();
		right = Chem::symbolFromAtomId(a);
	} else {
		auto labelIterRight = depict.phonyAtomToString.find(nonAtomIterRight->second.getAtomId());
		assert(labelIterRight != end(depict.phonyAtomToString));
		right = labelIterRight->second;
	}
	if(left == right) return left;
	else return R"X($\langle$)X" + left + ", " + right + R"X($\rangle$)X";
}

BondType DepictionData::K::getBondData(KEdge e) const {
	const auto &rDPO = depict.lr.getRule();
	const auto &pMol = get_molecule(depict.lr);
	const auto eL = get(getMorL(rDPO), getK(rDPO), getL(rDPO), e);
	const auto eR = get(getMorR(rDPO), getK(rDPO), getR(rDPO), e);
	const auto l = pMol.getLeft()[eL];
	const auto r = pMol.getRight()[eR];
	if(l == r) return l;
	else return BondType::Invalid;
}

std::string DepictionData::K::getEdgeLabel(KEdge e) const {
	const auto &rDPO = depict.lr.getRule();
	const auto eL = get(getMorL(rDPO), getK(rDPO), getL(rDPO), e);
	const auto eR = get(getMorR(rDPO), getK(rDPO), getR(rDPO), e);
	const auto bt = getBondData(e);
	if(bt != BondType::Invalid)
		return std::string(1, Chem::bondToChar(bt));
	std::string left, right;
	const auto iterLeft = depict.leftData.nonBondEdges.find(eL);
	const auto iterRight = depict.rightData.nonBondEdges.find(eR);
	const auto &pMol = get_molecule(depict.lr);
	if(iterLeft != end(depict.leftData.nonBondEdges)) left = iterLeft->second;
	else left = std::string(1, Chem::bondToChar(pMol.getLeft()[eL]));
	if(iterRight != end(depict.rightData.nonBondEdges)) right = iterRight->second;
	else right = std::string(1, Chem::bondToChar(pMol.getRight()[eR]));
	if(left == right) return left;
	else return "$\\langle$" + left + ", " + right + "$\\rangle$";
}

bool DepictionData::K::hasImportantStereo(KVertex v) const {
	if(!has_stereo(depict.lr)) return false;
	return get_stereo(depict.lr).inContext(v) && depict.hasImportantStereo(v);
}

lib::IO::Graph::Write::EdgeFake3DType DepictionData::K::getEdgeFake3DType(KEdge e, bool withHydrogen) const {
	// TODO: translate v to vL and vR
	const auto &g = depict.lr.getRule().getCombinedGraph();
	const auto vSrc = source(e, g);
	const auto vTar = target(e, g);
	if(!hasImportantStereo(vSrc) && !hasImportantStereo(vTar))
		return lib::IO::Graph::Write::EdgeFake3DType::None;
#ifndef MOD_HAVE_OPENBABEL
		throw FatalError(MOD_NO_OPENBABEL_ERROR_STR);
#else
	assert(depict.hasMoleculeEncoding);
	const auto idSrc = get(boost::vertex_index_t(), g, vSrc);
	const auto idTar = get(boost::vertex_index_t(), g, vTar);
	const CoordData &cData = withHydrogen ? depict.cDataAll : depict.cDataNoHydrogen;
	if(has_stereo(depict.lr) && get_stereo(depict.lr).inContext(vSrc) && get_stereo(depict.lr).inContext(vTar))
		return cData.obMolLeft.getBondFake3D(idSrc, idTar);
	else
		return lib::IO::Graph::Write::EdgeFake3DType::None;
#endif
}

bool DepictionData::K::getHasCoordinates() const {
	return depict.getHasCoordinates();
}

double DepictionData::K::getX(KVertex v, bool withHydrogen) const {
	return depict.getX(v, withHydrogen);
}

double DepictionData::K::getY(KVertex v, bool withHydrogen) const {
	return depict.getY(v, withHydrogen);
}

const AtomData &DepictionData::K::operator()(KVertex v) const {
	const auto &rDPO = depict.lr.getRule();
	const auto vL = get(getMorL(rDPO), getK(rDPO), getL(rDPO), v);
	// for now, we just return whatever is in left, it's fake data anyway
	return depict.getLeft()(vL);
}

BondType DepictionData::K::operator()(KEdge e) const {
	const auto bt = getBondData(e);
	return bt == BondType::Invalid ? BondType::Single : bt;
}

//------------------------------------------------------------------------------
// Combined
//------------------------------------------------------------------------------

DepictionData::Combined::Combined(const DepictionData &depict) : depict(depict) {}

const AtomData &DepictionData::Combined::operator()(CombinedVertex v) const {
	if(VERBOSE) std::cout << "DepictionData::Combined(" << &depict << "): v=" << v << std::endl;
	// fake data, for creating OBMol
	const auto &rDPO = depict.lr.getRule();
	const auto &pMol = get_molecule(depict.lr);
	// return whatever we find
	const auto m = rDPO.getCombinedGraph()[v].membership;
	if(m != Membership::R) {
		const auto vL = get_inverse(rDPO.getLtoCG(), getL(rDPO), rDPO.getCombinedGraph(), v);
		if(VERBOSE) std::cout << "  vL=" << vL << std::endl;
		const auto &atomData = pMol.getLeft()[vL];
		const auto atomId = atomData.getAtomId();
		if(atomId != AtomIds::Invalid) return atomData;
		else {
			const auto iter = depict.leftData.nonAtomToPhonyAtom.find(vL);
			assert(iter != end(depict.leftData.nonAtomToPhonyAtom));
			return iter->second;
		}
	} else {
		const auto vR = get_inverse(rDPO.getRtoCG(), getR(rDPO), rDPO.getCombinedGraph(), v);
		if(VERBOSE) std::cout << "  vR=" << vR << std::endl;
		const auto &atomData = pMol.getRight()[vR];
		const auto atomId = atomData.getAtomId();
		if(atomId != AtomIds::Invalid) return atomData;
		else {
			const auto iter = depict.rightData.nonAtomToPhonyAtom.find(vR);
			assert(iter != end(depict.rightData.nonAtomToPhonyAtom));
			return iter->second;
		}
	}
}

BondType DepictionData::Combined::operator()(CombinedEdge e) const {
	// fake data, for creating OBMol
	const auto &rDPO = depict.lr.getRule();
	const auto &g = rDPO.getCombinedGraph();
	const auto &pMol = get_molecule(depict.lr);
	// if there is agreement, return that, otherwise prefer invalid bonds
	// this should give a bit more freedom in bond angles
	const auto m = g[e].membership;
	BondType l = BondType::Single, r = BondType::Single;
	if(m != Membership::R) {
		const auto eL = get_inverse(rDPO.getLtoCG(), getL(rDPO), g, e);
		l = pMol.getLeft()[eL];
	}
	if(m != Membership::L) {
		const auto eR = get_inverse(rDPO.getRtoCG(), getR(rDPO), g, e);
		r = pMol.getRight()[eR];
	}
	if(l == r) return l;
	else return BondType::Invalid;
}

//------------------------------------------------------------------------------
// DepictionData
//------------------------------------------------------------------------------

DepictionData::DepictionData(const LabelledRule &lr)
		: lr(lr), hasMoleculeEncoding(true) {
	if(VERBOSE) std::cout << "DepictionData(" << this << "):" << std::endl;

	const auto &rDPO = lr.getRule();
	const auto &g = get_graph(lr);
	const auto &pString = get_string(lr);
	const auto &pMol = get_molecule(lr);
	{ // vertexData
		std::vector<bool> atomUsed(AtomIds::Max + 1, false);
		Chem::markSpecialAtomsUsed(atomUsed);
		const auto findNonChemicalVertices = [&atomUsed](const lib::DPO::CombinedRule::SideGraphType &g,
		                                                 const PropMolecule::Side &pMol) {
			std::vector<SideVertex> res;
			if(VERBOSE) std::cout << "    |V|=" << num_vertices(g) << std::endl;
			for(const auto v: asRange(vertices(g))) {
				const auto atomId = pMol[v].getAtomId();
				if(atomId != AtomIds::Invalid) atomUsed[atomId] = true;
				else {
					if(VERBOSE) std::cout << "    v=" << v << " invalid" << std::endl;
					res.emplace_back(v);
				}
			}
			return res;
		};
		if(VERBOSE) std::cout << "  findNonChemicalVertices(L):" << std::endl;
		const auto vsToProcessL = findNonChemicalVertices(getL(rDPO), pMol.getLeft());
		if(VERBOSE) std::cout << "  findNonChemicalVertices(R):" << std::endl;
		const auto vsToProcessR = findNonChemicalVertices(getR(rDPO), pMol.getRight());

		// map non-atom labels to atoms
		std::map<std::string, AtomId, std::less<>> atomIdFromLabel;
		auto atomIdIter = atomUsed.begin() + 1; // skip invalid
		const auto processVertices = [this, &atomIdFromLabel, &atomUsed, &atomIdIter](
				const std::vector<SideVertex> &vs, const PropString::Side &pString, const PropMolecule::Side &pMol,
				SideData &data) {
			if(VERBOSE) std::cout << "    |vs|=" << vs.size() << std::endl;
			for(const auto v: vs) {
				std::string_view label = Chem::parseVertexLabel(pString[v]).rest;
				if(VERBOSE) std::cout << "    v=" << v << ", label=" << label << std::endl;
				const auto atomId = [this, &atomIdFromLabel, &label, &atomUsed, &atomIdIter]() {
					const auto iter = atomIdFromLabel.find(label);
					if(iter != end(atomIdFromLabel)) {
						if(VERBOSE) std::cout << "      already handled" << std::endl;
						return iter->second;
					}
					atomIdIter = std::find(atomIdIter, atomUsed.end(), false);
					if(atomIdIter == atomUsed.end()) {
						if(VERBOSE) std::cout << "      no more atomIds" << std::endl;
						hasMoleculeEncoding = false;
						return AtomIds::Invalid;
					}
					unsigned char atomId = atomIdIter - atomUsed.begin();
					if(VERBOSE) std::cout << "      atomId=" << int(atomId) << std::endl;
					atomUsed[atomId] = true;
					phonyAtomToString[AtomId(atomId)] = label;
					atomIdFromLabel.emplace(label, AtomId(atomId));
					return AtomId(atomId);
				}();
				if(atomId == AtomIds::Invalid) break;
				const auto charge = pMol[v].getCharge();
				const bool radical = pMol[v].getRadical();
				if(VERBOSE) std::cout << "      nonAtomToPhonyAtom[" << v << "]" << std::endl;
				data.nonAtomToPhonyAtom[v] = AtomData(atomId, charge, radical);
			}
		};
		if(VERBOSE) std::cout << "  processVertices(L):" << std::endl;
		processVertices(vsToProcessL, pString.getLeft(), pMol.getLeft(), leftData);
		if(VERBOSE) std::cout << "  processVertices(R):" << std::endl;
		processVertices(vsToProcessR, pString.getRight(), pMol.getRight(), rightData);
	}
	{ // edgeData
		const auto handleEdges = [](const lib::DPO::CombinedRule::SideGraphType &g,
		                            const PropString::Side &pString, const PropMolecule::Side &pMol,
		                            SideData &data) {
			for(const auto e: asRange(edges(g))) {
				const auto bt = pMol[e];
				if(bt == BondType::Invalid) {
					if(VERBOSE)
						std::cout << "    e=" << e << "(" << e.get_property() << "), label=" << pString[e] << std::endl;
					data.nonBondEdges[e] = pString[e];
				}
			}
		};
		if(VERBOSE) std::cout << "  handleEdges(L):" << std::endl;
		handleEdges(getL(rDPO), pString.getLeft(), pMol.getLeft(), leftData);
		if(VERBOSE) std::cout << "  handleEdges(R):" << std::endl;
		handleEdges(getR(rDPO), pString.getRight(), pMol.getRight(), rightData);
	}

	if(hasMoleculeEncoding) {
#ifdef MOD_HAVE_OPENBABEL
		const auto doIt = [&](CoordData &cData, const bool withHydrogen) {
			std::tie(cData.obMol, cData.obMolLeft, cData.obMolRight)
					= Chem::makeOBMol(lr, getCombined(), getCombined(),
					                  getLeft(), getLeft(),
					                  getRight(), getRight(),
					                  [this](const lib::DPO::CombinedRule::CombinedVertex v) {
						                  return mayCollapse(v);
					                  }, withHydrogen);
			cData.x.resize(num_vertices(g));
			cData.y.resize(num_vertices(g));
			for(const auto v: asRange(vertices(g))) {
				const auto vId = get(boost::vertex_index_t(), g, v);
				if(cData.obMol.hasAtom(vId)) {
					cData.x[vId] = cData.obMol.getAtomX(vId);
					cData.y[vId] = cData.obMol.getAtomY(vId);
				} else {
					assert(!withHydrogen);
					cData.x[vId] = std::numeric_limits<double>::quiet_NaN();
					cData.y[vId] = std::numeric_limits<double>::quiet_NaN();
				}
			}
		};
		doIt(cDataAll, true);
		doIt(cDataNoHydrogen, false);
#endif
	}
}

bool DepictionData::hasImportantStereo(CombinedVertex v) const {
	if(!has_stereo(lr)) return false;
	const auto &g = get_graph(lr);
	const auto m = g[v].membership;
	// TODO: map v to side vertices
	//assert(false);
	if(m != Membership::R && !get_stereo(get_labelled_left(lr))[v]->morphismDynamicOk()) return true;
	if(m != Membership::L && !get_stereo(get_labelled_right(lr))[v]->morphismDynamicOk()) return true;
	return false;
}

bool DepictionData::mayCollapse(CombinedVertex v) const {
	const auto &rDPO = lr.getRule();
	const auto &g = rDPO.getCombinedGraph();
	if(g[v].membership != lib::rule::Membership::K)
		return false;
	for(const auto e: asRange(out_edges(v, g)))
		if(g[e].membership != lib::rule::Membership::K)
			return false;
	// Do not use cgAtom and cgBond, as they provide fake data.
	// Instead, create some differently fake data specifically for hydrogen collapsing.
	const auto atomData = [&](const CombinedVertex v) {
		const auto vL = get_inverse(rDPO.getLtoCG(), getL(rDPO), g, v);
		const auto vR = get_inverse(rDPO.getRtoCG(), getR(rDPO), g, v);
		const auto l = getLeft().getAtomData(vL);
		const auto r = getRight().getAtomData(vR);
		if(l == r) return l;
		else return AtomData(AtomIds::Max);
	};
	const auto bondData = [&](const CombinedEdge e) {
		const auto eL = get_inverse(rDPO.getLtoCG(), getL(rDPO), g, e);
		const auto eR = get_inverse(rDPO.getRtoCG(), getR(rDPO), g, e);
		const auto l = getLeft().getBondData(eL);
		const auto r = getRight().getBondData(eR);
		if(l == r) return l;
		else return BondType::Invalid;
	};
	return lib::Chem::isCollapsibleHydrogen(v, g, atomData, bondData, [this](const CombinedVertex v) {
		return hasImportantStereo(v);
	});
}

bool DepictionData::getHasCoordinates() const {
#ifdef MOD_HAVE_OPENBABEL
	return hasMoleculeEncoding;
#else
	return false;
#endif
}

double DepictionData::getX(CombinedVertex v, bool withHydrogen) const {
	if(!getHasCoordinates()) MOD_ABORT;
	const auto &g = get_graph(lr);
	const auto vId = get(boost::vertex_index_t(), g, v);
	const CoordData &cData = withHydrogen ? cDataAll : cDataNoHydrogen;
	assert(vId < cData.x.size());
	return cData.x[vId];
}

double DepictionData::getY(CombinedVertex v, bool withHydrogen) const {
	if(!getHasCoordinates()) MOD_ABORT;
	const auto &g = get_graph(lr);
	const auto vId = get(boost::vertex_index_t(), g, v);
	const CoordData &cData = withHydrogen ? cDataAll : cDataNoHydrogen;
	assert(vId < cData.y.size());
	return cData.y[vId];
}

void DepictionData::copyCoords(const DepictionData &other, const std::map<CombinedVertex, CombinedVertex> &vMap) {
	if(!other.getHasCoordinates()) return;
	const auto &g = get_graph(lr);
	const auto doIt = [&](CoordData &cData, const bool withHydrogen) {
		cData.x.resize(num_vertices(g));
		cData.y.resize(num_vertices(g));
		for(const auto v: asRange(vertices(g))) {
			const auto iter = vMap.find(v);
			if(iter == end(vMap)) {
				std::cout << "Vertex " << v << " (id=" << get(boost::vertex_index_t(), g, v) << ") not mapped."
				          << std::endl;
				std::cout << "Map:" << std::endl;
				for(auto p: vMap) std::cout << "\t" << p.first << " => " << p.second << std::endl;
				std::cout << "num_vertices: " << num_vertices(g) << std::endl;
				std::cout << "other.num_vertices: " << num_vertices(get_graph(other.lr)) << std::endl;
				MOD_ABORT;
			}
			const auto vOther = iter->second;
			const auto vId = get(boost::vertex_index_t(), g, v);
			cData.x[vId] = other.getX(vOther, withHydrogen);
			cData.y[vId] = other.getY(vOther, withHydrogen);
		}
		if(hasMoleculeEncoding) {
#ifdef MOD_HAVE_OPENBABEL
			cData.obMol.setCoordinates(cData.x, cData.y);
			cData.obMolLeft.setCoordinates(cData.x, cData.y);
			cData.obMolRight.setCoordinates(cData.x, cData.y);
#endif
		}
	};
	doIt(cDataAll, true);
	doIt(cDataNoHydrogen, false);
}

DepictionData::Side DepictionData::getLeft() const {
	if(!hasMoleculeEncoding) MOD_ABORT;
	return {*this, leftData, &CoordData::obMolLeft, getL(lr.getRule()),
	        lr.getRule().getLtoCG(),
	        get_molecule(lr).getLeft(),
	        [this]() { return get_stereo(lr).getLeft(); }};
}

DepictionData::K DepictionData::getContext() const {
	if(!hasMoleculeEncoding) MOD_ABORT;
	return {*this};
}

DepictionData::Side DepictionData::getRight() const {
	if(!hasMoleculeEncoding) MOD_ABORT;
	return {*this, rightData, &CoordData::obMolRight, getR(lr.getRule()),
	        lr.getRule().getRtoCG(),
	        get_molecule(lr).getRight(),
	        [this]() { return get_stereo(lr).getRight(); }};
}

DepictionData::Combined DepictionData::getCombined() const {
	return {*this};
}

} // namespace mod::lib::rule::Write