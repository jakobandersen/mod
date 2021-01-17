#ifndef MOD_LIB_RC_COMMONSG_HPP
#define MOD_LIB_RC_COMMONSG_HPP

#include <mod/lib/GraphMorphism/LabelledMorphism.hpp>
#include <mod/lib/GraphMorphism/McGregorCommonFinder.hpp>
#include <mod/lib/RC/MatchMaker/LabelledMatch.hpp>
#include <mod/lib/Rules/Real.hpp>

#include <jla_boost/graph/morphism/callbacks/Unwrapper.hpp>
#include <jla_boost/graph/morphism/models/Vector.hpp>

namespace mod::lib::RC {

struct Common {
	Common(int verbosity, IO::Logger logger, bool maximum, bool connected)
			: verbosity(verbosity), logger(logger), maximum(maximum), connected(connected) {}

	template<typename Callback>
	void makeMatches(const lib::Rules::Real &rFirst,
	                 const lib::Rules::Real &rSecond,
	                 Callback callback,
	                 LabelSettings labelSettings) {
		using MapImpl = std::vector<lib::Rules::Vertex>;
		std::vector<MapImpl> maps;
		const auto mr = [&rFirst, &rSecond, &callback, this, &maps](auto &&m, const auto &gSecond, const auto &gFirst) -> bool {
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
		auto finder = lib::GraphMorphism::McGregorCommonFinder(maximum, connected);
		if(labelSettings.relation == LabelRelation::Specialisation) {
			MOD_ABORT;
		}
		if(labelSettings.withStereo && labelSettings.stereoRelation == LabelRelation::Specialisation) {
			MOD_ABORT;
		}
		lib::GraphMorphism::morphismSelectByLabelSettings(lgDom, lgCodom, labelSettings, finder, mr);
	}
private:
	const int verbosity;
	IO::Logger logger;
	const bool maximum;
	const bool connected;
};

} // namespace mod::lib::RC

#endif // MOD_LIB_RC_COMMONSG_HPP