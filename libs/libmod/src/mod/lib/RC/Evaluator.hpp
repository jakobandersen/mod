#ifndef MOD_LIB_RC_EVALCONTEXT_HPP
#define MOD_LIB_RC_EVALCONTEXT_HPP

#include <mod/Config.hpp>
#include <mod/rule/ForwardDecl.hpp>
#include <mod/lib/Rule/GraphAsRuleCache.hpp>

#include <boost/graph/adjacency_list.hpp>

#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace mod::lib::RC {

struct Evaluator {
	enum class VertexKind {
		Rule, Composition
	};

	enum class EdgeKind {
		First, Second, Result
	};

	struct VProp {
		VertexKind kind;
		const lib::rule::Rule *rule;
	};

	struct EProp {
		EdgeKind kind;
	};

	using GraphType = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, VProp, EProp>;
	using Vertex = boost::graph_traits<GraphType>::vertex_descriptor;
	using Edge = boost::graph_traits<GraphType>::edge_descriptor;
public:
	Evaluator(std::unordered_set<std::shared_ptr<mod::rule::Rule>> database, LabelSettings labelSettings);
	const std::unordered_set<std::shared_ptr<mod::rule::Rule>> &getRuleDatabase() const;
	const std::unordered_set<std::shared_ptr<mod::rule::Rule>> &getCreatedRules() const;
	std::vector<std::shared_ptr<mod::rule::Rule>> eval(const mod::rule::RCExp::Expression &exp, bool onlyUnique, int verbosity);
	void print() const;
	const GraphType &getGraph() const;
public: // evaluation interface
	// adds a rule to the database, returns true iff it was a new rule
	bool addRule(std::shared_ptr<mod::rule::Rule> r);
	// adds a rule to the product graph list
	void giveProductStatus(std::shared_ptr<mod::rule::Rule> r);
	// searches the database for an isomorphic rule
	// if found, the rule is deleted and the database rule is returned
	// otherwise, the rule is wrapped and returned
	// does NOT add to the database
	std::shared_ptr<mod::rule::Rule> checkIfNew(lib::rule::Rule *rCand) const;
	// records a composition
	void suggestComposition(const lib::rule::Rule *rFirst,
	                        const lib::rule::Rule *rSecond,
	                        const lib::rule::Rule *rResult);
private: // graph interface
	Vertex getVertexFromRule(const lib::rule::Rule *r);
	Vertex getVertexFromArgs(const lib::rule::Rule *rFirst, const lib::rule::Rule *rSecond);
public:
	const LabelSettings labelSettings;
	rule::GraphAsRuleCache graphAsRuleCache;
private:
	std::unordered_set<std::shared_ptr<mod::rule::Rule>> database, createdRules;
private:
	GraphType rcg;
	std::unordered_map<const lib::rule::Rule *, Vertex> ruleToVertex;
	std::map<std::pair<const lib::rule::Rule *, const lib::rule::Rule *>, Vertex> argsToVertex;
};

} // namespace mod::lib::RC

#endif // MOD_LIB_RC_EVALCONTEXT_HPP