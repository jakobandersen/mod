#include "Molecule.hpp"

#include <mod/Error.hpp>
#include <mod/lib/Chem/MoleculeUtil.hpp>
#include <mod/lib/Chem/OBabel.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/IO/IO.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <iostream>

namespace mod::lib::Graph {

PropMolecule::PropMolecule(const GraphType &g, const PropString &pString) : Base(g), isMolecule(true) {
	// atomData
	this->vertexState.resize(num_vertices(g));
	for(Vertex v : asRange(vertices(g))) {
		AtomId atomId;
		Isotope isotope;
		Charge charge;
		bool radical;
		std::tie(atomId, isotope, charge, radical) = Chem::decodeVertexLabel(pString[v]);
		//		std::cout << "Decode(" << pString[v] << "): " << atomId << ", " << isotope << ", " << charge << ", " << radical << std::endl;
		this->vertexState[get(boost::vertex_index_t(), g, v)] = AtomData(atomId, isotope, charge, radical);
		if(atomId == AtomIds::Invalid) isMolecule = false;
	}

	// edgeData
	this->edgeState.reserve(num_edges(g));
	for(Edge e : asRange(edges(g))) {
		BondType bt = Chem::decodeEdgeLabel(pString[e]);
		assert(get(boost::edge_index_t(), g, e) == this->edgeState.size());
		this->edgeState.push_back(bt);
		if(bt == BondType::Invalid) isMolecule = false;
	}

	verify(&g);
}

bool PropMolecule::getIsMolecule() const {
	return isMolecule;
}

#ifdef MOD_HAVE_OPENBABEL

const lib::Chem::OBMolHandle &PropMolecule::getOBMol() const {
	if(!isMolecule) {
		std::cout << "MoleculeState: Trying to create OpenBabel::OBMol from non-molecule." << std::endl
		          << "Should DepictionData be used instead?" << std::endl;
		MOD_ABORT;
	}
	if(!obMol) {
		obMol = Chem::makeOBMol(*this->g, [this](Vertex v) -> const AtomData & {
			return (*this)[v];
		}, [this](Edge e) {
			return (*this)[e];
		}, jla_boost::AlwaysFalse(), true, nullptr);
	}
	return obMol;
}

#endif

double PropMolecule::getExactMass() const {
	if(!getIsMolecule()) MOD_ABORT;
	if(!exactMass) {
		const auto vs = vertices(*g);
		exactMass = std::accumulate(vs.first, vs.second, 0.0, [&](double val, const auto v) {
			const auto &ad = (*this)[v];
			return val + lib::Chem::exactMass(ad.getAtomId(), ad.getIsotope()) - lib::Chem::electronMass * ad.getCharge();
		});
	}
	return *exactMass;
}

double PropMolecule::getEnergy() const {
	if(!energy) {
#ifndef MOD_HAVE_OPENBABEL
		MOD_NO_OPENBABEL_ERROR
		std::cout << "Energy calculation is not possible without Open Babel." << std::endl;
		std::cout << "Energy values can be manually cached on graphs if calculation is not desired." << std::endl;
		std::exit(1);
#else
		energy = getOBMol().getEnergy();
#endif
	}
	return *energy;
}

void PropMolecule::cacheEnergy(double value) const {
	if(energy) {
		std::cout << "Can not cache energy on graph already with energy." << std::endl;
		MOD_ABORT;
	}
	energy = value;
}

} // namespace mod::lib::Graph