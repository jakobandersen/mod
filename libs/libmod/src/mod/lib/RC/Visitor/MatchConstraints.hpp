#ifndef MOD_LIB_RC_VISITOR_MATCH_CONSTRAINTS_HPP
#define MOD_LIB_RC_VISITOR_MATCH_CONSTRAINTS_HPP

#include <mod/lib/GraphMorphism/Constraints/AllVisitor.hpp>
#include <mod/lib/RC/Visitor/Compound.hpp>
#include <mod/lib/Rules/LabelledRule.hpp>

namespace mod::lib::RC::Visitor {
namespace detail {
template<typename G>
using ConstraintVisitor = lib::GraphMorphism::Constraints::AllVisitor<G>;
template<typename G>
using Constraint = lib::GraphMorphism::Constraints::Constraint<G>;
template<typename G>
using ConstraintAdj = lib::GraphMorphism::Constraints::VertexAdjacency<G>;
template<typename G>
using ConstraintSP = lib::GraphMorphism::Constraints::ShortestPath<G>;

template<LabelType labelType, typename RuleFirst, typename RuleSecond, typename Result>
struct ConvertFirst : public ConstraintVisitor<// GraphFirstLeft
/*   */ typename RuleFirst::SideGraphType
/*   */ > {
	using GraphResult = typename Result::RuleResult::GraphType;
	using GraphFirst = typename RuleFirst::GraphType;
	using GraphSecond = typename RuleSecond::GraphType;

	using GraphResultLeft = typename Result::RuleResult::SideGraphType;
	using GraphFirstLeft = typename RuleFirst::SideGraphType;
	using GraphSecondLeft = typename RuleSecond::SideGraphType;

	ConvertFirst(const RuleFirst &rFirst, Result &result) : rFirst(rFirst), result(result) {}

	virtual void operator()(const ConstraintAdj<GraphFirstLeft> &c) override {
		auto cResult = std::make_unique<ConstraintAdj<GraphResultLeft> >(c);
		switch(labelType) {
		case LabelType::String:
			cResult->vertexTerms.clear();
			cResult->edgeTerms.clear();
			break;
		case LabelType::Term:
			cResult->vertexLabels.clear();
			cResult->edgeLabels.clear();
			break;
		}
		auto vFirst = cResult->vConstrained;
		auto vResult = get(result.mFirstToResult, get_graph(rFirst), result.rDPO->getCombinedGraph(), vFirst);
		assert(vResult != boost::graph_traits<GraphResult>::null_vertex());
		cResult->vConstrained = vResult;
		// std::cout << "WARNING: check transfered constraint for " << rFirst.getName() << " -> " << rSecond.getName() << std::endl;
		this->cResult = std::move(cResult);
	}
public:
	virtual void operator()(const ConstraintSP<GraphFirstLeft> &c) override {
		auto cResult = std::make_unique<ConstraintSP<GraphResultLeft> >(c);
		MOD_ABORT;
	}
public:
	const RuleFirst &rFirst;
	Result &result;
	std::unique_ptr<Constraint<GraphResultLeft> > cResult;
};

template<LabelType labelType, typename RuleFirst, typename RuleSecond, typename InvertibleVertexMap, typename Result>
struct ConvertSecond : public ConstraintVisitor<// GraphSecondLeft
/*   */ typename RuleSecond::SideGraphType
/*   */ > {
	using GraphResult = typename Result::RuleResult::GraphType;
	using GraphFirst = typename RuleFirst::GraphType;
	using GraphSecond = typename RuleSecond::GraphType;

	using GraphResultLeft = typename Result::RuleResult::SideGraphType;
	using GraphFirstLeft = typename RuleFirst::SideGraphType;
	using GraphSecondLeft = typename RuleSecond::SideGraphType;

	ConvertSecond(const RuleFirst &rFirst, const RuleSecond &rSecond, InvertibleVertexMap &match, Result &result)
			: rFirst(rFirst), rSecond(rSecond), match(match), result(result) {}

	virtual void operator()(const ConstraintAdj<GraphSecondLeft> &c) override {
		const auto vSecond = c.vConstrained;
		const auto vResult = get(result.mSecondToResult, get_graph(rSecond), result.rDPO->getCombinedGraph(), vSecond);
		// vResult may be null_vertex
		if(vResult == boost::graph_traits<GraphResult>::null_vertex()) {
			// std::cout << "WARNING: constrained vertex " << vSecondId << " deleted in " << rFirst.getName() << " -> " << rSecond.getName() << std::endl;
			return;
		}

		auto mResult = result.rDPO->getCombinedGraph()[vResult].membership;
		if(mResult == lib::DPO::Membership::R) {
			// std::cout << "WARNING: constrained vertex " << vSecondId << " changed to right side in " << rFirst.getName() << " -> " << rSecond.getName() << std::endl;
			return;
		}

		if(get(match, get_graph(get_labelled_left(rSecond)), get_graph(get_labelled_right(rFirst)), vSecond) !=
		   boost::graph_traits<GraphFirst>::null_vertex()) {
			// std::cout << "WARNING: maybe missing constraint on " << vNew << " for " << rFirst.getName() << " -> " << rSecond.getName() << std::endl;
			return;
		}

		auto cResult = std::make_unique<ConstraintAdj<GraphResultLeft> >(c);
		cResult->vConstrained = vResult;
		switch(labelType) {
		case LabelType::String:
			cResult->vertexTerms.clear();
			cResult->edgeTerms.clear();
			break;
		case LabelType::Term: {
			cResult->vertexLabels.clear();
			cResult->edgeLabels.clear();
			// we need to make sure the terms referred to are actually in the result machine,
			// and they need to be correct
			auto vTerms = std::move(cResult->vertexTerms);
			auto eTerms = std::move(cResult->edgeTerms);
			cResult->vertexTerms.clear();
			cResult->edgeTerms.clear();
			auto &m = getMachine(*result.pTerm);
			const auto handleTerm = [&m](const auto tSecond) {
				m.verify();
				//			lib::IO::Term::Write::wam(m, lib::Term::getStrings(), std::cout << "Copy " << addr << "\n");
				auto res = m.copyFromTemp(tSecond);
				//			lib::IO::Term::Write::wam(m, lib::Term::getStrings(), std::cout << "After copy " << addr << "\n");
				m.verify();
				return res.addr;
			};
			for(const auto tSecond: vTerms) cResult->vertexTerms.insert(handleTerm(tSecond));
			for(const auto tSecond: eTerms) cResult->edgeTerms.insert(handleTerm(tSecond));
			break;
		}
		}
		this->cResult = std::move(cResult);
		// std::cout << "WARNING: check converted constraint on vertex " << vNew << " for " << rFirst.getName() << " -> " << rSecond.getName() << std::endl;
	}
public:
	virtual void operator()(const ConstraintSP<GraphSecondLeft> &c) override {
		const auto &gResult = result.rDPO->getCombinedGraph();
		auto vResultSrc = get(result.mSecondToResult, get_graph(rSecond), gResult, c.vSrc);
		auto vResultTar = get(result.mSecondToResult, get_graph(rSecond), gResult, c.vTar);
		auto isSrcDeleted = vResultSrc == boost::graph_traits<GraphResult>::null_vertex();
		auto isTarDeleted = vResultTar == boost::graph_traits<GraphResult>::null_vertex();
		if(isSrcDeleted && isTarDeleted) {
			MOD_ABORT;
		}
		if(isSrcDeleted || isTarDeleted) {
			MOD_ABORT;
		}

		auto isInRightSrc = gResult[vResultSrc].membership == lib::DPO::Membership::R;
		auto isInRightTar = gResult[vResultTar].membership == lib::DPO::Membership::R;
		if(isInRightSrc || isInRightTar) return; // must have been checked

		auto cResult = std::make_unique<ConstraintSP<GraphResultLeft> >(c);
		cResult->vSrc = vResultSrc;
		cResult->vTar = vResultTar;
		this->cResult = std::move(cResult);
	}
public:
	const RuleFirst &rFirst;
	const RuleSecond &rSecond;
	InvertibleVertexMap &match;
	Result &result;
	mutable std::unique_ptr<Constraint<GraphResultLeft> > cResult;
};

} // namespace detail

template<LabelType labelType>
struct MatchConstraints : Null {
	MatchConstraints(const lib::Rules::LabelledRule &rFirst, const lib::Rules::LabelledRule &rSecond)
			: rFirst(rFirst), rSecond(rSecond) {}

	template<bool Verbose, typename InvertibleVertexMap, typename Result>
	bool finalize(IO::Logger logger, const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
	              InvertibleVertexMap &match, Result &result) {
		using RuleFirst = lib::Rules::LabelledRule;
		using RuleSecond = lib::Rules::LabelledRule;

		assert(&dpoFirst == &rFirst.getRule());
		assert(&dpoSecond == &rSecond.getRule());

		if(!getConfig().rc.composeConstraints.get()) return true;

		for(const auto &cFirst: get_match_constraints(get_labelled_left(rFirst))) {
			detail::ConvertFirst<labelType, RuleFirst, RuleSecond, Result> visitor(rFirst, result);
			cFirst->accept(visitor);
			assert(visitor.cResult);
			result.matchConstraints.push_back(std::move(visitor.cResult));
		}
		for(const auto &cSecond: get_match_constraints(get_labelled_left(rSecond))) {
			detail::ConvertSecond<labelType, RuleFirst, RuleSecond, InvertibleVertexMap, Result> visitor(
					rFirst, rSecond, match, result);
			cSecond->accept(visitor);
			if(visitor.cResult)
				result.matchConstraints.push_back(std::move(visitor.cResult));
		}
		return true;
	}
private:
	const lib::Rules::LabelledRule &rFirst;
	const lib::Rules::LabelledRule &rSecond;
};

} // namespace mod::lib::RC::Visitor

#endif // MOD_LIB_RC_VISITOR_MATCH_CONSTRAINTS_HPP