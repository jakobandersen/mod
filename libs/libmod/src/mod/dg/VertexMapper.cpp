#include "VertexMapper.hpp"

#include <mod/Error.hpp>
#include <mod/graph/Union.hpp>
#include <mod/VertexMap.hpp>
#include <mod/lib/DG/Hyper.hpp>
#include <mod/lib/DG/VertexMapping.hpp>
#include <mod/lib/Graph/Single.hpp>
#include <mod/lib/Rules/Real.hpp>

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
	// capture owner->p in the callbacks to keep the mapping data alive
	auto m = VertexMap<graph::Union, graph::Union>(
			owner->getLeft(), owner->getRight(),
			[p = owner->p, i = this->i](graph::Union::Vertex vLeft) -> graph::Union::Vertex {
				assert(vLeft.getId() < num_vertices(p->res.gLeft));
				const auto v = vertices(p->res.gLeft).first[vLeft.getId()];
				const auto vRes = get(p->res.maps[i].second, p->res.gLeft, p->res.gRight, v);
				if(vRes == p->res.gRight.null_vertex())
					return {};
				return p->getRight().vertices()[get(boost::vertex_index_t(), p->res.gRight, vRes)];
			},
			[p = owner->p, i = this->i](graph::Union::Vertex vRight) -> graph::Union::Vertex {
				assert(vRight.getId() < num_vertices(p->res.gRight));
				const auto v = vertices(p->res.gRight).first[vRight.getId()];
				const auto vRes = get_inverse(p->res.maps[i].second, p->res.gLeft, p->res.gRight, v);
				if(vRes == p->res.gLeft.null_vertex())
					return {};
				return p->getLeft().vertices()[get(boost::vertex_index_t(), p->res.gLeft, vRes)];
			}
	);
	return {owner->p->res.maps[i].first->getAPIReference(), std::move(m)};
}

// =================================================================================

VertexMapper::VertexMapper(DG::HyperEdge e) : VertexMapper(e, true, 1, 1 << 30, 0) {}

VertexMapper::VertexMapper(DG::HyperEdge e, bool upToIsomorphismG, int leftLimit, int rightLimit, int verbosity)
		: p(new Pimpl()) {
	if(!e) throw LogicError("Can not find vertex maps for null edge.");
	const auto &dg = e.getDG()->getHyper();
	p->e = e;
	lib::IO::Logger logger(std::cout);
	p->res = lib::DG::calculateVertexMaps(dg, dg.getInternalVertex(e), upToIsomorphismG, leftLimit, rightLimit,
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