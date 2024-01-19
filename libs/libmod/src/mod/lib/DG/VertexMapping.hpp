#ifndef MOD_LIB_DG_VERTEXMAPPING_HPP
#define MOD_LIB_DG_VERTEXMAPPING_HPP

#include <mod/lib/Algorithm/Container.hpp>
#include <mod/lib/Graph/LabelledGraph.hpp>
#include <mod/lib/Graph/Properties/Stereo.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/Graph/Properties/Term.hpp>
#include <mod/lib/GraphMorphism/LabelledMorphism.hpp>
#include <mod/lib/GraphMorphism/VF2Finder.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/LabelledFilteredGraph.hpp>
#include <mod/lib/LabelledUnionGraph.hpp>
#include <mod/lib/RC/ComposeRuleReal.hpp>
#include <mod/lib/RC/MatchMaker/Sub.hpp>
#include <mod/lib/RC/MatchMaker/Super.hpp>
#include <mod/lib/Rules/GraphToRule.hpp>
#include <mod/lib/Rules/LabelledRule.hpp>

namespace mod::lib::Rules {
struct Real;
} // namespace mod::lib::Rules
namespace mod::lib::DG::VertexMapping {

struct Input {
	Input() = default;
	Input(const Rules::Real &r) : r(&r) {}
	const lib::Rules::Real &getG() const;
	const lib::Rules::Real &getH() const;
public:
	const Rules::Real *r = nullptr;
	LabelledUnionGraph<lib::Graph::LabelledGraph> lugG, lugH;
private:
	mutable std::unique_ptr<lib::Rules::Real> idG, idH;
};

template<typename OnResult>
// OnResult: bool(logger, rPtr)
std::vector<std::unique_ptr<Rules::Real>>
foreachRuleMatchedToG(const LabelSettings ls, const Input &input, IO::Logger logger, const int verbosity,
                      const bool upToIsomorphism, OnResult onResult) {
	if(verbosity != 0) {
		logger.indent() << "VertexMapping::foreachG(upToIsomorphism=" << std::boolalpha
		                << upToIsomorphism << "):" << std::endl;
		++logger.indentLevel;
	}
	std::vector<std::unique_ptr<Rules::Real>> res;
	lib::RC::Super mm(verbosity - 10 + 2 /* the same as in RC Evaluator.cpp */, logger, false, true);
	lib::RC::composeRuleRealByMatchMaker(
			input.getG(), *input.r, mm,
			[ls, loggerOrig = logger, verbosity, upToIsomorphism, onResult, &res](std::unique_ptr<Rules::Real> r) {
				auto logger = loggerOrig;
				if(verbosity != 0) {
					logger.indent() << "VertexMapping::foreachG(upToIsomorphism=" << std::boolalpha
					                << upToIsomorphism << "): got candidate rule" << std::endl;
					++logger.indentLevel;
				}
				if(upToIsomorphism) {
					for(const auto &rOther: res) {
						if(lib::Rules::makeIsomorphismPredicate(ls.type, ls.withStereo)(rOther, r)) {
							if(verbosity != 0)
								logger.indent() << "VertexMapping::foreachG(upToIsomorphism=" << std::boolalpha
								                << upToIsomorphism << "): isomorphic rule already found" << std::endl;
							return true; // we know it already, find the next
						}
					}
				}
				res.push_back(std::move(r));
				const bool cont = onResult(logger, res.back().get());
				if(verbosity != 0)
					logger.indent() << "VertexMapping::foreachG(upToIsomorphism=" << std::boolalpha
					                << upToIsomorphism << "): onResult=" << cont << std::endl;
				return cont;
			}, ls);
	return res;
}

template<typename OnResult>
// OnResult: bool(logger, rPtr)
std::vector<std::unique_ptr<Rules::Real>>
foreachRuleMatchedToGandH(const LabelSettings ls, const Input &input, IO::Logger logger, const int verbosity,
                          const bool upToIsomorphismG, const int rightLimit, OnResult onResult) {
	return foreachRuleMatchedToG(
			ls, input, logger, verbosity, upToIsomorphismG,
			[ls, &input, verbosity, rightLimit, onResult](IO::Logger logger, const Rules::Real *rPtr) {
				if(verbosity != 0) {
					logger.indent() << "VertexMapping::foreachGandH(rightLimit=" << rightLimit << "):"
					                << std::endl;
					++logger.indentLevel;
				}
				assert(rPtr);
				const auto &&Runwrapped = get_labelled_right(rPtr->getDPORule());
				LabelledFilteredGraph R(Runwrapped);
				// The request from the user needs both to go into the morphism enumeration,
				// and then when no more morphisms, then the last request goes to the next outer enumeration.
				bool cont = true;
				auto mr = jla_boost::GraphMorphism::makeLimit(
						rightLimit, [loggerOrig = logger, verbosity, rightLimit, onResult, &rPtr, &cont](
								const auto &vm, const auto &gDom,
								const auto &gCodom) -> bool {
							auto logger = loggerOrig;
							if(verbosity != 0) {
								logger.indent()
										<< "VertexMapping::foreachGandH(rightLimit="
										<< rightLimit
										<< "): got isomorphism to H" << std::endl;
								++logger.indentLevel;
							}
							// and now we let the user decide if we should pull another solution
							cont = onResult(logger, rPtr, vm);
							if(verbosity != 0)
								logger.indent()
										<< "VertexMapping::foreachGandH(rightLimit="
										<< rightLimit
										<< "): onResult=" << cont << std::endl;
							return cont;
						});
				// the Limit callback has state, so send it by reference
				lib::GraphMorphism::morphismSelectByLabelSettings(
						R, input.lugH, ls, GraphMorphism::VF2Isomorphism(), std::ref(mr));
				if(verbosity != 0)
					logger.indent() << "VertexMapping::foreachGandH(rightLimit=" << rightLimit
					                << "): done, returning " << std::boolalpha << cont << std::endl;
				return cont;
			});
}

} // namespace mod::lib::DG::VertexMapping

#include <mod/lib/DG/GraphDecl.hpp>

namespace mod::lib::DG {
struct Hyper;

struct VertexMappingResult {
	using Graph = jla_boost::union_graph<lib::Graph::GraphType>;
	using Map = jla_boost::GraphMorphism::InvertibleVectorVertexMap<Graph, Graph>;
public:
	std::vector<HyperVertex> leftDGVertices, rightDGVertices;
	Graph gLeft, gRight;
	std::vector<std::pair<const lib::Rules::Real *, Map>> maps;
};

VertexMappingResult calculateVertexMaps(const Hyper &dg, HyperVertex v, IO::Logger logger, const int verbosity);
VertexMappingResult calculateVertexMaps(const Hyper &dg, HyperVertex v,
                                        bool upToIsomorphismG, int leftLmit, int rightLimit,
                                        IO::Logger logger, const int verbosity);

} // namespace mod::lib::DG

#endif // MOD_LIB_DG_VERTEXMAPPING_HPP