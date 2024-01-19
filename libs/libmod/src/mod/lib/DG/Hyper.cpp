#include "Hyper.hpp"

#include <mod/Config.hpp>
#include <mod/Error.hpp>
#include <mod/Function.hpp>
#include <mod/dg/DG.hpp>
#include <mod/graph/Graph.hpp>
#include <mod/lib/DG/NonHyper.hpp>
#include <mod/lib/Graph/Single.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/Rules/Real.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <boost/graph/adjacency_list.hpp>

namespace mod::lib::DG {

//------------------------------------------------------------------------------
// HyperCreator
//------------------------------------------------------------------------------

HyperCreator::HyperCreator(Hyper &hyper,
                           std::shared_ptr<Function<void(dg::DG::Vertex)>> onNewVertex,
                           std::shared_ptr<Function<void(dg::DG::HyperEdge)>> onNewHyperEdge)
						   : owner(&hyper), onNewVertex(onNewVertex), onNewHyperEdge(onNewHyperEdge) {}

HyperCreator::HyperCreator(HyperCreator &&other)
		: owner(other.owner), onNewVertex(std::move(other.onNewVertex)), onNewHyperEdge(std::move(other.onNewHyperEdge)) {
	other.owner = nullptr;
}

HyperCreator &HyperCreator::operator=(HyperCreator &&other) {
	if(&other == this) return *this;
	std::swap(owner, other.owner);
	std::swap(onNewVertex, other.onNewVertex);
	std::swap(onNewHyperEdge, other.onNewHyperEdge);
	return *this;
}

HyperCreator::~HyperCreator() {
	if(owner) owner->hasCalculated = true;
}

void HyperCreator::addVertex(const lib::Graph::Single *g) {
	assert(owner);
	const auto[v, isNew] = owner->addVertex(g);
	if(isNew && onNewVertex) (*onNewVertex)(owner->getInterfaceVertex(v));
}

HyperVertex HyperCreator::addEdge(NonHyper::Edge eNon) {
	assert(owner);
	const auto &dg = owner->nonHyper.getGraphDuringCalculation();
	auto &hyper = owner->hyper;
	NonHyper::Vertex vSrcNon = source(eNon, dg), vTarNon = target(eNon, dg);
	Hyper::Vertex v = add_vertex(hyper);
	hyper[v].kind = HyperVertexKind::Edge;
	hyper[v].graph = nullptr;
	hyper[v].edge = eNon;

	{ // source edges
		for(const lib::Graph::Single *g: dg[vSrcNon].graphs) {
			Hyper::Vertex vSrc = owner->getVertexFromGraph(g);
			add_edge(vSrc, v, hyper);
		}
	}
	{ // target edges
		for(const lib::Graph::Single *g: dg[vTarNon].graphs) {
			Hyper::Vertex vTar = owner->getVertexFromGraph(g);
			add_edge(v, vTar, hyper);
		}
	}

	if(onNewHyperEdge) (*onNewHyperEdge)(owner->getInterfaceEdge(v));
	return v;
}

//------------------------------------------------------------------------------
// Hyper
//------------------------------------------------------------------------------

Hyper::Hyper(const NonHyper &dg)
		: hasCalculated(false), nonHyper(dg) { }

std::pair<std::unique_ptr<Hyper>, HyperCreator> Hyper::makeHyper(
		const NonHyper &dg,
		std::shared_ptr<Function<void(dg::DG::Vertex)>> onNewVertex,
		std::shared_ptr<Function<void(dg::DG::HyperEdge)>> onNewHyperEdge) {
	auto hyper = std::unique_ptr<Hyper>(new Hyper(dg));
	auto &hyperRef = *hyper;
	return std::make_pair(std::move(hyper), HyperCreator(hyperRef, onNewVertex, onNewHyperEdge));
}

//Hyper::Hyper(const NonHyper &dgClass, int dummy)
//: hasCalculated(true), nonHyper(dgClass), vpGraph(nullptr) {
//	const NonHyperGraphType &dg = dgClass.getGraph();
//
//	// first go through each vertex in dg, and create the needed vertices in dgHyper representing graphs
//
//	for(Vertex v : asRange(vertices(dg))) {
//		const lib::Graph::Base *g = dg[v].graph;
//		assert(g);
//		for(const lib::Graph::Single *gSingle : g->getSingles()) addVertex(gSingle);
//	}
//
//	// then go through all edge and add vertices for them, and edges
//	std::map<NonHyperEdge, Vertex> edgeToHyper;
//
//	for(NonHyperEdge e : asRange(edges(dg))) {
//		const lib::Rules::Base *r = dg[e].rule;
//		Vertex from = source(e, dg);
//		const lib::Graph::Base *gFrom = dg[from].graph;
//		Vertex to = target(e, dg);
//		const lib::Graph::Base *gTo = dg[to].graph;
//		// it's a hyper edge, so first add the vertex implemening the edge
//		Hyper::Vertex v = add_vertex(hyper);
//		hyper[v].kind = VertexKind::Edge;
//		hyper[v].graph = nullptr;
//		hyper[v].rule = r;
//		hyper[v].reverse = hyper.null_vertex();
//		hyper[v].inVertex = from;
//		hyper[v].outVertex = to;
//		// insert to map
//		edgeToHyper[e] = v;
//		// fix reverse
//		std::optional<NonHyperEdge> reverse = dg[e].reverse;
//		if(reverse) {
//			auto iter = edgeToHyper.find(reverse.get());
//			if(iter != end(edgeToHyper)) {
//				// if both edge have been added, then fix it
//				Vertex ourReverse = iter->second;
//				hyper[v].reverse = ourReverse;
//				hyper[ourReverse].reverse = v;
//			}
//		}
//
//		// add in edges to the rule
//		{ // from
//			for(const lib::Graph::Single *g : gFrom->getSingles()) {
//				// get the vertex descriptor representing this graph
//				Hyper::Vertex vFrom = getVertexFromGraph(g);
//				add_edge(vFrom, v, hyper);
//			}
//		}
//
//		// add out edges from the rule
//		{ // to
//			for(const lib::Graph::Single *g : gTo->getSingles()) {
//				// get the vertex descriptor representing this graph
//				Hyper::Vertex vTo = getVertexFromGraph(g);
//				add_edge(v, vTo, hyper);
//			}
//		}
//	}
//}

std::pair<HyperVertex, bool> Hyper::addVertex(const lib::Graph::Single *g) {
	{ // sanity check
		assert(nonHyper.getGraphDatabase().contains(g->getAPIReference()));
	}
	const auto iter = graphToHyperVertex.find(g);
	if(iter == graphToHyperVertex.end()) { // create the vertex
		Vertex vNew = add_vertex(hyper);
		hyper[vNew].kind = HyperVertexKind::Vertex;
		hyper[vNew].graph = g;
		graphToHyperVertex[g] = vNew;
		return {vNew, true};
	} else {
		return {iter->second, false};
	}
}

Hyper::~Hyper() = default;

const NonHyper &Hyper::getNonHyper() const {
	return nonHyper;
}

const Hyper::GraphType &Hyper::getGraph() const {
	return hyper;
}

void Hyper::printStats(std::ostream &s) const {
	// statistics stuff
	unsigned int numVerts = 0, numIn = 0, numOut = 0, numReverse = 0;
	unsigned int numEdges = 0, numEdgePairs = 0;
	std::map<unsigned int, unsigned int> countNumIn, countNumOut;
	double avgInReverseRatio = 0, avgInReverseRatioWithVirtual = 0;
	double avgOutReverseRatio = 0, avgOutReverseRatioWithVirtual = 0;
	unsigned int projectedNumTransit = 0;

	unsigned int numUniqueIn = 0, numUniqueOut = 0;
	unsigned int projectedNumTransitTotal = 0;
	unsigned int numUniqueInOnlyReverse = 0, numUniqueOutOnlyReverse = 0;
	unsigned int projectedNumTransitCollapseNonReverse = 0;

	for(Vertex v: asRange(vertices(hyper))) {
		if(hyper[v].kind != HyperVertexKind::Vertex) {
			numEdges++;
			if(getReverseEdge(v) != hyper.null_vertex()) numEdgePairs++;
			continue;
		}
		numVerts++;
		numIn += in_degree(v, hyper);
		countNumIn[in_degree(v, hyper)]++;
		numOut += out_degree(v, hyper);
		countNumOut[out_degree(v, hyper)]++;
		for(Edge e: asRange(out_edges(v, hyper)))
			if(getReverseEdge(target(e, hyper)) != hyper.null_vertex())
				numReverse++;
		unsigned int numInReverse = 0;
		for(Edge e: asRange(in_edges(v, hyper)))
			if(getReverseEdge(source(e, hyper)) != hyper.null_vertex())
				numInReverse++;
		unsigned int numOutReverse = 0;
		for(Edge e: asRange(out_edges(v, hyper)))
			if(getReverseEdge(target(e, hyper)) != hyper.null_vertex())
				numOutReverse++;
		if(in_degree(v, hyper) > 0) {
			avgInReverseRatio += numInReverse / (double) in_degree(v, hyper);
		}
		avgInReverseRatioWithVirtual += (numInReverse + 1) / (double) (in_degree(v, hyper) + 1);
		if(out_degree(v, hyper)) {
			avgOutReverseRatio += numOutReverse / (double) out_degree(v, hyper);
		}
		avgOutReverseRatioWithVirtual += (numOutReverse + 1) / (double) (out_degree(v, hyper) + 1);
		if(in_degree(v, hyper) >= 10 || out_degree(v, hyper) >= 10) {
			s << "DG\t|in| = " << in_degree(v, hyper) << ", |out| = " << out_degree(v, hyper)
			  << "\t" << hyper[v].graph->getName() << std::endl;
			s << "\t" << numInReverse << " inReverse, " << numOutReverse << " outReverse" << std::endl;
		}
		projectedNumTransit += (numInReverse + 2) * (numOutReverse + 2);

		auto invAdjIters = asRange(inv_adjacent_vertices(v, hyper));
		std::unordered_set<Vertex> uniqueIn(begin(invAdjIters), end(invAdjIters));
		auto adjIters = asRange(adjacent_vertices(v, hyper));
		std::unordered_set<Vertex> uniqueOut(begin(adjIters), end(adjIters));

		unsigned int localNumUniqueInOnlyReverse = 1; // +1 from the virtual input/output edges
		unsigned int localNumUniqueOutOnlyReverse = 1;
		for(Vertex eIn: asRange(inv_adjacent_vertices(v, hyper))) {
			if(getReverseEdge(eIn) != hyper.null_vertex()) localNumUniqueInOnlyReverse++;
		}
		for(Vertex eOut: asRange(adjacent_vertices(v, hyper))) {
			if(getReverseEdge(eOut) != hyper.null_vertex()) localNumUniqueOutOnlyReverse++;
		}
		assert(localNumUniqueInOnlyReverse == localNumUniqueOutOnlyReverse); // right?
		numUniqueInOnlyReverse += localNumUniqueInOnlyReverse;
		numUniqueOutOnlyReverse += localNumUniqueOutOnlyReverse;

		// we only add these now because we implicitly had them in local-only-reverse-calculation
		uniqueIn.insert(hyper.null_vertex());
		uniqueOut.insert(hyper.null_vertex());

		numUniqueIn += uniqueIn.size();
		numUniqueOut += uniqueOut.size();
		projectedNumTransitTotal +=
				uniqueIn.size() * uniqueOut.size()
				- localNumUniqueInOnlyReverse; // we subtract these because they would be zero-constrained anyway

		// we add 1 because the catch-all would also make a sub-vertex
		projectedNumTransitCollapseNonReverse +=
				(localNumUniqueInOnlyReverse + 1) * (localNumUniqueOutOnlyReverse + 1)
				- localNumUniqueInOnlyReverse; // we subtract these because they would be zero-constrained anyway
	}
	numEdgePairs /= 2;
	avgInReverseRatio /= numVerts;
	avgInReverseRatioWithVirtual /= numVerts;
	avgOutReverseRatio /= numVerts;
	avgOutReverseRatioWithVirtual /= numVerts;

	s << "Stat--------------------------------------------------------------" << std::endl;
	s << "numVerts:\t" << numVerts << std::endl << "numIn:\t\t" << numIn << std::endl
	  << "numOut:\t\t" << numOut << std::endl << "numReverse:\t" << numReverse << std::endl
	  << "numEdges:\t" << numEdges << std::endl << "numEdgePairs:\t" << numEdgePairs << std::endl;
	s << "out/vert:\t" << (numOut / ((double) numVerts)) << std::endl;
	s << "in/vert:\t" << (numIn / ((double) numVerts)) << std::endl;
	s << "reverse/vert:\t" << (numReverse / ((double) numVerts)) << std::endl;
	s << "pairs/edge:\t" << (numEdgePairs / ((double) numEdges)) << std::endl;
	s << "------------------------------------------------------------------" << std::endl;
	s << "numIn histogram:" << std::endl;
	for(const auto &p: countNumIn) s << p.first << "\t" << p.second << std::endl;
	s << "------------------------------------------------------------------" << std::endl;
	s << "numOut histogram:" << std::endl;
	for(const auto &p: countNumIn) s << p.first << "\t" << p.second << std::endl;
	s << "------------------------------------------------------------------" << std::endl;
	s << "avgInReverseRatio\t" << avgInReverseRatio << std::endl;
	s << "avgInReverseRatioWithVirtual\t" << avgInReverseRatioWithVirtual << std::endl;
	s << "avgOutReverseRatio\t" << avgOutReverseRatio << std::endl;
	s << "avgOutReverseRatioWithVirtual\t" << avgOutReverseRatioWithVirtual << std::endl;
	s << "------------------------------------------------------------------" << std::endl;
	s << "projected numTransit:\t" << projectedNumTransit << std::endl;
	s << "------------------------------------------------------------------" << std::endl;
	s << "------------------------------------------------------------------" << std::endl;
	s << "numUniqueIn:              " << numUniqueIn << std::endl;
	s << "numUniqueOut:             " << numUniqueOut << std::endl;
	s << "projectedNumTransitTotal: " << projectedNumTransitTotal << std::endl;
	s << "------------------------------------------------------------------" << std::endl;
	s << "numUniqueInOnlyReverse:                " << numUniqueInOnlyReverse << std::endl;
	s << "numUniqueOutOnlyReverse:               " << numUniqueOutOnlyReverse << std::endl;
	s << "projectedNumTransitCollapseNonReverse: " << projectedNumTransitCollapseNonReverse << std::endl;
	s << "------------------------------------------------------------------" << std::endl;
}

bool Hyper::isVertexGraph(const lib::Graph::Single *g) const {
	const auto iter = graphToHyperVertex.find(g);
	return iter != graphToHyperVertex.end();
}

Hyper::Vertex Hyper::getVertexOrNullFromGraph(const lib::Graph::Single *g) const {
	const auto iter = graphToHyperVertex.find(g);
	if(iter == graphToHyperVertex.end()) return getGraph().null_vertex();
	else return iter->second;
}

Hyper::Vertex Hyper::getVertexFromGraph(const lib::Graph::Single *g) const {
	const auto iter = graphToHyperVertex.find(g);
	assert(iter != graphToHyperVertex.end());
	return iter->second;
}

Hyper::Vertex Hyper::getReverseEdge(Vertex e) const {
	assert(hyper[e].kind == HyperVertexKind::Edge);
	const auto &dgNon = getNonHyper().getGraph();
	const auto eNonReverse = dgNon[hyper[e].edge].reverse;
	if(eNonReverse)
		return dgNon[*eNonReverse].hyper;
	else
		return hyper.null_vertex();
}

const std::vector<const lib::Rules::Real *> &Hyper::getRulesFromEdge(Vertex e) const {
	assert(hyper[e].kind == HyperVertexKind::Edge);
	const auto &dgNon = getNonHyper().getGraph();
	return dgNon[hyper[e].edge].rules;
}

dg::DG::Vertex Hyper::getInterfaceVertex(Vertex v) const {
	if(v == hyper.null_vertex()) return dg::DG::Vertex();
	assert(hyper[v].kind == HyperVertexKind::Vertex);
	return dg::DG::Vertex(getNonHyper().getAPIReference(), get(boost::vertex_index_t(), getGraph(), v));
}

dg::DG::HyperEdge Hyper::getInterfaceEdge(Vertex e) const {
	if(e == hyper.null_vertex()) return dg::DG::HyperEdge();
	assert(hyper[e].kind == HyperVertexKind::Edge);
	return dg::DG::HyperEdge(getNonHyper().getAPIReference(), get(boost::vertex_index_t(), getGraph(), e));
}

Hyper::Vertex Hyper::getInternalVertex(const dg::DG::Vertex &v) const {
	using boost::vertices;
	const auto &dg = getGraph();
	const auto iter = vertices(dg).first + v.getId();
	assert(dg[*iter].kind == HyperVertexKind::Vertex);
	return *iter;
}

Hyper::Vertex Hyper::getInternalVertex(const dg::DG::HyperEdge &e) const {
	using boost::vertices;
	const auto &dg = getGraph();
	const auto iter = vertices(dg).first + e.getId();
	assert(dg[*iter].kind == HyperVertexKind::Edge);
	return *iter;
}

mod::Derivation Hyper::getDerivation(Vertex v) const {
	assert(v != hyper.null_vertex());
	assert(hyper[v].kind == HyperVertexKind::Edge);
	Derivation d;
	for(Vertex vIn: asRange(inv_adjacent_vertices(v, hyper)))
		d.left.push_back(hyper[vIn].graph->getAPIReference());
	const auto &rules = getRulesFromEdge(v);
	if(!rules.empty())
		d.r = rules.front()->getAPIReference();
	for(Vertex vOut: asRange(adjacent_vertices(v, hyper)))
		d.right.push_back(hyper[vOut].graph->getAPIReference());
	return d;
}

} // namespace mod::lib::DG