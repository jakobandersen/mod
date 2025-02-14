#ifndef MOD_LIB_RC_RESULT_HPP
#define MOD_LIB_RC_RESULT_HPP

#include <boost/concept_check.hpp>

namespace mod::lib::RC {

template<typename Result>
struct ResultConcept {
	BOOST_CONCEPT_USAGE(ResultConcept) {
//		const auto &g = *result.graph;
//		using Graph = typename std::remove_cv<typename std::remove_reference < decltype(g)>::type>::type;
//		BOOST_CONCEPT_ASSERT((boost::MutableGraphConcept<Graph>));
//		auto &mLeft = result.mLeftToResult;
//		auto &mRight = result.mRightToResult;
//		using MapLeft = typename std::remove_reference < decltype(mLeft)>::type;
//		using MapRight = typename std::remove_reference < decltype(mRight)>::type;
//		BOOST_CONCEPT_ASSERT((jla_boost::GraphMorphism::InvertibleVertexMapConcept<MapLeft>));
//		BOOST_CONCEPT_ASSERT((jla_boost::GraphMorphism::InvertibleVertexMapConcept<MapRight>));
//		(void) g;
//		(void) mLeft;
//		(void) mRight;
	}
private:
	Result result;
};

} // namespace mod::lib::RC

#endif // MOD_LIB_RC_RESULT_HPP