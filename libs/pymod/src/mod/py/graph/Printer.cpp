#include <mod/py/Common.hpp>

#include <mod/graph/Printer.hpp>

#include <iomanip>

namespace mod::graph::Py {

void Printer_doExport() {
	// rst: .. class:: GraphPrinter
	// rst:
	// rst:		This class is used to configure how graphs are visualised.
	// rst:
	// rst:		.. warning:: Some of these options greatly alter how graphs are depicted
	// rst:			and the result may not accurately represent the underlying graph,
	// rst:			and may make non-molecules look like molecules.
	// rst:
	// rst:		.. method:: __init__()
	// rst:
	// rst:			The default constructor enables edges as bonds, raised charges, and raised isotopes.
	py::class_<Printer>("GraphPrinter")
			.def(py::self == py::self)
			.def(py::self != py::self)
					// rst:		.. method:: setMolDefault()
					// rst:
					// rst:			Shortcut for enabling all but thickening and index printing.
			.def("setMolDefault", &Printer::setMolDefault)
					// rst:		.. method:: setReactionDefault()
					// rst:
					// rst:			Shortcut for enabling all but thickening, index printing and simplification of carbon atoms.
			.def("setReactionDefault", &Printer::setReactionDefault)
					// rst:		.. method:: disableAll()
					// rst:
					// rst:			Disable all special printing features.
			.def("disableAll", &Printer::disableAll)
					// rst:		.. method:: enableAll()
					// rst:
					// rst:			Enable all special printing features, except typewriter font.
			.def("enableAll", &Printer::enableAll)
					// rst:		.. attribute:: edgesAsBonds
					// rst:
					// rst:			Control whether edges with special labels are drawn as chemical bonds.
					// rst:
					// rst:			:type: bool
			.add_property("edgesAsBonds", &Printer::getEdgesAsBonds, &Printer::setEdgesAsBonds)
					// rst:		.. attribute:: collapseHydrogens
					// rst:
					// rst:			Control whether vertices representing hydrogen atoms are collapsed into their neighbours labels.
					// rst:
					// rst:			:type: bool
			.add_property("collapseHydrogens", &Printer::getCollapseHydrogens, &Printer::setCollapseHydrogens)
					// rst:		.. attribute:: raiseIsotopes
					// rst:
					// rst:			Control whether a vertex label prefix encoding an isotope is written as a superscript to the rest of the label.
					// rst:
					// rst:			:type: bool
			.add_property("raiseIsotopes", &Printer::getRaiseIsotopes, &Printer::setRaiseIsotopes)
					// rst:		.. attribute:: raiseCharges
					// rst:
					// rst:			Control whether a vertex label suffix encoding a charge is written as a superscript to the rest of the label.
					// rst:
					// rst:			:type: bool
			.add_property("raiseCharges", &Printer::getRaiseCharges, &Printer::setRaiseCharges)
					// rst:		.. attribute:: simpleCarbons
					// rst:
					// rst:			Control whether some vertices encoding carbon atoms are depicted without any label.
					// rst:
					// rst:			:type: bool
			.add_property("simpleCarbons", &Printer::getSimpleCarbons, &Printer::setSimpleCarbons)
					// rst:		.. attribute:: thick
					// rst:
					// rst:			Control whether all edges are drawn thicker than normal and all labels are written in bold.
					// rst:
					// rst:			:type: bool
			.add_property("thick", &Printer::getThick, &Printer::setThick)
					// rst:		.. attribute:: withColour
					// rst:
					// rst:			Control whether colour is applied to certain elements of the graph which are molecule-like.
					// rst:
					// rst:			:type: bool
			.add_property("withColour", &Printer::getWithColour, &Printer::setWithColour)
					// rst:		.. attribute:: withIndex
					// rst:
					// rst:			Control whether the underlying indices of the vertices are printed.
					// rst:
					// rst:			:type: bool
			.add_property("withIndex", &Printer::getWithIndex, &Printer::setWithIndex)
					// rst:		.. attribute:: withTexttt
					// rst:
					// rst:			Control whether the vertex and edge labels are written with typewriter font.
					// rst:
					// rst:			:type: bool
			.add_property("withTexttt", &Printer::getWithTexttt, &Printer::setWithTexttt)
					// rst:		.. attribute:: withRawStereo
					// rst:
					// rst:			Control whether the vertices and edges are annotated with the raw stereo properties.
					// rst:
					// rst:			:type: bool
			.add_property("withRawStereo", &Printer::getWithRawStereo, &Printer::setWithRawStereo)
					// rst:		.. attribute:: withPrettyStereo
					// rst:
					// rst:			Control whether the vertices and edges are annotated with stylized stereo properties.
					// rst:
					// rst:			:type: bool
			.add_property("withPrettyStereo", &Printer::getWithPrettyStereo, &Printer::setWithPrettyStereo)
					// rst:		.. attribute:: rotation
					// rst:
					// rst:			Rotation of internally computed coordinates.
					// rst:
					// rst:			:type: int
			.add_property("rotation", &Printer::getRotation, &Printer::setRotation)
					// rst:		.. attribute:: mirror
					// rst:
					// rst:			Mirror internally computed coordinates in the y-axis.
					// rst:
					// rst:			:type: bool
			.add_property("mirror", &Printer::getMirror, &Printer::setMirror)
					// rst:		.. attribute:: withGraphvizCoords
					// rst:
					// rst:			Do not use Open Babel for coordinate generation, but only the Graphviz fallback
					// rst:			during post-processing.
					// rst:			When setting this to ``True`` consider setting `simpleCarbons = False`` to avoid
					// rst:			misleading depictions due to collinear carbon chains.
					// rst:
					// rst:			:type: bool
			.add_property("withGraphvizCoords", &Printer::getWithGraphvizCoords, &Printer::setWithGraphvizCoords)
					// ===================================================================
					// rst:		.. attribute:: graphvizPrefix
					// rst:
					// rst:			The string that will be inserted into generated DOT files,
					// rst:			just after the graph declaration.
					// rst:			DOT files are only generated when ``withGraphvizCoords == True``.
					// rst:
					// rst:			:type: str
			.add_property("graphvizPrefix",
			              py::make_function(&Printer::getGraphvizPrefix,
			                                py::return_value_policy<py::copy_const_reference>()),
			              &Printer::setGraphvizPrefix);
}

} // namespace mod::graph::Py