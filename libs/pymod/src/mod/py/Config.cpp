#include <mod/py/Common.hpp>

#include <mod/Config.hpp>

#include <boost/mpl/vector.hpp>

namespace mod::Py {
namespace {

std::string LabelType_str(LabelType lt) {
	return boost::lexical_cast<std::string>(lt);
}

std::string LabelRelation_str(LabelRelation lr) {
	return boost::lexical_cast<std::string>(lr);
}

std::string IsomorphismPolicy_str(IsomorphismPolicy p) {
	return boost::lexical_cast<std::string>(p);
}

std::string SmilesClassPolicy_str(SmilesClassPolicy p) {
	return boost::lexical_cast<std::string>(p);
}

std::string Action_str(Action a) {
	return boost::lexical_cast<std::string>(a);
}

} // namespace

void Config_doExport() {
	// rst: .. class:: LabelType
	// rst:
	// rst:		Selector for which type of label to use in algorithms.
	// rst:
	py::enum_<LabelType>("LabelType")
			// rst:		.. attribute:: String
			// rst:
			// rst:			Vertices and edges are considered to be labelled with
			// rst:			character strings. If only first-order terms are present,
			// rst:			then strings are generated as a serialisation of the term.
			.value("String", LabelType::String)
					// rst:
					// rst:		.. attribute:: Term
					// rst:
					// rst:			Vertices and edges are considered to be labelled with
					// rst:			first-order terms. If only strings are present, then first-order
					// rst:			terms are generated by parsing the strings. This may result
					// rst:			in an :class:`TermParsingError` if a string can not be parsed.
			.value("Term", LabelType::Term);
	py::def("_LabelType__str__", &LabelType_str);

	// rst: .. class:: LabelRelation
	// rst:
	// rst:		Selector for which type of labelled morphism to use in an algorithm.
	// rst:		For strings they are all defined to be equivalent to string equality.
	// rst:
	py::enum_<LabelRelation>("LabelRelation")
			// rst:		.. attribute:: Isomorphism
			// rst:
			// rst:			Terms are considered isomorphic when their most general unifier
			// rst:			is a renaming.
			.value("Isomorphism", LabelRelation::Isomorphism)
					// rst:		.. attribute:: Specialisation
					// rst:
					// rst:			A term :math:`t_2` is more special than, or isomorphic to, a term :math:`t_1` if there is a substitution
					// rst:			which can be applied to :math: `t_1` to make the terms equal.
					// rst:			This relation means that the right-hand side of a comparison is the more specialised term.
			.value("Specialisation", LabelRelation::Specialisation)
					// rst:		.. attribute:: Unification
					// rst:
					// rst:			Terms unify if a most general unifier (MGU) exists. The found MGU
					// rst:			is used for substitution in some algorithms.
			.value("Unification", LabelRelation::Unification);
	py::def("_LabelRelation__str__", &LabelRelation_str);

	// rst: .. class:: LabelSettings
	// rst:
	// rst:		A group label settings commonly used together in algorithms.
	// rst:
	py::class_<LabelSettings>("LabelSettings", py::no_init)
			// rst:		.. method:: __init__(type, relation)
			// rst:		               __init__(type, relation, stereoRelation)
			// rst:		               __init__(type, relation, withStereo, stereoRelation)
			// rst:
			// rst:			Construct label settings that only uses at least the vertex and edge labels.
			// rst:			If ``stereoRelation`` is given but ``withStereo`` is not, then ``withStereo`` defaults to ``True``.
			// rst:
			// rst:			:param LabelType type: How to interpret labels.
			// rst:			:param LabelRelation relation: The relation that should hold in morphisms between two labels.
			// rst:			:param bool withStereo: A flag to specify if stereo information should be included.
			// rst:				Defaults to ``False``, unless ``stereoRelation`` is given, then ``True``.
			// rst:			:param LabelRelation stereoRelation: The relation that should hold in morphisms between stereo data.
			// rst:				Defaults to :class:`LabelRelation.Isomorphism`, but is only used when ``withStereo`` is ``True``.
			.def(py::init<LabelType, LabelRelation>())
			.def(py::init<LabelType, LabelRelation, LabelRelation>())
			.def(py::init<LabelType, LabelRelation, bool, LabelRelation>())
			.def(py::self == py::self)
			.def(py::self != py::self)
			.def(str(py::self))
					// rst:		.. attribute:: type
					// rst:
					// rst:			:type: LabelType
			.def_readwrite("type", &LabelSettings::type)
					// rst:		.. attribute:: relation
					// rst:
					// rst:			:type: LabelRelation
			.def_readwrite("relation", &LabelSettings::relation)
					// rst:		.. attribute:: withStereo
					// rst:
					// rst:			:type: bool
			.def_readwrite("withStereo", &LabelSettings::withStereo)
					// rst:		.. attribute:: stereoRelation
					// rst:
					// rst:			:type: LabelRelation
			.def_readwrite("stereoRelation", &LabelSettings::stereoRelation);

	// rst: .. class:: IsomorphismPolicy
	// rst:
	// rst:		For some functions there is a choice of how to handle given arguments
	// rst:		where two different objects may be isomorphic. Most notably the case is with graphs (:class:`Graph`).
	// rst:
	py::enum_<IsomorphismPolicy>("IsomorphismPolicy")
			// rst:		.. attribute:: Check
			// rst:
			// rst:			Objects are checked for isomorphism as needed and exceptions are thrown when different objects are isomorphic.
			// rst:			If in doubt, use this.
			.value("Check", IsomorphismPolicy::Check)
					// rst:		.. attribute:: TrustMe
					// rst:
					// rst:			No checks are performed and the function trusts the caller to have performed the equivalent isomorphism checks.
					// rst:			Only use this when you are completely sure that no exceptions would have been thrown if using :attr:`Check`.
					// rst:
					// rst:			.. warning:: Generally the library has undefined behaviour if you use this option
					// rst:				but an exception would have been thrown with :attr:`Check`.
			.value("TrustMe", IsomorphismPolicy::TrustMe);
	py::def("_IsomorphismPolicy__str__", &IsomorphismPolicy_str);

	// rst: .. class:: SmilesClassPolicy
	// rst:
	// rst:		When loading SMILES strings, the class labels can be recorded and mapped into the corresponding
	// rst:		vertices of the loaded graph. This policy dictates what should happen when the same class label
	// rst:		is written on multiple atoms.
	// rst:
	py::enum_<SmilesClassPolicy>("SmilesClassPolicy")
			// rst:		.. attribute:: AllOrNone
			// rst:
			// rst:			If a class label is duplicated, then no labels are mapped to vertices.
			.value("NoneOnDuplicate", SmilesClassPolicy::NoneOnDuplicate)
					// rst:		.. attribute:: ThrowOnDuplicate
					// rst:
					// rst:			If a class label is duplicated, throw a :class:`InputError`.
			.value("ThrowOnDuplicate", SmilesClassPolicy::ThrowOnDuplicate)
					// rst:		.. attribute:: MapUnique
					// rst:
					// rst:			Map all class labels that are unique to vertices.
			.value("MapUnique", SmilesClassPolicy::MapUnique);
	py::def("_SmilesClassPolicy__str__", &SmilesClassPolicy_str);

	// rst: .. class:: Action
	// rst:
	// rst:		Utility enum for deciding what to do in certain cases.
	// rst:
	py::enum_<Action>("Action")
			// rst:		.. attribute:: Error
			// rst:
			// rst:			Abort the function and produce an error message, e.g., through and exception.
			.value("Error", Action::Error)
					// rst:		.. attribute:: Warn
					// rst:
					// rst:			Write a warning, but otherwise do as if it was `Ignore`.
			.value("Warn", Action::Warn)
					// rst:		.. attribute:: Ignore
					// rst:
					// rst:			Ignore the case. The function taking the action as argument should describe what this means.
			.value("Ignore", Action::Ignore);
	py::def("_Action__str__", &Action_str);

	// rst: .. class:: MDLOptions
	// rst:
	// rst:		An aggregation of options for the various loading functions for MDL formats.
	// rst:		Generally each option is defaulted to follow the specification of the formats,
	// rst:		unless it is harmless to deviate (e.g., relaxed white-space parsing).
	// rst:
	py::class_<MDLOptions>("MDLOptions")
			// rst:		.. attribute:: addHydrogens = True
			// rst:
			// rst:			Use the MDL valence model to add hydrogens to atoms with default valence, or disable all hydrogen addition.
			// rst:
			// rst:			:type: bool
			.def_readwrite("addHydrogens", &MDLOptions::addHydrogens)
					// rst:		.. attribute:: allowAbstract = False
					// rst:
					// rst:			Allow non-standard atom symbols. The standard symbols are the element symbols and those specifying wildcard atoms.
					// rst:
					// rst:			:type: bool
			.def_readwrite("allowAbstract", &MDLOptions::allowAbstract)
					// rst:		.. attribute:: applyV2000AtomAliases = True
					// rst:
					// rst:			In MOL V2000 CTAB blocks, replace atom labels by their aliases.
					// rst:			After application, the atom is considered abstract without errors, and hydrogen addition is suppressed.
					// rst:
					// rst:			:type: bool
			.def_readwrite("applyV2000AtomAliases", &MDLOptions::applyV2000AtomAliases)
					// rst:		.. attribute:: Action onPatternIsotope = Action::Error
					// rst:		               Action onPatternCharge = Action::Error;
					// rst:		               Action onPatternRadical = Action::Error;
					// rst:
					// rst:			What to do when an atom with symbol ``*`` has an isotope, charge, or radical.
					// rst:			``Action.Ignore`` means assuming the isotope, charge, or radical was not there.
			.def_readwrite("onPatternIsotope", &MDLOptions::onPatternIsotope)
			.def_readwrite("onPatternCharge", &MDLOptions::onPatternCharge)
			.def_readwrite("onPatternRadical", &MDLOptions::onPatternRadical)
					// rst:		.. attribute:: onImplicitValenceOnAbstract = Action.Error
					// rst:
					// rst:			What to do when ``addHydrogens and allowAbstract`` and an abstract atom is encountered with implicit valence.
					// rst:			``Action.Ignore`` means adding no hydrogens.
					// rst:
					// rst:			:type: Action
			.def_readwrite("onImplicitValenceOnAbstract", &MDLOptions::onImplicitValenceOnAbstract)
					// rst:		.. attribute:: onV2000UnhandledProperty = Action.Warn
					// rst:
					// rst:			What to do when a property line in a V2000 MOL file is not recognized.
					// rst:			``Action.Ignore`` means simply ignoring that particular line.
					// rst:
					// rst:			:type: Action
			.def_readwrite("onV2000UnhandledProperty", &MDLOptions::onV2000UnhandledProperty)
					// rst:		.. attribute:: fullyIgnoreV2000UnhandledKnownProperty = False
					// rst:
					// rst:			Warnings are usually stored as "loading warnings", even when they are ignored as during parsing.
					// rst:			Setting this to ``True`` will act as if ``onV2000UnhandledProperty = Actions::Ignore`` and
					// rst:			skip the storage, but only for a pre-defined known subset of properties.
					// rst:
					// rst:			:type: bool
			.def_readwrite("fullyIgnoreV2000UnhandledKnownProperty",
			               &MDLOptions::fullyIgnoreV2000UnhandledKnownProperty)
					// rst:		.. attribute:: onV3000UnhandledAtomProperty = Action.Warn
					// rst:
					// rst:			What to do when a property in atom line in a V3000 MOL file is not recognized.
					// rst:			``Action.Ignore`` means simply ignoring that particular property.
					// rst:
					// rst:			:type: Action
			.def_readwrite("onV3000UnhandledAtomProperty", &MDLOptions::onV3000UnhandledAtomProperty)
					// rst:		.. attribute:: onV2000Charge4 = Action.Warn
					// rst:
					// rst:			What to do when an atom in a V2000 MOL file has the charge 4 (doublet radical).
					//	rst:			``Action.Ignore`` means assuming it was charge 0.
					// rst:
					// rst:			:type: Action
			.def_readwrite("onV2000Charge4", &MDLOptions::onV2000Charge4)
					// rst:		.. attribute:: onV2000AbstractISO = Action.Warn
					// rst:
					// rst:			What to do when an abstract atom in a V2000 MOL file has a non-default ISO or mass difference value.
					//	rst:			``Action.Ignore`` means assuming it had no ISO or mass difference value.
					// rst:
					// rst:			:type: Action
			.def_readwrite("onV2000AbstractISO", &MDLOptions::onV2000AbstractISO)
					// rst:		.. attribute:: onRAD1 = Action.Error
					// rst:		               onRAD3 = Action.Error
					// rst:		               onRAD4 = Action.Error
					// rst:		               onRAD5 = Action.Error
					// rst:		               onRAD6 = Action.Error
					// rst:
					// rst:			What to do when an atom has assigned the indicated radical state.
					// rst:			``Action.Ignore`` means pretending the atom has no radical state assigned.
					// rst:
					// rst:			:type: Action
			.def_readwrite("onRAD1", &MDLOptions::onRAD1)
			.def_readwrite("onRAD3", &MDLOptions::onRAD3)
			.def_readwrite("onRAD4", &MDLOptions::onRAD4)
			.def_readwrite("onRAD5", &MDLOptions::onRAD5)
			.def_readwrite("onRAD6", &MDLOptions::onRAD6)
					// rst:		.. attribute:: onUnsupportedQueryBondType = Action.Error
					// rst:
					// rst:			What to do when a bond type 5, 6, or 7 are encountered (constrained query bond types).
					// rst:			``Action.Ignore`` means assigning a term variable, as if the type was 8.
					// rst:
					// rst:			:type: Action
			.def_readwrite("onUnsupportedQueryBondType", &MDLOptions::onUnsupportedQueryBondType);


#define NSIter(rNS, dataNS, tNS)                                                \
         BOOST_PP_SEQ_FOR_EACH_I(SettingIter, ~,                                \
            BOOST_PP_TUPLE_ELEM(MOD_CONFIG_DATA_NS_SIZE(), 2, tNS))
#define SettingIter(rSetting, dataSetting, iSetting, tSetting)                  \
         (BOOST_PP_TUPLE_ELEM(MOD_CONFIG_DATA_SETTING_SIZE(), 0, tSetting))
#undef NSIter
#undef SettingIter

	auto configClass = py::class_<Config, boost::noncopyable>("Config", py::no_init);
	{ // Config scope
		py::scope configScope = configClass;
		py::enum_<Config::IsomorphismAlg>("IsomorphismAlg")
				.value("VF2", mod::Config::IsomorphismAlg::VF2)
				.value("Canon", mod::Config::IsomorphismAlg::Canon)
				.value("SmilesCanonVF2", mod::Config::IsomorphismAlg::SmilesCanonVF2);

#define NSIter(rNS, dataNS, tNS)                                                                        \
   py::class_<Config:: BOOST_PP_TUPLE_ELEM(MOD_CONFIG_DATA_NS_SIZE(), 0, tNS), boost::noncopyable>      \
      (MOD_toString(BOOST_PP_TUPLE_ELEM(MOD_CONFIG_DATA_NS_SIZE(), 0, tNS)), py::no_init)               \
         BOOST_PP_SEQ_FOR_EACH_I(SettingIter,                                                           \
            BOOST_PP_TUPLE_ELEM(MOD_CONFIG_DATA_NS_SIZE(), 0, tNS),                                     \
            BOOST_PP_TUPLE_ELEM(MOD_CONFIG_DATA_NS_SIZE(), 2, tNS))                                     \
      ;
#define SettingIter(rSetting, dataSetting, iSetting, tSetting)                                          \
      .def_readwrite(MOD_toString(BOOST_PP_TUPLE_ELEM(MOD_CONFIG_DATA_SETTING_SIZE(), 1, tSetting)),    \
         &Config::dataSetting::BOOST_PP_TUPLE_ELEM(MOD_CONFIG_DATA_SETTING_SIZE(), 1, tSetting))

		BOOST_PP_SEQ_FOR_EACH(NSIter, ~, MOD_CONFIG_DATA())

#undef NSIter
#undef SettingIter

		py::list classNames;
#define NSIter(rNS, dataNS, tNS)                                                \
      classNames.append<std::string>("Config" MOD_toString(BOOST_PP_TUPLE_ELEM(MOD_CONFIG_DATA_NS_SIZE(), 0, tNS)));
		BOOST_PP_SEQ_FOR_EACH(NSIter, ~, MOD_CONFIG_DATA())
#undef NSIter
		configScope.attr("classNames") =
				classNames;
	} // Config scope

#define NSIter(rNS, dataNS, tNS)                                                       \
   .def_readonly(MOD_toString(BOOST_PP_TUPLE_ELEM(MOD_CONFIG_DATA_NS_SIZE(), 1, tNS)), \
      &Config::BOOST_PP_TUPLE_ELEM(MOD_CONFIG_DATA_NS_SIZE(), 1, tNS))

	configClass
			BOOST_PP_SEQ_FOR_EACH(NSIter, ~, MOD_CONFIG_DATA());

#undef NSIter
#undef SettingIter

// rst: .. class:: Config
// rst:
// rst:		This class contains a range of inner classes of config settings. See :doc:`the libMØD documentation</libmod/Config>`.

// rst: .. data:: config
// rst:
// rst:		This variable is initialised to the return value of :func:`getConfig`, i.e., just use this instead of the method.
// rst:
// rst: .. method:: getConfig()
// rst:
// rst:		:returns: the singleton :cpp:class:`Config` instance used by the library.
	py::def("getConfig", &getConfig,
	        py::return_value_policy<py::reference_existing_object>()
	);
}

} // namespace mod::Py