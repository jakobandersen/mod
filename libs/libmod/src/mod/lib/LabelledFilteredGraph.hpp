#ifndef MOD_LIB_LABELLEDFILTEREDGRAPH_HPP
#define MOD_LIB_LABELLEDFILTEREDGRAPH_HPP

#include <mod/lib/LabelledGraph.hpp>

#include <jla_boost/graph/FilteredWrapper.hpp>

namespace mod::lib {

template<typename LGraph>
struct LabelledFilteredGraph {
	using InnerGraph = typename LabelledGraphTraits<LGraph>::GraphType;
public:
	using GraphType = jla_boost::FilteredWrapper<InnerGraph>;
	using PropStringType = typename LabelledGraphTraits<LGraph>::PropStringType;
	using PropTermType = typename LabelledGraphTraits<LGraph>::PropTermType;
	using PropStereoType = typename LabelledGraphTraits<LGraph>::PropStereoType;
public:
	LabelledFilteredGraph(const LGraph &lg) : lg(lg), g(jla_boost::makeFilteredWrapper(get_graph(lg))) {}

	friend const GraphType &get_graph(const LabelledFilteredGraph &g) {
		return g.g;
	}

	friend PropStringType get_string(const LabelledFilteredGraph &g) {
		return get_string(g.lg);
	}

	friend PropTermType get_term(const LabelledFilteredGraph &g) {
		return get_term(g.lg);
	}

	friend bool has_stereo(const LabelledFilteredGraph &g) {
		return has_stereo(g.lg);
	}

	friend PropStereoType get_stereo(const LabelledFilteredGraph &g) {
		return get_stereo(g.lg);
	}
private:
	const LGraph &lg;
	const GraphType g;
};


} // namespace mod::lib

#endif // MOD_LIB_LABELLEDFILTEREDGRAPH_HPP