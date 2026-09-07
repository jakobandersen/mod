#include "Petri.hpp"

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

// define to enable asserts in functions that may be used in inner loops
//#define MOD_LIB_CAUSALITY_PETRI_ASSERT

namespace mod::lib::Causality {

Net::Net(const lib::DG::Hyper &dg) : dg(dg) {
	syncSize();
}

void Net::syncSize() {
	assert(dg.getNonHyper().getHasStartedCalculation());
	const auto &g = dg.getGraph();
	placeMap.resize(num_vertices(g));
	transitionMap.resize(num_vertices(g));
	auto vs = vertices(g);
	for(auto iter = vs.first + net.numPlaces() + net.numTransitions(); iter != vs.second; ++iter) {
		const auto v = *iter;
		const auto vId = get(boost::vertex_index_t(), g, v);
		if(g[v].kind == lib::DG::HyperVertexKind::Vertex) {
			const petri::Place p = placeMap[vId] = net.addPlace();
			(void) p;
			assert(p.getId() == places.size());
			places.push_back(v);
		} else {
			assert(g[v].kind == lib::DG::HyperVertexKind::Edge);
			const petri::Transition t = transitionMap[vId] = net.addTransition();
			assert(t.getId() == transitions.size());
			transitions.push_back(v);
			for(auto vIn : asRange(inv_adjacent_vertices(v, g))) {
				const auto vInId = get(boost::vertex_index_t(), g, vIn);
				const petri::Place pIn = placeMap[vInId];
				assert(pIn);
				net.addArc(pIn, t, 1);
			}
			for(auto vOut : asRange(adjacent_vertices(v, g))) {
				const auto vOutId = get(boost::vertex_index_t(), g, vOut);
				const petri::Place pOut = placeMap[vOutId];
				assert(pOut);
				net.addArc(t, pOut, 1);
			}
		}
	}
}

std::vector<lib::DG::HyperVertex> Net::getPostPlaces(lib::DG::HyperVertex e) const {
	std::vector<lib::DG::HyperVertex> res;
	petri::Transition t = getTransition(e);
	const auto vt = net.vertexFromTransition(t);
	const auto &g = net.getGraph();
	for(auto eOut : asRange(out_edges(vt, g))) {
		const auto pvOut = target(eOut, g);
		assert(g[pvOut].kind == petri::Net::Kind::Place);
		petri::Place pOut = net.placeFromVertex(pvOut);
		assert(pOut);
		res.push_back(places[pOut.getId()]);
	}
	return res;
}

//==============================================================================

Marking::Marking(const Net &net) : net(net), m(net.getNet()) {}

void Marking::syncSize() {
	m.syncSize();
}

int Marking::add(lib::DG::HyperVertex v, int c) {
	petri::Place p = net.getPlace(v);
	return m.add(p, c);
}

int Marking::remove(lib::DG::HyperVertex v, int c) {
	petri::Place p = net.getPlace(v);
	return m.remove(p, c);
}

int Marking::getNumTokens() const {
	return m.getNumTokens();
}

int Marking::operator[](lib::DG::HyperVertex v) const {
	petri::Place p = net.getPlace(v);
	return m[p];
}

std::vector<lib::DG::HyperVertex> Marking::getAllEnabled() const {
	std::vector<lib::DG::HyperVertex> res;
	for(const auto e : net.transitions) {
		petri::Transition t = net.getTransition(e);
		if(m.isEnabled(t)) res.push_back(e);
	}
	return res;
}

std::vector<lib::DG::HyperVertex> Marking::getNonZeroPlaces() const {
	std::vector<lib::DG::HyperVertex> res;
	for(const auto v : net.places) {
		if((*this)[v] != 0)
			res.push_back(v);
	}
	return res;
}

std::vector<lib::DG::HyperVertex> Marking::getEmptyPostPlaces(lib::DG::HyperVertex e) const {
	std::vector<lib::DG::HyperVertex> postPlaces = net.getPostPlaces(e);
	std::vector<lib::DG::HyperVertex> emptyPlaces;
	for(const auto v : postPlaces) {
		petri::Place p = net.getPlace(v);
		if(m[p] == 0)
			emptyPlaces.push_back(v);
	}
	return emptyPlaces;
}

void Marking::fire(lib::DG::HyperVertex e) {
	const auto t = net.getTransition(e);
	m.fire(t);
}

} // namespace mod::lib::Causality