#ifndef MOD_LIB_RC_MATCHBUILDER_HPP
#define MOD_LIB_RC_MATCHBUILDER_HPP

#include <mod/lib/Rule/Rule.hpp>
#include <jla_boost/graph/morphism/finders/CommonSubgraph.hpp>

namespace mod::lib::RC {

struct MatchBuilder {
	explicit MatchBuilder(const lib::rule::Rule &rFirst, const lib::rule::Rule &rSecond, LabelSettings labelSettings);
	lib::rule::Vertex getSecondFromFirst(lib::rule::Vertex v) const;
	lib::rule::Vertex getFirstFromSecond(lib::rule::Vertex v) const;
	std::size_t size() const;
	bool push(lib::rule::Vertex vFirst, lib::rule::Vertex vSecond);
	void pop();
	std::unique_ptr<lib::rule::Rule> compose(bool verbose) const;
	std::vector<std::unique_ptr<lib::rule::Rule>> composeAll(bool maximum, bool verbose) const;
private:
	struct VertexPred {
		bool operator()(lib::rule::Vertex vSecond, lib::rule::Vertex vFirst);
	public:
		const lib::rule::Rule &rFirst;
		const lib::rule::Rule &rSecond;
		const LabelSettings labelSettings;
	};
	struct EdgePred {
		bool operator()(lib::rule::Edge eSecond, lib::rule::Edge eFirst);
	public:
		const lib::rule::Rule &rFirst;
		const lib::rule::Rule &rSecond;
		const LabelSettings labelSettings;
	};
public:
	const lib::rule::Rule &rFirst;
	const lib::rule::Rule &rSecond;
	const LabelSettings labelSettings;
private:
	// note: the match has rSecond as domain and rFirst as codomain, i.e., <-,
	//       so everything to do with match is somewhat "backwards"
	jla_boost::GraphMorphism::CommonSubgraphEnumerator<false,
			lib::DPO::CombinedRule::SideGraphType, lib::DPO::CombinedRule::SideGraphType,
			EdgePred, VertexPred> match;
};

} // namespace mod::lib::RC

#endif // MOD_LIB_RC_MATCHBUILDER_HPP