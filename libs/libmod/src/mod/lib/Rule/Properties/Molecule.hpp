#ifndef MOD_LIB_RULES_PROP_MOLECULE_HPP
#define MOD_LIB_RULES_PROP_MOLECULE_HPP

#include <mod/lib/Chem/MoleculeUtil.hpp>
#include <mod/lib/Rule/GraphDecl.hpp>
#include <mod/lib/Rule/Properties/Property.hpp>

namespace mod {
struct AtomData;
enum class BondType;
} // namespace mod
namespace mod::lib::rule {
struct PropString;

struct PropMolecule : private PropBase<PropMolecule, AtomData, BondType> {
	// read-only of data, so do 'using' of things needed
	using Base = PropBase<PropMolecule, AtomData, BondType>;
	using Base::Side;
public:
	PropMolecule(const RuleType &rule, const PropString &pString);
public: // to be able to form pointers to getLeft and getRight it is not enough to 'using' them
	Side getLeft() const { return {*this, vPropL, ePropL, getL(rule)}; }
	Side getRight() const { return {*this, vPropR, ePropR, getR(rule)}; }
public:
	using Base::invert;
private:
	bool isReaction;
};

} // namespace mod::lib::rule

#endif // MOD_LIB_RULES_STATE_MOLECULE_HPP