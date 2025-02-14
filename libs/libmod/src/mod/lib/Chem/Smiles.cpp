#include "Smiles.hpp"

#include <mod/Error.hpp>
#include <mod/lib/Chem/MoleculeUtil.hpp>
#include <mod/lib/Graph/Properties/String.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

namespace mod::lib::Chem {

const std::vector<AtomId> &getSmilesOrganicSubset() {
	using namespace AtomIds;
	static std::vector<AtomId> atomIds{
		Boron,
		Carbon,
		Nitrogen,
		Oxygen,
		Phosphorus,
		Sulfur,
		Fluorine,
		Chlorine,
		Bromine,
		Iodine
	};
	return atomIds;
}

bool isInSmilesOrganicSubset(AtomId atomId) {
	// B, C, N, O, P, S, F, Cl, Br, I
	using namespace AtomIds;
	switch(atomId) {
	case Boron:
	case Carbon:
	case Nitrogen:
	case Oxygen:
	case Phosphorus:
	case Sulfur:
	case Fluorine:
	case Chlorine:
	case Bromine:
	case Iodine: return true;
	default: return false;
	}
}

void addImplicitHydrogens(lib::graph::GraphType &g, lib::graph::PropString &pString, lib::graph::Vertex v, AtomId atomId,
		std::function<void(lib::graph::GraphType&, lib::graph::PropString&, lib::graph::Vertex)> hydrogenAdder) {
	//==========================================================================
	// WARNING: keep in sync with the smiles writer
	//==========================================================================
	assert(isInSmilesOrganicSubset(atomId));
	// B, C, N, O, P, S, F, Cl, Br, I
	// B			3
	// C			4
	// N			3, 5, {{-, :, :}}, {{-, -, =}}, {{:, :, :}}
	// O			2
	// P			3, 5
	// S			2, 4, 6, {{:, :}}
	// halogens		1
	int numSingle = 0, numDouble = 0, numTriple = 0, numAromatic = 0, valenceNoAromatic = 0;
	for(lib::graph::Edge eOut : asRange(out_edges(v, g))) {
		BondType bt = decodeEdgeLabel(pString[eOut]);
		switch(bt) {
		case BondType::Invalid: MOD_ABORT;
		case BondType::Single:
			++numSingle;
			break;
		case BondType::Double:
			++numDouble;
			break;
		case BondType::Triple:
			++numTriple;
			break;
		case BondType::Aromatic:
			++numAromatic;
			break;
		}
	}
	valenceNoAromatic = numSingle + numDouble * 2 + numTriple * 3;
	using BondVector = std::tuple<int, int, int, int>;
	BondVector numBonds{numSingle, numDouble, numTriple, numAromatic};
	using namespace AtomIds;
	int valence = valenceNoAromatic + numAromatic;
	if(numAromatic > 0) ++valence;
	const auto fMissingH = [ = ]() -> int {
		switch(atomId) {
		case Boron:
			if(valence <= 3) return 3 - valence;
			else return 0;
		case Carbon:
			if(valence <= 4) return 4 - valence;
			else return 0;
		case Nitrogen:
			if(numBonds == BondVector(1, 0, 0, 2)) return 0;
			if(numBonds == BondVector(2, 1, 0, 0)) return 0;
			if(numBonds == BondVector(0, 0, 0, 3)) return 0;
			[[fallthrough]];
		case Phosphorus:
			if(valence <= 3) return 3 - valence;
			else if(valence <= 5) return 5 - valence;
			else return 0;
		case Oxygen:
			if(valence <= 2) return 2 - valence;
			else return 0;
		case Sulfur:
			if(numBonds == BondVector(0, 0, 0, 2)) return 0;
			if(valence <= 2) return 2 - valence;
			else if(valence <= 4) return 4 - valence;
			else if(valence <= 6) return 6 - valence;
			else return 0;
		case Fluorine:
		case Chlorine:
		case Bromine:
		case Iodine:
			if(valence <= 1) return 1 - valence;
			else return 0;
		default: MOD_ABORT;
		}
		return 0;
	};
	const int hToAdd = fMissingH();
	for(unsigned char i = 0; i < hToAdd; i++) hydrogenAdder(g, pString, v);
}

} // namespace mod::lib::Chem