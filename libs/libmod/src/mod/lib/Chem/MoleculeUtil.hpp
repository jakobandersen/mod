#ifndef MOD_LIB_CHEM_MOLECULEUTIL_HPP
#define MOD_LIB_CHEM_MOLECULEUTIL_HPP

#include <mod/Chem.hpp>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace mod {

inline bool isCleanHydrogen(const AtomData &ad) {
	return ad == AtomData(AtomIds::H);
}

} // namespace mod
namespace mod::lib::Chem {

struct VertexLabelParseResultExtra {
	Isotope isotope;
	Charge charge;
	bool radical;
};

struct VertexLabelParseResult : VertexLabelParseResultExtra {
	std::string_view rest;
};

struct VertexLabelDecodeResult : VertexLabelParseResultExtra {
	AtomId atomId;
public:
	AtomData asAtomData() const {
		return {atomId, isotope, charge, radical};
	}
};

AtomId atomIdFromSymbol(std::string_view label);
VertexLabelParseResult parseVertexLabel(std::string_view label);
// we don't return an AtomData, because it could potentially have extra stuff that we don't decode
VertexLabelDecodeResult decodeVertexLabel(std::string_view label);
BondType decodeEdgeLabel(std::string_view label);
void markSpecialAtomsUsed(std::vector<bool> &used);
std::string symbolFromAtomId(AtomId atomId);
void appendSymbolFromAtomId(std::string &s, AtomId atomId);
char bondToChar(BondType bt);
std::string chargeSuffix(Charge c);

double exactMass(AtomId a, Isotope i); // implemented in Mass.cpp
constexpr double electronMass = 0.000548580;
constexpr double GasConstant = 1.98720425864083e-3; // kcal/(K * mol)

template<typename Graph, typename AtomDataT, typename EdgeDataT, typename HasImportantStereo>
bool isCollapsibleHydrogen(const typename boost::graph_traits<Graph>::vertex_descriptor v,
                           const Graph &g,
                           const AtomDataT &atomData,
                           const EdgeDataT &edgeData,
                           const HasImportantStereo &hasImportantStereo) {
	if(!isCleanHydrogen(atomData(v))) return false;
	if(out_degree(v, g) != 1) return false;
	const auto e = *out_edges(v, g).first;
	const auto eType = edgeData(e);
	if(eType != BondType::Single) return false;
	const auto vAdj = target(e, g);
	if(hasImportantStereo(vAdj)) return false;
	if(!isCleanHydrogen(atomData(vAdj))) return true;
	// collapse the highest id vertex in H_2
	return get(boost::vertex_index_t(), g, v) > get(boost::vertex_index_t(), g, vAdj);
}

} // namespace mod::lib::Chem

#endif // MOD_LIB_CHEM_MOLECULEUTIL_HPP