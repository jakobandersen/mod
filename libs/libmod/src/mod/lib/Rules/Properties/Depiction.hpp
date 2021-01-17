#ifndef MOD_LIB_RULES_STATE_DEPICTION_HPP
#define MOD_LIB_RULES_STATE_DEPICTION_HPP

#include <mod/BuildConfig.hpp>
#include <mod/lib/Chem/OBabel.hpp>
#include <mod/lib/Rules/GraphDecl.hpp>
#include <mod/lib/Rules/LabelledRule.hpp>

#include <map>
#include <vector>

namespace mod {
struct AtomId;
struct Charge;
struct AtomData;
enum class BondType;
} // namespace mod
namespace mod::lib::Rules {
struct PropStringCore;
struct PropMoleculeCore;

struct DepictionDataCore {

	template<Membership membership>
	struct DepictionData { // instantiated in the cpp file
		DepictionData(const DepictionDataCore &depict);
		AtomId getAtomId(Vertex v) const; // shortcut to moleculeState
		Isotope getIsotope(Vertex v) const; // shortcut to moleculeState
		Charge getCharge(Vertex v) const; // shortcut to moleculeState
		bool getRadical(Vertex v) const; // shortcut to moleculeState
		BondType getBondData(Edge e) const; // shortcut to moleculeState
		std::string getVertexLabelNoIsotopeChargeRadical(Vertex v) const;
		std::string getEdgeLabel(Edge e) const;
		const AtomData &operator()(Vertex v) const; // fake data
		BondType operator()(Edge e) const; // fake data
		bool hasImportantStereo(Vertex v) const;
		bool getHasCoordinates() const;
		double getX(Vertex v, bool withHydrogen) const;
		double getY(Vertex v, bool withHydrogen) const;
		lib::IO::Graph::Write::EdgeFake3DType getEdgeFake3DType(Edge e, bool withHydrogen) const;
		//	private:
		//		bool isAtomIdInvalidContext(Vertex v) const;
	private:
		const DepictionDataCore &depict;
	};
public:
	DepictionDataCore(const LabelledRule &lr);
	DepictionDataCore(const DepictionDataCore&) = delete;
	DepictionDataCore &operator=(const DepictionDataCore&) = delete;
	const AtomData &operator()(Vertex v) const; // fake data, for creating OBMol
	BondType operator()(Edge e) const; // fake data, for creating OBMol
	bool hasImportantStereo(Vertex v) const;
	bool getHasCoordinates() const;
	double getX(Vertex v, bool withHydrogen) const;
	double getY(Vertex v, bool withHydrogen) const;
	// vMap: this -> other
	void copyCoords(const DepictionDataCore &other, const std::map<Vertex, Vertex> &vMap);
public: // projections
	DepictionData<Membership::Left> getLeft() const;
	DepictionData<Membership::Context> getContext() const;
	DepictionData<Membership::Right> getRight() const;
private:
	const LabelledRule &lr;
	bool hasMoleculeEncoding, hasCoordinates;
	std::map<Vertex, AtomData> nonAtomToPhonyAtomLeft, nonAtomToPhonyAtomRight;
	std::map<AtomId, std::string> phonyAtomToString;
	std::map<Edge, std::string> nonBondEdgesLeft, nonBondEdgesRight;

	struct CoordData {
		std::vector<double> x, y;
#ifdef MOD_HAVE_OPENBABEL
		lib::Chem::OBMolHandle obMol; // the pushout, for generating coords
		lib::Chem::OBMolHandle obMolLeft, obMolRight; // each side, with copied coords, for stereo
#endif
	} cDataAll, cDataNoHydrogen;
};

} // namespace mod::lib::Rules

#endif // MOD_LIB_RULES_STATE_DEPICTION_HPP