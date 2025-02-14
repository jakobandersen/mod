#include "Molecule.hpp"

#include <mod/Error.hpp>
#include <mod/lib/Chem/MoleculeUtil.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/Rule/Properties/String.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

namespace mod::lib::rule {

PropMolecule::PropMolecule(const RuleType &rule, const PropString &pString)
		: Base(rule), isReaction(true) {
	const auto decodeVLabel = [this](const std::string &label) {
		const auto p = Chem::decodeVertexLabel(label);
		if(p.atomId == AtomIds::Invalid) isReaction = false;
		return p.asAtomData();
	};
	const auto decodeELabel = [this](const std::string &label) {
		const auto bondType = Chem::decodeEdgeLabel(label);
		if(bondType == BondType::Invalid) isReaction = false;
		return bondType;
	};

	vPropL.resize(num_vertices(getL(rule)));
	vPropR.resize(num_vertices(getR(rule)));
	for(const auto v: asRange(vertices(getL(rule)))) {
		const auto vId = get(boost::vertex_index_t(), getL(rule), v);
		vPropL[vId] = decodeVLabel(pString.getLeft()[v]);
	}
	for(const auto v: asRange(vertices(getR(rule)))) {
		const auto vId = get(boost::vertex_index_t(), getR(rule), v);
		vPropR[vId] = decodeVLabel(pString.getRight()[v]);
	}

	ePropL.resize(num_edges(getL(rule)));
	ePropR.resize(num_edges(getR(rule)));
	for(const auto e: asRange(edges(getL(rule)))) {
		const auto eId = get(boost::edge_index_t(), getL(rule), e);
		ePropL[eId] = decodeELabel(pString.getLeft()[e]);
	}
	for(const auto e: asRange(edges(getR(rule)))) {
		const auto eId = get(boost::edge_index_t(), getR(rule), e);
		ePropR[eId] = decodeELabel(pString.getRight()[e]);
	}
}

} // namespace mod::lib::rule