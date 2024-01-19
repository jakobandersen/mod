#include "NonHyper.hpp"

// for debugging
#include <mod/Misc.hpp>
#include <mod/graph/GraphInterface.hpp>
// end for debugging

#include <mod/Config.hpp>
#include <mod/Error.hpp>
#include <mod/dg/DG.hpp>
#include <mod/dg/GraphInterface.hpp>
#include <mod/graph/Graph.hpp>
#include <mod/rule/Rule.hpp>
#include <mod/lib/Chem/MoleculeUtil.hpp>
#include <mod/lib/DG/Hyper.hpp>
#include <mod/lib/DG/IO/Write.hpp>
#include <mod/lib/Graph/Single.hpp>
#include <mod/lib/Graph/Properties/Molecule.hpp>
#include <mod/lib/Graph/Properties/Stereo.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/Graph/Properties/Term.hpp>
#include <mod/lib/IO/IO.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>

#include <iostream>
#include <unordered_set>

namespace mod::lib::DG {
namespace {
std::size_t nextDGNum = 0;
} // namespace

NonHyper::NonHyper(LabelSettings labelSettings,
                   const std::vector<std::shared_ptr<graph::Graph> > &graphDatabase, IsomorphismPolicy graphPolicy)
		: id(nextDGNum++),
		  labelSettings(labelSettings),
		  graphDatabase(labelSettings, getConfig().graph.isomorphismAlg.get()) {
	switch(graphPolicy) {
	case IsomorphismPolicy::TrustMe:
		for(const auto &gCand : graphDatabase) {
			assert(gCand);
			this->graphDatabase.trustInsert(gCand);
		}
		break;
	case IsomorphismPolicy::Check: {
		const auto ls = LabelSettings{this->labelSettings.type, LabelRelation::Isomorphism,
		                              this->labelSettings.withStereo, LabelRelation::Isomorphism};
		for(const auto &gCand : graphDatabase) {
			if(ls.type == LabelType::Term) {
				const auto &term = get_term(gCand->getGraph().getLabelledGraph());
				if(!isValid(term)) {
					std::string msg = "Parsing failed for graph '" + gCand->getName() +
					                  "' in graph database. " + term.getParsingError();
					throw TermParsingError(std::move(msg));
				}
			}
			const auto gp = this->graphDatabase.tryInsert(gCand);
			// we don't care if the user gives the same graph multiple times
			if(gp.first != gCand) {
				std::string msg = "Isomorphic graphs '" + gp.first->getName() + "' and '" + gCand->getName() +
				                  "' in initial graph database.";
				throw LogicError(std::move(msg));
			}
		}
	} // case Check
		break;
	} // switch(graphPolicy)
}

NonHyper::~NonHyper() = default;

std::size_t NonHyper::getId() const {
	return id;
}

std::shared_ptr<dg::DG> NonHyper::getAPIReference() const {
	if(apiReference.use_count() > 0) return std::shared_ptr<dg::DG>(apiReference);
	else std::abort();
}

void NonHyper::setAPIReference(std::shared_ptr<dg::DG> dg) {
	assert(apiReference.use_count() == 0);
	apiReference = dg;
	assert(&dg->getNonHyper() == this);
}

LabelSettings NonHyper::getLabelSettings() const {
	return labelSettings;
}

bool NonHyper::getHasStartedCalculation() const {
	return hasStartedCalculation;
}

bool NonHyper::getHasCalculated() const {
	return hasCalculated;
}

void NonHyper::calculatePrologue(
		std::shared_ptr<Function<void(dg::DG::Vertex)>> onNewVertex,
		std::shared_ptr<Function<void(dg::DG::HyperEdge)>> onNewHyperEdge) {
	if(getHasStartedCalculation()) MOD_ABORT;
	if(hyperCreator) MOD_ABORT;
	hasStartedCalculation = true;
	auto p = Hyper::makeHyper(*this, onNewVertex, onNewHyperEdge);
	hyper = std::move(p.first);
	hyperCreator.reset(new HyperCreator(std::move(p.second)));
}

void NonHyper::calculateEpilogue() {
	assert(getHasStartedCalculation());
	// annotate the graph with reversible pairs
	findReversiblePairs();
	hyperCreator.reset();
	hasCalculated = true;
}

void NonHyper::tryAddGraph(std::shared_ptr<graph::Graph> gCand) {
	if(getHasCalculated()) std::abort();
	const auto ls = LabelSettings{
			getLabelSettings().type,
			LabelRelation::Isomorphism,
			getLabelSettings().withStereo,
			LabelRelation::Isomorphism};
	if(ls.type == LabelType::Term) {
		const auto &term = get_term(gCand->getGraph().getLabelledGraph());
		if(!isValid(term)) {
			std::string msg = "Parsing failed for graph '" + gCand->getName() + "'. " +
			                  term.getParsingError();
			throw TermParsingError(std::move(msg));
		}
	}
	const auto gp = this->graphDatabase.tryInsert(gCand);
	// (we don't care if the user gives the same graph multiple times)
	if(gp.first != gCand) {
		std::string msg =
				"Isomorphic graphs. Candidate graph '" + gCand->getName() + "' is isomorphic to '" + gp.first->getName() +
				"' in the graph database.";
		throw LogicError(std::move(msg));
	}
}

bool NonHyper::trustAddGraph(std::shared_ptr<graph::Graph> g) {
	if(getHasCalculated()) std::abort();
	return graphDatabase.trustInsert(g);
}

bool NonHyper::trustAddGraphAsVertex(std::shared_ptr<graph::Graph> g) {
	if(getHasCalculated()) std::abort();
	bool inserted = trustAddGraph(g);
	getVertex(GraphMultiset(&g->getGraph()));
	return inserted;
}

std::pair<std::shared_ptr<graph::Graph>, std::unique_ptr<lib::Graph::Single>>
NonHyper::checkIfNew(std::unique_ptr<lib::Graph::Single> gCand) const {
	assert(gCand);
	{
		const auto g = graphDatabase.findIsomorphic(gCand.get());
		if(g) return {g, std::move(gCand)};
	}
	auto g = graph::Graph::create(std::move(gCand));
	return {g, nullptr};
}

bool NonHyper::addProduct(std::shared_ptr<graph::Graph> g) {
	assert(g);
	const bool isNewGraph = trustAddGraph(g);
	if(isNewGraph) {
		g->setName("p_{" + boost::lexical_cast<std::string>(getId())
		           + "," + boost::lexical_cast<std::string>(productNum++)
		           + "}");
		assert(std::find(begin(products), end(products), g) == end(products));
		products.push_back(g);
	}
	return isNewGraph;
}

std::pair<NonHyper::Edge, bool> NonHyper::isDerivation(const GraphMultiset &gmsSrc,
                                                       const GraphMultiset &gmsTar,
                                                       const lib::Rules::Real *r) const {
	const auto iterLeft = multisetToVertex.find(gmsSrc);
	if(iterLeft == end(multisetToVertex)) return std::make_pair(Edge(), false);
	const auto iterRight = multisetToVertex.find(gmsTar);
	if(iterRight == end(multisetToVertex)) return std::make_pair(Edge(), false);
	return edge(iterLeft->second, iterRight->second, dg);
}

std::pair<NonHyper::Edge, bool> NonHyper::suggestDerivation(
		const GraphMultiset &gmsSrc, const GraphMultiset &gmsTar, const lib::Rules::Real *r) {
	assert(!gmsSrc.empty());
	assert(!gmsTar.empty());
	// make vertices for to and from
	Vertex vSrc = getVertex(gmsSrc), vTar = getVertex(gmsTar);
	std::pair<Edge, bool> e = edge(vSrc, vTar, dg);
	if(!e.second) { // note: add_edge does not yet support rvalue refs, so we modify later
		e = add_edge(vSrc, vTar, dg);
		if(r) dg[e.first].rules.push_back(r);
		dg[e.first].hyper = hyperCreator->addEdge(e.first);
	} else {
		e.second = false;
		if(r) {
			auto &rules = dg[e.first].rules;
			auto iter = std::find(rules.begin(), rules.end(), r);
			if(iter == rules.end())
				rules.push_back(r);
		}
	}
	return e;
}

const NonHyper::GraphType &NonHyper::getGraphDuringCalculation() const {
	return dg;
}

NonHyper::Vertex NonHyper::getVertex(const GraphMultiset &gms) {
	const auto iter = multisetToVertex.find(gms);
	if(iter != multisetToVertex.end()) return iter->second;
	assert(hyperCreator);
	Vertex v = add_vertex(dg);
	dg[v].graphs = gms;
	multisetToVertex[gms] = v;
	for(auto *gSub : gms) hyperCreator->addVertex(gSub);
	return v;
}

void NonHyper::findReversiblePairs() {
	for(Edge e : asRange(edges(dg))) {
		Vertex vSource = source(e, dg);
		Vertex vTarget = target(e, dg);
		for(Edge eBack : asRange(out_edges(vTarget, dg))) {
			if(target(eBack, dg) == vSource)
				dg[e].reverse = eBack;
		}
	}
}

const NonHyper::GraphType &NonHyper::getGraph() const {
	if(!hyper) MOD_ABORT;
	return dg;
}

const Hyper &NonHyper::getHyper() const {
	if(!hyper) MOD_ABORT;
	return *hyper;
}

const Graph::Collection &NonHyper::getGraphDatabase() const {
	return graphDatabase;
}

const std::vector<std::shared_ptr<graph::Graph> > &NonHyper::getProducts() const {
	return products;
}

void NonHyper::print() const {
	if(!getHasCalculated()) std::abort();
	std::string fileNoExt = Write::pdfNonHyper(*this);
	IO::post() << "summaryDGNonHyper \"dg_" << getId() << "\" \"" << fileNoExt << "\"" << std::endl;
}

HyperVertex NonHyper::getHyperEdge(Edge e) const {
	return dg[e].hyper;
}

HyperVertex NonHyper::findHyperEdge(const std::vector<Hyper::Vertex> &sources,
                                    const std::vector<Hyper::Vertex> &targets) const {
	const auto &dgHyper = getHyper().getGraph();
#ifndef NDEBUG
	for(auto v : sources) assert(dgHyper[v].kind == HyperVertexKind::Vertex);
	for(auto v : targets) assert(dgHyper[v].kind == HyperVertexKind::Vertex);
#endif

	std::vector<const lib::Graph::Single *> srcGraphs, tarGraphs;
	srcGraphs.reserve(sources.size());
	tarGraphs.reserve(targets.size());
	for(const auto v : sources) srcGraphs.push_back(dgHyper[v].graph);
	for(const auto v : targets) tarGraphs.push_back(dgHyper[v].graph);
	GraphMultiset gmsSrc(std::move(srcGraphs)), gmsTar(std::move(tarGraphs));
	const auto iterSrc = multisetToVertex.find(gmsSrc);
	if(iterSrc == end(multisetToVertex))
		return boost::graph_traits<HyperGraphType>::null_vertex();
	const auto iterTar = multisetToVertex.find(gmsTar);
	if(iterTar == end(multisetToVertex))
		return boost::graph_traits<HyperGraphType>::null_vertex();
	const auto vSrc = iterSrc->second, vTar = iterTar->second;

	const std::pair<Edge, bool> p = edge(vSrc, vTar, dg);
	if(!p.second)
		return boost::graph_traits<HyperGraphType>::null_vertex();
	return dg[p.first].hyper;
}

void NonHyper::diff(const NonHyper &dg1, const NonHyper &dg2) {
	bool headerPrinted = false;
	const auto printHeader = [&headerPrinted, &dg1, &dg2]() -> std::ostream & {
		if(headerPrinted) return std::cout;
		else headerPrinted = true;
		std::cout << "--- DG " << dg1.getId() << std::endl;
		std::cout << "+++ DG " << dg2.getId() << std::endl;
		return std::cout;
	};
	std::ostream &s = std::cout;
	// diff vertices
	std::unordered_set<std::shared_ptr<graph::Graph> > vertexGraphsUnique;
	for(const auto v : dg1.getAPIReference()->vertices()) vertexGraphsUnique.insert(v.getGraph());
	for(const auto v : dg2.getAPIReference()->vertices()) vertexGraphsUnique.insert(v.getGraph());
	std::vector<std::shared_ptr<graph::Graph> > vertexGraphs(begin(vertexGraphsUnique), end(vertexGraphsUnique));
	std::sort(begin(vertexGraphs), end(vertexGraphs),
	          [](std::shared_ptr<graph::Graph> g1, std::shared_ptr<graph::Graph> g2) {
		          return g1->getName() < g2->getName();
	          });
	for(const auto &g : vertexGraphs) {
		const bool in1 = dg1.getHyper().isVertexGraph(&g->getGraph());
		const bool in2 = dg2.getHyper().isVertexGraph(&g->getGraph());
		assert(in1 || in2);
		if(!in1) printHeader() << "-";
		if(!in2) printHeader() << "+";
		if(!in1 || !in2) s << *g << std::endl;
	}
	// diff edges
	const auto makeDers = [](const auto &dg) {
		const auto order = [](Derivation d) -> Derivation {
			std::sort(begin(d.left), end(d.left), graph::GraphLess());
			std::sort(begin(d.right), end(d.right), graph::GraphLess());
			return d;
		};
		std::vector<Derivation> ders;
		for(const auto &e : dg.edges()) {
			for(const auto &r : e.rules()) {
				Derivation d;
				for(const auto &v : e.sources())
					d.left.push_back(v.getGraph());
				d.r = r;
				for(const auto &v : e.targets())
					d.right.push_back(v.getGraph());
				ders.push_back(order(std::move(d)));
			}
		}
		return ders;
	};
	std::vector<Derivation>
			ders1 = makeDers(*dg1.getAPIReference()),
			ders2 = makeDers(*dg2.getAPIReference());
	const auto derLess = [](const Derivation &d1, const Derivation &d2) {
		if(std::lexicographical_compare(begin(d1.left), end(d1.left),
		                                begin(d2.left), end(d2.left),
		                                graph::GraphLess()))
			return true;
		else if(std::lexicographical_compare(begin(d2.left), end(d2.left),
		                                     begin(d1.left), end(d1.left),
		                                     graph::GraphLess()))
			return false;
		if(std::lexicographical_compare(begin(d1.right), end(d1.right),
		                                begin(d2.right), end(d2.right),
		                                graph::GraphLess()))
			return true;
		else if(std::lexicographical_compare(begin(d2.right), end(d2.right),
		                                     begin(d1.right), end(d1.right),
		                                     graph::GraphLess()))
			return false;
		return d1.r->getId() < d2.r->getId();
	};
	std::sort(begin(ders1), end(ders1), derLess);
	std::sort(begin(ders2), end(ders2), derLess);
	auto iter1 = begin(ders1), iter2 = begin(ders2);
	while(true) {
		if(iter1 == end(ders1)) {
			for(; iter2 != end(ders2); iter2++) printHeader() << "+" << *iter2 << std::endl;
			break;
		}
		if(iter2 == end(ders2)) {
			for(; iter1 != end(ders1); iter1++) printHeader() << "-" << *iter1 << std::endl;
			break;
		}
		if(derLess(*iter1, *iter2)) {
			printHeader() << "-" << *iter1 << std::endl;
			iter1++;
		} else if(derLess(*iter2, *iter1)) {
			printHeader() << "+" << *iter2 << std::endl;
			iter2++;
		} else {
			iter1++;
			iter2++;
		}
	}
}

} // namespace mod::lib::DG
