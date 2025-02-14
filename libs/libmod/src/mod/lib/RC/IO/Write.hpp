#ifndef MOD_LIB_RC_IO_WRITE_HPP
#define MOD_LIB_RC_IO_WRITE_HPP

#include <mod/lib/Rule/Rule.hpp>

#include <boost/bimap.hpp>

#include <string>

namespace mod::lib::RC {
struct Evaluator;
} // namespace mod::lib::RC
namespace mod::lib::RC::Write {
std::string dot(const Evaluator &rc);
std::string svg(const Evaluator &rc);
std::string pdf(const Evaluator &rc);

using CoreCoreMap = boost::bimap<lib::rule::Vertex, lib::rule::Vertex>;

void test(const lib::rule::Rule &rFirst, const lib::rule::Rule &rSecond, const CoreCoreMap &match, const lib::rule::Rule &rNew);

template<typename VertexMap>
void test(const lib::rule::Rule &rFirst, const lib::rule::Rule &rSecond, const VertexMap &m, const lib::rule::Rule &rNew) {
	using GraphDom = lib::rule::LabelledRule::SideProjectedGraphType;
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

} // namespace mod::lib::RC::Write

#endif // MOD_LIB_RC_IO_WRITE_HPP