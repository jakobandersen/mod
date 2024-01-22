#ifndef MOD_LIB_RC_LABELLEDCOMPOSITION_HPP
#define MOD_LIB_RC_LABELLEDCOMPOSITION_HPP

// This file is specific to the label settings we use.

#include <mod/Config.hpp>
#include <mod/Error.hpp>
#include <mod/lib/RC/Compose.hpp>
#include <mod/lib/RC/Visitor/MatchConstraints.hpp>
#include <mod/lib/RC/Visitor/Stereo.hpp>
#include <mod/lib/RC/Visitor/String.hpp>
#include <mod/lib/RC/Visitor/Term.hpp>
#include <mod/lib/Rules/LabelledRule.hpp>

namespace mod::lib::RC {
namespace detail {

template<bool Verbose, typename Result, typename InvertibleVertexMap, typename VisitorT>
bool composeLabelledFinallyDoIt(IO::Logger logger, Result &result,
                                const lib::Rules::LabelledRule &rFirst,
                                const lib::Rules::LabelledRule &rSecond,
                                InvertibleVertexMap &match, VisitorT visitor) {
	return compose<Verbose>(logger, result, rFirst.getRule(), rSecond.getRule(), match, std::move(visitor));
}

template<LabelType T>
struct LabelTypeToVisitor;

template<>
struct LabelTypeToVisitor<LabelType::String> {
	static Visitor::String make(const lib::Rules::LabelledRule &rFirst, const lib::Rules::LabelledRule &rSecond) {
		return {rFirst, rSecond};
	}
};

template<>
struct LabelTypeToVisitor<LabelType::Term> {
	static Visitor::Term make(const lib::Rules::LabelledRule &rFirst, const lib::Rules::LabelledRule &rSecond) {
		return {rFirst, rSecond};
	}
};

template<bool withStereo>
struct WithStereoVisitor;

template<>
struct WithStereoVisitor<true> {
	static Visitor::Stereo make(const lib::Rules::LabelledRule &rFirst, const lib::Rules::LabelledRule &rSecond) {
		return {rFirst, rSecond};
	}
};

template<>
struct WithStereoVisitor<false> {
	static Visitor::Null make(const lib::Rules::LabelledRule &rFirst, const lib::Rules::LabelledRule &rSecond) {
		return {};
	}
};

} // namespace detail

template<bool Verbose, LabelType labelType, bool withStereo,
		typename Result, typename InvertibleVertexMap, typename VisitorT = Visitor::Null>
bool composeLabelled(IO::Logger logger, Result &result,
                     const lib::Rules::LabelledRule &rFirst, const lib::Rules::LabelledRule &rSecond,
                     InvertibleVertexMap &match,
                     VisitorT visitor = Visitor::Null()) {
	return detail::composeLabelledFinallyDoIt<Verbose>(
			logger, result, rFirst, rSecond, match,
			Visitor::makeVisitor(
					std::move(visitor),
					detail::LabelTypeToVisitor<labelType>::make(rFirst, rSecond),
					detail::WithStereoVisitor<withStereo>::make(rFirst, rSecond)
			));
}

} // namespace mod::lib::RC

#endif // MOD_LIB_RC_LABELLEDCOMPOSITION_HPP