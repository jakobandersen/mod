#include "VertexMapping.hpp"

#include <mod/Post.hpp>
#include <mod/lib/DG/Hyper.hpp>
#include <mod/lib/Rule/Rule.hpp>
#include <mod/lib/RC/LabelledResult.hpp>

namespace GM = jla_boost::GraphMorphism;
namespace GM_MOD = mod::lib::GraphMorphism;

namespace mod::lib::DG {

VertexMappingResult
calculateVertexMaps(const Hyper &dg, const HyperVertex v, IO::Logger logger, const int verbosity) {
	return calculateVertexMaps(dg, v, true, 1 << 30, logger, verbosity);
}

VertexMappingResult
calculateVertexMaps(const Hyper &dg, const HyperVertex v,
                    const bool upToIsomorphismGDH, const int rightLimit,
                    IO::Logger loggerOrig, const int verbosity) {
	VertexMappingResult res;
	const auto &dgGraph = dg.getGraph();
	assert(dgGraph[v].kind == HyperVertexKind::Edge);
	const auto invAdj = inv_adjacent_vertices(v, dgGraph);
	const auto adj = adjacent_vertices(v, dgGraph);
	res.leftDGVertices.insert(res.leftDGVertices.begin(), invAdj.first, invAdj.second);
	res.rightDGVertices.insert(res.rightDGVertices.begin(), adj.first, adj.second);
	const auto vLess = [&dgGraph](HyperVertex a, HyperVertex b) {
		return get(boost::vertex_index_t(), dgGraph, a) < get(boost::vertex_index_t(), dgGraph, b);
	};
	std::sort(begin(res.leftDGVertices), end(res.leftDGVertices), vLess);
	std::sort(begin(res.rightDGVertices), end(res.rightDGVertices), vLess);

	const auto ls = dg.getNonHyper().getLabelSettings();
	LabelledUnionGraph<lib::graph::LabelledGraph> lugG, lugH;
	for(const auto vAdj: res.leftDGVertices)
		lugG.push_back(&dgGraph[vAdj].graph->getLabelledGraph());
	for(const auto vAdj: res.rightDGVertices)
		lugH.push_back(&dgGraph[vAdj].graph->getLabelledGraph());
	res.gLeft = get_graph(lugG);
	res.gRight = get_graph(lugH);
	std::unique_ptr<lib::rule::Rule> rGIdPtr = lib::rule::graphToRule(lugG, lib::rule::Membership::K, "G");
	const auto &rGId = *rGIdPtr;

	for(const auto *r: dg.getRulesFromEdge(v)) {
		auto logger = loggerOrig;
		assert(r);
		if(verbosity != 0) {
			logger.indent() << "calculateVertexMaps: with rule " << r->getName() << std::endl;
			++logger.indentLevel;
		}
		VertexMapping::foreachRuleMatchedToGandH(
				ls, *r, rGId, lugH, logger, verbosity, upToIsomorphismGDH, rightLimit,
				[verbosity, &r, &rGId, &res](
				IO::Logger logger, const rule::Rule &rGDH, const lib::RC::ResultMaps &mResult, auto &&mHtoHInput) {
					if(verbosity != 0) {
						logger.indent() << "calculateVertexMaps: got span, making vertex maps" << std::endl;
						++logger.indentLevel;
					}

					const auto &rDPO = r->getDPORule().getRule();
					const auto &coreGDH = rGDH.getGraph();
					const auto &&Gunwrapped = get_labelled_left(rGDH.getDPORule());
					const auto &&Hunwrapped = get_labelled_right(rGDH.getDPORule());
					const LabelledFilteredGraph G(Gunwrapped);
					const LabelledFilteredGraph H(Hunwrapped);

					VertexMappingResult::Map map(res.gLeft, res.gRight);
					VertexMappingResult::Match mLG(getL(rDPO), res.gLeft);
					VertexMappingResult::Match mRH(getR(rDPO), res.gRight);
					for(const auto vL: asRange(vertices(getL(rDPO)))) {
						const auto vCG_G =
								get(mResult.mSecondToResult, r->getGraph(), rGDH.getGraph(), vL);
						const auto vCG_Gid = get_inverse(mResult.mFirstToResult, rGId.getGraph(),
						                                 rGDH.getGraph(), vCG_G);
						const auto vId_CG_Gid = get(boost::vertex_index_t(), rGId.getGraph(), vCG_Gid);
						const auto vGInput = vertex(vId_CG_Gid, res.gLeft);
						put(mLG, getL(rDPO), res.gLeft, vL, vGInput);
					}
					for(const auto vR: asRange(vertices(getR(rDPO)))) {
						const auto vCG_H = get(mResult.mSecondToResult, r->getGraph(), rGDH.getGraph(), vR);
						const auto vH = get_inverse(rDPO.getRtoCG(), getR(rDPO), rDPO.getCombinedGraph(), vCG_H);
						const auto vHInput = get(mHtoHInput, get_graph(H), res.gRight, vH);
						put(mRH, getR(rDPO), res.gRight, vR, vHInput);
					}
					for(const auto vLeft: asRange(vertices(res.gLeft))) {
						const auto vIdLeft = get(boost::vertex_index_t(), res.gLeft, vLeft);
						const auto vCG_Gid = vertex(vIdLeft, rGId.getGraph());
						const auto vCoreGDH =
								get(mResult.mFirstToResult, rGId.getGraph(), rGDH.getGraph(), vCG_Gid);
						assert(vCoreGDH != coreGDH.null_vertex());
						assert(coreGDH[vCoreGDH].membership != lib::rule::Membership::R);
						if(coreGDH[vCoreGDH].membership == lib::rule::Membership::L) continue;
						const auto vRight = get(mHtoHInput, get_graph(H), res.gRight, vCoreGDH);
						assert(vRight != res.gRight.null_vertex());
						put(map, res.gLeft, res.gRight, vLeft, vRight);
					}
					res.maps.push_back(VertexMappingResult::Entry{
						r, std::move(map), std::move(mLG), mRH
					});
					return true;
				});
	} // for each derivation rule
	return res;
}

} // namespace mod::lib::DG