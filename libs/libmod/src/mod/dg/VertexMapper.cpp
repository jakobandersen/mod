#include "VertexMapper.hpp"

#include <mod/Error.hpp>
#include <mod/graph/Union.hpp>
#include <mod/VertexMap.hpp>
#include <mod/lib/DG/Hyper.hpp>
#include <mod/lib/DG/VertexMapping.hpp>
#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/Rule/Rule.hpp>

namespace mod::dg {

struct VertexMapper::Pimpl {
	graph::Union getLeft() const {
		std::vector<std::shared_ptr<graph::Graph>> gs;
		gs.reserve(res.leftDGVertices.size());
		const auto &dg = e.getDG()->getHyper().getGraph();
		for(const auto v: res.leftDGVertices)
			gs.push_back(dg[v].graph->getAPIReference());
		return graph::Union(std::move(gs));
	}

	graph::Union getRight() const {
		std::vector<std::shared_ptr<graph::Graph>> gs;
		gs.reserve(res.rightDGVertices.size());
		const auto &dg = e.getDG()->getHyper().getGraph();
		for(const auto v: res.rightDGVertices)
			gs.push_back(dg[v].graph->getAPIReference());
		return graph::Union(std::move(gs));
	}
public:
	DG::HyperEdge e;
	lib::DG::VertexMappingResult res;
};

VertexMapper::Result VertexMapper::iterator::dereference() const {
	assert(owner);
	assert(i < owner->size());
	auto r = owner->p->res.maps[i].r->getAPIReference();
	// capture owner->p in the callbacks to keep the mapping data alive
	auto m = VertexMap<graph::Union, graph::Union>(
			owner->getLeft(), owner->getRight(),
			[p = owner->p, i = this->i](graph::Union::Vertex vLeft) -> graph::Union::Vertex {
				assert(vLeft.getId() < num_vertices(p->res.gLeft));
				const auto v = vertices(p->res.gLeft).first[vLeft.getId()];
				const auto vRes = get(p->res.maps[i].mGH, p->res.gLeft, p->res.gRight, v);
				if(vRes == p->res.gRight.null_vertex())
					return {};
				return p->getRight().vertices()[get(boost::vertex_index_t(), p->res.gRight, vRes)];
			},
			[p = owner->p, i = this->i](graph::Union::Vertex vRight) -> graph::Union::Vertex {
				assert(vRight.getId() < num_vertices(p->res.gRight));
				const auto v = vertices(p->res.gRight).first[vRight.getId()];
				const auto vRes = get_inverse(p->res.maps[i].mGH, p->res.gLeft, p->res.gRight, v);
				if(vRes == p->res.gLeft.null_vertex())
					return {};
				return p->getLeft().vertices()[get(boost::vertex_index_t(), p->res.gLeft, vRes)];
			}
			);
	auto match = VertexMap<rule::Rule::LeftGraph, graph::Union>(
			r->getLeft(), owner->getLeft(),
			[p = owner->p, i = this->i](rule::Rule::LeftGraph::Vertex vLOuter) -> graph::Union::Vertex {
				const auto &data = p->res.maps[i];
				const auto &lr = data.r->getDPORule();
				const auto &rDPO = lr.getRule();
				const auto vLInner = data.r->getLeftInternalVertex(vLOuter);
				const auto vLeftInner = get(data.mLG, getL(rDPO), p->res.gLeft, vLInner);
				assert(vLeftInner != p->res.gLeft.null_vertex());
				return p->getLeft().vertices()[get(boost::vertex_index_t(), p->res.gLeft, vLeftInner)];
			},
			[p = owner->p, i = this->i](graph::Union::Vertex vLeftOuter) -> rule::Rule::LeftGraph::Vertex {
				assert(vLeftOuter.getId() < num_vertices(p->res.gLeft));
				const auto &data = p->res.maps[i];
				const auto &lr = data.r->getDPORule();
				const auto &rDPO = lr.getRule();
				const auto vLeftInner = vertices(p->res.gLeft).first[vLeftOuter.getId()];
				const auto vLInner = get_inverse(data.mLG, getL(rDPO), p->res.gLeft, vLeftInner);
				if(vLInner == getL(rDPO).null_vertex())
					return {};
				return data.r->getLeftInterfaceVertex(vLInner);
			}
			);
	auto comatch = VertexMap<rule::Rule::RightGraph, graph::Union>(
			r->getRight(), owner->getRight(),
			[p = owner->p, i = this->i](rule::Rule::RightGraph::Vertex vROuter) -> graph::Union::Vertex {
				const auto &data = p->res.maps[i];
				const auto &lr = data.r->getDPORule();
				const auto &rDPO = lr.getRule();
				const auto vRInner = data.r->getRightInternalVertex(vROuter);
				const auto vRightInner = get(data.mRH, getR(rDPO), p->res.gRight, vRInner);
				assert(vRightInner != p->res.gRight.null_vertex());
				return p->getRight().vertices()[get(boost::vertex_index_t(), p->res.gRight, vRightInner)];
			},
			[p = owner->p, i = this->i](graph::Union::Vertex vRightOuter) -> rule::Rule::RightGraph::Vertex {
				assert(vRightOuter.getId() < num_vertices(p->res.gRight));
				const auto &data = p->res.maps[i];
				const auto &lr = data.r->getDPORule();
				const auto &rDPO = lr.getRule();
				const auto vRightInner = vertices(p->res.gRight).first[vRightOuter.getId()];
				const auto vRInner = get_inverse(data.mRH, getR(rDPO), p->res.gRight, vRightInner);
				if(vRInner == getR(rDPO).null_vertex())
					return {};
				return data.r->getRightInterfaceVertex(vRInner);
			}
			);
	return {r, std::move(m), std::move(match), std::move(comatch)};
}

// =================================================================================

VertexMapper::VertexMapper(DG::HyperEdge e) : VertexMapper(e, true, 1 << 30, 0) {}

VertexMapper::VertexMapper(DG::HyperEdge e, bool upToIsomorphismGDH, int rightLimit, int verbosity)
	: p(new Pimpl()) {
	if(!e) throw LogicError("Can not find vertex maps for null edge.");
	const auto &dg = e.getDG()->getHyper();
	p->e = e;
	lib::IO::Logger logger(std::cout);
	p->res = lib::DG::calculateVertexMaps(dg, dg.getInternalVertex(e), upToIsomorphismGDH, rightLimit,
	                                      logger, verbosity);
}

VertexMapper::~VertexMapper() = default;

DG::HyperEdge VertexMapper::getEdge() const {
	return p->e;
}

graph::Union VertexMapper::getLeft() const {
	return p->getLeft();
}

graph::Union VertexMapper::getRight() const {
	return p->getRight();
}

VertexMapper::const_iterator VertexMapper::begin() const {
	return {this, 0};
}

VertexMapper::const_iterator VertexMapper::end() const {
	return {this, p->res.maps.size()};
}

std::size_t VertexMapper::size() const {
	return p->res.maps.size();
}

VertexMapper::Result VertexMapper::operator[](std::size_t i) const {
	if(i >= p->res.maps.size())
		throw LogicError("Index " + std::to_string(i) + " is out of range (" + std::to_string(size()) + ").");
	return begin()[i];
}

} // namespace mod::dg