#ifndef MOD_LIB_RULES_IO_DEPICTIONDATA_HPP
#define MOD_LIB_RULES_IO_DEPICTIONDATA_HPP

#include <mod/BuildConfig.hpp>
#include <mod/lib/Chem/OBabel.hpp>
#include <mod/lib/Rule/GraphDecl.hpp>
#include <mod/lib/Rule/LabelledRule.hpp>

#include <map>
#include <vector>

namespace mod {
struct AtomId;
struct Charge;
struct AtomData;
enum class BondType;
} // namespace mod
namespace mod::lib::rule {
struct PropString;
struct PropMolecule;
} // namespace mod::lib::rule
namespace mod::lib::rule::Write {

struct DepictionData {
	using CombinedVertex = lib::DPO::CombinedRule::CombinedVertex;
	using CombinedEdge = lib::DPO::CombinedRule::CombinedEdge;
	using SideVertex = lib::DPO::CombinedRule::SideVertex;
	using SideEdge = lib::DPO::CombinedRule::SideEdge;
	using KVertex = lib::DPO::CombinedRule::KVertex;
	using KEdge = lib::DPO::CombinedRule::KEdge;
	using SideToCG = lib::DPO::CombinedRule::ToCombinedMorphismSide;
private:
	struct SideData;
	struct CoordData;
public:
	struct Side {
		Side(const DepictionData &depict, const SideData &data,
		     const Chem::OBMolHandle CoordData::*obSide,
		     const lib::DPO::CombinedRule::SideGraphType &g,
		     SideToCG mToCG,
		     PropMolecule::Side pMol, // we need definitely need mol data
				// we don't want to instantiate stereo data unless needed
           std::function<PropStereo::Side()> fStereo);
	public: // used in GraphWriteGeneric
		AtomData getAtomData(SideVertex vS) const; // shortcut to moleculeState
		AtomId getAtomId(SideVertex vS) const; // shortcut to moleculeState
		Isotope getIsotope(SideVertex vS) const; // shortcut to moleculeState
		Charge getCharge(SideVertex vS) const; // shortcut to moleculeState
		bool getRadical(SideVertex vS) const; // shortcut to moleculeState
		std::string getVertexLabelNoIsotopeChargeRadical(SideVertex vS) const;
		BondType getBondData(SideEdge eS) const; // shortcut to moleculeState
		std::string getEdgeLabel(SideEdge eS) const;
		bool hasImportantStereo(SideVertex vS) const;
		lib::IO::Graph::Write::EdgeFake3DType getEdgeFake3DType(SideEdge eS, bool withHydrogen) const;
		bool getHasCoordinates() const;
		double getX(SideVertex vS, bool withHydrogen) const;
		double getY(SideVertex vS, bool withHydrogen) const;
	public: // for coordinates
		const AtomData &operator()(SideVertex vS) const; // fake data
		BondType operator()(SideEdge eS) const; // fake data
	private:
		const DepictionData &depict;
		const SideData &data;
		const Chem::OBMolHandle CoordData::*obSide;
		const lib::DPO::CombinedRule::SideGraphType &g;
		const SideToCG mToCG;
		PropMolecule::Side pMol;
		std::function<PropStereo::Side()> fStereo;
	};
	struct K {
		K(const DepictionData &depict);
	public: // used in GraphWriteGeneric
		AtomId getAtomId(KVertex v) const; // shortcut to moleculeState
		Isotope getIsotope(KVertex v) const; // shortcut to moleculeState
		Charge getCharge(KVertex v) const; // shortcut to moleculeState
		bool getRadical(KVertex v) const; // shortcut to moleculeState
		std::string getVertexLabelNoIsotopeChargeRadical(KVertex v) const;
		BondType getBondData(KEdge e) const; // shortcut to moleculeState
		std::string getEdgeLabel(KEdge e) const;
		bool hasImportantStereo(KVertex v) const;
		lib::IO::Graph::Write::EdgeFake3DType getEdgeFake3DType(KEdge e, bool withHydrogen) const;
		bool getHasCoordinates() const;
		double getX(KVertex v, bool withHydrogen) const;
		double getY(KVertex v, bool withHydrogen) const;
	public: // for coordinates
		const AtomData &operator()(KVertex v) const; // fake data
		BondType operator()(KEdge e) const; // fake data
	private:
		const DepictionData &depict;
	};
	struct Combined {
		Combined(const DepictionData &depict);
	public: // for coordinates
		const AtomData &operator()(CombinedVertex v) const; // fake data
		BondType operator()(CombinedEdge e) const; // fake data
	private:
		const DepictionData &depict;
	};
public:
	DepictionData(const DepictionData &) = delete;
	DepictionData &operator=(const DepictionData &) = delete;
public:
	DepictionData(const LabelledRule &lr);
	bool hasImportantStereo(CombinedVertex v) const;
	bool mayCollapse(CombinedVertex v) const;
	bool getHasCoordinates() const;
	double getX(CombinedVertex v, bool withHydrogen) const;
	double getY(CombinedVertex v, bool withHydrogen) const;
	// vMap: this -> other
	void copyCoords(const DepictionData &other, const std::map<CombinedVertex, CombinedVertex> &vMap);
public: // projections
	Side getLeft() const;
	K getContext() const;
	Side getRight() const;
	Combined getCombined() const;
private:
	const LabelledRule &lr;
	bool hasMoleculeEncoding;
	std::map<AtomId, std::string> phonyAtomToString;
	struct SideData {
		std::map<SideVertex, AtomData> nonAtomToPhonyAtom;
		std::map<SideEdge, std::string> nonBondEdges;
	} leftData, rightData;

	struct CoordData {
		std::vector<double> x, y;
#ifdef MOD_HAVE_OPENBABEL
		// the pushout, for generating coords
		lib::Chem::OBMolHandle obMol;
		// each side, for stereo, with coords copied from the pushout
		lib::Chem::OBMolHandle obMolLeft, obMolRight;
#endif
	} cDataAll, cDataNoHydrogen;
};

} // namespace mod::lib::rule

#endif // MOD_LIB_RULES_IO_DEPICTIONDATA_HPP