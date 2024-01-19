#ifndef MOD_LIB_LABELLEDGRAPH_HPP
#define MOD_LIB_LABELLEDGRAPH_HPP

#include <mod/lib/Term/WAM.hpp>

#include <boost/concept_check.hpp>

namespace mod::lib {

template<typename G>
struct LabelledGraphTraits {
	using GraphType = typename G::GraphType;
	using PropStringType = typename G::PropStringType;
	using PropTermType = typename G::PropTermType;
	using PropStereoType = typename G::PropStereoType;
};

template<typename G>
struct LabelledGraphConcept {
	using GraphType = typename LabelledGraphTraits<G>::GraphType;
	using PropStringType = typename LabelledGraphTraits<G>::PropStringType;
	using PropTermType = typename LabelledGraphTraits<G>::PropTermType;
	using PropStereoType = typename LabelledGraphTraits<G>::PropStereoType;
public:
	BOOST_CONCEPT_USAGE(LabelledGraphConcept) {
		const G &gOuterConst = gOuter;

		[[maybe_unused]] const GraphType &gConst = get_graph(gOuterConst);
		[[maybe_unused]] const PropStringType &pString = get_string(gOuterConst);
		[[maybe_unused]] const PropTermType &pTerm = get_term(gOuterConst);
		[[maybe_unused]] const bool pIsValid = isValid(pTerm);
		[[maybe_unused]] const lib::Term::Wam &machine = getMachine(pTerm);
		[[maybe_unused]] const bool hasStereo = has_stereo(gOuterConst);
		[[maybe_unused]] const PropStereoType &pStereo = get_stereo(gOuterConst);
	}
private:
	G gOuter;
};

//template<typename G>
//struct MutableLabelledGraphConcept : LabelledGraphConcept<G>  {
//	using GraphType = typename G::GraphType;
//public:
//
//	BOOST_CONCEPT_USAGE(MutableLabelledGraphConcept) {		
//		GraphType &g = get_graph(gOuter);
//	}
//private:
//	G gOuter;
//};

struct IdentityPropertyHandler {
	template<typename VEProp, typename LabGraphDom, typename LabGraphCodom, typename F, typename ...Args>
	static auto fmap2(const VEProp &l, const VEProp &r, const LabGraphDom &gDom, const LabGraphCodom &gCodom, F &&f, Args&&... args) {
		return f(l, r, gDom, gCodom, std::forward<Args>(args)...);
	}

	template<typename Op, typename Val>
	static auto reduce(Op &&, Val &&val) {
		return std::forward<Val>(val);
	}
};

} // namespace mod::lib

#endif // MOD_LIB_LABELLEDGRAPH_HPP