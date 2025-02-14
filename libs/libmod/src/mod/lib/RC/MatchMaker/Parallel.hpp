#ifndef MOD_LIB_RC_MATCH_MAKER_PARALLEL_HPP
#define MOD_LIB_RC_MATCH_MAKER_PARALLEL_HPP

#include <mod/lib/RC/MatchMaker/LabelledMatch.hpp>
#include <mod/lib/Rule/Rule.hpp>

#include <jla_boost/graph/morphism/models/InvertibleVector.hpp>

namespace mod::lib::RC {

struct Parallel {
	Parallel(int verbosity, IO::Logger logger) : verbosity(verbosity), logger(logger) {}

	template<typename Callback>
	void makeMatches(const lib::rule::Rule &rFirst,
						  const lib::rule::Rule &rSecond,
						  Callback callback,
						  LabelSettings labelSettings) {
		using GraphDom = lib::rule::LabelledRule::SideGraphType;
		using GraphCodom = lib::rule::LabelledRule::SideGraphType;
		using Map = jla_boost::GraphMorphism::InvertibleVectorVertexMap<GraphDom, GraphCodom>;
		const auto &gDom = get_graph(get_labelled_left(rSecond.getDPORule()));
		const auto &gCodom = get_graph(get_labelled_right(rFirst.getDPORule()));
		handleMapByLabelSettings(rFirst, rSecond, Map(gDom, gCodom), callback, labelSettings, verbosity, logger);
	}

private:
	const int verbosity;
	IO::Logger logger;
};

} // namespace mod::lib::RC

#endif // MOD_LIB_RC_MATCH_MAKER_PARALLEL_HPP