#ifndef MOD_LIB_RULES_GRAPHTORULE_HPP
#define MOD_LIB_RULES_GRAPHTORULE_HPP

#include <mod/lib/Rule/Rule.hpp>
#include <mod/lib/Stereo/CloneUtil.hpp>

#include <ostream>

namespace mod::lib::rule {

template<typename LGraph>
std::unique_ptr<Rule> graphToRule(const LGraph &lg, Membership membership, const std::string &name) {
	using EdgeCat = lib::Stereo::EdgeCategory;
	const auto &g = get_graph(lg);
	const auto &pStringGraph = get_string(lg);
	assert(membership == Membership::L || membership == Membership::K || membership == Membership::R);
	auto cRule = std::make_unique<lib::DPO::CombinedRule>();
	auto &gCore = cRule->getCombinedGraph();
	auto pStringPtr = std::make_unique<PropString>(*cRule);
	auto &pString = *pStringPtr;

	for(const auto v: asRange(vertices(g))) {
		const auto vCore = add_vertex(gCore);
		assert(get(boost::vertex_index_t(), g, v) == get(boost::vertex_index_t(), gCore, vCore));
		gCore[vCore].membership = membership;
		const auto &label = pStringGraph[v];
		switch(membership) {
		case Membership::L:
			pString.add(vCore, label, "");
			break;
		case Membership::R:
			pString.add(vCore, "", label);
			break;
		case Membership::K:
			pString.add(vCore, label, label);
			break;
		}
	}

	for(const auto e: asRange(edges(g))) {
		const auto vSrcCore = vertex(get(boost::vertex_index_t(), g, source(e, g)), gCore);
		const auto vTarCore = vertex(get(boost::vertex_index_t(), g, target(e, g)), gCore);
		const auto eCore = add_edge(vSrcCore, vTarCore, {membership}, gCore).first;
		assert(get(boost::edge_index_t(), g, e) == get(boost::edge_index_t(), gCore, eCore));
		const auto &label = pStringGraph[e];
		switch(membership) {
		case Membership::L:
			pString.add(eCore, label, "");
			break;
		case Membership::R:
			pString.add(eCore, "", label);
			break;
		case Membership::K:
			pString.add(eCore, label, label);
			break;
		}
	}

	std::unique_ptr<PropStereo> pStereoPtr;
	if(has_stereo(lg)) {
		const auto &pStereoGraph = get_stereo(lg);
		std::vector<EdgeCat> eStereo(num_edges(g));

		for(const auto e: asRange(edges(g))) {
			const auto eId = get(boost::edge_index_t(), g, e);
			const auto eCore = *std::next(edges(gCore).first, eId); // TODO: get rid of O(n) lookup
			const auto eIdCore = get(boost::edge_index_t(), gCore, eCore);
			assert(eId == eIdCore);
			eStereo[eIdCore] = pStereoGraph[e];
		}

		const auto vertexMap = [&gCore, &lg](const auto &vCore) {
			const auto vIdCore = get(boost::vertex_index_t(), gCore, vCore);
			const auto &g = get_graph(lg);
			return vertex(vIdCore, g);
		};
		const auto edgeMap = [&gCore, &lg](const auto &eCore) {
			const auto &g = get_graph(lg);
			const auto eIdCore = get(boost::edge_index_t(), gCore, eCore);
			const auto e = *std::next(edges(g).first, eIdCore);
			assert(get(boost::edge_index_t(), g, e) == eIdCore);
			return e;
		};
		const auto inf = Stereo::makeCloner(lg, gCore, vertexMap, edgeMap);
		pStereoPtr = std::make_unique<PropStereo>(*cRule, inf, inf, jla_boost::AlwaysTrue(),
		                                          jla_boost::AlwaysTrue());
	}

	std::string completeName;
	switch(membership) {
	case Membership::L:
		completeName += "unbind";
		break;
	case Membership::K:
		completeName += "id";
		break;
	case Membership::R:
		completeName += "bind";
		break;
	default:
		MOD_ABORT;
		break;
	}
	completeName += "<";
	completeName += name;
	completeName += ">";
	LabelledRule lRule(std::move(cRule), std::move(pStringPtr), std::move(pStereoPtr));
	auto res = std::make_unique<Rule>(std::move(lRule), std::nullopt);
	res->setName(completeName);
	return res;
}

} // namespace mod::lib::rule

#endif // MOD_LIB_RULES_GRAPHTORULE_HPP