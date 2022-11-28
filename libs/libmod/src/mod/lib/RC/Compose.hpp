#ifndef MOD_LIB_RC_COMPOSE_HPP
#define MOD_LIB_RC_COMPOSE_HPP

// Generic DPO rule composition for rules represented as the pushout of
// L <- K -> R, i.e., the "core" graph.
// The graphs may not have parallel edges.

#include <mod/Error.hpp>
#include <mod/lib/DPO/Concepts.hpp>
#include <mod/lib/RC/Visitor/Compound.hpp>
#include <mod/lib/RC/detail/CompositionHelper.hpp>

#include <jla_boost/graph/morphism/Concepts.hpp>

namespace mod::lib::RC {

template<bool Verbose, typename Result, typename RuleFirst, typename RuleSecond, typename InvertibleVertexMap, typename VisitorT>
bool compose(Result &result,
				 const RuleFirst &rFirst, const RuleSecond &rSecond, const InvertibleVertexMap &match,
             VisitorT visitor) {
	// match must be rSecond -> rFirst
	using WrappedVisitor = Visitor::Compound<VisitorT>;
	// TODO: enable these concepts again
//	BOOST_CONCEPT_ASSERT((lib::DPO::WritableRuleConcept<RuleResult>));
	BOOST_CONCEPT_ASSERT((lib::DPO::RuleConcept<RuleFirst>));
	BOOST_CONCEPT_ASSERT((lib::DPO::RuleConcept<RuleSecond>));
	BOOST_CONCEPT_ASSERT((jla_boost::GraphMorphism::InvertibleVertexMapConcept<InvertibleVertexMap>));
	return detail::CompositionHelper<Verbose, Result, RuleFirst, RuleSecond,
			InvertibleVertexMap, WrappedVisitor>(result, rFirst, rSecond, match,
	                                           Visitor::makeVisitor(std::move(visitor)))();
}

} // namespace mod::lib::RC

#endif // MOD_LIB_RC_COMPOSE_HPP