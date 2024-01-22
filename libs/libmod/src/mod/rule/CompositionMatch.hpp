#ifndef MOD_RULE_COMPOSITIONMATCH_HPP
#define MOD_RULE_COMPOSITIONMATCH_HPP

#include <mod/BuildConfig.hpp>
#include <mod/rule/GraphInterface.hpp>

namespace mod::rule {

// rst-class: rule::CompositionMatch
// rst:
// rst:		A class for interactively constructing match between two rules used for composition.
// rst:		That is, for two rules
// rst:		:math:`p_1 = (L_1 \leftarrow K_1 \rightarrow R_1)` and
// rst:		:math:`p_2 = (L_2 \leftarrow K_2 \rightarrow R_2)`,
// rst:		an object represents match between :math:`R_1` and :math:`L_2` which indicates
// rst:		a common subgraph :math:`R_1 \leftarrow M \rightarrow L_2`.
// rst-class-start:
struct MOD_DECL CompositionMatch {
	// rst: .. function:: explicit CompositionMatch(std::shared_ptr<Rule> rFirst, std::shared_ptr<Rule> rSecond, LabelSettings labelSettings)
	// rst:
	// rst:		Construct an empty overlap.
	// rst:
	// rst:		:throws LogicError: if either rule is null.
	explicit CompositionMatch(std::shared_ptr<Rule> rFirst, std::shared_ptr<Rule> rSecond, LabelSettings labelSettings);
	// rst: .. function:: CompositionMatch(const CompositionMatch &other)
	// rst:               CompositionMatch &operator=(const CompositionMatch &other)
	CompositionMatch(const CompositionMatch &other);
	CompositionMatch &operator=(const CompositionMatch &other);
	~CompositionMatch();
	// rst: .. function:: friend std::ostream &operator<<(std::ostream &s, const CompositionMatch &mb)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const CompositionMatch &mb);
	// rst: .. function:: std::shared_ptr<Rule> getFirst() const
	// rst:               std::shared_ptr<Rule> getSecond() const
	// rst:
	// rst:		:returns: the rules :math:`p_1` and :math:`p_2`.
	std::shared_ptr<Rule> getFirst() const;
	std::shared_ptr<Rule> getSecond() const;
	// rst: .. function:: LabelSettings getLabelSettings() const
	// rst:
	// rst:		:returns: the settings for the morphisms in the match.
	LabelSettings getLabelSettings() const;
	// rst: .. function:: int size() const
	// rst:
	// rst:		:returns: the number of currently mapped pairs of vertices.
	int size() const;
	// rst: .. function:: Rule::LeftGraph::Vertex operator[](Rule::RightGraph::Vertex vFirst) const
	// rst:               Rule::RightGraph::Vertex operator[](Rule::LeftGraph::Vertex vSecond) const
	// rst:
	// rst:		:returns: the vertex the given vertex is mapped to, or a null vertex if it is not mapped.
	// rst:		:throws LogicError: if either vertex is a null vertex.
	// rst:		:throws LogicError: if either vertex does not belong to their respective rules.
	Rule::LeftGraph::Vertex operator[](Rule::RightGraph::Vertex vFirst) const;
	Rule::RightGraph::Vertex operator[](Rule::LeftGraph::Vertex vSecond) const;
	// rst: .. function:: void push(Rule::RightGraph::Vertex vFirst, Rule::LeftGraph::Vertex vSecond)
	// rst:
	// rst:		Extend the common subgraph (match) of :math:`R_1` and :math:`L_2`
	// rst:		by identifying the given vertices from each of the graphs.
	// rst:
	// rst:		:throws LogicError: if either vertex is a null vertex.
	// rst:		:throws LogicError: if either vertex does not belong to their respective rules.
	// rst:		:throws LogicError: if either vertex is already mapped.
	// rst:		:throws LogicError: if the extension of the match is infeasible.
	void push(Rule::RightGraph::Vertex vFirst, Rule::LeftGraph::Vertex vSecond);
	// rst: .. function:: void pop()
	// rst:
	// rst:		Pop the latest pushed vertex pair.
	// rst:
	// rst:		:throws LogicError: if `size() == 0`.
	void pop();
	// rst: .. function:: std::shared_ptr<Rule> compose(bool verbose) const
	// rst:
	// rst:		:param verbose: whether to output debug messages from the composition algorithm.
	// rst:		:returns: the composition of the two rules along the match.
	// rst:			If the composition is not defined, a null pointer is returned.
	std::shared_ptr<Rule> compose(bool verbose) const;
	// rst: .. function:: std::vector<std::shared_ptr<Rule>> composeAll(bool maximum, bool verbose) const
	// rst:
	// rst:		:param maximum: whether to only compose using the matches of maximum cardinality.
	// rst:		:param verbose: whether to output debug messages from the composition algorithm.
	// rst:		:returns: the composition of the two rules along enumerated matches.
	// rst:			The enumerated matches are all those that are extensions of the current match,
	// rst:			and the match itself.
	std::vector<std::shared_ptr<Rule>> composeAll(bool maximum, bool verbose) const;
private:
	struct Pimpl;
	std::unique_ptr<Pimpl> p;
};
// rst-class-end:

} // namespace mod::rule

#endif // MOD_RULE_COMPOSITIONMATCH_HPP
