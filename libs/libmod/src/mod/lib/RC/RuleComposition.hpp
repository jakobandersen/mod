#ifndef MOD_LIB_RC_RULECOMPOSITION_HPP
#define MOD_LIB_RC_RULECOMPOSITION_HPP

// Composition of lib::rule::LabelledRule x lib::rule::LabelledRule -> lib::rule::LabelledRule
// Composition of lib::rule::Rule x lib::rule::Rule -> lib::rule::Rule

#include <mod/lib/Rule/Rule.hpp>
#include <mod/lib/RC/LabelledComposition.hpp>
#include <mod/lib/RC/LabelledResult.hpp>
#include <mod/lib/RC/IO/Write.hpp>

#include <optional>

namespace mod::lib::RC {

template<typename InvertibleVertexMap>
std::optional<std::pair<lib::rule::LabelledRule, ResultMaps>>
composeRules(IO::Logger logger, const bool verbose,
             const lib::rule::LabelledRule &rFirst, const lib::rule::LabelledRule &rSecond,
             InvertibleVertexMap &match) {
	LabelledResult result(rFirst.getRule(), rSecond.getRule());

	using HasTerm = GraphMorphism::HasTermData<InvertibleVertexMap>;
	constexpr LabelType labelType = HasTerm::value ? LabelType::Term : LabelType::String;
	const bool success = composeLabelled(
			logger, verbose, result, rFirst, rSecond, match,
			Visitor::MatchConstraints<labelType>(rFirst, rSecond));
	if(!success) return {};
	return std::move(result).createRule();
}

namespace detail {

template<typename GraphDom, typename GraphCodom, typename VertexMap>
void printMap(std::ostream &s, const GraphDom &gDom, const GraphCodom &gCodom, const VertexMap &m, bool swap) {
	bool first = true;
	for(const auto vDom: asRange(vertices(gDom))) {
		const auto vCodom = get(m, gDom, gCodom, vDom);
		if(vCodom == gCodom.null_vertex()) continue;
		if(first) {
			s << " ";
			first = false;
		} else s << ", ";
		const auto idxDom = get(boost::vertex_index_t(), gDom, vDom);
		const auto idxCodom = get(boost::vertex_index_t(), gCodom, vCodom);
		if(swap) s << "(" << idxCodom << ", " << idxDom << ")";
		else s << "(" << idxDom << ", " << idxCodom << ")";
	}
}

} // namespace detail

template<typename InvertibleVertexMap>
std::optional<std::pair<std::unique_ptr<lib::rule::Rule>, ResultMaps>>
composeRules(IO::Logger logger, const bool verbose,
             const lib::rule::Rule &rFirst, const lib::rule::Rule &rSecond, InvertibleVertexMap &match) {
	using HasTerm = GraphMorphism::HasTermData<InvertibleVertexMap>;
	using HasStereo = GraphMorphism::HasStereoData<InvertibleVertexMap>;
	if(verbose) {
		logger.indent() << "RuleComp(" << rFirst.getName() << ", " << rSecond.getName() << "):" << std::endl;
		++logger.indentLevel;
		logger.indent() << "hasTerm=" << std::boolalpha << HasTerm::value << ", hasStereo=" << HasStereo::value
				<< std::endl;
		logger.indent() << "map:";
		const auto &gFirst = get_R_projected(rFirst.getDPORule());
		const auto &gSecond = get_L_projected(rSecond.getDPORule());
		detail::printMap(logger.s, gSecond, gFirst, match, true);
		logger.s << std::endl;
	}
	auto resultOpt = composeRules(
			logger, verbose, rFirst.getDPORule(), rSecond.getDPORule(), match);
	if(!resultOpt) {
		if(verbose)
			logger.indent() << "RuleComp(" << rFirst.getName() << ", " << rSecond.getName() << ") failed" << std::endl;
		return {};
	}
	constexpr LabelType labelType = HasTerm::value ? LabelType::Term : LabelType::String;
	auto rResult = std::make_unique<lib::rule::Rule>(std::move(resultOpt->first), labelType);
	if(verbose) {
		logger.indent() << "RuleComp(" << rFirst.getName() << ", " << rSecond.getName()
				<< ") success: result=" << rResult->getName() << std::endl;
		logger.indent(1) << "mapFirstToResult:";
		detail::printMap(logger.s,
		                 get_graph(rFirst.getDPORule()),
		                 get_graph(rResult->getDPORule()),
		                 resultOpt->second.mFirstToResult, false);
		logger.s << std::endl;
		logger.indent(1) << "mapSecondToResult:";
		detail::printMap(logger.s,
		                 rSecond.getDPORule().getRule().getCombinedGraph(),
		                 rResult->getDPORule().getRule().getCombinedGraph(),
		                 resultOpt->second.mSecondToResult, false);
		logger.s << std::endl;
	}
	if(getConfig().rc.printMatches)
		RC::Write::test(rFirst, rSecond, match, *rResult);
	return {{std::move(rResult), std::move(resultOpt->second)}};
}

} // namespoce mod::lib::RC

#endif // MOD_LIB_RC_RULECOMPOSITION_HPP