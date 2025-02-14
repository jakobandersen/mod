#ifndef MOD_RULE_RULE_HPP
#define MOD_RULE_RULE_HPP

#include <mod/BuildConfig.hpp>
#include <mod/Config.hpp>
#include <mod/graph/ForwardDecl.hpp>
#include <mod/rule/ForwardDecl.hpp>

#include <iosfwd>
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace mod::rule {

// rst-class: rule::Rule
// rst:
// rst:		This class models a graph transformation rule in the Double Pushout formalism,
// rst:		as the span :math:`L \leftarrow K \rightarrow R`.
// rst:		The three graphs are referred to as respectively
// rst:		the "left", "context", and "right" graphs of the rule.
// rst:		See :ref:`graph-model` for more details.
// rst:
// rst:		See :ref:`cpp-rule/GraphInterface` for the documentation for the
// rst:		graph interface for this class.
// rst:
// rst-class-start:
struct MOD_DECL Rule {
	using Handle = std::shared_ptr<Rule>;
	class Vertex;
	class Edge;
	class VertexIterator;
	class VertexRange;
	class EdgeIterator;
	class EdgeRange;
	class IncidentEdgeIterator;
	class IncidentEdgeRange;
public:
	class LeftGraph;
	class ContextGraph;
	class RightGraph;
private:
	Rule(Rule &&) = delete;
	Rule &operator=(Rule &&) = delete;
private:
	Rule(std::unique_ptr<lib::rule::Rule> r);
public:
	~Rule();
	// rst: .. function:: unsigned int getId() const
	// rst:
	// rst:		:returns: the unique instance id among all :class:`Rule` instances.
	unsigned int getId() const;
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const Rule &r);
	const lib::rule::Rule &getRule() const;
public: // graph interface
	// rst: .. function:: std::size_t numVertices() const
	// rst:
	// rst:		:returns: the number of vertices in the rule.
	std::size_t numVertices() const;
	// rst: .. function:: VertexRange vertices() const
	// rst:
	// rst:		:returns: a range of all vertices in the rule.
	VertexRange vertices() const;
	// rst: .. function:: std::size_t numEdges() const
	// rst:
	// rst:		:returns: the number of edges in the rule.
	std::size_t numEdges() const;
	// rst: .. function:: EdgeRange edges() const
	// rst:
	// rst:		:returns: a range of all edges in the rule.
	EdgeRange edges() const;
	// rst: .. function:: LeftGraph getLeft() const
	// rst:
	// rst:		:returns: a proxy object representing the left graph of the rule.
	LeftGraph getLeft() const;
	// rst: .. function:: ContextGraph getContext() const
	// rst:
	// rst:		:returns: a proxy object representing the context graph of the rule.
	ContextGraph getContext() const;
	// rst: .. function:: RightGraph getRight() const
	// rst:
	// rst:		:returns: a proxy object representing the rule graph of the rule.
	RightGraph getRight() const;
public:
	// rst:	.. function:: std::shared_ptr<Rule> makeInverse() const
	// rst:
	// rst:			:returns: a rule representing the inversion of this rule.
	// rst:			:raises: :class:`LogicError` if inversion is not possible (due to matching constraints).
	std::shared_ptr<Rule> makeInverse() const;
	// rst: .. function:: std::pair<std::string, std::string> print() const
	// rst:               std::pair<std::string, std::string> print(bool printCombined) const
	// rst:               std::pair<std::string, std::string> print(const graph::Printer &first, const graph::Printer &second) const
	// rst:               std::pair<std::string, std::string> print(const graph::Printer &first, const graph::Printer &second, bool printCombined) const
	// rst:
	// rst:		Print the rule, using either the default options or the options in `first` and `second`.
	// rst:		If `first` and `second` are the same, only one depiction will be made.
	// rst:
	// rst:		:param printCombined: whether a depiction of the rule as a single combined graph is printed. Defaults to ``false``.
	// rst:		:returns: a pair of filename prefixes for the PDF-files that will be compiled in post-processing.
	// rst:			The actual names can be obtained by appending ``_L.pdf``, ``_K.pdf``, and ``_R.pdf`` for
	// rst:			respectively the left side, context, and right side graphs.
	// rst:			If `first` and `second` are the same, the two file prefixes are equal.
	std::pair<std::string, std::string> print() const;
	std::pair<std::string, std::string> print(bool printCombined) const;
	std::pair<std::string, std::string> print(const graph::Printer &first, const graph::Printer &second) const;
	std::pair<std::string, std::string>
	print(const graph::Printer &first, const graph::Printer &second, bool printCombined) const;
	// rst: .. function:: void printTermState() const
	// rst:
	// rst:		Print the term state for the rule.
	void printTermState() const;
	// rst: .. function:: std::string getGMLString(bool withCoords = false) const
	// rst:
	// rst:		:returns: the :ref:`GML <rule-gml>` representation of the rule,
	// rst:		          optionally with generated 2D coordinates.
	// rst:		:throws: :any:`LogicError` when coordinates are requested, but
	// rst:		         none can be generated.
	std::string getGMLString(bool withCoords = false) const;
	// rst: .. function:: std::string printGML(bool withCoords = false) const
	// rst:
	// rst:		Print the rule in :ref:`GML <rule-gml>` format,
	// rst:		optionally with generated 2D coordinates.
	// rst:
	// rst:		:returns: the filename of the printed GML file.
	// rst:		:throws: :any:`LogicError` when coordinates are requested, but
	// rst:		         none can be generated.
	std::string printGML(bool withCoords = false) const;
	// rst: .. function:: const std::string &getName() const
	// rst:	              void setName(std::string name)
	// rst:
	// rst:		Access the name of the rule.
	const std::string &getName() const;
	void setName(std::string name);
	// rst: .. function:: std::optional<LabelType> getLabelType() const
	// rst:
	// rst:		:returns: the intended label type for this rule, or nothing if no specific label type is intended.
	std::optional<LabelType> getLabelType() const;
	// rst: .. function:: std::size_t getNumLeftComponents() const
	// rst:
	// rst:		:returns: the number of connected components in the left graph.
	std::size_t getNumLeftComponents() const;
	// rst: .. function:: std::size_t getNumRightComponents() const
	// rst:
	// rst:		:returns: the number of connected components in the right graph.
	std::size_t getNumRightComponents() const;
	// rst: .. function:: std::size_t isomorphism(std::shared_ptr<Rule> r, std::size_t maxNumMatches, LabelSettings labelSettings) const
	// rst:
	// rst:		:returns: the number of isomorphisms found between `r` and this graph, but at most `maxNumMatches`.
	std::size_t isomorphism(std::shared_ptr<Rule> r, std::size_t maxNumMatches, LabelSettings labelSettings) const;
	// rst: .. function:: std::size_t monomorphism(std::shared_ptr<Rule> r, std::size_t maxNumMatches, LabelSettings labelSettings) const
	// rst:
	// rst:		:returns: the number of monomorphism from this rule to subgraphs of `r`, though at most `maxNumMatches`.
	std::size_t monomorphism(std::shared_ptr<Rule> r, std::size_t maxNumMatches, LabelSettings labelSettings) const;
	// rst: .. function:: bool isomorphicLeftRight(std::shared_ptr<Rule> r, LabelSettings labelSettings) const
	// rst:
	// rst:		:returns: whether the left-hand sides are isomorphic and the right-hand sides are isomorphic.
	bool isomorphicLeftRight(std::shared_ptr<Rule> r, LabelSettings labelSettings) const;
	// rst: .. function:: Vertex getVertexFromExternalId(int id) const
	// rst:
	// rst:		If the rule was not loaded from an external data format, then this function
	// rst:		always return a null descriptor.
	// rst:
	// rst:		.. note:: In general there is no correlation between external and internal ids.
	// rst:
	// rst:		:returns: the vertex descriptor for the given external id.
	// rst:		          The descriptor is null if the external id was not used.
	Vertex getVertexFromExternalId(int id) const;
	// rst: .. function:: int getMinExternalId() const
	// rst:               int getMaxExternalId() const
	// rst:
	// rst:		If the rule was not loaded from an external data format, then these functions
	// rst:		always return 0. Otherwise, return the minimum/maximum external id from which
	// rst:		non-null vertices can be obtained from :cpp:func:`getVertexFromExternalId`.
	// rst:		If no such minimum and maximum exists, then 0 will be returned.
	int getMinExternalId() const;
	int getMaxExternalId() const;
public:
	// rst: .. function:: static std::shared_ptr<Rule> fromGMLString(const std::string &data, bool invert, bool printStereoWarnings)
	// rst:               static std::shared_ptr<Rule> fromGMLFile(const std::string &file, bool invert, bool printStereoWarnings)
	// rst:
	// rst:		Load a rule from a :ref:`GML <rule-gml>` string or file, and store either that rule or its inverse.
	// rst:		The name of the rule is the one specified in the GML string, though when ``invert=True``
	// rst:		the string ", inverse" is appended to the name.
	// rst:
	// rst:		.. note::
	// rst:
	// rst:			If the GML string specifies matching constraints it is not possible to invert the rule.
	// rst:			There is however a configuration option to ignore matching constraints when inverting rules.
	// rst:
	// rst:		:param printStereoWarnings: whether to print warnings due to unhandled stereo information.
	// rst:		:returns: the loaded (possibly inverted) rule.
	// rst:		:throws: :class:`InputError` on bad data and when inversion fails due to constraints.
	static std::shared_ptr<Rule> fromGMLString(const std::string &data, bool invert, bool printStereoWarnings = true);
	static std::shared_ptr<Rule> fromGMLFile(const std::string &file, bool invert, bool printStereoWarnings = true);
	// rst: .. function:: static std::shared_ptr<Rule> fromDFS(const std::string &data, bool invert)
	// rst:
	// rst:		Load a rule from a :ref:`RuleDFS <format-ruleDFS>` string, and store either that rule or its inverse.
	// rst:		The name of the rule is the one specified, though when ``invert=True``
	// rst:		the string ", inverse" is appended to the name.
	// rst:
	// rst:		:returns: the loaded (possibly inverted) rule.
	// rst:		:throws: :class:`InputError` on bad data and when inversion fails due to constraints.
	static std::shared_ptr<Rule> fromDFS(const std::string &data, bool invert);
	// rst: .. function:: static std::shared_ptr<Rule> makeRule(std::unique_ptr<lib::rule::Real> r)
	// rst:               static std::shared_ptr<Rule> makeRule(std::unique_ptr<lib::rule::Real> r, std::map<int, std::size_t> externalToInternalIds)
	// rst:
	// rst:		:returns: a rule wrapping the given internal rule object. If an id mapping is given, it will be used for the :cpp:func:`getVertexFromExternalId` function.
	static std::shared_ptr<Rule> makeRule(std::unique_ptr<lib::rule::Rule> r);
	static std::shared_ptr<Rule>
	makeRule(std::unique_ptr<lib::rule::Rule> r, std::map<int, std::size_t> externalToInternalIds);
private:
	struct Pimpl;
	std::unique_ptr<Pimpl> p;
};
// rst-class-end:

struct RuleLess {
	bool operator()(std::shared_ptr<Rule> r1, std::shared_ptr<Rule> r2) const {
		return r1->getId() < r2->getId();
	}
};

} // namespace mod::rule

#endif // MOD_RULE_RULE_HPP