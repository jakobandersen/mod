#ifndef MOD_LIB_DG_VERTEXMAPPING_HPP
#define MOD_LIB_DG_VERTEXMAPPING_HPP

#include <mod/lib/Algorithm/Container.hpp>
#include <mod/lib/DG/GraphDecl.hpp>
#include <mod/lib/Graph/LabelledGraph.hpp>
#include <mod/lib/Graph/Properties/Stereo.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/Graph/Properties/Term.hpp>
#include <mod/lib/GraphMorphism/LabelledMorphism.hpp>
#include <mod/lib/GraphMorphism/VF2Finder.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/LabelledFilteredGraph.hpp>
#include <mod/lib/LabelledUnionGraph.hpp>
#include <mod/lib/RC/ComposeFromMatchMaker.hpp>
#include <mod/lib/RC/MatchMaker/Sub.hpp>
#include <mod/lib/RC/MatchMaker/Super.hpp>
#include <mod/lib/Rule/GraphToRule.hpp>
#include <mod/lib/Rule/LabelledRule.hpp>

namespace mod::lib::rule {
struct Rule;
} // namespace mod::lib::rule
namespace mod::lib::DG::VertexMapping {

template<typename OnResult>
// rGId: G <- G -> G
// r: L <- K -> R
// make all rcSuper(allowPartial=False) compositions, each yielding
// rGDH: G <- D -> H
// with mResult containing maps rGId <-> rGDB <-> r
//
// OnResult: (logger, rGDH, mResult) -> bool for "continue"
std::vector<std::unique_ptr<rule::Rule>>
foreachRuleMatchedToG(const LabelSettings ls, const lib::rule::Rule &r, const lib::rule::Rule &rGId,
                      IO::Logger logger, const int verbosity,
                      const bool upToIsomorphism, OnResult onResult) {
	if(verbosity != 0) {
		logger.indent() << "VertexMapping::foreachG(upToIsomorphism=" << std::boolalpha
				<< upToIsomorphism << "):" << std::endl;
		++logger.indentLevel;
	}
	std::vector<std::unique_ptr<rule::Rule>> res;
	lib::RC::Super mm(verbosity - 10 + 2 /* the same as in RC Evaluator.cpp */, logger, false, true);
	lib::RC::composeFromMatchMaker(
			rGId, r, mm,
			[ls, loggerOrig = logger, verbosity, upToIsomorphism, onResult, &res](
			std::unique_ptr<rule::Rule> rGDHPtr, const lib::RC::ResultMaps &mResult) {
				auto logger = loggerOrig;
				if(verbosity != 0) {
					logger.indent() << "VertexMapping::foreachG(upToIsomorphism=" << std::boolalpha
							<< upToIsomorphism << "): got candidate rule" << std::endl;
					++logger.indentLevel;
				}
				if(upToIsomorphism) {
					for(const auto &rOther: res) {
						if(lib::rule::makeIsomorphismPredicate(ls.type, ls.withStereo)(rOther, rGDHPtr)) {
							if(verbosity != 0)
								logger.indent() << "VertexMapping::foreachG(upToIsomorphism=" << std::boolalpha
										<< upToIsomorphism << "): isomorphic rule already found" << std::endl;
							return true; // we know it already, find the next
						}
					}
				}
				const bool cont = onResult(logger, *rGDHPtr, mResult);
				if(verbosity != 0)
					logger.indent() << "VertexMapping::foreachG(upToIsomorphism=" << std::boolalpha
							<< upToIsomorphism << "): onResult=" << cont << std::endl;
				if(upToIsomorphism)
					res.push_back(std::move(rGDHPtr));
				return cont;
			}, ls);
	return res;
}

template<typename OnResult>
// rGId: G <- G -> G
// r: L <- K -> R
// make all rcSuper(allowPartial=False) compositions, each yielding
// rGDH: G <- D -> H
// with mResult containing maps rGId <-> rGDB <-> r
// then make all isomorphisms H <-> lugH, each yielding mHtoHInput
//
// OnResult: (logger, rGDH, mResult, mHtoHInput) -> bool for "continue"
std::vector<std::unique_ptr<rule::Rule>>
foreachRuleMatchedToGandH(const LabelSettings ls, const lib::rule::Rule &r, const lib::rule::Rule &rGId,
                          const LabelledUnionGraph<lib::graph::LabelledGraph> &lugH,
                          IO::Logger logger, const int verbosity,
                          const bool upToIsomorphismGDH, const int rightLimit, OnResult onResult) {
	return foreachRuleMatchedToG(
			ls, r, rGId, logger, verbosity, upToIsomorphismGDH,
			[ls, &lugH, verbosity, rightLimit, onResult](
			IO::Logger logger, const rule::Rule &rGDH, const lib::RC::ResultMaps &mResult) {
				if(verbosity != 0) {
					logger.indent() << "VertexMapping::foreachGandH(rightLimit=" << rightLimit << "):"
							<< std::endl;
					++logger.indentLevel;
				}
				const auto &&Hunwrapped = get_labelled_right(rGDH.getDPORule());
				LabelledFilteredGraph H(Hunwrapped);
				// The request from the user needs both to go into the morphism enumeration,
				// and then when no more morphisms, then the last request goes to the next outer enumeration.
				bool cont = true;
				auto mr = jla_boost::GraphMorphism::makeLimit(
						rightLimit,
						[loggerOrig = logger, verbosity, rightLimit, onResult, &rGDH, &
							mResult, &cont
						](
						const auto &mHtoHInput, const auto &gDom, const auto &gCodom) -> bool {
							auto logger = loggerOrig;
							if(verbosity != 0) {
								logger.indent()
										<< "VertexMapping::foreachGandH(rightLimit="
										<< rightLimit
										<< "): got isomorphism to H" << std::endl;
								++logger.indentLevel;
							}
							// and now we let the user decide if we should pull another solution
							cont = onResult(logger, rGDH, mResult, mHtoHInput);
							if(verbosity != 0)
								logger.indent()
										<< "VertexMapping::foreachGandH(rightLimit="
										<< rightLimit
										<< "): onResult=" << cont << std::endl;
							return cont;
						});
				// the Limit callback has state, so send it by reference
				lib::GraphMorphism::morphismSelectByLabelSettings(
						H, lugH, ls, GraphMorphism::VF2Isomorphism(), std::ref(mr));
				if(verbosity != 0)
					logger.indent() << "VertexMapping::foreachGandH(rightLimit=" << rightLimit
							<< "): done, returning " << std::boolalpha << cont << std::endl;
				return cont;
			});
}

} // namespace mod::lib::DG::VertexMapping
namespace mod::lib::DG {
struct Hyper;

struct VertexMappingResult {
	using Graph = jla_boost::union_graph<lib::graph::GraphType>;
	using Map = jla_boost::GraphMorphism::InvertibleVectorVertexMap<Graph, Graph>;
	using Match = jla_boost::GraphMorphism::InvertibleVectorVertexMap<lib::DPO::CombinedRule::SideGraphType, Graph>;

	struct Entry {
		const lib::rule::Rule *r;
		Map mGH;
		Match mLG, mRH;
	};
public:
	std::vector<HyperVertex> leftDGVertices, rightDGVertices;
	Graph gLeft, gRight;
	std::vector<Entry> maps;
};

VertexMappingResult calculateVertexMaps(const Hyper &dg, HyperVertex v, IO::Logger logger, const int verbosity);
VertexMappingResult calculateVertexMaps(const Hyper &dg, HyperVertex v,
                                        bool upToIsomorphismGDH, int rightLimit,
                                        IO::Logger logger, const int verbosity);

} // namespace mod::lib::DG

#endif // MOD_LIB_DG_VERTEXMAPPING_HPP