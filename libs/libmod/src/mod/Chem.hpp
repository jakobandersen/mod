#ifndef MOD_CHEM_HPP
#define MOD_CHEM_HPP

#include <mod/BuildConfig.hpp>

#include <cassert>
#include <iosfwd>
#include <tuple>
#include <string>

#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/tuple/elem.hpp>

namespace mod {

// rst-class: AtomId
// rst:
// rst:		Representation of the chemical element of an atom.
// rst-class-start:
struct MOD_DECL AtomId {
	// rst: .. function:: constexpr AtomId()
	// rst:
	// rst:		Construct an :cpp:var:`AtomIds::Invalid` atom id.
	constexpr AtomId();
	// rst: .. function:: explicit constexpr AtomId(unsigned int id)
	// rst:
	// rst:		Construct a specific atom id. Pre-condition: the id must be at most that of :cpp:var:`AtomIds::Max`.
	explicit constexpr AtomId(unsigned char id);
	// rst: .. function:: constexpr operator unsigned char() const
	// rst:
	// rst:		Implicit conversion to an integer type.
	constexpr operator unsigned char() const;
	// rst: .. function:: std::string symbol() const
	// rst:
	// rst:		:returns: the symbol represented by the atom id.
	// rst:		:raises: :cpp:class:`LogicError` if the id is invalid.
	std::string symbol() const;
	// rst: .. function:: friend std::ostream &operator<<(std::ostream &s, AtomId atomId)
	// rst:
	// rst:		Inserts the `int` value of the atom id into the stream.
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, AtomId atomId);
private:
	unsigned char id;
};
// rst-class-end:

// rst-class: Isotope
// rst:
// rst:		Representation of the isotope of an atom.
// rst:
// rst-class-start:
struct MOD_DECL Isotope {
	// rst: .. function:: constexpr Isotope()
	// rst:
	// rst:		Construct a representation of the most abundant isotope.
	// rst:		
	// rst:		.. note:: This is different from explicitly specifying the isotope that is the most abundant one.
	constexpr Isotope();
	// rst: .. function:: explicit constexpr Isotope(int i)
	// rst:
	// rst:		Construct a specific isotope. Pre-condition: the isotope must either be at least 1 or be -1.
	// rst:		Passing -1 is equivalent to default-construction.
	explicit constexpr Isotope(int i);
	// rst: .. function:: constexpr operator int() const
	// rst:
	// rst:		Implicit conversion to an integer type.
	constexpr operator int() const;
	// rst: .. function:: friend std::ostream &operator<<(std::ostream &s, Isotope iso)
	// rst:
	// rst:		Inserts the integer value of the isotope into the stream.
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, Isotope iso);
private:
	int i;
};
// rst-class-end:

// rst-class: Charge
// rst:
// rst:		Representation of the charge of an atom.
// rst-class-start:
struct MOD_DECL Charge {
	// rst: .. function:: constexpr Charge()
	// rst:
	// rst:		Construct a neutral charge.
	constexpr Charge();
	// rst: .. function:: explicit constexpr Charge(signed char c)
	// rst:
	// rst:		Construct a specific charge. Pre-condition: the charge must be in the range :math:`[-9, 9]`.
	explicit constexpr Charge(signed char c);
	// rst: .. function:: constexpr operator signed char() const
	// rst:
	// rst:		Implicit conversion to an integer type.
	constexpr operator signed char() const;
	// rst: .. function:: friend std::ostream &operator<<(std::ostream &s, Charge charge)
	// rst:
	// rst:		Inserts the `int` value of the charge into the stream.
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, Charge charge);
private:
	signed char c;
};
// rst-class-end:

// rst-class: AtomData
// rst:
// rst:		Representation of basic data of an atom.
// rst-class-start:
struct MOD_DECL AtomData {
	// rst: .. function:: constexpr AtomData()
	// rst:
	// rst:		Construct atom data with default values:
	// rst:
	// rst:		- :cpp:var:`AtomIds::Invalid` atom id,
	// rst:		- :cpp:texpr:`Isotope()` as isotope,
	// rst:		- neutral charge, and
	// rst:		- no radical.
	constexpr AtomData() = default;
	// rst: .. function:: constexpr explicit AtomData(AtomId atomId)
	// rst:
	// rst:		Construct atom data the given atom id, and otherwise default values (see above).
	constexpr explicit AtomData(AtomId atomId);
	// rst: .. function:: constexpr AtomData(AtomId atomId, Charge charge, bool radical)
	// rst:
	// rst:		Construct atom data with given atom id, charge, and radical, but with default isotope.
	constexpr AtomData(AtomId atomId, Charge charge, bool radical);
	// rst: .. function:: constexpr AtomData(AtomId atomId, Isotope isotope, Charge charge, bool radical)
	// rst:
	// rst:		Construct atom data with given atom id, isotope, charge, and radical.
	constexpr AtomData(AtomId atomId, Isotope isotope, Charge charge, bool radical);
	// rst: .. function:: constexpr AtomId getAtomId() const
	// rst:
	// rst:		Retrieve the atom id.
	constexpr AtomId getAtomId() const;
	// rst: .. function:: constexpr Isotope getIsotope() const
	// rst:
	// rst:		Retrieve the isotope.
	constexpr Isotope getIsotope() const;
	// rst: .. function:: constexpr Charge getCharge() const
	// rst:
	// rst:		Retrieve the charge.
	constexpr Charge getCharge() const;
	// rst: .. function:: constexpr bool getRadical() const
	// rst:
	// rst:		Retrieve the radical status.
	constexpr bool getRadical() const;
	friend constexpr bool operator==(const AtomData &a1, const AtomData &a2);
	friend constexpr bool operator!=(const AtomData &a1, const AtomData &a2);
	friend constexpr bool operator<(const AtomData &a1, const AtomData &a2);
	// rst: .. function:: friend std::ostream &operator<<(std::ostream &s, const AtomData &data)
	// rst:
	// rst:		Format the atom data adhering to the string encoding of atoms (see :ref:`mol-enc`).
	// rst:
	// rst:		:throws: :class:`LogicError` if the atom id is :cpp:var:`AtomIds::Invalid`.
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const AtomData &data);
private:
	AtomId atomId;
	Isotope isotope;
	Charge charge;
	bool radical = false;
};
// rst-class-end:

// rst: Enum ``BondType``
// rst: -------------------
// rst:
// rst: .. enum-struct:: BondType
// rst:
// rst:		Representation of a bond type (see :ref:`mol-enc`).
// rst:
enum class BondType {
	// rst:		.. enumerator:: Invalid
	// rst:		.. enumerator:: Single
	// rst:		.. enumerator:: Aromatic
	// rst:		.. enumerator:: Double
	// rst:		.. enumerator:: Triple
	Invalid, Single, Aromatic, Double, Triple
};
// rst:	.. function:: std::ostream &operator<<(std::ostream &s, BondType bt)
// rst:
// rst:		Format the bond type adhering to the string encoding of bonds (see :ref:`mol-enc`).
// rst:
// rst:		:throws: :class:`LogicError` if the bond type is :cpp:any:`BondType::Invalid`.
MOD_DECL std::ostream &operator<<(std::ostream &s, BondType bt);

//------------------------------------------------------------------------------
// AtomIds
//------------------------------------------------------------------------------

// BOOST_PP_SEQ(BOOST_PP_TUPLE(atomId, symbol, name))
#define MOD_CHEM_ATOM_DATA()     \
      ((1, H, Hydrogen))         \
      ((2, He, Helium))          \
      ((3, Li, Lithium))         \
      ((4, Be, Beryllium))       \
      ((5, B, Boron))            \
      ((6, C, Carbon))           \
      ((7, N, Nitrogen))         \
      ((8, O, Oxygen))           \
      ((9, F, Fluorine))         \
      ((10, Ne, Neon))           \
      ((11, Na, Sodium))         \
      ((12, Mg, Magnesium))      \
      ((13, Al, Aluminium))      \
      ((14, Si, Silicon))        \
      ((15, P, Phosphorus))      \
      ((16, S, Sulfur))          \
      ((17, Cl, Chlorine))       \
      ((18, Ar, Argon))          \
      ((19, K, Potassium))       \
      ((20, Ca, Calcium))        \
      ((21, Sc, Scandium))       \
      ((22, Ti, Titanium))       \
      ((23, V, Vanadium))        \
      ((24, Cr, Chromium))       \
      ((25, Mn, Manganese))      \
      ((26, Fe, Iron))           \
      ((27, Co, Cobalt))         \
      ((28, Ni, Nickel))         \
      ((29, Cu, Copper))         \
      ((30, Zn, Zinc))           \
      ((31, Ga, Gallium))        \
      ((32, Ge, Germanium))      \
      ((33, As, Arsenic))        \
      ((34, Se, Selenium))       \
      ((35, Br, Bromine))        \
      ((36, Kr, Krypton))        \
      ((37, Rb, Rubidium))       \
      ((38, Sr, Strontium))      \
      ((39, Y, Yttrium))         \
      ((40, Zr, Zirconium))      \
      ((41, Nb, Niobium))        \
      ((42, Mo, Molybdenum))     \
      ((43, Tc, Technetium))     \
      ((44, Ru, Ruthenium))      \
      ((45, Rh, Rhodium))        \
      ((46, Pd, Palladium))      \
      ((47, Ag, Silver))         \
      ((48, Cd, Cadmium))        \
      ((49, In, Indium))         \
      ((50, Sn, Tin))            \
      ((51, Sb, Antimony))       \
      ((52, Te, Tellurium))      \
      ((53, I, Iodine))          \
      ((54, Xe, Xenon))          \
      ((55, Cs, Caesium))        \
      ((56, Ba, Barium))         \
      ((57, La, Lanthanum))      \
      ((58, Ce, Cerium))         \
      ((59, Pr, Praseodymium))   \
      ((60, Nd, Neodymium))      \
      ((61, Pm, Promethium))     \
      ((62, Sm, Samarium))       \
      ((63, Eu, Europium))       \
      ((64, Gd, Gadolinium))     \
      ((65, Tb, Terbium))        \
      ((66, Dy, Dysprosium))     \
      ((67, Ho, Holmium))        \
      ((68, Er, Erbium))         \
      ((69, Tm, Thulium))        \
      ((70, Yb, Ytterbium))      \
      ((71, Lu, Lutetium))       \
      ((72, Hf, Hafnium))        \
      ((73, Ta, Tantalum))       \
      ((74, W, Tungsten))        \
      ((75, Re, Rhenium))        \
      ((76, Os, Osmium))         \
      ((77, Ir, Iridium))        \
      ((78, Pt, Platinum))       \
      ((79, Au, Gold))           \
      ((80, Hg, Mercury))        \
      ((81, Tl, Thallium))       \
      ((82, Pb, Lead))           \
      ((83, Bi, Bismuth))        \
      ((84, Po, Polonium))       \
      ((85, At, Astatine))       \
      ((86, Rn, Radon))          \
      ((87, Fr, Francium))       \
      ((88, Ra, Radium))         \
      ((89, Ac, Actinium))       \
      ((90, Th, Thorium))        \
      ((91, Pa, Protactinium))   \
      ((92, U, Uranium))         \
      ((93, Np, Neptunium))      \
      ((94, Pu, Plutonium))      \
      ((95, Am, Americium))      \
      ((96, Cm, Curium))         \
      ((97, Bk, Berkelium))      \
      ((98, Cf, Californium))    \
      ((99, Es, Einsteinium))    \
      ((100, Fm, Fermium))       \
      ((101, Md, Mendelevium))   \
      ((102, No, Nobelium))      \
      ((103, Lr, Lawrencium))    \
      ((104, Rf, Rutherfordium)) \
      ((105, Db, Dubnium))       \
      ((106, Sg, Seaborgium))    \
      ((107, Bh, Bohrium))       \
      ((108, Hs, Hassium))       \
      ((109, Mt, Meitnerium))    \
      ((110, Ds, Darmstadtium))  \
      ((111, Rg, Roentgenium))   \
      ((112, Cn, Copernicium))   \
      ((113, Uut, Ununtrium))    \
      ((114, Fl, Flerovium))     \
      ((115, Uup, Ununpentium))  \
      ((116, Lv, Livermorium))   \
      ((117, Uus, Ununseptium))  \
      ((118, Uuo, Ununoctium))
#define MOD_CHEM_ATOM_DATA_ELEM_SIZE() 3

// rst:
// rst: Namespace ``AtomIds``
// rst: ----------------------
// rst:
// rst: This namespace contains constants for each chemical element, both as their abbreviations and their full names.
// rst: Two additional constants are provided for convenience.
// rst:
// rst: .. var:: AtomId AtomIds::Invalid
// rst:
// rst:		Constant for the invalid atom id.
// rst:
// rst: .. var:: AtomId AtomIds::Max
// rst:
// rst:		An atom id equal to the atom id with the maximum valid id.
// rst:

//------------------------------------------------------------------------------
// Implementation Details
//------------------------------------------------------------------------------

// AtomId
//------------------------------------------------------------------------------

inline constexpr AtomId::AtomId() : id(0) {}

inline constexpr AtomId::AtomId(unsigned char id) : id(id) {}

inline constexpr AtomId::operator unsigned char() const {
	return id;
}

// AtomIds
//------------------------------------------------------------------------------

namespace AtomIds {
constexpr AtomId Invalid;
#define MDO_CHEM_atomIdIter(r, data, t)                                         \
   constexpr AtomId BOOST_PP_TUPLE_ELEM(3, 2, t)(BOOST_PP_TUPLE_ELEM(MOD_CHEM_ATOM_DATA_ELEM_SIZE(), 0, t));
BOOST_PP_SEQ_FOR_EACH(MDO_CHEM_atomIdIter, ~, MOD_CHEM_ATOM_DATA())
#undef MDO_CHEM_atomIdIter
#define MDO_CHEM_atomIdIter(r, data, t)                                         \
   constexpr AtomId BOOST_PP_TUPLE_ELEM(3, 1, t)(BOOST_PP_TUPLE_ELEM(MOD_CHEM_ATOM_DATA_ELEM_SIZE(), 0, t));
BOOST_PP_SEQ_FOR_EACH(MDO_CHEM_atomIdIter, ~, MOD_CHEM_ATOM_DATA())
#undef MDO_CHEM_atomIdIter
constexpr AtomId Max(BOOST_PP_SEQ_SIZE(MOD_CHEM_ATOM_DATA()));
static_assert(Max == 118, "Atom ids should be contiguous.");
} // namespace AtomIds

// Charge
//------------------------------------------------------------------------------

inline constexpr Charge::Charge() : c(0) {}

inline constexpr Charge::Charge(signed char c) : c(c) {}

inline constexpr Charge::operator signed char() const {
	return c;
}

// Isotope
//------------------------------------------------------------------------------

inline constexpr Isotope::Isotope() : i(-1) {}

inline constexpr Isotope::Isotope(int i) : i(i) {}

inline constexpr Isotope::operator int() const {
	return i;
}

// AtomData
//------------------------------------------------------------------------------

inline constexpr AtomData::AtomData(AtomId atomId) : atomId(atomId) {}

inline constexpr AtomData::AtomData(AtomId atomId, Charge charge, bool radical)
		: atomId(atomId), charge(charge), radical(radical) {}

inline constexpr AtomData::AtomData(AtomId atomId, Isotope isotope, Charge charge, bool radical)
		: atomId(atomId), isotope(isotope), charge(charge), radical(radical) {}

inline constexpr AtomId AtomData::getAtomId() const {
	return atomId;
}

inline constexpr Isotope AtomData::getIsotope() const {
	return isotope;
}

inline constexpr Charge AtomData::getCharge() const {
	return charge;
}

inline constexpr bool AtomData::getRadical() const {
	return radical;
}

inline constexpr bool operator==(const AtomData &a1, const AtomData &a2) {
	return a1.getAtomId() == a2.getAtomId()
	       && a1.getIsotope() == a2.getIsotope()
	       && a1.getCharge() == a2.getCharge()
	       && a1.getRadical() == a2.getRadical();
}

inline constexpr bool operator!=(const AtomData &a1, const AtomData &a2) {
	return !(a1 == a2);
}

inline constexpr bool operator<(const AtomData &a1, const AtomData &a2) {
	return std::make_tuple(a1.getAtomId(), a1.getIsotope(), a1.getCharge(), a1.getRadical())
	       < std::make_tuple(a2.getAtomId(), a2.getIsotope(), a2.getCharge(), a2.getRadical());
}

} // namespace mod

#endif // MOD_CHEM_HPP