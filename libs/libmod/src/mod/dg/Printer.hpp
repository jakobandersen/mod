#ifndef MOD_DG_PRINTER_HPP
#define MOD_DG_PRINTER_HPP

#include <mod/BuildConfig.hpp>
#include <mod/dg/DG.hpp>

#include <functional>
#include <memory>

namespace mod::dg {

// rst-class: dg::PrintData
// rst:
// rst:		This class is used to hold extra data about how a specific derivation graph
// rst:		is visualised.
// rst:
// rst:		The idea is that in the visualized network each vertex/hyperedge is specified
// rst:		by a pair :math:`(id, dup)` where :math:`id` is the ID of the vertex/hyperedge
// rst:		and :math:`dup` is an versioning integer that can be specified in objects of this class.
// rst:
// rst:		Initially, each vertex/hyperedge has only one version: duplicate number 0.
// rst:		The duplication is primarily specified via the hyperedges, with the duplication
// rst:		of vertices being induced afterwards.
// rst:		Thus hyperedge duplicates are managed by :func:`makeDuplicate` and :func:`removeDuplicate`,
// rst:		while the vertex duplicates are managed implicitly by :func:`reconnectSource` and :func:`reconnectTarget`.
// rst:		In the end, when the data is used for printing, it will be compiled to form the actual duplication data.
// rst:
// rst-class-start:
struct MOD_DECL PrintData {
	// rst: .. function:: explicit PrintData(std::shared_ptr<DG> dg)
	// rst:
	// rst:		Construct a data object where all derivations have a single version, duplicate number 0,
	// rst:		connected to version 0 of all heads and tails.
	// rst:
	// rst:		:throws: :class:`LogicError` if `!dg->isLocked()`.
	explicit PrintData(std::shared_ptr<DG> dg);
	PrintData(const PrintData &other);
	PrintData &operator=(const PrintData &other);
	~PrintData();
	lib::DG::Write::Data &getData();
	const lib::DG::Write::Data &getData() const;
	// rst: .. function:: std::shared_ptr<DG> getDG() const
	// rst:
	// rst:		:returns: the derivation graph the object holds data for.
	std::shared_ptr<DG> getDG() const;
	// rst: .. function:: void makeDuplicate(DG::HyperEdge e, int eDup)
	// rst:
	// rst:		Create another version of the given hyperedge and give it the given duplicate number.
	// rst:		It will connect to duplicate 0 of all head and tail vertices.
	// rst:
	// rst:		:throws: :class:`LogicError` if `!e`.
	// rst:		:throws: :class:`LogicError` if `e.getDG() != getDG()`.
	// rst:		:throws: :class:`LogicError` if duplicate `eDup` already exists for `e`.
	void makeDuplicate(DG::HyperEdge e, int eDup);
	// rst: .. function:: void removeDuplicate(DG::HyperEdge e, int eDup)
	// rst:
	// rst:		Remove the version of the given hyperedge with the given duplicate number.
	// rst:
	// rst:		:throws: :class:`LogicError` if `!e`.
	// rst:		:throws: :class:`LogicError` if `e.getDG() != getDG()`.
	// rst:		:throws: :class:`LogicError` if duplicate `eDup` does not exist for `e`.
	void removeDuplicate(DG::HyperEdge e, int eDup);
	// rst: .. function:: void reconnectSource(DG::HyperEdge e, int eDup, DG::Vertex v, int vDupTar)
	// rst:
	// rst:		For the given hyperedge duplicate, reconnect the given source vertex to the given duplicate of that source.
	// rst:		If the vertex is a source multiple times, then an arbitrary one of them is reconnected.
	// rst:
	// rst:		:throws: :class:`LogicError` if `!e`.
	// rst:		:throws: :class:`LogicError` if `e.getDG() != getDG()`.
	// rst:		:throws: :class:`LogicError` if `!v`.
	// rst:		:throws: :class:`LogicError` if `v.getDG() != getDG()`.
	// rst:		:throws: :class:`LogicError` if `v` is not a source vertex of `e`.
	// rst:		:throws: :class:`LogicError` if duplicate `eDup` does not exist for `e`.
	void reconnectSource(DG::HyperEdge e, int eDup, DG::Vertex v, int vDupTar);
	// rst: .. function:: void reconnectTarget(DG::HyperEdge e, int eDup, DG::Vertex v, int vDupTar)
	// rst:
	// rst:		For the given hyperedge duplicate, reconnect the given head to the given duplicate of that head.
	// rst:		If the vertex is a head multiple times, then an arbitrary one of them is reconnected.
	// rst:
	// rst:		:throws: :class:`LogicError` if `!e`.
	// rst:		:throws: :class:`LogicError` if `e.getDG() != getDG()`.
	// rst:		:throws: :class:`LogicError` if `!v`.
	// rst:		:throws: :class:`LogicError` if `v.getDG() != getDG()`.
	// rst:		:throws: :class:`LogicError` if `v` is not a target vertex of `e`.
	// rst:		:throws: :class:`LogicError` if duplicate `eDup` does not exist for `e`.
	void reconnectTarget(DG::HyperEdge e, int eDup, DG::Vertex v, int vDupTar);
private:
	std::shared_ptr<DG> dg;
	std::unique_ptr<lib::DG::Write::Data> data;
};
// rst-class-end:

// rst-class: dg::Printer
// rst:
// rst:		This class is used to configure how derivation graphs are visualised,
// rst:		how much is visualised and which extra properties are printed.
// rst: 
// rst-class-start:
struct MOD_DECL Printer {
	Printer();
	Printer(const Printer &) = delete;
	Printer &operator=(const Printer &) = delete;
	~Printer();
	lib::DG::Write::Printer &getPrinter() const;
	// rst: .. function:: graph::Printer &getGraphPrinter()
	// rst:               const graph::Printer &getGraphPrinter() const
	// rst:
	// rst:		:returns: a reference to the :class:`graph::Printer` used for graphs in vertices of the DG.
	graph::Printer &getGraphPrinter();
	const graph::Printer &getGraphPrinter() const;
	// rst: .. function:: void setWithShortcutEdges(bool value)
	// rst:               bool getWithShortcutEdges() const
	// rst:
	// rst:		Control whether hyperedges :math:`(T, H)` with :math:`|T| = |H| = 1`
	// rst:		are printed as a simple arrow without the usual rectangle.
	void setWithShortcutEdges(bool value);
	bool getWithShortcutEdges() const;
	// rst: .. function:: 	void setWithGraphImages(bool value)
	// rst:                 bool getWithGraphImages() const
	// rst:
	// rst:		Control whether each vertex is printed with an image of its graph in it.
	void setWithGraphImages(bool value);
	bool getWithGraphImages() const;
	// rst: .. function:: void setLabelsAsLatexMath(bool value)
	// rst:               bool getLabelsAsLatexMath() const
	// rst:
	// rst:		Control whether labels on vertices and hyperedges are put inline :math:`\LaTeX` math.
	// rst:		Specifically, a label ``label`` is written as ``$\mathrm{label'}$``, with ``label'`` being
	// rst:		``label`` with all space characters escaped.
	void setLabelsAsLatexMath(bool value);
	bool getLabelsAsLatexMath() const;
	// rst: .. function:: void pushVertexVisible(std::function<bool(DG::Vertex)> f)
	// rst:
	// rst:		Add another function controlling the visibility of vertices.
	// rst:		All visibility functions must return `true` for a vertex to be visible.
	// rst:
	// rst:		:throws: :class:`LogicError` if `!f`.
	void pushVertexVisible(std::function<bool(DG::Vertex)> f);
	// rst: .. function:: void popVertexVisible()
	// rst:
	// rst:		Remove the last pushed vertex visibility function.
	// rst:
	// rst:		:throws: :class:`LogicError` if no callback is left to pop.
	void popVertexVisible();
	// rst: .. function:: void pushEdgeVisible(std::function<bool(DG::HyperEdge) > f)
	// rst:
	// rst:		Add another function controlling the visibility of hyperedges.
	// rst:		All visibility functions must return `true` for a hyperedge to be visible.
	// rst:
	// rst:		:throws: :class:`LogicError` if `!f`.
	void pushEdgeVisible(std::function<bool(DG::HyperEdge)> f);
	// rst: .. function:: void popEdgeVisible()
	// rst:
	// rst:		Remove the last pushed hyperedge visibility function.
	// rst:
	// rst:		:throws: :class:`LogicError` if no callback is left to pop.
	void popEdgeVisible();
	// rst: .. function:: void setWithShortcutEdgesAfterVisibility(bool value)
	// rst:               bool getWithShortcutEdgesAfterVisibility() const
	// rst:
	// rst:		Control whether hyperedges are depicted as simple arrows when all but 1 head and tail are hidden.
	void setWithShortcutEdgesAfterVisibility(bool value);
	bool getWithShortcutEdgesAfterVisibility() const;
	// rst: .. function:: void setVertexLabelSep(std::string sep)
	// rst:               const std::string &getVertexLabelSep()
	// rst:
	// rst:		Set/get the string used as separator between each part of each vertex label.
	void setVertexLabelSep(std::string sep);
	const std::string &getVertexLabelSep();
	// rst: .. function:: void setEdgeLabelSep(std::string sep)
	// rst:               const std::string &getEdgeLabelSep()
	// rst:
	// rst:		Set/get the string used as separator between each part of each edge label.
	void setEdgeLabelSep(std::string sep);
	const std::string &getEdgeLabelSep();
	// rst: .. function:: void pushVertexLabel(std::function<std::string(DG::Vertex)> f)
	// rst:
	// rst:		Add another function for vertex labelling. The result of this function is appended to each label.
	// rst:
	// rst:		:throws: :class:`LogicError` if `!f`.
	void pushVertexLabel(std::function<std::string(DG::Vertex)> f);
	// rst: .. function:: void popVertexLabel()
	// rst:
	// rst:		Remove the last pushed vertex labelling function.
	// rst:
	// rst:		:throws: :class:`LogicError` if no callback is left to pop.
	void popVertexLabel();
	// rst: .. function:: void pushEdgeLabel(std::function<std::string(DG::HyperEdge)> f)
	// rst:
	// rst:		Add another function for edge labelling. The result of this function is appended to each label.
	// rst:
	// rst:		:throws: :class:`LogicError` if `!f`.
	void pushEdgeLabel(std::function<std::string(DG::HyperEdge)> f);
	// rst: .. function:: void popEdgeLabel()
	// rst:
	// rst:		Remove the last pushed edge labelling function.
	// rst:
	// rst:		:throws: :class:`LogicError` if no callback is left to pop.
	void popEdgeLabel();
	// rst: .. function:: void setWithGraphName(bool value)
	// rst:               bool getWithGraphName() const
	// rst:
	// rst:		Control whether graph names are appended to the vertex labels (see also :func:`pushVertexLabel`).
	void setWithGraphName(bool value);
	bool getWithGraphName() const;
	// rst: .. function:: void setWithRuleName(bool value)
	// rst:               bool getWithRuleName() const
	// rst:
	// rst:		Control whether rules names are appended to the edge labels (see also :func:`pushEdgeLabel`).
	void setWithRuleName(bool value);
	bool getWithRuleName() const;
	// rst: .. function:: void setWithRuleId(bool value)
	// rst:               bool getWithRuleId() const
	// rst:
	// rst:		Control whether rules IDs are appended to the edge labels (see also :func:`pushEdgeLabel`).
	void setWithRuleId(bool value);
	bool getWithRuleId() const;
	// rst: .. function:: void setWithInlineGraphs(bool value)
	// rst:               bool getWithInlineGraphs() const
	// rst:
	// rst:		Control whether graph depictions should be precompiled, or be included inline in the DG figure.
	void setWithInlineGraphs(bool value);
	bool getWithInlineGraphs() const;
	// rst: .. function:: void pushVertexColour(std::function<std::string(DG::Vertex)> f, bool extendToEdges)
	// rst:
	// rst:		Add another function for colouring vertices. The final colour of a vertex is the result of the first colour function returning a non-empty string.
	// rst:		The functions are evaluated in the order they are pushed and the resulting string is used directly as a colour in Tikz.
	// rst:		A hyperedge is also coloured if at least one head and one tail *can* be coloured with a colour for which `extendToEdges` is `true`.
	// rst:		In this case, the hyperedge (and a subset of the head and tail connectors) is coloured with the first applicable colour.
	// rst:		The edge extension of vertex colour takes lower precedence than explicitly added hyperedge colouring functions.
	// rst:
	// rst:		:throws: :class:`LogicError` if `!f`.
	void pushVertexColour(std::function<std::string(DG::Vertex)> f, bool extendToEdges);
	// rst: .. function:: void popVertexColour()
	// rst:
	// rst:		Remove the last pushed vertex colouring function.
	// rst:
	// rst:		:throws: :class:`LogicError` if no callback is left to pop.
	void popVertexColour();
	// rst: .. function:: void pushEdgeColour(std::function<std::string(DG::HyperEdge)> f)
	// rst:
	// rst:		Add another function for colouring hyperedges. The final colour of a hyperedge (and all of its head and tail connectors) is the result of the
	// rst:		first colour function returning a non-empty string.
	// rst:
	// rst:		:throws: :class:`LogicError` if `!f`.
	void pushEdgeColour(std::function<std::string(DG::HyperEdge)> f);
	// rst: .. function:: void popEdgeColour()
	// rst:
	// rst:		Remove the last pushed hyperedge colouring function.
	// rst:
	// rst:		:throws: :class:`LogicError` if no callback is left to pop.
	void popEdgeColour();
public:
	// rst: .. function:: void setRotationOverwrite(std::function<int(std::shared_ptr<graph::Graph>)> f)
	// rst:
	// rst:		Overwrite the rotation set in the nested :cpp:class:`graph::Printer`. The given function will be used
	// rst:		to set the rotation of each printed graph.
	// rst:
	// rst:		:throws: :class:`LogicError` if `!f`.
	void setRotationOverwrite(std::function<int(std::shared_ptr<graph::Graph>)> f);
	// rst: .. function:: void setMirrorOverwrite(std::function<bool(std::shared_ptr<graph::Graph>) > f)
	// rst:
	// rst:		Overwrite the mirror set in the nested :cpp:class:`graph::Printer`. The given function will be used
	// rst:		to set the mirror of each printed graph.
	// rst:
	// rst:		:throws: :class:`LogicError` if `!f`.
	void setMirrorOverwrite(std::function<bool(std::shared_ptr<graph::Graph>)> f);
public:
	// rst: .. function:: void setImageOverwrite(std::function<std::pair<std::string, std::string>(DG::Vertex v, int dupNum)> f)
	// rst:
	// rst:		Overwrite the image generation for graphs depicted in the vertices of the printed derivation graph.
	// rst:		For each duplicate of each vertex to be depicted, the given callback is called.
	// rst:		It must then return two strings:
	// rst:
	// rst:		1. Either
	// rst:
	// rst:		  - an empty string if the standard depiction should be used, in which case the second string is ignored, or
	// rst:		  - the filename of the PDF that should be included in the final compiled figure.
	// rst:
	// rst:		2. Either
	// rst:
	// rst:		  - an empty string if no additional post-processing command is needed, or
	// rst:		  - a string of Bash code which will be inserted in the post-processing instructions.
	// rst:		    For example, one can write out source code in the callback, and then return a command that compiles
	// rst:		    that code into the PDF needed by inclusion.
	// rst:
	// rst:		The image overwrite can be removed by calling with `nullptr`.
	void setImageOverwrite(std::function<std::pair<std::string, std::string>(DG::Vertex v, int dupNum)> f);
public:
	// rst: .. function:: void setGraphvizPrefix(const std::string &prefix)
	// rst:               const std::string &getGraphvizPrefix() const
	// rst:
	// rst:		Access the string that will be inserted into generated DOT files,
	// rst:		just after the graph declaration.
	void setGraphvizPrefix(const std::string &prefix);
	const std::string &getGraphvizPrefix() const;
	// rst: .. function:: void setTikzpictureOption(const std::string &option)
	// rst:               const std::string &getTikzpictureOption() const
	// rst:
	// rst:		Access the string that will be inserted into generated Tikz files,
	// rst:		in the options for the ``\tikzpicture`` macro used for the DG.
	void setTikzpictureOption(const std::string &option);
	const std::string &getTikzpictureOption() const;
private:
	std::unique_ptr<graph::Printer> graphPrinter;
	std::unique_ptr<lib::DG::Write::Printer> printer;
};
// rst-class-end:

} // namespace mod::dg

#endif // MOD_DG_PRINTER_HPP