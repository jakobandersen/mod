#ifndef MOD_LIB_IO_RC_HPP
#define MOD_LIB_IO_RC_HPP

#include <mod/lib/Rules/Real.hpp>

#include <boost/bimap.hpp>

#include <string>

namespace mod::lib::RC {
struct Evaluator;
} // namespace mod::lib::RC
namespace mod::lib::IO::RC::Write {
std::string dot(const lib::RC::Evaluator &rc);
std::string svg(const lib::RC::Evaluator &rc);
std::string pdf(const lib::RC::Evaluator &rc);

using CoreCoreMap = boost::bimap<lib::Rules::Vertex, lib::Rules::Vertex>;

void test(const lib::Rules::Real &rFirst, const lib::Rules::Real &rSecond, const CoreCoreMap &match, const lib::Rules::Real &rNew);

template<typename VertexMap>
void test(const lib::Rules::Real &rFirst, const lib::Rules::Real &rSecond, const VertexMap &m, const lib::Rules::Real &rNew) {
	using GraphDom = lib::Rules::LabelledRule::LeftGraphType;
	const auto &gDom = get_graph(get_labelled_left(rSecond.getDPORule()));
	const auto &gCodom = get_graph(get_labelled_right(rFirst.getDPORule()));
	CoreCoreMap match;
	for(const auto vCodom : asRange(vertices(gCodom))) {
		const auto vDom = get_inverse(m, gDom, gCodom, vCodom);
		if(vDom == boost::graph_traits<GraphDom>::null_vertex()) continue;
		match.insert(CoreCoreMap::relation(vCodom, vDom));
	}
	test(rFirst, rSecond, match, rNew);
}

} // namespace mod::lib::IO::RC::Write

#endif // MOD_LIB_IO_RC_HPP