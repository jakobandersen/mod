#include <mod/py/Common.hpp>

#include <mod/rule/CompositionMatch.hpp>

namespace mod::rule::Py {

void CompositionMatch_doExport() {
	// rst: .. class:: RCMatch
	// rst:
	// rst:		A class for interactively constructing match between two rules used for composition.
	// rst:		That is, for two rules
	// rst:		:math:`p_1 = (L_1 \leftarrow K_1 \rightarrow R_1)` and
	// rst:		:math:`p_2 = (L_2 \leftarrow K_2 \rightarrow R_2)`,
	// rst:		an object represents match between :math:`R_1` and :math:`L_2` which indicates
	// rst:		a common subgraph :math:`R_1 \leftarrow M \rightarrow L_2`.
	// rst:
	// rst:		See also [AFMS-RC]_ for details of how composition of rules can be computed.
	// rst:
	auto matchClass = py::class_<CompositionMatch>("RCMatch", py::no_init)
	                  // rst:		.. method:: __init__(rFirst, rSecond, labelSettings=LabelSettings(LabelType.String, LabelRelation.Specialisation))
	                  // rst:
	                  // rst:			Construct an empty overlap.
	                  // rst:
	                  // rst:			:param Rule rFirst: the first rule :math:`p_1`.
	                  // rst:			:param Rule rSecond: the second rule :math:`p_2`.
	                  // rst:			:param LabelSettings labelSettings: the settings to use for the morphisms :math:`R_1 \leftarrow M \rightarrow L_2`.
	                  // rst:			:raises LogicError: if either rule is ``None``.
	                  .def(py::init<std::shared_ptr<Rule>, std::shared_ptr<Rule>, LabelSettings>(
	                  (py::args("rFirst"), py::args("rSecond"),
		                  py::args("labelSettings") = LabelSettings{LabelType::String, LabelRelation::Specialisation})))
	                  .def(str(py::self))
	                  // rst:		.. attribute:: first
	                  // rst:		               second
	                  // rst:
	                  // rst:			(Read-only) The rules :math:`p_1` and :math:`p_2`.
	                  // rst:
	                  // rst:			:type: Rule
	                  .def_readonly("first", &CompositionMatch::getFirst)
	                  .def_readonly("second", &CompositionMatch::getSecond)
	                  // rst:		.. attribute:: labelSettings
	                  // rst:
	                  // rst:			(Read-only) The settings for the morphisms in the match.
	                  // rst:
	                  // rst:			:type: LabelSettings
	                  .def_readonly("labelSettings", &CompositionMatch::getLabelSettings)
	                  // rst:		.. attribute:: size
	                  // rst:
	                  // rst:			(Read-only) The number of currently mapped pairs of vertices.
	                  // rst:
	                  // rst:			:type: int
	                  .def_readonly("size", &CompositionMatch::size)
	                  // rst:		.. method:: __getitem__(vFirst)
	                  // rst:		            __getitem__(vSecond)
	                  // rst:
	                  // rst:			:param Rule.RightGraph.Vertex vFirst: a vertex of :math:`R_1` to return the mapped vertex for.
	                  // rst:			:param Rule.LeftGraph.Vertex vSecond: a vertex of :math:`L_2` to return the mapped vertex for.
	                  // rst:			:returns: the vertex the given vertex is mapped to, or a null vertex if it is not mapped.
	                  // rst:			:raises LogicError: if either vertex is a null vertex.
	                  // rst:			:raises LogicError: if either vertex does not belong to their respective rules.
	                  .def("__getitem__",
	                       static_cast<Rule::LeftGraph::Vertex (CompositionMatch::*)(
			                       Rule::RightGraph::Vertex) const>(&CompositionMatch::operator[]))
	                  .def("__getitem__",
	                       static_cast<Rule::RightGraph::Vertex (CompositionMatch::*)(
			                       Rule::LeftGraph::Vertex) const>(&CompositionMatch::operator[]))
	                  // rst:		.. method:: push(vFirst, vSecond)
	                  // rst:
	                  // rst:			Extend the common subgraph (match) of :math:`R_1` and :math:`L_2`
	                  // rst:			by identifying the given vertices from each of the graphs.
	                  // rst:
	                  // rst:			:raises LogicError: if either vertex is a null vertex.
	                  // rst:			:raises LogicError: if either vertex does not belong to their respective rules.
	                  // rst:			:raises LogicError: if either vertex is already mapped.
	                  // rst:			:raises LogicError: if the extension of the match is infeasible.
	                  .def("push", &CompositionMatch::push)
	                  // rst:		.. method:: pop()
	                  // rst:
	                  // rst:			Pop the latest pushed vertex pair.
	                  // rst:
	                  // rst:			:raises LogicError: if ``self.size == 0``.
	                  .def("pop", &CompositionMatch::pop)
	                  // rst:		.. method:: compose(*, verbose=False)
	                  // rst:		            composeWithMaps(*, verbose=False)
	                  // rst:
	                  // rst:			:param bool verbose: whether to output debug messages from the composition algorithm.
	                  // rst:			:returns: the composition of the two rules along the match, either just the composed rule
	                  // rst: 			or the composed rule with maps relating its vertices and the vertices in the input.
	                  // rst:				If the composition is not defined, ``None`` is returned.
	                  // rst:			:rtype: Rule or Result
					  // rst:
					  // rst:			.. versionadded:: 1.1
					  // rst:				The ``composeWithMaps`` method.
			          // rst:
					  // rst:			.. todo:: check version added before release
	                  .def("compose", &CompositionMatch::compose, py::args("verbose") = false)
	                  .def("composeWithMaps", &CompositionMatch::composeWithMaps, py::args("verbose") = false)
	                  // rst:		.. method:: composeAll(*, maximum=False, verbose=False)
	                  // rst:		            composeAllWithMaps(*, maximum=False, verbose=False)
	                  // rst:
	                  // rst:			:param bool maximum: whether to only compose using the matches of maximum cardinality.
	                  // rst:			:param bool verbose: whether to output debug messages from the composition algorithm.
	                  // rst:			:returns: the composition of the two rules along enumerated matches, either just the composed rules
	                  // rst: 			or each of the composed rules with maps relating their vertices and the vertices in the input,
	                  // rst: 			including the full match used for each particular composition.
	                  // rst: 			The enumerated matches are all those that are extensions of the current match,
	                  // rst: 			and the match itself.
	                  // rst:			:rtype: list[Rule] or list[Result]
					  // rst:
					  // rst:			.. versionadded:: 1.1
					  // rst:				The ``composeAllWithMaps`` method.
					  // rst:
					  // rst:			.. todo:: check version added before release
	                  .def("composeAll", &CompositionMatch::composeAll,
	                       (py::args("maximum") = false, py::args("verbose") = false))
	                  .def("composeAllWithMaps", &CompositionMatch::composeAllWithMaps,
	                       (py::args("maximum") = false, py::args("verbose") = false));

	py::to_python_converter<std::optional<CompositionMatch::Result>, mod::Py::ToPythonOptionalValue<
		                        CompositionMatch::Result>>();

	py::scope matchClassScope(matchClass);
	// rst: .. class:: RCMatch.Result
	// rst:
	// rst:		Representation of the full result of a composition.
	// rst:
	py::class_<CompositionMatch::Result>("Result", py::no_init)
			// rst: 	.. attribute:: rule
			// rst:
			// rst: 		The composed rule.
			// rst:
			// rst: 		:type: Rule
			.def_readonly("rule", &CompositionMatch::Result::rule)
			// rst: 	.. attribute:: mFirstToSecond
			// rst:
			// rst: 		A map of vertices in :math:`p_1` to the vertices of :math:`p_2`, i.e., the match used for composition.
			// rst:
			// rst: 		:type: VertexMapRuleRule
			.def_readonly("mFirstToSecond", &CompositionMatch::Result::mFirstToSecond)
			// rst: 	.. attribute:: mFirstToRes
			// rst:
			// rst: 		A map of vertices in :math:`p_1` to the vertices of the composed rule.
			// rst:
			// rst: 		:type: VertexMapRuleRule
			.def_readonly("mFirstToRes", &CompositionMatch::Result::mFirstToRes)
			// rst: 	.. attribute:: mSecondToRes
			// rst:
			// rst: 		A map of vertices in :math:`p_2` to the vertices of the composed rule.
			// rst:
			// rst: 		:type: VertexMapRuleRule
			.def_readonly("mSecondToRes", &CompositionMatch::Result::mSecondToRes);
}

} // namespace mod::rule::Py