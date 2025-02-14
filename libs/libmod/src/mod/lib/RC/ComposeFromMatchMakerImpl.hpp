#ifndef MOD_LIB_RC_COMPOSE_FROM_MATCH_MAKER_IMPL_HPP
#define MOD_LIB_RC_COMPOSE_FROM_MATCH_MAKER_IMPL_HPP

#include <mod/Config.hpp>
#include <mod/Misc.hpp>
#include <mod/lib/GraphMorphism/LabelledMorphism.hpp>
#include <mod/lib/RC/RuleComposition.hpp>
#include <mod/lib/RC/IO/Write.hpp>

namespace mod::lib::RC {

template<typename MatchMaker>
void composeFromMatchMakerImpl(const lib::rule::Rule &rFirst,
                               const lib::rule::Rule &rSecond,
                               MatchMaker mm,
                               std::function<bool(std::unique_ptr<lib::rule::Rule>, const ResultMaps &)> rr,
                               LabelSettings labelSettings) {
	if(getConfig().rc.printMatches)
		IO::post() << "summarySection \"RC Matches\"\n";
	mm.makeMatches(rFirst, rSecond,
	               [&rr](const lib::rule::Rule &rFirst, const lib::rule::Rule &rSecond, auto &&m,
	                     const bool verbose, IO::Logger logger) {
		               auto resultOpt = composeRules(logger, verbose, rFirst, rSecond, m);
		               if(!resultOpt) return true;
		               auto &[rResult, mResult] = *resultOpt;
		               return rr(std::move(rResult), mResult);
	               },
	               labelSettings);
}

} // namespace mod::lib::RC

#endif // MOD_LIB_RC_COMPOSE_FROM_MATCH_MAKER_IMPL_HPP