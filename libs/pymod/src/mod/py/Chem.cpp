#include <mod/py/Common.hpp>

#include <mod/Chem.hpp>

#include <ostream>

namespace mod::Py {
namespace {

std::string bondTypeToString(mod::BondType b) {
	std::stringstream ss;
	ss << b;
	return ss.str();
}

} // namespace

void Chem_doExport() {
	// rst: .. class:: AtomId
	// rst:
	// rst:		Representation of the chemical element of an atom.
	// rst:
	py::class_<mod::AtomId>("AtomId", py::no_init)
			// rst:		.. method:: __init__(id=AtomIds.Invalid)
			// rst:
			// rst:			Construct an atom ID from an integer.
			// rst:
			// rst:			:param int id: the atomic number to construct from, or :const:`AtomIds.Invalid` to explicitly signal an invalid atom.
			// rst:				Must be at most the same value as :const:`AtomIds.Max`.
			.def(py::init<>())
			.def(py::init<unsigned char>())
					// rst:		.. attribute:: symbol
					// rst:
					// rst:		    (Read-only) The symbol represented by the atom id.
					// rst:
					// rst:			:type: str
					// rst:			:raises: :class:`LogicError` if the id is invalid.
			.add_property("symbol", &mod::AtomId::symbol)
					// rst: 	.. method:: __int__()
					// rst:
					// rst: 		Implicit conversion to an integer type.
			.def(int_(py::self))
			.def(str(py::self))
			.def(py::self == py::self);

	// rst: .. class:: Isotope
	// rst:
	// rst:		Representation of the isotope of an atom.
	// rst:
	py::class_<mod::Isotope>("Isotope", py::no_init)
			// rst:		.. method:: __init__()
			// rst:		            __init__(i)
			// rst:
			// rst:			Construct a representation of an isotope.
			// rst:			If an isotope number is given, that specific one is constructed,
			// rst:			otherwise the most abundant one is implicitly constructed.
			// rst:		
			// rst:			.. note:: There is a difference between constructing the most abundant isotope implicitly and explicitly.
			// rst:
			// rst:			:param int i: An isotope number for explicit construction. Passing -1 is equivalent to default-construction.
			// rst:				The number must be either -1 or at least 1.
			.def(py::init<>())
			.def(py::init<int>())
					// rst:		.. method:: __int__()
					// rst:
					// rst:			Implicit conversion to an integer type.
			.def(int_(py::self))
			.def(str(py::self))
			.def(py::self == py::self)
			.def(py::self == int());

	// rst: .. class:: Charge
	// rst:
	// rst:		Representation of the charge of an atom.
	// rst:
	py::class_<mod::Charge>("Charge", py::no_init)
					// rst:		.. method:: __init__(c=0)
					// rst:
					// rst:			Construct a charge.
					// rst:
					// rst:			:param int c: The charge to construct. Must be in the range :math:`[-9, 9]`.
			.def(py::init<signed char>(py::args("c") = 0))
					// rst:		.. method:: __int__()
					// rst:
					// rst:			Implicit conversion to an integer type.
			.def(int_(py::self))
			.def(str(py::self))
			.def(py::self == py::self)
			.def(py::self == int());

	// rst: .. class:: AtomData
	// rst:
	// rst:		Representation of basic data of an atom.
	// rst:
	py::class_<mod::AtomData>("AtomData", py::no_init)
			// rst:		.. method:: __init__(atomId=AtomIds.Invalid, isotope=Isotope(), charge=Charge(), radical=False)
			// rst:
			// rst:			Construct an atom data object.
			// rst:
			.def(py::init<AtomId, Isotope, Charge, bool>(
					(py::args("atomId") = AtomIds::Invalid, py::args("isotope") = Isotope(),
					 py::args("charge") = Charge(), py::args("radical") = false)))
					// rst:		.. attribute:: atomId
					// rst:
					// rst:			(Read-only) The atom id.
					// rst:
					// rst:			:type: AtomId
			.add_property("atomId", &mod::AtomData::getAtomId)
					// rst:		.. attribute:: isotope
					// rst:
					// rst:			(Read-only) The isotope.
					// rst:
					// rst:			:type: Isotope
			.add_property("isotope", &mod::AtomData::getIsotope)
					// rst:		.. attribute:: charge
					// rst:
					// rst:			(Read-only) The charge.
					// rst:
					// rst:			:type: Charge
			.add_property("charge", &mod::AtomData::getCharge)
					// rst:		.. :attribute:: radical
					// rst:
					// rst:			(Read-only) The radical.
					// rst:
					// rst:			:type: bool
			.add_property("radical", &mod::AtomData::getRadical)
					// rst:		.. :method:: __str__()
					// rst:
					// rst:			:returns: A string representation of the atom data adhering to the string encoding of atoms (see :ref:`mol-enc`).
					// rst:			:raises: ::class:`LogicError` if the atom id is :const:`AtomIds.Invalid`.
			.def(str(py::self))
			.def(py::self == py::self)
			.def(py::self < py::self);

	// rst: .. class:: BondType
	// rst:
	// rst:		Representation of a bond type.
	// rst:
	py::enum_<mod::BondType>("BondType")
			// rst:		.. attribute:: Invalid
			.value("Invalid", mod::BondType::Invalid)
					// rst:		.. attribute:: Single
			.value("Single", mod::BondType::Single)
					// rst:		.. attribute:: Aromatic
			.value("Aromatic", mod::BondType::Aromatic)
					// rst:		.. attribute:: Double
			.value("Double", mod::BondType::Double)
					// rst:		.. attribute:: Triple
			.value("Triple", mod::BondType::Triple)
		// rst:		.. method:: __str__()
		// rst:
		// rst:			:returns: A string representation of the bond type adhering to the string encoding of bonds (see :ref:`mol-enc`).
		// rst:			:raises: :class:`LogicError` if the bond type is :const:`Invalid`.
			;
	// TODO: py::enum_ does not support overriding of methods, so we set __str__ on the Python side.
	py::def("_bondTypeToString", &bondTypeToString);


	// rst: .. class:: AtomIds
	// rst:
	// rst:		This class contains constants for each chemical element, both as their abbreviations and their full names.
	// rst:		Two additional constants are provided for convenience.
	// rst:
	// rst:		.. attribute:: Invalid
	// rst:
	// rst:			Constant for the invalid atom id.
	// rst:
	// rst:			:type: AtomId
	// rst:
	// rst:		.. attribute:: Max
	// rst:
	// rst:			An atom id equal to the atom id with the maximum valid id.
	// rst:
	// rst:			:type: AtomId
	// rst:

	struct Dummy {
	};
	auto atomIds = py::class_<Dummy, boost::noncopyable>("AtomIds", py::no_init);
	atomIds.def_readonly("Invalid", &mod::AtomIds::Invalid);
	atomIds.def_readonly("Max", &mod::AtomIds::Max);
#define MOD_toString(s) MOD_toString1(s)
#define MOD_toString1(s) #s
#define MDO_CHEM_atomIdIter(r, data, t)                                         \
   atomIds.def_readonly(MOD_toString(BOOST_PP_TUPLE_ELEM(3, 1, t)), &mod::AtomIds::BOOST_PP_TUPLE_ELEM(3, 1, t));
	BOOST_PP_SEQ_FOR_EACH(MDO_CHEM_atomIdIter, ~, MOD_CHEM_ATOM_DATA())
#undef MDO_CHEM_atomIdIter
#define MDO_CHEM_atomIdIter(r, data, t)                                         \
   atomIds.def_readonly(MOD_toString(BOOST_PP_TUPLE_ELEM(3, 2, t)), &mod::AtomIds::BOOST_PP_TUPLE_ELEM(3, 2, t));
	BOOST_PP_SEQ_FOR_EACH(MDO_CHEM_atomIdIter, ~, MOD_CHEM_ATOM_DATA())
#undef MDO_CHEM_atomIdIter
}

} // namespace mod::Py