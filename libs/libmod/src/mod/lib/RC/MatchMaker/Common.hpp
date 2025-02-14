#ifndef MOD_LIB_RC_COMMONSG_HPP
#define MOD_LIB_RC_COMMONSG_HPP

#include <mod/lib/GraphMorphism/LabelledMorphism.hpp>
#include <mod/lib/GraphMorphism/CommonSubgraphFinder.hpp>
#include <mod/lib/RC/MatchMaker/LabelledMatch.hpp>
#include <mod/lib/Rule/Rule.hpp>

#include <jla_boost/graph/morphism/callbacks/Unwrapper.hpp>
#include <jla_boost/graph/morphism/models/InvertibleVector.hpp>

namespace mod::lib::RC {

struct Common {
	Common(int verbosity, IO::Logger logger, bool maximum, bool connected)
			: verbosity(verbosity), logger(logger), maximum(maximum), connected(connected) {}

	template<typename Callback>
	void makeMatches(const lib::rule::Rule &rFirst,
	                 const lib::rule::Rule &rSecond,
	                 Callback callback,
	                 LabelSettings labelSettings) {
		using MapImpl = std::vector<lib::DPO::CombinedRule::SideVertex>;
		std::vector<MapImpl> maps;
		const auto mr = [&rFirst, &rSecond, &callback, this, &maps]
				(auto &&m, const auto &gSecond, const auto &gFirst) -> bool {
			MapImpl map(num_vertices(gFirst));
			for(const auto v : asRange(vertices(gFirst)))
				map[get(boost::vertex_index_t(), gFirst, v)] = get_inverse(m, gSecond, gFirst, v);
			if(std::find(begin(maps), end(maps), map) != end(maps))
				return true;
			maps.push_back(std::move(map));
			return callback(rFirst, rSecond, std::move(m), verbosity, logger);
		};
		const auto &lgDom = get_labelled_left(rSecond.getDPORule());
		const auto &lgCodom = get_labelled_right(rFirst.getDPORule());
		if(labelSettings.relation == LabelRelation::Specialisation) {
			MOD_ABORT;
		}
		if(labelSettings.withStereo && labelSettings.stereoRelation == LabelRelation::Specialisation) {
			MOD_ABORT;
		}
		if(getConfig().rc.useBoostCommonSubgraph) {
			lib::GraphMorphism::morphismSelectByLabelSettings(
					lgDom, lgCodom, labelSettings,
					lib::GraphMorphism::CommonSubgraphFinder<true>(maximum, connected),
					mr);
		} else {
			if(connected) {
				lib::GraphMorphism::morphismSelectByLabelSettings(
						lgDom, lgCodom, labelSettings,
						lib::GraphMorphism::CommonSubgraphFinder<false>(maximum, connected),
						mr);
			} else {
				lib::GraphMorphism::morphismSelectByLabelSettings(
						lgDom, lgCodom, labelSettings,
						lib::GraphMorphism::CommonSubgraphFinder<false>(maximum, connected),
						[&rFirst, &rSecond, &callback, this]
								(auto &&m, const auto &gSecond, const auto &gFirst) -> bool {
							return callback(rFirst, rSecond, std::move(m), verbosity, logger);
						});
			}
		}
	}
private:
	const int verbosity;
	IO::Logger logger;
	const bool maximum;
	const bool connected;
};

} // namespace mod::lib::RC

#endif // MOD_LIB_RC_COMMONSG_HPP