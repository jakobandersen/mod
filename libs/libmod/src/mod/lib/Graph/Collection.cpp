#include "Collection.hpp"

#include <mod/Error.hpp>
#include <mod/lib/Graph/Single.hpp>

namespace mod::lib::Graph {
namespace {

inline CollectionStats getStats(const lib::Graph::Single *g) {
	const auto &graph = g->getGraph();
	return {num_vertices(graph), num_edges(graph)};
}


} // namespace

struct Collection::Store {
	bool trustInsert(const lib::Graph::Single *g) {
		return graphs.insert(g).second;
	}

	auto end() const {
		return graphs.end();
	}

	auto find(const lib::Graph::Single *g) const {
		return graphs.find(g);
	}

	std::shared_ptr<graph::Graph> findIsomorphic(const lib::Graph::Single *g, LabelSettings ls) const {
		for(const auto &gCand : graphs) {
			const bool iso = lib::Graph::Single::isomorphic(*g, *gCand, ls);
			if(iso) return gCand->getAPIReference();
			// if iso:
			//			if(getConfig().dg.calculateDetailsVerbose.get()) {
			//				std::cout << "Discarding product " << gCand->getName() << ", isomorphic to " << g->getName() << std::endl;
			//				std::cout << "\tLabelSettings: withStereo=" << std::boolalpha << this->labelSettings.withStereo << std::endl;
			//				mod::postSection("Discarded");
			//				mod::lib::IO::Graph::Write::Options opts, optsGraph;
			//				opts.edgesAsBonds = opts.withIndex = true;
			//				optsGraph.collapseHydrogens = optsGraph.edgesAsBonds = optsGraph.raiseCharges = true;
			//				optsGraph.simpleCarbons = optsGraph.withColour = optsGraph.withIndex = true;
			//				mod::lib::IO::Graph::Write::summary(*gCand, optsGraph, optsGraph);
			//				std::shared_ptr<graph::Graph> gCandWrapped = graph::Graph::makeGraph(std::move(gCand));
			//				for(auto v : gCandWrapped->vertices()) {
			//					if(v.getStringLabel() == "C")
			//						v.printStereo();
			//				}
			//			}
		}
		return nullptr;
	}

private:
	std::unordered_set<const lib::Graph::Single *> graphs;
};

Collection::Collection(LabelSettings ls, Config::IsomorphismAlg alg)
		: ls(ls.type, LabelRelation::Isomorphism,
			  ls.withStereo, LabelRelation::Isomorphism) {}

Collection::~Collection() = default;

const std::vector<std::shared_ptr<graph::Graph>> &Collection::asList() const {
	return graphs;
}

bool Collection::contains(std::shared_ptr<graph::Graph> g) const {
	const auto *gLib = &g->getGraph();
	const auto stats = getStats(gLib);
	const auto iterStore = graphStore.find(stats);
	if(iterStore == end(graphStore)) return false;
	const auto iterGraph = iterStore->second->find(gLib);
	return iterGraph != iterStore->second->end();
}

std::shared_ptr<graph::Graph> Collection::findIsomorphic(std::shared_ptr<graph::Graph> g) const {
	const auto *gLib = &g->getGraph();
	const auto stats = getStats(gLib);
	const auto iterStore = graphStore.find(stats);
	if(iterStore == end(graphStore)) return nullptr;
	const auto iterGraph = iterStore->second->find(gLib);
	if(iterGraph != iterStore->second->end()) return g;
	return iterStore->second->findIsomorphic(gLib, ls);
}

std::shared_ptr<graph::Graph> Collection::findIsomorphic(lib::Graph::Single *g) const {
	const auto stats = getStats(g);
	const auto iterStore = graphStore.find(stats);
	if(iterStore == end(graphStore)) return nullptr;
	return iterStore->second->findIsomorphic(g, ls);
}

bool Collection::trustInsert(std::shared_ptr<graph::Graph> g) {
	const auto *gLib = &g->getGraph();
	const auto stats = getStats(gLib);
	auto iterStore = graphStore.find(stats);
	if(iterStore == end(graphStore)) {
		iterStore = graphStore.emplace(stats, std::make_unique<Store>()).first;
		iterStore->second->trustInsert(gLib);
		graphs.push_back(g);
		return true;
	} else {
		const bool res = iterStore->second->trustInsert(gLib);
		if(res) graphs.push_back(g);
		return res;
	}
}

std::pair<std::shared_ptr<graph::Graph>, bool> Collection::tryInsert(std::shared_ptr<graph::Graph> g) {
	const auto gIns = findIsomorphic(g);
	if(gIns) return {gIns, false};
	trustInsert(g);
	return {g, true};
}

} // namespace mod::lib::Graph