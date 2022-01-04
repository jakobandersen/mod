#ifndef MOD_LIB_DG_RULEAPPLICATIONUTILS_HPP
#define MOD_LIB_DG_RULEAPPLICATIONUTILS_HPP

#include <mod/graph/Graph.hpp>
#include <mod/lib/Graph/Single.hpp>
#include <mod/lib/Graph/Properties/Stereo.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/RC/ComposeRuleReal.hpp>
#include <mod/lib/RC/MatchMaker/Super.hpp>
#include <mod/lib/Rules/GraphAsRuleCache.hpp>
#include <mod/lib/Rules/Real.hpp>
#include <mod/lib/Stereo/CloneUtil.hpp>

namespace mod::lib::DG {

struct BoundRule {
	const lib::Rules::Real *rule;
	std::vector<const lib::Graph::Single *> boundGraphs;
	int nextGraphOffset;
public:
	void makeCanonical() {
		std::sort(begin(boundGraphs), end(boundGraphs), [](const auto *a, const auto *b) {
			return a->getId() < b->getId();
		});
	}

	// pre: this->makeCanonical() and other.makeCanonical() must have been called.
	bool isomorphicTo(const BoundRule &other, LabelSettings ls) const {
		if(boundGraphs != other.boundGraphs) return false;
		if(nextGraphOffset != other.nextGraphOffset) return false;
		ls.relation = ls.stereoRelation = LabelRelation::Isomorphism;
		return 1 == lib::Rules::Real::isomorphism(*rule, *other.rule, 1, ls);
	}

	friend std::ostream &operator<<(std::ostream &s, const BoundRule &br) {
		s << "{rule=" << br.rule->getName() << ", boundGraphs=[";
		bool first = true;
		for(const auto *g : br.boundGraphs) {
			if(!first) s << ", ";
			else first = false;
			s << g->getName();
		}
		s << "], nextGraphOffset=" << br.nextGraphOffset << "}";
		return s;
	}
};

constexpr int V_RuleApplication = 2;
constexpr int V_RuleApplication_Binding = 4;

inline int toRCVerbosity(int verbosity) {
	return std::max(0, verbosity - V_RuleApplication_Binding);
}

// BoundRules are given to onOutput. It must return a boolean indicating
// whether to continue the search.
// If a rule in a BoundRule given to onOutput is only-right-side,
// then onOutput gains responsibility for it.
// Otherwise, if there are still left-hand elements, then
// bindGraphs retains responsibility, and will return a list of all these.
// This is to do isomorphism checks.
template<typename Iter, typename OnOutput>
[[nodiscard]] std::vector<BoundRule> bindGraphs(
		const int verbosity, IO::Logger &logger,
		const int bindRound,
		const Iter firstGraph, const Iter lastGraph,
		const std::vector<BoundRule> &inputRules,
		Rules::GraphAsRuleCache &graphAsRuleCache,
		const LabelSettings labelSettings,
		OnOutput onOutput) {
	if(verbosity >= V_RuleApplication) {
		logger.indent() << "Bind round " << (bindRound + 1) << " with "
		                << (lastGraph - firstGraph) << " graphs "
		                << "and " << inputRules.size() << " rules." << std::endl;
		++logger.indentLevel;
	}
	int numDup = 0;
	int numUnique = 0;
	std::vector<BoundRule> outputRules;
	for(const BoundRule &brInput : inputRules) {
		if(verbosity >= V_RuleApplication_Binding) {
			logger.indent() << "Processing input rule " << brInput << std::endl;
			++logger.indentLevel;
		}
		// try to bind with all graphs that haven't been tried yet
		assert(brInput.nextGraphOffset <= lastGraph - firstGraph);
		const auto brFirstGraph = firstGraph + brInput.nextGraphOffset;
		for(auto iterGraph = brFirstGraph; iterGraph != lastGraph; ++iterGraph) {
			const auto *g = *iterGraph;
			if(verbosity >= V_RuleApplication_Binding) {
				logger.indent() << "Trying to bind " << g->getName() << " to " << brInput << ":" << std::endl;
				++logger.indentLevel;
			}
			const auto reporter =
					[labelSettings, &logger, &brInput, &outputRules, firstGraph, iterGraph, onOutput, &numUnique, &numDup]
							(std::unique_ptr<lib::Rules::Real> r) -> bool {
						BoundRule brOutput{r.release(), brInput.boundGraphs,
						                   static_cast<int>(iterGraph - firstGraph)};
						brOutput.boundGraphs.push_back(*iterGraph);
						if(!brOutput.rule->isOnlyRightSide()) {
							// check if we have it already
							brOutput.makeCanonical();
							for(const BoundRule &brStored : outputRules) {
								if(brStored.isomorphicTo(brOutput, labelSettings)) {
									delete brOutput.rule;
									++numDup;
									return true;
								}
							}
							// we store a copy of the bound info so the user can mess with their copy
							outputRules.push_back(brOutput);
						}
						++numUnique;
						return onOutput(logger, std::move(brOutput));
					};
			const lib::Rules::Real &rFirst = graphAsRuleCache.getBindRule(g)->getRule();
			const lib::Rules::Real &rSecond = *brInput.rule;
			lib::RC::Super mm(toRCVerbosity(verbosity), logger, true, true);
			lib::RC::composeRuleRealByMatchMaker(rFirst, rSecond, mm, reporter, labelSettings);
			if(verbosity >= V_RuleApplication_Binding)
				--logger.indentLevel;
		}
		if(verbosity >= V_RuleApplication_Binding)
			--logger.indentLevel;
	}
	if(verbosity >= V_RuleApplication) {
		logger.indent() << "Result of bind round " << (bindRound + 1) << ": "
		                << numUnique << " rules + " << numDup << " duplicates" << std::endl;
		--logger.indentLevel;
	}
	return outputRules;
}


struct BoundRuleStorage {
	BoundRuleStorage(bool verbose, IO::Logger logger,
	                 LabelType labelType,
	                 bool withStereo,
	                 std::vector<BoundRule> &ruleStore,
	                 const BoundRule &rule,
	                 const lib::Graph::Single *graph)
			: verbose(verbose), logger(logger), labelType(labelType), withStereo(withStereo),
			  ruleStore(ruleStore), rule(rule), graph(graph) {}

	void add(lib::Rules::Real *r) {
		BoundRule p{r, rule.boundGraphs, -1};
		p.boundGraphs.push_back(graph);
		bool found = false;
		const bool doBoundRulesDuplicateCheck = true;
		// if it's only right side, we will rather split it instead
		if(doBoundRulesDuplicateCheck && !r->isOnlyRightSide()) {
			std::vector<const lib::Graph::Single *> &rThis = p.boundGraphs;

			std::sort(begin(rThis), end(rThis), [](const lib::Graph::Single *g1, const lib::Graph::Single *g2) {
				return g1->getId() < g2->getId();
			});

			for(BoundRule &rp : ruleStore) {
				if(rThis.size() != rp.boundGraphs.size()) continue;
				std::vector<const lib::Graph::Single *> &rOther = rp.boundGraphs;

				std::sort(rOther.begin(), rOther.end(), [](const lib::Graph::Single *g1, const lib::Graph::Single *g2) {
					return g1->getId() < g2->getId();
				});
				bool doContinue = false;
				for(unsigned int i = 0; i < rThis.size(); i++) {
					if(rThis[i] != rOther[i]) {
						doContinue = true;
						break;
					}
				}
				if(doContinue) continue;
				found = 1 == lib::Rules::Real::isomorphism(*r, *rp.rule, 1,
				                                           {labelType, LabelRelation::Isomorphism, withStereo,
				                                            LabelRelation::Isomorphism});
				if(found) break;
			}
		}
		if(found) {
			//			std::cout << "Duplicate BRP found" << std::endl;
			delete r;
		} else {
			ruleStore.push_back(p);
			if(verbose) {
				logger.indent() << "BoundRules: added <" << r->getName() << ", {";
				for(const auto *g : p.boundGraphs)
					logger.s << " " << g->getName();
				logger.s << " }> onlyRight: " << std::boolalpha << r->isOnlySide(lib::Rules::Membership::Right)
				         << std::endl;
			}
		}
	}

private:
	const bool verbose;
	IO::Logger logger;
	const LabelType labelType;
	const bool withStereo;
	std::vector<BoundRule> &ruleStore;
	const BoundRule &rule;
	const lib::Graph::Single *graph;
};


// ===========================================================================
// ===========================================================================
// ===========================================================================

template<typename CheckIfNew, typename OnDup>
std::vector<std::shared_ptr<graph::Graph>> getAPIGraphs(std::vector<std::unique_ptr<Graph::Single>> graphs,
                                                     const LabelType labelType,
                                                     const bool withStereo,
                                                     CheckIfNew checkIfNew,
                                                     OnDup onDup) {
	std::vector<std::shared_ptr<graph::Graph>> out;
	for(auto &gCand : graphs) {
		// check against the database
		std::shared_ptr<graph::Graph> gWrapped = checkIfNew(std::move(gCand));
		// checkIfNew does not add the graph, so we must check against the previous products as well
		for(auto gPrev : out) {
			const auto ls = mod::LabelSettings(labelType, LabelRelation::Isomorphism, withStereo,
			                                   LabelRelation::Isomorphism);
			const bool iso = lib::Graph::Single::isomorphic(gPrev->getGraph(), gWrapped->getGraph(), ls);
			if(iso) {
				onDup(gWrapped, gPrev);
				gWrapped = gPrev;
				break;
			}
		}
		out.push_back(gWrapped);
	}
	return out;
}



// ===========================================================================
// ===========================================================================
// ===========================================================================

struct GraphData {
	using SideVertex = boost::graph_traits<lib::Rules::DPOProjection>::vertex_descriptor;
public:
	GraphData() : gPtr(new lib::Graph::GraphType()), pStringPtr(new lib::Graph::PropString(*gPtr)) {}

public:
	std::unique_ptr<lib::Graph::GraphType> gPtr;
	std::unique_ptr<lib::Graph::PropString> pStringPtr;
	std::unique_ptr<lib::Graph::PropStereo> pStereoPtr;
	std::vector<SideVertex> vertexMap;
};

template<typename CheckIfNew, typename OnDup>
std::vector<std::shared_ptr<graph::Graph>> splitRule(const lib::Rules::LabelledRule &rDPO,
                                                     const LabelType labelType,
                                                     const bool withStereo,
                                                     CheckIfNew checkIfNew,
                                                     OnDup onDup) {
	if(rDPO.numRightComponents == 0) MOD_ABORT; // continue;
	using Vertex = lib::Graph::Vertex;
	using Edge = lib::Graph::Edge;
	using SideVertex = boost::graph_traits<lib::Rules::DPOProjection>::vertex_descriptor;
	using SideEdge = boost::graph_traits<lib::Rules::DPOProjection>::edge_descriptor;

	std::vector<GraphData> products(rDPO.numRightComponents);
	const auto &compMap = rDPO.rightComponents;
	const auto &gRight = get_right(rDPO);
	auto rpString = get_string(get_labelled_right(rDPO));
	assert(num_vertices(gRight) == num_vertices(get_graph(rDPO)));
	std::vector<Vertex> vertexMap(num_vertices(gRight));
	for(const auto vSide : asRange(vertices(gRight))) {
		const auto comp = compMap[get(boost::vertex_index_t(), gRight, vSide)];
		auto &p = products[comp];
		const auto v = add_vertex(*p.gPtr);
		vertexMap[get(boost::vertex_index_t(), gRight, vSide)] = v;
		p.pStringPtr->addVertex(v, rpString[vSide]);
	}
	for(const auto eSide : asRange(edges(gRight))) {
		const auto vSideSrc = source(eSide, gRight);
		const auto vSideTar = target(eSide, gRight);
		const auto comp = compMap[get(boost::vertex_index_t(), gRight, vSideSrc)];
		assert(comp == compMap[get(boost::vertex_index_t(), gRight, vSideTar)]);
		const auto vCompSrc = vertexMap[get(boost::vertex_index_t(), gRight, vSideSrc)];
		const auto vCompTar = vertexMap[get(boost::vertex_index_t(), gRight, vSideTar)];
		const auto epComp = add_edge(vCompSrc, vCompTar, *products[comp].gPtr);
		assert(epComp.second);
		products[comp].pStringPtr->addEdge(epComp.first, rpString[eSide]);
	}

	if(withStereo && has_stereo(rDPO)) {
		// make the inverse vertex maps
		for(auto &p : products)
			p.vertexMap.resize(num_vertices(*p.gPtr));
		for(const auto vSide : asRange(vertices(gRight))) {
			const auto comp = compMap[get(boost::vertex_index_t(), gRight, vSide)];
			auto &p = products[comp];
			const auto v = vertexMap[get(boost::vertex_index_t(), gRight, vSide)];
			p.vertexMap[get(boost::vertex_index_t(), *p.gPtr, v)] = vSide;
		}

		for(auto &p : products) {
			const auto &lgRight = get_labelled_right(rDPO);
			assert(has_stereo(lgRight));
			const auto vertexMap = [&p](const auto &vProduct) {
				return p.vertexMap[get(boost::vertex_index_t(), *p.gPtr, vProduct)];
			};
			const auto edgeMap = [&p, &lgRight](const auto &eProduct) {
				const auto &g = *p.gPtr;
				const auto &gSide = get_graph(lgRight);
				const auto vSrc = source(eProduct, g);
				const auto vTar = target(eProduct, g);
				const auto vSrcSide = p.vertexMap[get(boost::vertex_index_t(), g, vSrc)];
				const auto vTarSide = p.vertexMap[get(boost::vertex_index_t(), g, vTar)];
				const auto epSide = edge(vSrcSide, vTarSide, gSide);
				assert(epSide.second);
				return epSide.first;
			};
			const auto inf = Stereo::makeCloner(lgRight, *p.gPtr, vertexMap, edgeMap);
			p.pStereoPtr = std::make_unique<lib::Graph::PropStereo>(*p.gPtr, inf);
		} // end foreach product
	} // end of stereo prop
	// wrap them
	std::vector<std::shared_ptr<graph::Graph>> right;
	for(auto &g : products) {
		// check against the database
		auto gCand = std::make_unique<lib::Graph::Single>(std::move(g.gPtr), std::move(g.pStringPtr),
		                                                  std::move(g.pStereoPtr));
		std::shared_ptr<graph::Graph> gWrapped = checkIfNew(std::move(gCand));
		// checkIfNew does not add the graph, so we must check against the previous products as well
		for(auto gPrev : right) {
			const auto ls = mod::LabelSettings(labelType, LabelRelation::Isomorphism, withStereo,
			                                   LabelRelation::Isomorphism);
			const bool iso = lib::Graph::Single::isomorphic(gPrev->getGraph(), gWrapped->getGraph(), ls);
			if(iso) {
				onDup(gWrapped, gPrev);
				gWrapped = gPrev;
				break;
			}
		}
		right.push_back(gWrapped);
	}
	return right;
}

} // namespace mod::lib::DG

#endif // MOD_LIB_DG_RULEAPPLICATIONUTILS_HPP
