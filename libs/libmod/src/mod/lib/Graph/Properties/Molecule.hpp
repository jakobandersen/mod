#ifndef MOD_LIB_GRAPH_PROP_MOLECULE_HPP
#define MOD_LIB_GRAPH_PROP_MOLECULE_HPP

#include <mod/BuildConfig.hpp>
#include <mod/Chem.hpp>
#include <mod/lib/Chem/OBabel.hpp>
#include <mod/lib/Graph/Properties/Property.hpp>

#include <boost/optional.hpp>

namespace mod::lib::Graph {
struct PropString;

struct PropMolecule : Prop<PropMolecule, AtomData, BondType> {
	using Base = Prop<PropMolecule, AtomData, BondType>;
public:
	PropMolecule(const GraphType &g, const PropString &pString);
	bool getIsMolecule() const;
#ifdef MOD_HAVE_OPENBABEL
	const lib::Chem::OBMolHandle &getOBMol() const;
#endif
	double getExactMass() const;
	double getEnergy() const;
	void cacheEnergy(double value) const;
private:
	bool isMolecule;
	mutable boost::optional<double> exactMass;
#ifdef MOD_HAVE_OPENBABEL
	mutable lib::Chem::OBMolHandle obMol;
#endif
	mutable boost::optional<double> energy;
};

} // namespace mod::lib::Graph

#endif // MOD_LIB_GRAPH_PROP_MOLECULE_HPP