#ifndef MOD_GRAPH_PRINTER_HPP
#define MOD_GRAPH_PRINTER_HPP

#include <mod/BuildConfig.hpp>
#include <mod/graph/ForwardDecl.hpp>

#include <memory>

namespace mod::graph {

// rst-class: graph::Printer
// rst:
// rst: 	This class is used to configure how graphs are visualised.
// rst:
// rst: 	.. warning:: Some of these options greatly alter how graphs are depicted
// rst: 		and the result may not accurately represent the underlying graph,
// rst: 		and may make non-molecules look like molecules.
// rst:
// rst-class-start:
struct MOD_DECL Printer {
	// rst: .. function Printer()
	// rst:
	// rst:		The default constructor enables edges as bonds, raised charges, and raised isotopes.
	Printer();
	Printer(const Printer &that);
	Printer &operator=(const Printer &that);
	~Printer();
	const lib::IO::Graph::Write::Options &getOptions() const;
	// rst: .. function:: friend bool operator==(const Printer &a, const Printer &b)
	// rst:               friend bool operator!=(const Printer &a, const Printer &b)
	MOD_DECL friend bool operator==(const Printer &a, const Printer &b);
	MOD_DECL friend bool operator!=(const Printer &a, const Printer &b);
	// rst: .. function:: void setMolDefault()
	// rst:
	// rst:		Shortcut for enabling all but thickening and index printing.
	void setMolDefault();
	// rst: .. function:: void setReactionDefault()
	// rst:
	// rst:		Shortcut for enabling all but thickening, index printing and simplification of carbon atoms.
	void setReactionDefault();
	// rst: .. function:: void disableAll()
	// rst:
	// rst:		Disable all special printing features.
	void disableAll();
	// rst: .. function:: void enableAll()
	// rst:
	// rst:		Enable all special printing features, except typewriter font.
	void enableAll();
	// rst: .. function:: void setEdgesAsBonds(bool value)
	// rst:               bool getEdgesAsBonds() const
	// rst:
	// rst:		Control whether edges with special labels are drawn as chemical bonds.
	void setEdgesAsBonds(bool value);
	bool getEdgesAsBonds() const;
	// rst: .. function:: void setCollapseHydrogens(bool value)
	// rst:               bool getCollapseHydrogens() const
	// rst:
	// rst:		Control whether vertices representing hydrogen atoms are collapsed into their neighbours labels.
	void setCollapseHydrogens(bool value);
	bool getCollapseHydrogens() const;
	// rst: .. function:: void setRaiseIsotopes(bool value)
	// rst:               bool getRaiseIsotopes() const
	// rst:
	// rst:		Control whether a vertex label prefix encoding an isotope is written as a superscript to the rest of the label.
	void setRaiseIsotopes(bool value);
	bool getRaiseIsotopes() const;
	// rst: .. function:: void setRaiseCharges(bool value)
	// rst:               bool getRaiseCharges() const
	// rst:
	// rst:		Control whether a vertex label suffix encoding a charge is written as a superscript to the rest of the label.
	void setRaiseCharges(bool value);
	bool getRaiseCharges() const;
	// rst: .. function:: void setSimpleCarbons(bool value)
	// rst:               bool getSimpleCarbons() const
	// rst:
	// rst:		Control whether some vertices encoding carbon atoms are depicted without any label.
	void setSimpleCarbons(bool value);
	bool getSimpleCarbons() const;
	// rst: .. function:: void setThick(bool value)
	// rst:               bool getThick() const
	// rst:
	// rst:		Control whether all edges are drawn thicker than normal and all labels are written in bold.
	void setThick(bool value);
	bool getThick() const;
	// rst: .. function:: void setWithColour(bool value)
	// rst:               bool getWithColour() const
	// rst:
	// rst:		Control whether colour is applied to certain elements of the graph which are molecule-like.
	void setWithColour(bool value);
	bool getWithColour() const;
	// rst: .. function:: void setWithIndex(bool value)
	// rst:               bool getWithIndex() const
	// rst:
	// rst:		Control whether the underlying indices of the vertices are printed.
	void setWithIndex(bool value);
	bool getWithIndex() const;
	// rst: .. function:: void setWithTexttt(bool value)
	// rst:               bool getWithTexttt() const
	// rst:
	// rst:		Control whether the vertex and edge labels are written with typewriter font.
	void setWithTexttt(bool value);
	bool getWithTexttt() const;
	// rst: .. function:: void setWithRawStereo(bool value)
	// rst:               bool getWithRawStereo() const
	// rst:
	// rst:		Control whether the vertices and edges are annotated with the raw stereo properties.
	void setWithRawStereo(bool value);
	bool getWithRawStereo() const;
	// rst: .. function:: void setWithPrettyStereo(bool value)
	// rst:               bool getWithPrettyStereo() const
	// rst:
	// rst:		Control whether the vertices and edges are annotated with stylized stereo properties.
	void setWithPrettyStereo(bool value);
	bool getWithPrettyStereo() const;
	// rst: .. function:: void setRotation(int degrees)
	// rst:               int getRotation() const
	// rst:
	// rst:		Rotation of internally computed coordinates.
	void setRotation(int degrees);
	int getRotation() const;
	// rst: .. function:: void setMirror(bool value)
	// rst:               bool getMirror() const
	// rst:
	// rst:		Mirror internally computed coordinates in the y-axis.
	void setMirror(bool value);
	bool getMirror() const;
	// rst: .. function:: void setWithGraphvizCoords(bool value)
	// rst:               bool getWithGraphvizCoords() const
	// rst:
	// rst:		Do not use Open Babel for coordinate generation, but only the Graphviz fallback
	// rst:		during post-processing.
	// rst:		When setting this to `true` consider setting `setSimpleCarbons(false)` to avoid
	// rst:		misleading depictions due to collinear carbon chains.
	void setWithGraphvizCoords(bool value);
	bool getWithGraphvizCoords() const;
public:
	// rst: .. function:: void setGraphvizPrefix(const std::string &prefix)
	// rst:               const std::string &getGraphvizPrefix() const
	// rst:
	// rst:		Access the string that will be inserted into generated DOT files,
	// rst:		just after the graph declaration.
	// rst:		DOT files are only generated when `getWithGraphvizCoords()`.
	void setGraphvizPrefix(const std::string &prefix);
	const std::string &getGraphvizPrefix() const;
private:
	std::unique_ptr<lib::IO::Graph::Write::Options> options;
};
// rst-class-end:

} // namespace mod::graph

#endif // MOD_GRAPH_PRINTER_HPP