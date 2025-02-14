#ifndef MOD_LIB_RC_LABELLEDCOMPOSITION_HPP
#define MOD_LIB_RC_LABELLEDCOMPOSITION_HPP

// This file is specific to the label settings we use,
// but generic with respect to the internal representation of such labelled rules.

#include <mod/lib/RC/Compose.hpp>
#include <mod/lib/RC/Visitor/MatchConstraints.hpp>
#include <mod/lib/RC/Visitor/Stereo.hpp>
#include <mod/lib/RC/Visitor/String.hpp>
#include <mod/lib/RC/Visitor/Term.hpp>

namespace mod::lib::RC {
namespace detail {

template<LabelType T>
struct LabelTypeToVisitor;

template<>
struct LabelTypeToVisitor<LabelType::String> {
	static Visitor::String make(const lib::rule::LabelledRule &rFirst, const lib::rule::LabelledRule &rSecond) {
		return {rFirst, rSecond};
	}
};

template<>
struct LabelTypeToVisitor<LabelType::Term> {
	static Visitor::Term make(const lib::rule::LabelledRule &rFirst, const lib::rule::LabelledRule &rSecond) {
		return {rFirst, rSecond};
	}
};

template<bool withStereo>
struct WithStereoVisitor;

template<>
struct WithStereoVisitor<true> {
	static Visitor::Stereo make(const lib::rule::LabelledRule &rFirst, const lib::rule::LabelledRule &rSecond) {
		return {rFirst, rSecond};
	}
};

template<>
struct WithStereoVisitor<false> {
	static Visitor::Null make(const lib::rule::LabelledRule &rFirst, const lib::rule::LabelledRule &rSecond) {
		return {};
	}
};

} // namespace detail

template<typename Result, typename LabelledRuleFirst, typename LabelledRuleSecond,
         typename InvertibleVertexMap, typename VisitorT = Visitor::Null>
bool composeLabelled(IO::Logger logger, const bool verbose, Result &result,
                     const LabelledRuleFirst &rFirst, const LabelledRuleSecond &rSecond,
                     InvertibleVertexMap &match, VisitorT visitor = Visitor::Null()) {
	using HasTerm = GraphMorphism::HasTermData<InvertibleVertexMap>;
	using HasStereo = GraphMorphism::HasStereoData<InvertibleVertexMap>;
	constexpr LabelType labelType = HasTerm::value ? LabelType::Term : LabelType::String;
	return compose(
			logger, verbose, result, rFirst.getRule(), rSecond.getRule(), match,
			Visitor::makeVisitor(
					std::move(visitor),
					detail::LabelTypeToVisitor<labelType>::make(rFirst, rSecond),
					detail::WithStereoVisitor<HasStereo::value>::make(rFirst, rSecond)
					));
}

} // namespace mod::lib::RC

#endif // MOD_LIB_RC_LABELLEDCOMPOSITION_HPP