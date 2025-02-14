#ifndef MOD_LIB_GRAPH_PROP_MOLECULE_HPP
#define MOD_LIB_GRAPH_PROP_MOLECULE_HPP

#include <mod/BuildConfig.hpp>
#include <mod/Chem.hpp>
#include <mod/lib/Chem/OBabel.hpp>
#include <mod/lib/Graph/Properties/Property.hpp>

#include <optional>

namespace mod::lib::graph {
struct PropString;

struct PropMolecule : Prop<PropMolecule, AtomData, BondType> {
	using Base = Prop<PropMolecule, AtomData, BondType>;
public:
	using Base::Prop;
	PropMolecule(const GraphType &g, const PropString &pString);
	bool getIsMolecule() const;
	bool getHasOnlyChemicalBonds() const;
#ifdef MOD_HAVE_OPENBABEL
	const lib::Chem::OBMolHandle &getOBMol() const;
#endif
	double getExactMass() const;
	double getEnergy() const;
	void cacheEnergy(double value) const;
private:
	bool isMolecule, hasOnlyChemicalBonds;
	mutable std::optional<double> exactMass;
#ifdef MOD_HAVE_OPENBABEL
	mutable lib::Chem::OBMolHandle obMol;
#endif
	mutable std::optional<double> energy;
};

} // namespace mod::lib::graph

#endif // MOD_LIB_GRAPH_PROP_MOLECULE_HPP