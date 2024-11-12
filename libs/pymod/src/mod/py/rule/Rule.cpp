#include <mod/py/Common.hpp>

#include <mod/graph/Printer.hpp>
#include <mod/rule/GraphInterface.hpp>
#include <mod/rule/Rule.hpp>

namespace mod::rule::Py {
namespace {

py::object getLabelType(std::shared_ptr<Rule> r) {
	if(r->getLabelType()) return py::object(*r->getLabelType());
	else return py::object();
}

} // namespace

void Rule_doExport() {
	std::pair<std::string, std::string>(Rule::*
	printWithoutOptions)(bool) const = &Rule::print;
	std::pair<std::string, std::string>(Rule::*
	printWithOptions)(const graph::Printer&, const graph::Printer&, bool) const = &Rule::print;

	// rst: .. class:: Rule
	// rst:
	// rst:		This class models a graph transformation rule in the Double Pushout formalism,
	// rst:		as the span :math:`L \leftarrow K \rightarrow R`.
	// rst:		The three graphs are referred to as respectively
	// rst:		the "left", "context", and "right" graphs of the rule.
	// rst:		See :ref:`graph-model` for more details.
	// rst:
	// rst:		The class implements the :class:`protocols.Graph` protocol,
	// rst:		which gives access to a graph view of the rule which has the left, context, and right graphs
	// rst:		combined into a single graph, called the core graph.
	// rst:		In addition to the combined graph view that this claas offers,
	// rst:		there are also three graph views representing the :attr:`left`, :attr:`context`
	// rst:		See :ref:`py-Rule/GraphInterface` for details of how to use these four graph interfaces.
	// rst:
	py::class_<Rule, std::shared_ptr<Rule>, boost::noncopyable>("Rule", py::no_init)
			// rst:		.. attribute:: id
			// rst:
			// rst:			(Read-only) The unique instance id among all :class:`Rule` instances.
			// rst:
			// rst:			:type: int
			.add_property("id", &Rule::getId)
			.def(str(py::self))
					//------------------------------------------------------------------
					// Graph
			.add_property("numVertices", &Rule::numVertices)
			.add_property("vertices", &Rule::vertices)
			.add_property("numEdges", &Rule::numEdges)
			.add_property("edges", &Rule::edges)
					// rst:		.. attribute:: left
					// rst:
					// rst:			(Read-only) A proxy object representing the left graph of the rule.
					// rst:
					// rst:			:type: LeftGraph
			.add_property("left", &Rule::getLeft)
					// rst:		.. attribute:: context
					// rst:
					// rst:			(Read-only) A proxy object representing the context graph of the rule.
					// rst:
					// rst:			:type: ContextGraph
			.add_property("context", &Rule::getContext)
					// rst:		.. attribute:: right
					// rst:
					// rst:			(Read-only) A proxy object representing the right graph of the rule.
					// rst:
					// rst:			:type: RightGraph
			.add_property("right", &Rule::getRight)
					//------------------------------------------------------------------
					// rst:		.. method:: makeInverse()
					// rst:
					// rst:			:returns: a rule representing the inversion of this rule.
					// rst:			:rtype: Rule
					// rst:			:raises: :class:`LogicError` if inversion is not possible (due to matching constraints).
			.def("makeInverse", &Rule::makeInverse)
					// rst:		.. method:: print(printCombined=False)
					// rst:		            print(first, second=None, printCombined=False)
					// rst:
					// rst:			Print the rule, using either the default options or the options in ``first`` and ``second``.
					// rst:			If ``first`` and ``second`` are the same, only one depiction will be made.
					// rst:
					// rst:			:param GraphPrinter first: the printing options used for the first depiction.
					// rst:			:param GraphPrinter second: the printing options used for the second depiction.
					// rst:				If it is ``None`` then it is set to ``first``.
					// rst:			:param bool printCombined: whether a depiction of the rule as a single combined graph is printed.
					// rst:			:returns: a pair of filename prefixes for the PDF-files that will be compiled in post-processing.
					// rst:				The actual names can be obtained by appending ``_L.pdf``, ``_K.pdf``, and ``_R.pdf`` for
					// rst:				respectively the left side, context, and right side graphs.
					// rst:				If ``first`` and ``second`` are the same, the two file prefixes are equal.
					// rst:			:rtype: tuple[str, str]
			.def("print", printWithoutOptions)
			.def("print", printWithOptions)
					// rst:		.. method:: printTermState
					// rst:
					// rst:			Print the term state for the rule.
			.def("printTermState", &Rule::printTermState)
					// rst:		.. method:: getGMLString(withCoords=False)
					// rst:
					// rst:			:returns: the :ref:`GML <rule-gml>` representation of the rule,
					// rst:			          optionally with generated 2D coordinates.
					// rst:			:rtype: str
					// rst:			:raises: :class:`LogicError` when coordinates are requested, but
					// rst:			         none can be generated.
			.def("getGMLString", &Rule::getGMLString)
					// rst:		.. method:: printGML(withCoords=False)
					// rst:
					// rst:			Print the rule in :ref:`GML <rule-gml>` format,
					// rst:			optionally with generated 2D coordinates.
					// rst:
					// rst:			:returns: the filename of the printed GML file.
					// rst:			:rtype: str
					// rst:			:raises: :class:`LogicError` when coordinates are requested, but
					// rst:			         none can be generated.
			.def("printGML", &Rule::printGML)
					// rst:		.. attribute:: name
					// rst:
					// rst:			The name of the rule. The default name includes the unique instance id.
					// rst:
					// rst:			:type: str
			.add_property("name", py::make_function(&Rule::getName,
			                                        py::return_value_policy<py::copy_const_reference>()), &Rule::setName)
					// rst:		.. attribute:: labelType
					// rst:
					// rst:			(Read-only) The intended label type for this rule, or None if no specific label type is intended.
					// rst:
					// rst:			:type: LabelType
			.add_property("labelType", &getLabelType)
					// rst:		.. attribute:: numLeftComponents
					// rst:
					// rst:			(Read-only) The number of connected components of the left graph.
					// rst:
					// rst:			:type: :cpp:type:`std::size_t`
			.add_property("numLeftComponents", &Rule::getNumLeftComponents)
					// rst:		.. attribute:: numRightComponents
					// rst:
					// rst:			(Read-only) The number of connected components of the right graph.
					// rst:
					// rst:			:type: :cpp:type:`std::size_t`
			.add_property("numRightComponents", &Rule::getNumRightComponents)
					// rst:		.. method:: isomorphism(other, maxNumMatches=1, labelSettings=LabelSettings(LabelType.String, LabelRelation.Isomorphism))
					// rst:
					// rst:			:param Rule other: the other :class:`Rule` for comparison.
					// rst:			:param int maxNumMatches: the maximum number of isomorphisms to search for.
					// rst:			:param LabelSettings labelSettings: the label settings to use during the search.
					// rst:			:returns: the number of isomorphisms found between ``other`` and this rule, but at most ``maxNumMatches``.
					// rst:			:rtype: int
			.def("isomorphism", &Rule::isomorphism)
					// rst:		.. method:: monomorphism(host, maxNumMatches=1, labelSettings=LabelSettings(LabelType.String, LabelRelation.Isomorphism))
					// rst:
					// rst:			:param Rule host: the host :class:`Rule` to check for subgraphs.
					// rst:			:param int maxNumMatches: the maximum number of morphisms to search for.
					// rst:			:param LabelSettings labelSettings: the label settings to use during the search.
					// rst:			:returns: the number of monomorphisms from this rule to subgraphs of ``host``, though at most ``maxNumMatches``.
					// rst:			:rtype: int
			.def("monomorphism", &Rule::monomorphism)
					// rst:		.. method:: isomorphicLeftRight(other, labelSettings=LabelSettings(LabelType.String, LabelRelation.Isomorphism))
					// rst:
					// rst:			:param Rule other: the other :class:`Rule` for comparison.
					// rst:			:param LabelSettings labelSettings: the label settings to use during the search.
					// rst:			:returns: whether the left-hand sides are isomorphic and the right-hand sides are isomorphic.
					// rst:			:rtype: bool
			.def("isomorphicLeftRight", &Rule::isomorphicLeftRight,
			     (py::arg("labelSettings") = LabelSettings(LabelType::String, LabelRelation::Isomorphism)))
					// rst:		.. method:: getVertexFromExternalId(id)
					// rst:
					// rst:			If the rule was not loaded from an external data format, then this function
					// rst:			always return a null descriptor.
					// rst:
					// rst:			.. note:: In general there is no correlation between external and internal ids.
					// rst:
					// rst:			:param int id: the external id to find the vertex descriptor for.
					// rst:			:returns: the vertex descriptor for the given external id.
					// rst:		            The descriptor is null if the external id was not used.
					// rst:			:rtype: Vertex
			.def("getVertexFromExternalId", &Rule::getVertexFromExternalId)
					// rst:		.. attribute:: minExternalId
					// rst:		                  maxExternalId
					// rst:
					// rst:			If the rule was not loaded from an external data format, then these attributes
					// rst:			are always return 0. Otherwise, they are the minimum/maximum external id from which
					// rst:			non-null vertices can be obtained from :meth:`getVertexFromExternalId`.
					// rst:			If no such minimum and maximum exists, then they are 0.
					// rst:
					// rst:			:type: int
			.add_property("minExternalId", &Rule::getMinExternalId)
			.add_property("maxExternalId", &Rule::getMaxExternalId)
					// rst:
					// rst: Loading Functions
					// rst: =================
					// rst:
					// rst: .. staticmethod:: Rule.fromGMLString(s, *, invert=False, add=True, printStereoWarnings=True)
					// rst:                   Rule.fromGMLFile(f, *, invert=False, add=True, printStereoWarnings=True)
					// rst:
					// rst:		Load a rule from a :ref:`GML <rule-gml>` string or file, and maybe store it in a global list.
					// rst:		The name of the rule is the one specified in the GML string, though when ``invert=True``
					// rst:		the string ", inverse" is appended to the name.
					// rst:
					// rst:		.. note::
					// rst:
					// rst:			If the GML string/file specifies matching constraints it is currently not possible to invert the rule.
					// rst:			There is however a configuration option to ignore matching constraints when inverting rules.
					// rst:
					// rst:		:param str s: the GML string to load a rule from.
					// rst:		:param f: name of the GML file to be loaded.
					// rst:		:type f: str or CWDPath
					// rst:		:param bool invert: whether to invert the loaded rule.
					// rst:		:param bool add: whether to append the rule to :data:`inputRules` or not.
					// rst:		:param bool printStereoWarnings: whether to print warnings due to unhandled stereo information.
					// rst:		:returns: the rule in the GML string, possibly inverted.
					// rst:		:rtype: Rule
			.def("fromGMLString", &Rule::fromGMLString)
			.staticmethod("fromGMLString")
			.def("fromGMLFile", &Rule::fromGMLFile)
			.staticmethod("fromGMLFile")
					// rst: .. staticmethod:: Rule.fromDFS(s, *, invert=False, add=True)
					// rst:
					// rst:		Load a rule from a :ref:`RuleDFS <format-ruleDFS>` string.
					// rst:
					// rst:		:param str s: the :ref:`RuleDFS <format-ruleDFS>` string to parse.
					// rst:		:param bool invert: whether to invert the loaded rule.
					// rst:		:param str name: the name of the rule. If none is given the default name is used.
					// rst:		:param bool add: whether to append the rule to :data:`inputRules` or not.
					// rst:		:returns: the loaded rule.
					// rst:		:rtype: Rule
					// rst:		:raises: :class:`InputError` on bad input.
			.def("fromDFS", &Rule::fromDFS)
			.staticmethod("fromDFS");

	// rst: .. function:: ruleGMLString(...)
	// rst:
	// rst:		Alias of :py:meth:`Rule.fromGMLString`.
	// rst: .. method:: ruleGML(...)
	// rst:		
	// rst:		Alias of :py:meth:`Rule.fromGMLFile`.
	// rst:
	// rst:
	// rst: .. data:: inputRules
	// rst:
	// rst:		A list of rules to which explicitly loaded rules as default are appended.
	// rst:
	// rst:		:type: list[Rule]
	// rst:
}

} // namespace mod::rule::Py