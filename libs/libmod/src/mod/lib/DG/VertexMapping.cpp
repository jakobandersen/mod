#include "VertexMapping.hpp"

#include <mod/Error.hpp>
#include <mod/Post.hpp>
#include <mod/rule/Rule.hpp>
#include <mod/lib/DG/Hyper.hpp>
#include <mod/lib/Rules/Real.hpp>
#include <mod/lib/Rules/IO/Write.hpp>

namespace GM = jla_boost::GraphMorphism;
namespace GM_MOD = mod::lib::GraphMorphism;

namespace mod::lib::DG::VertexMapping {

const lib::Rules::Real &Input::getG() const {
	if(!idG) idG = lib::Rules::graphToRule(lugG, lib::Rules::Membership::K, "G");
	return *idG;
}

const lib::Rules::Real &Input::getH() const {
	if(!idH) idH = lib::Rules::graphToRule(lugH, lib::Rules::Membership::K, "H");
	return *idH;
}

} // namespace mod::lib::DG::VertexMapping
namespace mod::lib::DG {

VertexMappingResult
calculateVertexMaps(const Hyper &dg, const HyperVertex v, IO::Logger logger, const int verbosity) {
	return calculateVertexMaps(dg, v, true, 1 << 30, 1, logger, verbosity);
}

VertexMappingResult
calculateVertexMaps(const Hyper &dg, const HyperVertex v,
                    const bool upToIsomorphismG, const int leftLimit, const int rightLimit,
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
	VertexMapping::Input input;
	for(const auto vAdj: res.leftDGVertices)
		input.lugG.push_back(&dgGraph[vAdj].graph->getLabelledGraph());
	for(const auto vAdj: res.rightDGVertices)
		input.lugH.push_back(&dgGraph[vAdj].graph->getLabelledGraph());
	res.gLeft = get_graph(input.lugG);
	res.gRight = get_graph(input.lugH);

	for(const auto *rDerivation: dg.getRulesFromEdge(v)) {
		auto logger = loggerOrig;
		assert(rDerivation);
		if(verbosity != 0) {
			logger.indent() << "calculateVertexMaps: with rule " << rDerivation->getName() << std::endl;
			++logger.indentLevel;
		}
		input.r = rDerivation;
		VertexMapping::foreachRuleMatchedToGandH(
				ls, input, logger, verbosity, upToIsomorphismG, rightLimit,
				[ls, leftLimit, verbosity, &input, &res](IO::Logger logger, const Rules::Real *rPtr, auto &&mapRight) {
					if(verbosity != 0) {
						logger.indent() << "calculateVertexMaps(leftLimit=" << leftLimit << "): got span" << std::endl;
						++logger.indentLevel;
					}

					const auto &&Lunwrapped = get_labelled_left(rPtr->getDPORule());
					LabelledFilteredGraph L(Lunwrapped);
					auto mr = GM::makeLimit(leftLimit, [verbosity, &input, &logger, &rPtr, &mapRight, &L, &res](
							const auto &mapLeft, const auto &gDom, const auto &gCodom) {
						if(verbosity != 0) logger.indent() << "got isomorphism to G, making map" << std::endl;

						const auto &&Runwrapped = get_labelled_right(rPtr->getDPORule());
						LabelledFilteredGraph R(Runwrapped);
						const auto &core = rPtr->getGraph();
						VertexMappingResult::Map map(res.gLeft, res.gRight);
						for(const auto vLeft: asRange(vertices(res.gLeft))) {
							const auto vCore = get(mapLeft, res.gLeft, get_graph(L), vLeft);
							assert(vCore != core.null_vertex());
							assert(core[vCore].membership != lib::Rules::Membership::R);
							if(core[vCore].membership == lib::Rules::Membership::L) continue;
							const auto vRight = get(mapRight, get_graph(R), res.gRight, vCore);
							assert(vRight != res.gRight.null_vertex());
							put(map, res.gLeft, res.gRight, vLeft, vRight);
						}
						res.maps.emplace_back(input.r, std::move(map));
						return true;
					});
					// the Limit callback has state, so send it by reference
					lib::GraphMorphism::morphismSelectByLabelSettings(
							input.lugG, L, ls, GraphMorphism::VF2Isomorphism(), std::ref(mr));
					return true;
				});
	} // for each derivation rule
	return res;
}

} // namespace mod::lib::DG