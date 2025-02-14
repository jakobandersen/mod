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
	py::class_<CompositionMatch>("RCMatch", py::no_init)
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
					// rst:
					// rst:			:param bool verbose: whether to output debug messages from the composition algorithm.
					// rst:			:returns: the composition of the two rules along the match.
					// rst:				If the composition is not defined, ``None`` is returned.
					// rst:			:rtype: Rule
			.def("compose", &CompositionMatch::compose, py::args("verbose") = false)
					// rst:		.. method:: composeAll(*, maximum=False, verbose=False)
					// rst:
					// rst:			:param bool maximum: whether to only compose using the matches of maximum cardinality.
					// rst:			:param bool verbose: whether to output debug messages from the composition algorithm.
					// rst:			:returns: the composition of the two rules along enumerated matches.
					// rst:				The enumerated matches are all those that are extensions of the current match,
					// rst:				and the match it self.
					// rst:			:rtype: list[Rule]
			.def("composeAll", &CompositionMatch::composeAll,
			     (py::args("maximum") = false, py::args("verbose") = false));
}

} // namespace mod::rule::Py