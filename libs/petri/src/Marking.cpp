#include <petri/Marking.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <numeric>

namespace petri {

Marking::Marking(const Net &net) : net(net) {
	counts.resize(net.numPlaces(), 0);
	transitions.resize(net.numTransitions());
}

void Marking::syncSize() {
	counts.resize(net.numPlaces(), 0);
	const auto oldSize = transitions.size();
	transitions.resize(net.numTransitions());
	for(auto tId = oldSize; tId != transitions.size(); ++tId) {
		const auto vt = net.vertexFromTransition(Transition(tId));
		if(isEnabledNoCache(vt)) enable(vt);
	}
}

int Marking::add(Place p, int m) {
	assert(m >= 0);
	const auto id = p.getId();
	counts[id] += m;
	// TODO: this should be strengthened: only update if get above highest token amount, or are already below it
	updateToEnabled(net.vertexFromPlace(p));
	return counts[id];
}

int Marking::remove(Place p, int m) {
	assert(m >= 0);
	const auto id = p.getId();
	counts[id] -= m;
	// TODO: this should be strengthened: only update if get/are below highest token amount
	updateToDisabled(net.vertexFromPlace(p));
	return counts[id];
}

int Marking::getNumTokens() const {
	return std::accumulate(counts.begin(), counts.end(), 0);
}

bool Marking::isEnabled(Transition t) const {
	return transitions[t.getId()].enabled;
}

bool Marking::isEnabledNoCache(Transition t) const {
	return isEnabledNoCache(net.vertexFromTransition(t));
}

bool Marking::isEnabledNoCache(Net::Vertex vt) const {
	const auto &g = net.getGraph();
	for(const auto eIn : jla_boost::asRange(in_edges(vt, g))) {
		const int weight = g[eIn];
		const auto pvIn = source(eIn, g);
		assert(g[pvIn].kind == Net::Kind::Place);
		const int pId = g[pvIn].id;
		if(counts[pId] < weight) return false;
	}
	return true;
}

void Marking::fire(Transition t) {
	assert(isEnabledNoCache(t));
	const auto &g = net.getGraph();
	const auto vt = net.vertexFromTransition(t);
	for(const auto eIn : jla_boost::asRange(in_edges(vt, g))) {
		const int weight = g[eIn];
		const auto pvIn = source(eIn, g);
		assert(g[pvIn].kind == Net::Kind::Place);
		const int pId = g[pvIn].id;
		counts[pId] -= weight;
	}
	for(const auto eOut : jla_boost::asRange(out_edges(vt, g))) {
		const int weight = g[eOut];
		const auto pvOut = target(eOut, g);
		assert(g[pvOut].kind == Net::Kind::Place);
		const int pId = g[pvOut].id;
		counts[pId] += weight;
	}
	// updated enabledness
	for(const auto vpIn : jla_boost::asRange(inv_adjacent_vertices(vt, g))) {
		assert(g[vpIn].kind == Net::Kind::Place);
		const int id = g[vpIn].id;
		const int c = counts[id];
		if(c == 0) {
			disableAll(vpIn);
		} else if(c >= net.getMaxInWeight()) {
			// great, nothing can be disabled
		} else {
			updateToDisabled(vpIn);
		}
	}
	for(const auto eOut : jla_boost::asRange(out_edges(vt, g))) {
		const int weight = g[eOut];
		const auto vpOut = target(eOut, g);
		const int id = g[vpOut].id;
		const int c = counts[id];
		if(c - weight >= net.getMaxInWeight()) {
			// already had enough tokens for everything, so skip
		} else {
			updateToEnabled(vpOut);
		}
	}
}

//void backfire(Transition t);

int Marking::operator[](Place p) const {
	return counts[p.getId()];
}

bool operator==(const Marking &a, const Marking &b) {
	return a.counts == b.counts;
}

void Marking::enable(Net::Vertex vt) {
	const auto &g = net.getGraph();
	assert(g[vt].kind == Net::Kind::Transition);
	const int id = g[vt].id;
	assert(!transitions[id].enabled);
	transitions[id].enabled = true;
}

void Marking::disable(Net::Vertex vt) {
	const auto &g = net.getGraph();
	assert(g[vt].kind == Net::Kind::Transition);
	const int id = g[vt].id;
	assert(transitions[id].enabled);
	transitions[id].enabled = false;
}

void Marking::updateToEnabled(Net::Vertex vp) {
	const auto &g = net.getGraph();
	assert(g[vp].kind == Net::Kind::Place);
	for(auto vt : jla_boost::asRange(adjacent_vertices(vp, g))) {
		assert(g[vt].kind == Net::Kind::Transition);
		const int id = g[vt].id;
		if(transitions[id].enabled) continue;
		if(isEnabledNoCache(vt)) enable(vt);
	}
}

void Marking::updateToDisabled(Net::Vertex vp) {
	const auto &g = net.getGraph();
	assert(g[vp].kind == Net::Kind::Place);
	for(auto vt : jla_boost::asRange(adjacent_vertices(vp, g))) {
		assert(g[vt].kind == Net::Kind::Transition);
		const int id = g[vt].id;
		if(!transitions[id].enabled) continue;
		if(!isEnabledNoCache(vt)) disable(vt);
	}
}

void Marking::disableAll(Net::Vertex vp) {
	const auto &g = net.getGraph();
	assert(g[vp].kind == Net::Kind::Place);
	for(auto vt : jla_boost::asRange(adjacent_vertices(vp, g))) {
		assert(g[vt].kind == Net::Kind::Transition);
		const int id = g[vt].id;
		if(!transitions[id].enabled) continue;
		disable(vt);
	}
}

} // namespace petri
