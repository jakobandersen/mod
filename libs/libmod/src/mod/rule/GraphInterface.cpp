#include "GraphInterface.hpp"

#include <mod/Chem.hpp>
#include <mod/graph/Printer.hpp>
#include <mod/lib/GraphPimpl.hpp>
#include <mod/lib/Rule/IO/DepictionData.hpp>
#include <mod/lib/Rule/Rule.hpp>
#include <mod/lib/Rule/IO/Write.hpp>
#include <mod/lib/Rule/Properties/Molecule.hpp>

namespace mod::rule {

//==============================================================================
// Left
//==============================================================================

std::ostream &operator<<(std::ostream &s, const Rule::LeftGraph &g) {
	return s << "RuleLeftGraph(" << *g.getRule() << ")";
}

std::size_t Rule::LeftGraph::numVertices() const {
	using boost::vertices;
	const auto &rDPO = r->getRule().getDPORule().getRule();
	const auto &vs = vertices(getL(rDPO));
	return std::distance(vs.first, vs.second);
}

std::size_t Rule::LeftGraph::numEdges() const {
	using boost::edges;
	const auto &rDPO = r->getRule().getDPORule().getRule();
	const auto &es = edges(getL(rDPO));
	return std::distance(es.first, es.second);
}

//------------------------------------------------------------------------------
// Vertex
//------------------------------------------------------------------------------

MOD_GRAPHPIMPL_Define_Vertex_noGraph_noId(Rule::LeftGraph, RuleLeftGraph,
                                          getL(this->g->getRule()->getRule().getDPORule().getRule()), g, ->getRule())
MOD_GRAPHPIMPL_Define_Vertex_graph(Rule::LeftGraph, g)

std::size_t Rule::LeftGraph::Vertex::getId() const {
	return getCore().getId();
}

MOD_GRAPHPIMPL_Define_Vertex_Undirected(Rule::LeftGraph,
                                        getL(this->g->getRule()->getRule().getDPORule().getRule()), g)

const std::string &Rule::LeftGraph::Vertex::getStringLabel() const {
	if(!*this) throw LogicError("Can not get string label on a null vertex.");
	const auto &libRule = g->getRule()->getRule();
	const auto &lr = libRule.getDPORule();
	const auto v = libRule.getLeftInternalVertex(*this);
	return get_string(lr).getLeft()[v];
}

AtomId Rule::LeftGraph::Vertex::getAtomId() const {
	if(!*this) throw LogicError("Can not get atom id on a null vertex.");
	const auto &libRule = g->getRule()->getRule();
	const auto &lr = libRule.getDPORule();
	const auto v = libRule.getLeftInternalVertex(*this);
	return get_molecule(lr).getLeft()[v].getAtomId();
}

Isotope Rule::LeftGraph::Vertex::getIsotope() const {
	if(!*this) throw LogicError("Can not get isotope on a null vertex.");
	const auto &libRule = g->getRule()->getRule();
	const auto &lr = libRule.getDPORule();
	const auto v = libRule.getLeftInternalVertex(*this);
	return get_molecule(lr).getLeft()[v].getIsotope();
}

Charge Rule::LeftGraph::Vertex::getCharge() const {
	if(!*this) throw LogicError("Can not get charge on a null vertex.");
	const auto &libRule = g->getRule()->getRule();
	const auto &lr = libRule.getDPORule();
	const auto v = libRule.getLeftInternalVertex(*this);
	return get_molecule(lr).getLeft()[v].getCharge();
}

bool Rule::LeftGraph::Vertex::getRadical() const {
	if(!*this) throw LogicError("Can not get radical status on a null vertex.");
	const auto &libRule = g->getRule()->getRule();
	const auto &lr = libRule.getDPORule();
	const auto v = libRule.getLeftInternalVertex(*this);
	return get_molecule(lr).getLeft()[v].getRadical();
}

std::string Rule::LeftGraph::Vertex::printStereo() const {
	graph::Printer p;
	p.setReactionDefault();
	p.setWithIndex(true);
	p.setCollapseHydrogens(false);
	p.setSimpleCarbons(false);
	return printStereo(p);
}

std::string Rule::LeftGraph::Vertex::printStereo(const graph::Printer &p) const {
	if(!*this) throw LogicError("Can not print stereo on a null vertex.");
	const auto &libRule = g->getRule()->getRule();
	const auto &lr = libRule.getDPORule();
	const auto &rDPO = lr.getRule();
	const auto v = libRule.getLeftInternalVertex(*this);
	const auto vCG = get(rDPO.getLtoCG(), getL(rDPO), rDPO.getCombinedGraph(), v);
	return lib::rule::Write::stereoSummary(libRule, vCG, lib::rule::Membership::L, p.getOptions());
}

Rule::Vertex Rule::LeftGraph::Vertex::getCore() const {
	if(isNull()) return Rule::Vertex();
	const auto &libRule = g->getRule()->getRule();
	const auto &lr = libRule.getDPORule();
	const auto &rDPO = lr.getRule();
	const auto v = libRule.getLeftInternalVertex(*this);
	const auto vCG = get(rDPO.getLtoCG(), getL(rDPO), rDPO.getCombinedGraph(), v);
	const auto vCGId = get(boost::vertex_index_t(), rDPO.getCombinedGraph(), vCG);
	return Rule::Vertex(g->getRule(), vCGId);
}

std::shared_ptr<Rule> Rule::LeftGraph::Vertex::getRule() const {
	if(!*this) throw LogicError("Can not get rule on a null vertex.");
	return g->getRule();
}

//------------------------------------------------------------------------------
// Edge
//------------------------------------------------------------------------------

MOD_GRAPHPIMPL_Define_Indices(Rule::LeftGraph, RuleLeftGraph,
                              getL(this->g->getRule()->getRule().getDPORule().getRule()), g, ->getRule())

BOOST_CONCEPT_ASSERT((boost::ForwardIterator<Rule::LeftGraph::VertexIterator>));
BOOST_CONCEPT_ASSERT((boost::ForwardIterator<Rule::LeftGraph::EdgeIterator>));
BOOST_CONCEPT_ASSERT((boost::ForwardIterator<Rule::LeftGraph::IncidentEdgeIterator>));

const std::string &Rule::LeftGraph::Edge::getStringLabel() const {
	if(!*this) throw LogicError("Can not get string label on a null edge.");
	const auto &lr = g->getRule()->getRule().getDPORule();
	const auto &rDPO = lr.getRule();
	using boost::vertices;
	const auto v = *std::next(vertices(getL(rDPO)).first, vId);
	const auto e = *std::next(out_edges(v, getL(rDPO)).first, eId);
	return get_string(lr).getLeft()[e];
}

BondType Rule::LeftGraph::Edge::getBondType() const {
	if(!*this) throw LogicError("Can not get bond type on a null edge.");
	const auto &lr = g->getRule()->getRule().getDPORule();
	const auto &rDPO = lr.getRule();
	using boost::vertices;
	const auto v = *std::next(vertices(getL(rDPO)).first, vId);
	const auto e = *std::next(out_edges(v, getL(rDPO)).first, eId);
	return get_molecule(lr).getLeft()[e];
}

Rule::Edge Rule::LeftGraph::Edge::getCore() const {
	if(isNull()) return Rule::Edge();
	const auto &rDPO = g->getRule()->getRule().getDPORule().getRule();
	using boost::vertices;
	const auto v = *std::next(vertices(getL(rDPO)).first, vId);
	const auto e = *std::next(out_edges(v, getL(rDPO)).first, eId);
	const auto vCG = get(rDPO.getLtoCG(), getL(rDPO), rDPO.getCombinedGraph(), v);
	const auto eCG = get(rDPO.getLtoCG(), getL(rDPO), rDPO.getCombinedGraph(), e);
	const auto vCGId = get(boost::vertex_index_t(), rDPO.getCombinedGraph(), vCG);
	const auto &esCG = out_edges(vCG, rDPO.getCombinedGraph());
	const auto eIter = std::find(esCG.first, esCG.second, eCG);
	assert(eIter != esCG.second);
	const auto eCGOffset = std::distance(esCG.first, eIter);
	return Rule::Edge(g->getRule(), vCGId, eCGOffset);
}

std::shared_ptr<Rule> Rule::LeftGraph::Edge::getRule() const {
	if(!*this) throw LogicError("Can not get rule on a null edge.");
	return g->getRule();
}


//==============================================================================
// Context
//==============================================================================

std::ostream &operator<<(std::ostream &s, const Rule::ContextGraph &g) {
	return s << "RuleContextGraph(" << *g.getRule() << ")";
}

std::size_t Rule::ContextGraph::numVertices() const {
	using boost::vertices;
	const auto &graph = getK(r->getRule().getDPORule().getRule());
	const auto &vs = vertices(graph);
	return std::distance(vs.first, vs.second);
}

std::size_t Rule::ContextGraph::numEdges() const {
	using boost::edges;
	const auto &graph = getK(r->getRule().getDPORule().getRule());
	const auto &es = edges(graph);
	return std::distance(es.first, es.second);
}

//------------------------------------------------------------------------------
// Vertex
//------------------------------------------------------------------------------

MOD_GRAPHPIMPL_Define_Vertex(Rule::ContextGraph, RuleContextGraph,
                             getK(this->g->getRule()->getRule().getDPORule().getRule()), g, ->getRule())
MOD_GRAPHPIMPL_Define_Vertex_Undirected(Rule::ContextGraph,
                                        getK(this->g->getRule()->getRule().getDPORule().getRule()), g)

Rule::Vertex Rule::ContextGraph::Vertex::getCore() const {
	if(isNull()) return Rule::Vertex();
	const auto &rDPO = g->getRule()->getRule().getDPORule().getRule();
	using boost::vertices;
	const auto v = *std::next(vertices(getK(rDPO)).first, vId);
	const auto vCG = get(rDPO.getKtoCG(), getK(rDPO), rDPO.getCombinedGraph(), v);
	const auto vCGId = get(boost::vertex_index_t(), rDPO.getCombinedGraph(), vCG);
	return Rule::Vertex(g->getRule(), vCGId);
}

std::shared_ptr<Rule> Rule::ContextGraph::Vertex::getRule() const {
	if(!g) throw LogicError("Can not get rule on a null vertex.");
	return g->getRule();
}

//------------------------------------------------------------------------------
// Edge
//------------------------------------------------------------------------------

MOD_GRAPHPIMPL_Define_Indices(Rule::ContextGraph, RuleContextGraph,
                              getK(this->g->getRule()->getRule().getDPORule().getRule()), g, ->getRule())

BOOST_CONCEPT_ASSERT((boost::ForwardIterator<Rule::ContextGraph::VertexIterator>));
BOOST_CONCEPT_ASSERT((boost::ForwardIterator<Rule::ContextGraph::EdgeIterator>));
BOOST_CONCEPT_ASSERT((boost::ForwardIterator<Rule::ContextGraph::IncidentEdgeIterator>));

Rule::Edge Rule::ContextGraph::Edge::getCore() const {
	if(isNull()) return Rule::Edge();
	const auto &rDPO = g->getRule()->getRule().getDPORule().getRule();
	using boost::vertices;
	const auto v = *std::next(vertices(getK(rDPO)).first, vId);
	const auto e = *std::next(out_edges(v, getK(rDPO)).first, eId);
	const auto vCG = get(rDPO.getKtoCG(), getK(rDPO), rDPO.getCombinedGraph(), v);
	const auto eCG = get(rDPO.getKtoCG(), getK(rDPO), rDPO.getCombinedGraph(), e);
	const auto vCGId = get(boost::vertex_index_t(), rDPO.getCombinedGraph(), vCG);
	const auto &esCG = out_edges(vCG, rDPO.getCombinedGraph());
	const auto eIter = std::find(esCG.first, esCG.second, eCG);
	assert(eIter != esCG.second);
	const auto eCGOffset = std::distance(esCG.first, eIter);
	return Rule::Edge(g->getRule(), vCGId, eCGOffset);
}

std::shared_ptr<Rule> Rule::ContextGraph::Edge::getRule() const {
	if(!g) throw LogicError("Can not get rule on a null edge.");
	return g->getRule();
}

//==============================================================================
// Right
//==============================================================================

std::ostream &operator<<(std::ostream &s, const Rule::RightGraph &g) {
	return s << "RuleRightGraph(" << *g.getRule() << ")";
}

std::size_t Rule::RightGraph::numVertices() const {
	using boost::vertices;
	const auto &graph = getR(r->getRule().getDPORule().getRule());
	const auto &vs = vertices(graph);
	return std::distance(vs.first, vs.second);
}

std::size_t Rule::RightGraph::numEdges() const {
	using boost::edges;
	const auto &graph = getR(r->getRule().getDPORule().getRule());
	const auto &es = edges(graph);
	return std::distance(es.first, es.second);
}

//------------------------------------------------------------------------------
// Vertex
//------------------------------------------------------------------------------

MOD_GRAPHPIMPL_Define_Vertex_noGraph_noId(Rule::RightGraph, RuleRightGraph,
                                          getR(this->g->getRule()->getRule().getDPORule().getRule()), g, ->getRule())
MOD_GRAPHPIMPL_Define_Vertex_graph(Rule::RightGraph, g)

std::size_t Rule::RightGraph::Vertex::getId() const {
	return getCore().getId();
}

MOD_GRAPHPIMPL_Define_Vertex_Undirected(Rule::RightGraph,
                                        getR(this->g->getRule()->getRule().getDPORule().getRule()), g)

const std::string &Rule::RightGraph::Vertex::getStringLabel() const {
	if(!g) throw LogicError("Can not get string label on a null vertex.");
	const auto &libRule = g->getRule()->getRule();
	const auto &lr = libRule.getDPORule();
	const auto v = libRule.getRightInternalVertex(*this);
	return get_string(lr).getRight()[v];
}

AtomId Rule::RightGraph::Vertex::getAtomId() const {
	if(!g) throw LogicError("Can not get atom id on a null vertex.");
	const auto &libRule = g->getRule()->getRule();
	const auto &lr = libRule.getDPORule();
	const auto v = libRule.getRightInternalVertex(*this);
	return get_molecule(lr).getRight()[v].getAtomId();
}

Isotope Rule::RightGraph::Vertex::getIsotope() const {
	if(!g) throw LogicError("Can not get isotope on a null vertex.");
	const auto &libRule = g->getRule()->getRule();
	const auto &lr = libRule.getDPORule();
	const auto v = libRule.getRightInternalVertex(*this);
	return get_molecule(lr).getRight()[v].getIsotope();
}

Charge Rule::RightGraph::Vertex::getCharge() const {
	if(!g) throw LogicError("Can not get charge on a null vertex.");
	const auto &libRule = g->getRule()->getRule();
	const auto &lr = libRule.getDPORule();
	const auto v = libRule.getRightInternalVertex(*this);
	return get_molecule(lr).getRight()[v].getCharge();
}

bool Rule::RightGraph::Vertex::getRadical() const {
	if(!g) throw LogicError("Can not get radical status on a null vertex.");
	const auto &libRule = g->getRule()->getRule();
	const auto &lr = libRule.getDPORule();
	const auto v = libRule.getRightInternalVertex(*this);
	return get_molecule(lr).getRight()[v].getRadical();
}

Rule::Vertex Rule::RightGraph::Vertex::getCore() const {
	if(isNull()) return Rule::Vertex();
	const auto &libRule = g->getRule()->getRule();
	const auto &lr = libRule.getDPORule();
	const auto &rDPO = lr.getRule();
	const auto v = libRule.getRightInternalVertex(*this);
	const auto vCG = get(rDPO.getRtoCG(), getR(rDPO), rDPO.getCombinedGraph(), v);
	const auto vCGId = get(boost::vertex_index_t(), rDPO.getCombinedGraph(), vCG);
	return Rule::Vertex(getRule(), vCGId);
}

std::shared_ptr<Rule> Rule::RightGraph::Vertex::getRule() const {
	if(!g) throw LogicError("Can not get rule on a null vertex.");
	return g->getRule();
}

//------------------------------------------------------------------------------

std::string Rule::RightGraph::Vertex::printStereo() const {
	graph::Printer p;
	p.setReactionDefault();
	p.setWithIndex(true);
	p.setCollapseHydrogens(false);
	p.setSimpleCarbons(false);
	return printStereo(p);
}

std::string Rule::RightGraph::Vertex::printStereo(const graph::Printer &p) const {
	if(!g) throw LogicError("Can not print stereo on a null vertex.");
	const auto &libRule = g->getRule()->getRule();
	const auto &lr = libRule.getDPORule();
	const auto &rDPO = lr.getRule();
	const auto v = libRule.getRightInternalVertex(*this);
	const auto vCG = get(rDPO.getRtoCG(), getR(rDPO), rDPO.getCombinedGraph(), v);
	return lib::rule::Write::stereoSummary(getRule()->getRule(), vCG, lib::rule::Membership::R, p.getOptions());
}

//------------------------------------------------------------------------------
// Edge
//------------------------------------------------------------------------------

MOD_GRAPHPIMPL_Define_Indices(Rule::RightGraph, RuleRightGraph,
                              getR(this->g->getRule()->getRule().getDPORule().getRule()), g, ->getRule())

BOOST_CONCEPT_ASSERT((boost::ForwardIterator<Rule::RightGraph::VertexIterator>));
BOOST_CONCEPT_ASSERT((boost::ForwardIterator<Rule::RightGraph::EdgeIterator>));
BOOST_CONCEPT_ASSERT((boost::ForwardIterator<Rule::RightGraph::IncidentEdgeIterator>));

const std::string &Rule::RightGraph::Edge::getStringLabel() const {
	if(!g) throw LogicError("Can not get string label on a null edge.");
	const auto &graph = getR(getRule()->getRule().getDPORule().getRule());
	using boost::vertices;
	const auto v = *std::next(vertices(graph).first, vId);
	const auto e = *std::next(out_edges(v, graph).first, eId);
	return get_string(getRule()->getRule().getDPORule()).getRight()[e];
}

BondType Rule::RightGraph::Edge::getBondType() const {
	if(!g) throw LogicError("Can not get bond type on a null edge.");
	const auto &graph = getR(getRule()->getRule().getDPORule().getRule());
	using boost::vertices;
	const auto v = *std::next(vertices(graph).first, vId);
	const auto e = *std::next(out_edges(v, graph).first, eId);
	return get_molecule(getRule()->getRule().getDPORule()).getRight()[e];
}

Rule::Edge Rule::RightGraph::Edge::getCore() const {
	if(isNull()) return Rule::Edge();
	const auto &rDPO = g->getRule()->getRule().getDPORule().getRule();
	using boost::vertices;
	const auto v = *std::next(vertices(getR(rDPO)).first, vId);
	const auto e = *std::next(out_edges(v, getR(rDPO)).first, eId);
	const auto vCG = get(rDPO.getRtoCG(), getR(rDPO), rDPO.getCombinedGraph(), v);
	const auto eCG = get(rDPO.getRtoCG(), getR(rDPO), rDPO.getCombinedGraph(), e);
	const auto vCGId = get(boost::vertex_index_t(), rDPO.getCombinedGraph(), vCG);
	const auto &esCG = out_edges(vCG, rDPO.getCombinedGraph());
	const auto eIter = std::find(esCG.first, esCG.second, eCG);
	assert(eIter != esCG.second);
	const auto eCGOffset = std::distance(esCG.first, eIter);
	return Rule::Edge(g->getRule(), vCGId, eCGOffset);
}

std::shared_ptr<Rule> Rule::RightGraph::Edge::getRule() const {
	if(!g) throw LogicError("Can not get rule on a null edge.");
	return g->getRule();
}


//==============================================================================
// Core
//==============================================================================

//------------------------------------------------------------------------------
// Vertex
//------------------------------------------------------------------------------

MOD_GRAPHPIMPL_Define_Vertex(Rule, Rule, r->getRule().getGraph(), r, /* VertexPrint */)
MOD_GRAPHPIMPL_Define_Vertex_Undirected(Rule, r->getRule().getGraph(), r)

Rule::LeftGraph::Vertex Rule::Vertex::getLeft() const {
	if(isNull()) return LeftGraph::Vertex();
	const auto &rDPO = r->getRule().getDPORule().getRule();
	const auto &gCombined = rDPO.getCombinedGraph();
	const auto vCG = vertex(vId, gCombined);
	if(gCombined[vCG].membership != lib::rule::Membership::R) {
		const auto vL = get_inverse(rDPO.getLtoCG(), getL(rDPO), gCombined, vCG);
		return r->getRule().getLeftInterfaceVertex(vL);
	} else return LeftGraph::Vertex();
}

Rule::ContextGraph::Vertex Rule::Vertex::getContext() const {
	if(isNull()) return ContextGraph::Vertex();
	const auto &rDPO = r->getRule().getDPORule().getRule();
	const auto &gCombined = rDPO.getCombinedGraph();
	const auto vCG = vertex(vId, gCombined);
	if(gCombined[vCG].membership == lib::rule::Membership::K) {
		const auto vK = get_inverse(rDPO.getKtoCG(), getK(rDPO), gCombined, vCG);
		return r->getRule().getContextInterfaceVertex(vK);
	} else return ContextGraph::Vertex();
}

Rule::RightGraph::Vertex Rule::Vertex::getRight() const {
	if(isNull()) return RightGraph::Vertex();
	const auto &rDPO = r->getRule().getDPORule().getRule();
	const auto &gCombined = rDPO.getCombinedGraph();
	const auto vCG = vertex(vId, gCombined);
	if(gCombined[vCG].membership != lib::rule::Membership::L) {
		const auto vR = get_inverse(rDPO.getRtoCG(), getR(rDPO), gCombined, vCG);
		return r->getRule().getRightInterfaceVertex(vR);
	} else return RightGraph::Vertex();
}

double Rule::Vertex::get2DX(bool withHydrogens) {
	if(isNull()) throw LogicError("Can not get coordinates on a null vertex.");
	const auto &rDPO = r->getRule().getDPORule().getRule();
	const auto &gCombined = rDPO.getCombinedGraph();
	const auto vCG = vertex(vId, gCombined);
	const auto &depict = r->getRule().getDepictionData();
	const double x = depict.getX(vCG, withHydrogens);
	if(std::isnan(x)) throw LogicError("Can not get coordinates for this vertex.");
	return x;
}

double Rule::Vertex::get2DY(bool withHydrogens) {
	if(isNull()) throw LogicError("Can not get coordinates on a null vertex.");
	const auto &rDPO = r->getRule().getDPORule().getRule();
	const auto &gCombined = rDPO.getCombinedGraph();
	const auto vCG = vertex(vId, gCombined);
	const auto &depict = r->getRule().getDepictionData();
	const double y = depict.getY(vCG, withHydrogens);
	if(std::isnan(y)) throw LogicError("Can not get coordinates for this vertex.");
	return y;
}

//------------------------------------------------------------------------------
// Edge
//------------------------------------------------------------------------------

MOD_GRAPHPIMPL_Define_Indices(Rule, Rule, r->getRule().getGraph(), r, /* EdgePrint */)

BOOST_CONCEPT_ASSERT((boost::ForwardIterator<Rule::VertexIterator>));
BOOST_CONCEPT_ASSERT((boost::ForwardIterator<Rule::EdgeIterator>));
BOOST_CONCEPT_ASSERT((boost::ForwardIterator<Rule::IncidentEdgeIterator>));

Rule::LeftGraph::Edge Rule::Edge::getLeft() const {
	if(isNull()) return LeftGraph::Edge();
	const auto &rDPO = r->getRule().getDPORule().getRule();
	const auto &gCombined = rDPO.getCombinedGraph();
	const auto vCG = vertex(vId, gCombined);
	const auto eCG = *std::next(out_edges(vCG, gCombined).first, eId);
	if(gCombined[eCG].membership != lib::rule::Membership::R) {
		const auto vL = get_inverse(rDPO.getLtoCG(), getL(rDPO), gCombined, vCG);
		const auto eL = get_inverse(rDPO.getLtoCG(), getL(rDPO), gCombined, eCG);
		return r->getRule().getLeftInterfaceEdge(vL, eL);
	} else return LeftGraph::Edge();
}

Rule::ContextGraph::Edge Rule::Edge::getContext() const {
	if(isNull()) return ContextGraph::Edge();
	const auto &rDPO = r->getRule().getDPORule().getRule();
	const auto &gCombined = rDPO.getCombinedGraph();
	const auto vCG = vertex(vId, gCombined);
	const auto eCG = *std::next(out_edges(vCG, gCombined).first, eId);
	if(gCombined[eCG].membership == lib::rule::Membership::K) {
		const auto vK = get_inverse(rDPO.getKtoCG(), getK(rDPO), gCombined, vCG);
		const auto eK = get_inverse(rDPO.getKtoCG(), getK(rDPO), gCombined, eCG);
		return r->getRule().getContextInterfaceEdge(vK, eK);
	} else return ContextGraph::Edge();
}

Rule::RightGraph::Edge Rule::Edge::getRight() const {
	if(isNull()) return RightGraph::Edge();
	const auto &rDPO = r->getRule().getDPORule().getRule();
	const auto &gCombined = rDPO.getCombinedGraph();
	const auto vCG = vertex(vId, gCombined);
	const auto eCG = *std::next(out_edges(vCG, gCombined).first, eId);
	if(gCombined[eCG].membership != lib::rule::Membership::L) {
		const auto vR = get_inverse(rDPO.getRtoCG(), getR(rDPO), gCombined, vCG);
		const auto eR = get_inverse(rDPO.getRtoCG(), getR(rDPO), gCombined, eCG);
		return r->getRule().getRightInterfaceEdge(vR, eR);
	} else return RightGraph::Edge();
}

} // namespace mod::rule