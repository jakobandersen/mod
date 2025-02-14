#ifndef MOD_LIB_RC_LABELLEDRESULT_HPP
#define MOD_LIB_RC_LABELLEDRESULT_HPP

#include <mod/lib/DPO/CombinedRule.hpp>

#include <jla_boost/graph/morphism/models/InvertibleVector.hpp>

namespace mod::lib::RC {

struct ResultMaps {
	using Rule = lib::DPO::CombinedRule;
	using CombinedGraph = Rule::CombinedGraphType;
public:
	ResultMaps(const Rule &rFirst, const Rule &rSecond, const Rule &rDPO)
		: mFirstToResult(rFirst.getCombinedGraph(), rDPO.getCombinedGraph()),
		  mSecondToResult(rSecond.getCombinedGraph(), rDPO.getCombinedGraph()) {}
	ResultMaps(const ResultMaps&) = delete;
	ResultMaps &operator=(const ResultMaps&) = delete;
	ResultMaps(ResultMaps&&) = default;
	ResultMaps &operator=(ResultMaps&&) = default;
public:
	jla_boost::GraphMorphism::InvertibleVectorVertexMap<CombinedGraph, CombinedGraph> mFirstToResult;
	jla_boost::GraphMorphism::InvertibleVectorVertexMap<CombinedGraph, CombinedGraph> mSecondToResult;
};

struct Result {
	using Rule = ResultMaps::Rule;
	using CombinedGraph = ResultMaps::CombinedGraph;
public:
	Result(const Rule &rFirst, const Rule &rSecond) : rDPO(new Rule()), mappings(rFirst, rSecond, *rDPO) {}
public:
	std::unique_ptr<Rule> rDPO;
	ResultMaps mappings;
};


struct LabelledResult : Result {
	using MatchConstraint = GraphMorphism::Constraints::Constraint<Rule::SideGraphType>;
	using PropStringType = lib::rule::PropString;
	using PropTermType = lib::rule::PropTerm;
public:
	using Result::Result;

	std::pair<lib::rule::LabelledRule, ResultMaps> createRule() && {
		assert(pString || pTerm);
		auto rule = pString
			            ? lib::rule::LabelledRule(std::move(rDPO), std::move(pString), std::move(pStereo))
			            : lib::rule::LabelledRule(std::move(rDPO), std::move(pTerm), std::move(pStereo));
		rule.leftData.matchConstraints = std::move(matchConstraints);
		return {std::move(rule), std::move(mappings)};
	}
public:
	std::unique_ptr<lib::rule::PropString> pString;
	std::unique_ptr<lib::rule::PropTerm> pTerm;
	std::unique_ptr<lib::rule::PropStereo> pStereo;
	std::vector<std::unique_ptr<MatchConstraint>> matchConstraints;
};

} // namespace mod::lib::Rc

#endif // MOD_LIB_RC_LABELLEDRESULT_HPP