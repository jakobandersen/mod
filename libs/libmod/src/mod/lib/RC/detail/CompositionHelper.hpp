#ifndef MOD_LIB_RC_DETAIL_COMPOSITIONHELPER_HPP
#define MOD_LIB_RC_DETAIL_COMPOSITIONHELPER_HPP

#include <mod/lib/DPO/Membership.hpp>
#include <mod/lib/DPO/Traits.hpp>
#include <mod/lib/IO/IO.hpp>

#include <optional>

namespace mod::lib::RC::detail {

using lib::DPO::Membership;

template<bool Verbose, typename Result, typename RuleFirst, typename RuleSecond, typename InvertibleVertexMap, typename Visitor>
struct CompositionHelper {
	using GraphResult = typename Result::CombinedGraph;
	using VertexResult = typename boost::graph_traits<GraphResult>::vertex_descriptor;
	using EdgeResult = typename boost::graph_traits<GraphResult>::edge_descriptor;

	using GraphFirst = typename RuleFirst::CombinedGraphType;
	using VertexFirst = typename boost::graph_traits<GraphFirst>::vertex_descriptor;

	using GraphSecond = typename RuleSecond::CombinedGraphType;
	using VertexSecond = typename boost::graph_traits<GraphSecond>::vertex_descriptor;
	using EdgeSecond = typename boost::graph_traits<GraphSecond>::edge_descriptor;
public:
	CompositionHelper(Result &result,
	                  const RuleFirst &rFirst, const RuleSecond &rSecond, const InvertibleVertexMap &match,
	                  Visitor visitor)
			: result(result), rFirst(rFirst), rSecond(rSecond), match(match), visitor(std::move(visitor)) {}

	bool operator()(IO::Logger logger) &&{
		if(Verbose) logger.sep('=');
		bool resInit = visitor.template init<Verbose>(logger, rFirst, rSecond, match, result);
		if(!resInit) {
			if(Verbose) logger.sep('=');
			return false;
		}
		// Vertices
		//--------------------------------------------------------------------------
		copyVerticesFirst(logger);
		composeVerticesSecond(logger);
		// Edges
		//--------------------------------------------------------------------------
		bool resFirst = copyEdgesFirstUnmatched(logger);
		if(!resFirst) {
			if(Verbose) logger.sep('=');
			return false;
		}
		bool resSecond = composeEdgesSecond(logger);
		if(!resSecond) {
			if(Verbose) logger.sep('=');
			return false;
		}

		// Finish it
		//--------------------------------------------------------------------------
		bool resFinal = visitor.template finalize<Verbose>(logger, rFirst, rSecond, match, result);
		if(!resFinal) {
			if(Verbose) logger.sep('=');
			return false;
		}
		if(Verbose) logger.sep('=');
		return true;
	}
private:
	VertexFirst getNullFirst() const {
		return boost::graph_traits<GraphFirst>::null_vertex();
	}

	VertexSecond getNullSecond() const {
		return boost::graph_traits<GraphSecond>::null_vertex();
	}

	VertexResult getNullResult() const {
		return boost::graph_traits<GraphResult>::null_vertex();
	}

	VertexFirst getVertexFirstChecked(VertexSecond vSecond) const {
		const auto &gCodom = getR(rFirst);
		const auto &gDom = getL(rSecond);
		const auto m = rSecond.getCombinedGraph()[vSecond].membership;
		if(m == Membership::R) return getNullFirst();
		else return get(match, gDom, gCodom, vSecond);
	}

	VertexSecond getVertexSecond(VertexFirst vFirst) const {
		const auto &gCodom = getR(rFirst);
		const auto &gDom = getL(rSecond);
		assert(rFirst.getCombinedGraph()[vFirst].membership != Membership::L);
		return get_inverse(match, gDom, gCodom, vFirst);
	}
private:
	void copyVerticesFirst(IO::Logger loggerOrig) {
		if(Verbose) {
			loggerOrig.indent() << "copyVerticesFirst\n";
			loggerOrig.sep('-');
		}
		auto &gResult = result.rDPO->getCombinedGraph();
		const auto &gFirst = rFirst.getCombinedGraph();
		const auto &gSecond = rSecond.getCombinedGraph();
		for(const auto vFirst: asRange(vertices(gFirst))) {
			auto logger = loggerOrig;
			if(Verbose) {
				++logger.indentLevel;
				logger.indent() << "rFirst node:\t"
				                << get(boost::vertex_index_t(), gFirst, vFirst)
				                << "(" << rFirst.getCombinedGraph()[vFirst].membership << ")"
				                << "(";
				visitor.printVertexFirst(rFirst, rSecond, match, result, logger.s, vFirst);
				logger.s << ")" << std::endl;
				++logger.indentLevel;
			}
			const bool getsDeleted = [&]() {
				// must be only in R to be deleted
				if(rFirst.getCombinedGraph()[vFirst].membership != Membership::R) return false;
				// must be matched to be deleted
				const auto vSecond = getVertexSecond(vFirst);
				if(vSecond == getNullSecond()) return false;
				// and the matched must only be in L
				return rSecond.getCombinedGraph()[vSecond].membership == Membership::L;
			}();
			if(getsDeleted) {
				put(result.mappings.mFirstToResult, gFirst, gResult, vFirst, getNullResult());
				if(Verbose) logger.indent() << "gets deleted" << std::endl;
			} else {
				const auto vResult = add_vertex(gResult);
				syncSize(result.mappings.mFirstToResult, gFirst, gResult);
				syncSize(result.mappings.mSecondToResult, gSecond, gResult);
				put(result.mappings.mFirstToResult, gFirst, gResult, vFirst, vResult);
				gResult[vResult].membership = rFirst.getCombinedGraph()[vFirst].membership;
				visitor.template copyVertexFirst<Verbose>(logger, rFirst, rSecond, match, result, vFirst, vResult);
				if(Verbose) {
					logger.indent() << "new node:\t"
					                << get(boost::vertex_index_t(), gResult, vResult)
					                << "(" << gResult[vResult].membership << ")(";
					visitor.printVertexResult(rFirst, rSecond, match, result, logger.s, vResult);
					logger.s << ")" << std::endl;
				}
			}
		}
	}

	void composeVerticesSecond(IO::Logger loggerOrig) {
		if(Verbose) {
			loggerOrig.indent() << "composeVerticesSecond\n";
			loggerOrig.sep('-');
		}
		const auto &gFirst = rFirst.getCombinedGraph();
		const auto &gSecond = rSecond.getCombinedGraph();
		auto &gResult = result.rDPO->getCombinedGraph();
		// copy nodes from second, but compose the matched ones which has already been created
		for(const auto vSecond: asRange(vertices(gSecond))) {
			auto logger = loggerOrig;
			if(Verbose) {
				++logger.indentLevel;
				logger.indent() << "rSecond node:\t"
				                << get(boost::vertex_index_t(), gSecond, vSecond)
				                << "(" << rSecond.getCombinedGraph()[vSecond].membership << ")"
				                << "(";
				visitor.printVertexSecond(rFirst, rSecond, match, result, logger.s, vSecond);
				logger.s << ")" << std::endl;
				++logger.indentLevel;
			}
			// check if the vertex is matched
			const auto vFirst = getVertexFirstChecked(vSecond);
			if(vFirst == getNullFirst()) {
				// unmatched vertex, create it
				const auto vResult = add_vertex(gResult);
				syncSize(result.mappings.mFirstToResult, gFirst, gResult);
				syncSize(result.mappings.mSecondToResult, gSecond, gResult);
				put(result.mappings.mSecondToResult, gSecond, gResult, vSecond, vResult);
				gResult[vResult].membership = rSecond.getCombinedGraph()[vSecond].membership;
				visitor.template copyVertexSecond<Verbose>(logger, rFirst, rSecond, match, result, vSecond, vResult);
				if(Verbose) {
					logger.indent() << "new node:\t" << get(boost::vertex_index_t(), gResult, vResult)
					                << "(" << gResult[vResult].membership << ")"
					                << "(";
					visitor.printVertexResult(rFirst, rSecond, match, result, logger.s, vResult);
					logger.s << ")" << std::endl;
				}
			} else { // vertex matched
				const auto vResult = get(result.mappings.mFirstToResult, gFirst, gResult, vFirst);
				put(result.mappings.mSecondToResult, gSecond, gResult, vSecond, vResult);
				if(vResult == getNullResult()) {
					if(Verbose) logger.indent() << "deleted" << std::endl;
				} else {
					if(Verbose) {
						logger.indent() << "match to:\t" << get(boost::vertex_index_t(), gResult, vResult)
						                << "(" << gResult[vResult].membership << ")"
						                << "(";
						visitor.template printVertexResult(rFirst, rSecond, match, result, logger.s, vResult);
						logger.s << ")" << std::endl;
					}
					// now we calculate the new membership for the node
					const auto mFirst = gResult[vResult].membership; // should be a copy of the one from rFirst
					const auto mSecond = rSecond.getCombinedGraph()[vSecond].membership;
					if(mFirst == Membership::R) {
						// vFirst appears
						if(mSecond == Membership::L) {
							// and vSecond disappears, so vResult should not exist at all
							// We should have caught that already.
							assert(false);
							std::abort();
						} else {
							// vSecond is CONTEXT, so vResult must be RIGHT (which it already is in rResult)
							visitor.template composeVertexRvsLR<Verbose>(logger, rFirst, rSecond, match, result, vResult,
							                                             vSecond);
						}
					} else { // vFirst is either LEFT or CONTEXT
						assert(mFirst != Membership::L); // we can't match a LEFT node in rRight
						// vFirst is CONTEXT
						if(mSecond == Membership::L) {
							// vSecond is disappearing, so vResult is disappearing as well
							visitor.template composeVertexLRvsL<Verbose>(logger, rFirst, rSecond, match, result, vResult,
							                                             vSecond);
							gResult[vResult].membership = Membership::L;
						} else { // both vFirst and vSecond is in both of their sides
							// let the visitor do its thing
							visitor.template composeVertexLRvsLR<Verbose>(logger, rFirst, rSecond, match, result, vResult,
							                                              vSecond);
							assert(gResult[vResult].membership == Membership::K);
						} // end if vSecond is LEFT
					} // end if vFirst is RIGHT
				} // end if vResult is null_vertex
			} // end if vSecond is unmatched
		} // end foreach vSecond
	}

	bool copyEdgesFirstUnmatched(IO::Logger logger) {
		if(Verbose) {
			logger.indent() << "copyEdgesFirstUnmatched\n";
			logger.sep('-');
		}
		const auto &gFirst = rFirst.getCombinedGraph();
		const auto &gSecond = rSecond.getCombinedGraph();
		auto &gResult = result.rDPO->getCombinedGraph();
		const auto processEdge = [&](IO::Logger logger, const auto eFirst) {
			++logger.indentLevel;
			// map source and target to core vertices
			const auto vSrcFirst = source(eFirst, gFirst);
			const auto vTarFirst = target(eFirst, gFirst);
			const auto meFirst = gFirst[eFirst].membership;
			if(Verbose) {
				const auto vSrcResult = get(result.mappings.mFirstToResult, gFirst, gResult, vSrcFirst);
				const auto vTarResult = get(result.mappings.mFirstToResult, gFirst, gResult, vTarFirst);
				// vSrcResult/vTarResult may be null_vertex
				logger.indent() << "Edge first:\t(" << vSrcFirst << ", " << vTarFirst << ") maybe copy to new ("
				                << vSrcResult << ", " << vTarResult << ")"
				                << "(" << meFirst << ")"
				                << "(";
				visitor.template printEdgeFirst(rFirst, rSecond, match, result, logger.s, eFirst);
				logger.s << ")" << std::endl;
				++logger.indentLevel;
			}
			const auto makeCopy = [&]() {
				const auto vSrcResult = get(result.mappings.mFirstToResult, gFirst, gResult, vSrcFirst);
				const auto vTarResult = get(result.mappings.mFirstToResult, gFirst, gResult, vTarFirst);
				// vResultSrc/vResultTar should be valid at this point
				assert(vSrcResult != getNullResult());
				assert(vTarResult != getNullResult());
				if(Verbose) logger.indent() << "Copy eFirst" << std::endl;
				const auto peResult = add_edge(vSrcResult, vTarResult, gResult);
				// adding shouldn't fail
				assert(peResult.second);
				const auto eResult = peResult.first;
				gResult[eResult].membership = gFirst[eFirst].membership;
				visitor.template copyEdgeFirst<Verbose>(logger, rFirst, rSecond, match, result, eFirst, eResult);
				assert(gResult[vSrcResult].membership == Membership::K || gResult[vSrcResult].membership == meFirst);
				assert(gResult[vTarResult].membership == Membership::K || gResult[vTarResult].membership == meFirst);
			};
			// and now the actual case analysis
			if(meFirst == Membership::L) {
				if(Verbose) logger.indent() << "eFirst in LEFT, clean copy" << std::endl;
				makeCopy();
				return true;
			}
			if(Verbose) logger.indent() << "eFirst in RIGHT or CONTEXT" << std::endl;
			const auto vSrcSecond = getVertexSecond(vSrcFirst);
			const auto vTarSecond = getVertexSecond(vTarFirst);
			const bool isSrcMatched = vSrcSecond != getNullSecond();
			const bool isTarMatched = vTarSecond != getNullSecond();
			if(isSrcMatched && isTarMatched) {
				if(Verbose) logger.indent() << "Both ends matched" << std::endl;
				const auto oeSecond = out_edges(vSrcSecond, gSecond);
				const auto eSecondIter = std::find_if(oeSecond.first, oeSecond.second,
				                                      [&gSecond, vTarSecond](const auto &eSecond) {
					                                      return target(eSecond, gSecond) == vTarSecond &&
					                                             gSecond[eSecond].membership != Membership::R;
				                                      });
				// TODO: why do we check the membership in the find_if?
				// There should be only one edge, so we can bail out here, right?
				const bool isEdgeMatched = eSecondIter != oeSecond.second;
				if(isEdgeMatched) {
					const auto eSecond = *eSecondIter;
					if(meFirst == Membership::R) {
						if(Verbose) logger.indent() << "eFirst matched and in RIGHT, skipping" << std::endl;
						return true;
					}

					if(Verbose)
						logger.indent() << "eFirst matched and in CONTEXT, copying to LEFT or CONTEXT (depending on eSecond ("
						                << gSecond[eSecond].membership << "))" << std::endl;
					const auto vSrcResult = get(result.mappings.mFirstToResult, gFirst, gResult, vSrcFirst);
					const auto vTarResult = get(result.mappings.mFirstToResult, gFirst, gResult, vTarFirst);
					// vResultSrc/vResultTar can not be null_vertex
					assert(vSrcResult != getNullResult());
					assert(vTarResult != getNullResult());
					const auto peResult = add_edge(vSrcResult, vTarResult, gResult);
					// adding shouldn't fail
					assert(peResult.second);
					const auto eResult = peResult.first;
					// we use the second membership, so do not use the common copy mechanism in the bottom of the function
					gResult[eResult].membership = gSecond[eSecond].membership;
					visitor.template copyEdgeFirst<Verbose>(logger, rFirst, rSecond, match, result, eFirst, eResult);
					assert(gResult[vSrcResult].membership != Membership::R);
					assert(gResult[vTarResult].membership != Membership::R);
					return true;
				}

				// eFirst not matched, the ends must be consistent and not deleted
				const auto vSrcResult = get(result.mappings.mFirstToResult, gFirst, gResult, vSrcFirst);
				const auto vTarResult = get(result.mappings.mFirstToResult, gFirst, gResult, vTarFirst);
				// vSrcResult/vTarResult may be null_vertex
				const bool endPointDeleted = vSrcResult == getNullResult() || vTarResult == getNullResult();
				if(endPointDeleted) {
					if(Verbose) logger.indent() << "Composition failure: at least one matched vertex deleted" << std::endl;
					return false;
				}

				// check consistency
				const auto mvSrcResult = gResult[vSrcResult].membership;
				const auto mvTarResult = gResult[vTarResult].membership;
				if(mvSrcResult == Membership::L || mvTarResult == Membership::L) {
					if(Verbose)
						logger.indent() << "Composition failure: at least one matched vertex has inconsistent context ("
						                << mvSrcResult << " and " << mvTarResult << "), eFirst is (" << meFirst << ")"
						                << std::endl;
					return false;
				}

				if(Verbose) logger.indent() << "eFirst not matched and not dangling" << std::endl;
				makeCopy();
				return true;
			}

			// at most 1 end matched
			if(isSrcMatched != isTarMatched) {
				if(Verbose) logger.indent() << "One end matched" << std::endl;
				const auto vSrcResult = get(result.mappings.mFirstToResult, gFirst, gResult, vSrcFirst);
				const auto vTarResult = get(result.mappings.mFirstToResult, gFirst, gResult, vTarFirst);
				// vResultSrc/vResultTar may be null_vertex
				const auto vResultMatched = isSrcMatched ? vSrcResult : vTarResult;
				const bool matchedDeleted = vResultMatched == boost::graph_traits<GraphResult>::null_vertex();
				if(matchedDeleted) {
					if(Verbose) logger.indent() << "Composition failure: matched vertex deleted" << std::endl;
					return false;
				}
				// matched, but is it consistent?
				const auto mvMatchedResult = gResult[vResultMatched].membership;
				if(mvMatchedResult == Membership::L) {
					if(Verbose)
						logger.indent() << "Composition failure: matched vertex has inconsistent context ("
						                << mvMatchedResult << "), eFirst is (" << meFirst << ")" << std::endl;
					return false;
				}

				if(Verbose) logger.indent() << "eFirst not matched and not dangling" << std::endl;
				makeCopy();
				return true;
			}

			if(Verbose) logger.indent() << "No ends matched" << std::endl;
			makeCopy();
			return true;
		};
		for(const auto eFirst: asRange(edges(gFirst))) {
			const bool ok = processEdge(logger, eFirst);
			if(!ok) return false;
		}
		return true;
	}

	bool composeEdgesSecond(IO::Logger loggerOrig) {
		if(Verbose) {
			loggerOrig.indent() << "composeEdgesSecond\n";
			loggerOrig.sep('-');
		}
		//		const auto &gFirst = rFirst.getCombinedGraph();
		const auto &gSecond = rSecond.getCombinedGraph();
		auto &gResult = result.rDPO->getCombinedGraph();
		for(auto eSecond: asRange(edges(gSecond))) {
			auto logger = loggerOrig;
			auto vSecondSrc = source(eSecond, gSecond);
			auto vSecondTar = target(eSecond, gSecond);
			auto vResultSrc = get(result.mappings.mSecondToResult, gSecond, gResult, vSecondSrc);
			auto vResultTar = get(result.mappings.mSecondToResult, gSecond, gResult, vSecondTar);
			if(Verbose) {
				++logger.indentLevel;
				// vSrcNew/vTarNew may be null_vertex
				logger.indent() << "Edge second:\t(" << vSecondSrc << ", " << vSecondTar << ")\tmapped to (" << vResultSrc
				                << ", " << vResultTar << ")"
				                << ", (" << gSecond[eSecond].membership << ")"
				                << "(";
				visitor.template printEdgeSecond(rFirst, rSecond, match, result, logger.s, eSecond);
				logger.s << ")" << std::endl;
				++logger.indentLevel;
			}
			// vResultSrc/vResultTar may be null_vertex
			// check for match on vFirstSrc and vFirstTar
			auto vFirstSrc = getVertexFirstChecked(vSecondSrc);
			auto vFirstTar = getVertexFirstChecked(vSecondTar);
			bool isSrcMatched = vFirstSrc != boost::graph_traits<GraphFirst>::null_vertex();
			bool isTarMatched = vFirstTar != boost::graph_traits<GraphFirst>::null_vertex();
			if(!isSrcMatched || !isTarMatched) { // new edge
				if(!isSrcMatched && !isTarMatched) {
					if(Verbose) logger.indent() << "No ends matched. Copy eSecond." << std::endl;
					// vResultSrc/vResultTar can not be null_vertex
					assert(vResultSrc != boost::graph_traits<GraphResult>::null_vertex());
					assert(vResultTar != boost::graph_traits<GraphResult>::null_vertex());
					auto peResult = add_edge(vResultSrc, vResultTar, gResult);
					// adding shouldn't fail
					assert(peResult.second);
					auto eResult = peResult.first;
					gResult[eResult].membership = gSecond[eSecond].membership;
					visitor.template copyEdgeSecond<Verbose>(logger, rFirst, rSecond, match, result, eSecond, eResult);
				} else { // one end matched
					assert(vResultSrc != boost::graph_traits<GraphResult>::null_vertex()
					       || vResultTar != boost::graph_traits<GraphResult>::null_vertex());
					bool isOk = composeEdgeSecond_oneEndMatched(logger, eSecond, vResultSrc, vResultTar, isSrcMatched,
					                                            isTarMatched);
					if(!isOk) return false;
				} // if 0 or 1 end matched
			} else { // maybe a matched edge
				bool isOk = composeEdgeSecond_bothEndsMatched(logger, eSecond, vFirstSrc, vFirstTar, vResultSrc,
				                                              vResultTar);
				if(!isOk) return false;
			} // end if 0 or 1 matched endNodes
		} // end for each eSecond
		return true;
	}
private:
	bool composeEdgeSecond_oneEndMatched(IO::Logger logger,
	                                     EdgeSecond eSecond,
	                                     VertexResult vResultSrc,
	                                     VertexResult vResultTar,
	                                     bool isSrcMatched,
	                                     bool isTarMatched) {
		auto &gResult = result.rDPO->getCombinedGraph();
		if(Verbose) logger.indent() << "One end matched, eSecond should be copied" << std::endl;
		assert(isSrcMatched ^ isTarMatched);
		// at most one of vResultSrc and vResultTar may be null_vertex
		assert(vResultSrc != boost::graph_traits<GraphResult>::null_vertex()
		       || vResultTar != boost::graph_traits<GraphResult>::null_vertex());
		auto vResultMatched = isSrcMatched ? vResultSrc : vResultTar;
		auto vResultOther = isSrcMatched ? vResultTar : vResultSrc;
		// vResultMatched may be null_vertex, if it's a RIGHT vs. LEFT vertex
		assert(vResultOther != boost::graph_traits<GraphResult>::null_vertex());
		const auto meSecond = rSecond.getCombinedGraph()[eSecond].membership;
		const auto mvResultOther = gResult[vResultOther].membership;
		(void) mvResultOther;
		assert(mvResultOther == Membership::K || mvResultOther == meSecond);

		if(Verbose) ++logger.indentLevel;

		if(vResultMatched == boost::graph_traits<GraphResult>::null_vertex()) {
			if(Verbose) logger.indent() << "Composition failure: matched vertex deleted" << std::endl;
			return false;
		}

		auto mvResultMatched = gResult[vResultMatched].membership;
		if(mvResultMatched != Membership::K && mvResultMatched != meSecond) {
			if(Verbose)
				logger.indent() << "Composition failure: matched vertex has inconsistent context ("
				                << mvResultMatched << "), eSecond is (" << meSecond << ")" << std::endl;
			return false;
		}

		if(Verbose) logger.indent() << "Copy edge" << std::endl;
		auto peResult = add_edge(vResultSrc, vResultTar, gResult);
		// adding shouldn't fail
		assert(peResult.second);
		auto eResult = peResult.first;
		gResult[eResult].membership = rSecond.getCombinedGraph()[eSecond].membership;
		visitor.template copyEdgeSecond<Verbose>(logger, rFirst, rSecond, match, result, eSecond, eResult);
		return true;
	}

	bool composeEdgeSecond_bothEndsMatched(IO::Logger logger,
	                                       EdgeSecond eSecond,
	                                       VertexFirst vFirstSrc,
	                                       VertexFirst vFirstTar,
	                                       VertexResult vResultSrc,
	                                       VertexResult vResultTar) {
		const auto &gFirst = rFirst.getCombinedGraph();
		auto &gResult = result.rDPO->getCombinedGraph();
		if(Verbose) logger.indent() << "Both ends matched" << std::endl;
		// vResultSrc/vResultTar may be null_vertex
		std::optional<Membership> omeFirst;
		// we search in coreFirst, because it has the matched edges
		for(const auto eFirst: asRange(out_edges(vFirstSrc, gFirst))) {
			if(target(eFirst, gFirst) != vFirstTar) continue;
			omeFirst = gFirst[eFirst].membership;
			break;
		}
		auto meSecond = rSecond.getCombinedGraph()[eSecond].membership;

		if(Verbose) ++logger.indentLevel;

		// possibilities:
		//    ->   vs. |  ->
		//    ->   vs.    -> |
		//    ->   vs. |  -> |
		// |  ->   vs. |  ->
		// |  ->   vs.    -> |
		// |  ->   vs. |  -> |
		//    -> | vs. |  ->	first not in result
		//    -> | vs.    -> |
		//    -> | vs. |  -> |	first not in result
		// |  -> | vs. |  -> 	first in result as LEFT
		// |  -> | vs.    -> |
		// |  -> | vs. |  -> |	first in result as CONTEXT
		// can we do a simple copy?
		if(!omeFirst) {
			//    ->   vs. |  ->
			//    ->   vs.    -> |
			//    ->   vs. |  -> |
			if(Verbose) logger.indent() << "Simple copy of eSecond" << std::endl;
			if(meSecond == Membership::L) {
				if(Verbose) logger.indent() << "eSecond in LEFT, check ends" << std::endl;
				// vResultSrc/vResultTar may be null_vertex
				if(vResultSrc == boost::graph_traits<GraphResult>::null_vertex()) {
					if(Verbose) logger.indent() << "Composition failure: vResultSrc deleted" << std::endl;
					return false;
				}
				if(vResultTar == boost::graph_traits<GraphResult>::null_vertex()) {
					if(Verbose) logger.indent() << "Composition failure: vResultTar deleted" << std::endl;
					return false;
				}
				if(gResult[vResultSrc].membership == Membership::R) {
					if(Verbose) logger.indent() << "Composition failure: vResultSrc not in LEFT or CONTEXT" << std::endl;
					return false;
				}
				if(gResult[vResultTar].membership == Membership::R) {
					if(Verbose) logger.indent() << "Composition failure: vResultTar not in LEFT or CONTEXT" << std::endl;
					return false;
				}
			}
			// vResultSrc/vResultTar may not be null_vertex
			assert(vResultSrc != boost::graph_traits<GraphResult>::null_vertex());
			assert(vResultTar != boost::graph_traits<GraphResult>::null_vertex());
			auto peResult = add_edge(vResultSrc, vResultTar, gResult);
			// adding shouldn't fail
			assert(peResult.second);
			auto eResult = peResult.first;
			gResult[eResult].membership = rSecond.getCombinedGraph()[eSecond].membership;
			visitor.template copyEdgeSecond<Verbose>(logger, rFirst, rSecond, match, result, eSecond, eResult);
			return true;
		}

		// check for parallel errors
		assert(omeFirst);
		auto meFirst = *omeFirst;
		if(meFirst == Membership::L && meSecond != Membership::R) {
			// creating parallel in left
			// |  ->   vs. |  ->
			// |  ->   vs. |  -> |
			if(Verbose) logger.indent() << "Composition failure: duplicate edge in L" << std::endl;
			return false;
		}
		if(meFirst != Membership::L && meSecond == Membership::R) {
			// creating parallel in right
			//    -> | vs.    -> |
			// |  -> | vs.    -> |
			if(Verbose)
				logger.indent()
						<< "Composition failure: eSecond in RIGHT, eFirst in CONTEXT or RIGHT, i.e., parallel edge in R"
						<< std::endl;
			return false;
		}
		// remaining possibilities:
		// |  ->   vs.    -> |
		//    -> | vs. |  ->	first not in result
		//    -> | vs. |  -> |	first not in result
		// |  -> | vs. |  -> 	first in result as LEFT
		// |  -> | vs. |  -> |	first in result as CONTEXT
		// check if we should just ignore the edge (matched or handled
		if(meSecond == Membership::L) {
			//    -> | vs. |  ->	first not in result
			// |  -> | vs. |  -> 	first in result as LEFT
			if(Verbose)
				logger.indent() << "eSecond matched in (" << meSecond << ")"
				                << ", either in LEFT or both eSecond and eFirst in CONTEXT" << std::endl;
			return true;
		}
		// remaining possibilities:
		// |  ->   vs.    -> |
		//    -> | vs. |  -> |	first not in result
		// |  -> | vs. |  -> |	first in result as CONTEXT
		// vResultSrc/vResultTar can not be null_vertex
		assert(vResultSrc != boost::graph_traits<GraphResult>::null_vertex());
		assert(vResultTar != boost::graph_traits<GraphResult>::null_vertex());
		if(meFirst == Membership::R) {
			// copy eSecond as RIGHT to result
			assert(meSecond == Membership::K);
			//    -> | vs. |  -> |
			if(Verbose) logger.indent() << "   -> | vs. |  -> |, copy to RIGHT" << std::endl;
			auto peResult = add_edge(vResultSrc, vResultTar, gResult);
			// adding shouldn't fail
			assert(peResult.second);
			auto eResult = peResult.first;
			gResult[eResult].membership = Membership::R;
			visitor.template copyEdgeSecond<Verbose>(logger, rFirst, rSecond, match, result, eSecond, eResult);
			return true;
		}
		// remaining possibilities:
		// |  ->   vs.    -> |
		// |  -> | vs. |  -> |	first in new as CONTEXT
		// we have to find the eNew in LEFT or CONTEXT
		auto edgesResult = out_edges(vResultSrc, gResult);
		auto edgeIter = std::find_if(edgesResult.first, edgesResult.second, [&gResult, vResultTar](EdgeResult eCand) {
			if(target(eCand, gResult) != vResultTar) return false;
			return true;
		});
		assert(edgeIter != edgesResult.second);
		auto eResult = *edgeIter;
		if(Verbose)
			logger.indent()
					<< "'|  ->   vs.    -> |' or '|  -> | vs. |  -> |', promote eNew to CONTEXT and set right from second right"
					<< std::endl;
		gResult[eResult].membership = Membership::K;
		visitor.template setEdgeResultRightFromSecondRight<Verbose>(logger, rFirst, rSecond, match, result, eResult,
		                                                            eSecond);
		return true;
	}
private:
	Result &result;
	const RuleFirst &rFirst;
	const RuleSecond &rSecond;
	const InvertibleVertexMap &match;
	Visitor visitor;
};

} // namespace mod::lib::RC::detail

#endif // MOD_LIB_RC_DETAIL_COMPOSITIONHELPER_HPP