#include "Expanded.hpp"

#include <mod/Error.hpp>
#include <mod/lib/DG/Hyper.hpp>
#include <mod/lib/DG/IO/Expanded.hpp>

#include <boost/functional/hash.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <ostream>

std::size_t std::hash<mod::lib::DG::Transit>::operator()(mod::lib::DG::Transit r) const {
	std::size_t seed = 0;
	boost::hash_combine(seed, r.eIn);
	boost::hash_combine(seed, r.v);
	boost::hash_combine(seed, r.eOut);
	return seed;
}

namespace mod::lib::DG {

Expanded::Expanded(const Hyper &dgHyper, TransitSet expressedTransits, TransitSet deletedTransits)
		: dgHyper(dgHyper) {
	// a deleted transit is also 'expressed'
	expressedTransits.insert(begin(deletedTransits), end(deletedTransits));

	const auto &dg = dgHyper.getGraph();
	// create the vertices for the hyper edges
	for(HyperVertex vHyper : asRange(vertices(dg))) {
		if(dg[vHyper].kind != HyperVertexKind::Edge) continue;
		ExpandedVertex vExp = add_vertex(ExpandedVProp(ExpandedVertexKind::Edge, vHyper, dg.null_vertex()), graph);
		hyperEdgeToExpanded[vHyper] = vExp;
	}

	std::unordered_map<HyperVertex, std::map<HyperVertex, unsigned int> > inExpressed, outExpressed; // vertex -> in/out vertex x position
	for(auto transit : expressedTransits) {
		inExpressed[transit.v].insert({transit.eIn, 0});
		outExpressed[transit.v].insert({transit.eOut, 0});
	}

	// sort the transits, so we can easier link them while creating the edges
	std::vector<Transit> expressedTransitVec(begin(expressedTransits), end(expressedTransits));
	std::vector<Transit> deletedTransitVec(begin(deletedTransits), end(deletedTransits));
	std::sort(begin(expressedTransitVec), end(expressedTransitVec));
	std::sort(begin(deletedTransitVec), end(deletedTransitVec));
	auto nextTransit = begin(expressedTransitVec);
	auto nextDeletedTransit = begin(deletedTransitVec);
	for(HyperVertex vHyper : asRange(vertices(dg))) {
		if(dg[vHyper].kind != HyperVertexKind::Vertex) continue;
		VertexData &vertexData = this->vertexData[vHyper];
		// do we have catch-all?
		const auto handleIO = [&](const bool isIn) {
			auto &ioVertices = isIn ? vertexData.inVertices : vertexData.outVertices;
			auto &expressed = isIn ? inExpressed[vHyper] : outExpressed[vHyper];
			// create catch-all transit vertex if needed
			bool hasCatchAll;
			if(isIn) {
				auto iters = inv_adjacent_vertices(vHyper, dg);
				vertexData.hasInCatchAll = hasCatchAll
						= // at least 1 is expressed, of real in-edges ...
						std::any_of(iters.first, iters.second,
						            [&](HyperVertex vAdj) {
							            return expressed.find(vAdj) == end(expressed);
						            }) ||
						// ... and of the input edge
						expressed.find(dg.null_vertex()) == end(expressed);
			} else {
				auto iters = adjacent_vertices(vHyper, dg);
				vertexData.hasOutCatchAll = hasCatchAll
						= // at least 1 is expressed, of real out-edges ...
						std::any_of(iters.first, iters.second,
						            [&](HyperVertex vAdj) {
							            return expressed.find(vAdj) == end(expressed);
						            }) ||
						// ... and of the input edge
						expressed.find(dg.null_vertex()) == end(expressed);
			}
			if(hasCatchAll) {
				const ExpandedVertex vTransitRest = add_vertex(
						ExpandedVProp(isIn ? ExpandedVertexKind::InVertex : ExpandedVertexKind::OutVertex,
						              vHyper, dg.null_vertex()),
						graph);
				ioVertices.push_back(vTransitRest);
			}
			// create input/output half-edge
			const ExpandedVertex vIOEdge = add_vertex(
					ExpandedVProp(ExpandedVertexKind::IOEdge, vHyper, dg.null_vertex()),
					graph);
			if(isIn) {
				vertexData.inputEdge = vIOEdge;
			} else {
				vertexData.outputEdge = vIOEdge;
			}
			// in/out edges
			const auto handleIOEdge = [&](HyperVertex vAdjPrev, ExpandedVertex vTransitPrev, HyperVertex vAdj) {
				ExpandedVertex vTransit;
				if(vAdjPrev == vAdj && vAdj != dg.null_vertex()) {
					// handle multi-edge in/out: put to same transit
					vTransit = vTransitPrev;
				} else {
					// if expressed, create its own transit io vertex,
					// otherwise, use the catch-all
					auto iter = expressed.find(vAdj);
					if(iter != end(expressed)) {
						vTransit = add_vertex(
								ExpandedVProp(isIn ? ExpandedVertexKind::InVertex : ExpandedVertexKind::OutVertex,
								              vHyper, vAdj),
								graph);
						iter->second = ioVertices.size();
						ioVertices.push_back(vTransit);
					} else {
						//					if(!hasCatchAll) {
						//						std::cout << "graph: " << dg[vHyper].graph->getName() << std::endl;
						//						std::cout << "vAdj: " << vAdj << std::endl;
						//					}
						assert(hasCatchAll);
						vTransit = ioVertices.front();
					}
				}
				const ExpandedVertex vEdge = vAdj == dg.null_vertex() ? vIOEdge : getExpandedFromHyperEdge(vAdj);
				if(isIn) add_edge(vEdge, vTransit, graph);
				else add_edge(vTransit, vEdge, graph);
				return vTransit;
			};
			// create the normal edges (sort them to make them in same order as the transits)
			std::vector<HyperVertex> sortedAdj;
			if(isIn) {
				auto iters = inv_adjacent_vertices(vHyper, dg);
				sortedAdj.insert(begin(sortedAdj), iters.first, iters.second);
			} else {
				auto iters = adjacent_vertices(vHyper, dg);
				sortedAdj.insert(begin(sortedAdj), iters.first, iters.second);
			}
			HyperVertex vAdjPrev = dg.null_vertex();
			ExpandedVertex vTransitPrev = graph.null_vertex();
			for(HyperVertex vAdj : sortedAdj) {
				vTransitPrev = handleIOEdge(vAdjPrev, vTransitPrev, vAdj);
				vAdjPrev = vAdj;
			}
			// create the input/output edge (should be last due to sort order)
			handleIOEdge(dg.null_vertex(), graph.null_vertex(), dg.null_vertex());
		};
		handleIO(true); // should be first due to sort order
		handleIO(false);
		// and now the transit edges
		bool inIsFirst = true;
		for(ExpandedVertex vIn : vertexData.inVertices) {
			bool outIsFirst = true;
			for(ExpandedVertex vOut : vertexData.outVertices) {
				bool transitFound = false, transitDeleted = false;
				if(nextTransit != end(expressedTransitVec)
				   && !(inIsFirst && vertexData.hasInCatchAll)
				   && !(outIsFirst && vertexData.hasOutCatchAll)) {
					HyperVertex vHyperIn = graph[vIn].eRepr;
					HyperVertex vHyperOut = graph[vOut].eRepr;
					Transit rCand{vHyperIn, vHyper, vHyperOut};
					if(*nextTransit == rCand) {
						transitFound = true;
						if(nextDeletedTransit != end(deletedTransitVec)
						   && *nextTransit == *nextDeletedTransit)
							transitDeleted = true;
					}
				}
				if(!transitDeleted) {
					ExpandedVertex vEdge = add_vertex(ExpandedVProp(ExpandedVertexKind::TransitEdge,
					                                                vHyper, dg.null_vertex()),
					                                  graph);
					add_edge(vIn, vEdge, graph);
					add_edge(vEdge, vOut, graph);
					if(transitFound) transitMap[*nextTransit] = vEdge;
				} else {
					transitMap[*nextTransit] = graph.null_vertex();
				}
				if(transitFound) {
					nextTransit++;
					if(transitDeleted) nextDeletedTransit++;
				}
				outIsFirst = false;
			}
			inIsFirst = false;
		}
	}
	assert(nextTransit == end(expressedTransitVec));
	assert(nextDeletedTransit == end(deletedTransitVec));
	//	IO::DG::Write::Printer p;
	//	IO::DGExpanded::Write::summary(*this, IO::DG::Write::Data(dgHyper), p);
}

const ExpandedGraphType &Expanded::getGraph() const {
	return graph;
}

ExpandedVertex Expanded::getExpandedFromHyperEdge(HyperVertex e) const {
	if(dgHyper.getGraph()[e].kind != HyperVertexKind::Edge) MOD_ABORT;
	auto iter = hyperEdgeToExpanded.find(e);
	assert(iter != end(hyperEdgeToExpanded));
	return iter->second;
}

const Expanded::VertexData &Expanded::getVertexData(HyperVertex v) const {
	if(dgHyper.getGraph()[v].kind != HyperVertexKind::Vertex) MOD_ABORT;
	auto iter = vertexData.find(v);
	assert(iter != end(vertexData));
	return iter->second;
}

bool Expanded::isCatchAll(ExpandedVertex v) const {
	auto kind = graph[v].kind;
	if(kind != ExpandedVertexKind::InVertex && kind != ExpandedVertexKind::OutVertex) return false;
	auto vHyper = graph[v].vHyper;
	const auto &vertexData = getVertexData(vHyper);
	if(kind == ExpandedVertexKind::InVertex)
		return vertexData.hasInCatchAll && vertexData.inVertices.front() == v;
	else return vertexData.hasOutCatchAll && vertexData.outVertices.front() == v;
}

bool Expanded::hasIOInternalTransitSeparated(HyperVertex vHyper) const {
	const auto &vertexData = getVertexData(vHyper);
	const auto vExpInput = vertexData.inputEdge;
	const auto vExpOutput = vertexData.outputEdge;
	assert(out_degree(vExpInput, graph) == 1);
	assert(in_degree(vExpOutput, graph) == 1);
	const auto vTransInput = *adjacent_vertices(vExpInput, graph).first;
	const auto vTransOutput = *inv_adjacent_vertices(vExpOutput, graph).first;
	assert(in_degree(vTransInput, graph) >= 1);
	assert(out_degree(vTransOutput, graph) >= 1);
	if(in_degree(vTransInput, graph) > 1) return false;
	if(out_degree(vTransOutput, graph) > 1) return false;
	return true;
}

void Expanded::addAllTransits(const Hyper &dgHyper, TransitSet &c) {
	addNonInverseTransits(dgHyper, c);
	addIOInverseTransits(dgHyper, c);
	addInternalInverseTransits(dgHyper, c);
}

void Expanded::addAllTransits(const Hyper &dgHyper, HyperVertex v, TransitSet &c) {
	addNonInverseTransits(dgHyper, v, c);
	addIOInverseTransits(dgHyper, v, c);
	addInternalInverseTransits(dgHyper, v, c);
}

void Expanded::addNonInverseTransits(const Hyper &dgHyper, TransitSet &c) {
	const auto &g = dgHyper.getGraph();
	for(HyperVertex v : asRange(vertices(g))) {
		if(g[v].kind != HyperVertexKind::Vertex) continue;
		addNonInverseTransits(dgHyper, v, c);
	}
}

void Expanded::addNonInverseTransits(const Hyper &dgHyper, HyperVertex v, TransitSet &c) {
	const auto &g = dgHyper.getGraph();
	assert(g[v].kind == HyperVertexKind::Vertex);
	using R = Transit;
	for(HyperVertex vOut : asRange(adjacent_vertices(v, g)))
		c.insert(R{g.null_vertex(), v, vOut});
	for(HyperVertex vIn : asRange(inv_adjacent_vertices(v, g))) {
		c.insert(R{vIn, v, g.null_vertex()});
		for(HyperVertex vOut : asRange(adjacent_vertices(v, g))) {
			if(dgHyper.getReverseEdge(vIn) == vOut) continue;
			c.insert(R{vIn, v, vOut});
		}
	}
}

void Expanded::addInternalInverseTransits(const Hyper &dgHyper, TransitSet &c) {
	const auto &g = dgHyper.getGraph();
	for(HyperVertex v : asRange(vertices(g))) {
		if(g[v].kind != HyperVertexKind::Vertex) continue;
		addInternalInverseTransits(dgHyper, v, c);
	}
}

void Expanded::addInternalInverseTransits(const Hyper &dgHyper, HyperVertex v, TransitSet &c) {
	const auto &g = dgHyper.getGraph();
	assert(g[v].kind == HyperVertexKind::Vertex);
	using R = Transit;
	for(HyperVertex vIn : asRange(inv_adjacent_vertices(v, g))) {
		HyperVertex vOut = dgHyper.getReverseEdge(vIn);
		if(vOut != g.null_vertex())
			c.insert(R{vIn, v, vOut});
	}
}

void Expanded::addIOInverseTransits(const Hyper &dgHyper, TransitSet &c) {
	const auto &g = dgHyper.getGraph();
	for(HyperVertex v : asRange(vertices(g))) {
		if(g[v].kind != HyperVertexKind::Vertex) continue;
		addIOInverseTransits(dgHyper, v, c);
	}
}

void Expanded::addIOInverseTransits(const Hyper &dgHyper, HyperVertex v, TransitSet &c) {
	const auto &g = dgHyper.getGraph();
	c.insert(Transit{g.null_vertex(), v, g.null_vertex()});
}

} // namespace mod::lib::DG