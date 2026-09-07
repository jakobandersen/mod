#include "Petri.hpp"

#include <mod/Error.hpp>
#include <mod/lib/Causality/Petri.hpp>

#include <algorithm>

namespace mod::causality {

struct Net::Pimpl {
	std::shared_ptr<dg::DG> dg_;
	lib::Causality::Net net;
};

Net::Net(std::shared_ptr<dg::DG> dg_)
		: p(new Pimpl{dg_, lib::Causality::Net(dg_->getHyper())}) {}

Net::~Net() = default;

std::shared_ptr<dg::DG> Net::getDG() const {
	return p->dg_;
}

const lib::Causality::Net &Net::getNet() const {
	return p->net;
}

std::ostream &operator<<(std::ostream &s, const Net &net) {
	return s << "Net{" << *net.getDG() << "}";
}

void Net::syncSize() {
	p->net.syncSize();
}

std::vector<dg::DG::Vertex> Net::getPostPlaces(dg::DG::HyperEdge e) const {
	const auto &dgHyper = p->dg_->getHyper();
	const auto inner = p->net.getPostPlaces(dgHyper.getInternalVertex(e));
	std::vector<dg::DG::Vertex> res;
	res.reserve(inner.size());
	for(const auto vInner: inner)
		res.push_back(dgHyper.getInterfaceVertex(vInner));
	return res;
}

std::shared_ptr<Net> Net::make(std::shared_ptr<dg::DG> dg_) {
	if(!dg_) throw LogicError("The derivation graph is a null pointer.");
	if(!(dg_->hasActiveBuilder() || dg_->isLocked()))
		throw LogicError("The DG neither has an active builder nor is locked yet.");
	return std::shared_ptr<Net>(new Net(dg_));
}

//==============================================================================

struct Marking::Pimpl {
	std::shared_ptr<Net> net;
	lib::Causality::Marking marking;
};

Marking::Marking(std::shared_ptr<Net> net) {
	if(!net) throw LogicError("The Petri net is a null pointer.");
	p.reset(new Pimpl{net, lib::Causality::Marking(net->getNet())});
}

Marking::Marking(std::shared_ptr<Net> net,
                 const lib::Causality::Marking &marking) {
	assert(&net->getNet() == &marking.getNet());
	p.reset(new Pimpl{net, marking});
}

Marking::~Marking() = default;
Marking::Marking(Marking &&) = default;
Marking &Marking::operator=(Marking &&) = default;

Marking::Marking(const Marking &other) {
	p.reset(new Pimpl(*other.p));
}

Marking &Marking::operator=(const Marking &other) {
	if(&other != this)
		p.reset(new Pimpl(*other.p));
	return *this;
}

std::shared_ptr<Net> Marking::getNet() const {
	return p->net;
}

lib::Causality::Marking &Marking::getMarking() {
	return p->marking;
}

const lib::Causality::Marking &Marking::getMarking() const {
	return p->marking;
}

void Marking::syncSize() const {
	p->net->syncSize();
	p->marking.syncSize();
}

std::ostream &operator<<(std::ostream &s, const Marking &m) {
	m.syncSize();
	s << "Marking{";
	bool first = true;
	for(const auto v: m.p->net->getDG()->vertices()) {
		const int c = m[v];
		if(c == 0) continue;
		if(!first) s << ", ";
		first = false;
		s << v.getGraph()->getName() << ": " << c;
	}
	s << "}";
	return s;
}

bool operator==(const Marking &a, const Marking &b) {
	if(a.getNet()->getDG() != b.getNet()->getDG()) return false;
	a.syncSize();
	b.syncSize();
	return a.p->marking.getMarking() == b.p->marking.getMarking();
}

bool operator!=(const Marking &a, const Marking &b) {
	return !(a == b);
}

int Marking::add(dg::DG::Vertex v, int c) {
	if(!v) throw LogicError("Can not add tokens to a null vertex.");
	if(v.getDG() != getNet()->getDG())
		throw LogicError("The vertex does not belong to the underlying derivation graph.");
	if(c < 0) throw LogicError("Can not add a negative token amount, use remove() instead.");
	return p->marking.add(p->net->getDG()->getHyper().getInternalVertex(v), c);
}

int Marking::add(std::shared_ptr<graph::Graph> g, int c) {
	if(!g) throw LogicError("The graph is a null pointer.");
	const auto v = getNet()->getDG()->findVertex(g);
	if(!v) throw LogicError("Can not find vertex with the graph in the underlying derivation graph.");
	return add(v, c);
}

int Marking::remove(dg::DG::Vertex v, int c) {
	if(!v) throw LogicError("Can not remove tokens from a null vertex.");
	if(v.getDG() != getNet()->getDG())
		throw LogicError("The vertex does not belong to the underlying derivation graph.");
	if(c < 0) throw LogicError("Can not remove a negative token amount, use add() instead.");
	if((*this)[v] < c) throw LogicError("Too few tokens left on the place.");
	return p->marking.remove(p->net->getDG()->getHyper().getInternalVertex(v), c);
}

int Marking::remove(std::shared_ptr<graph::Graph> g, int c) {
	if(!g) throw LogicError("The graph is a null pointer.");
	const auto v = getNet()->getDG()->findVertex(g);
	if(!v) throw LogicError("Can not find vertex with the graph in the underlying derivation graph.");
	return remove(v, c);
}

int Marking::operator[](dg::DG::Vertex v) const {
	if(!v) throw LogicError("Can not get token count for a null vertex.");
	if(v.getDG() != getNet()->getDG())
		throw LogicError("The vertex does not belong to the underlying derivation graph.");
	return p->marking[p->net->getDG()->getHyper().getInternalVertex(v)];
}

int Marking::operator[](std::shared_ptr<graph::Graph> g) const {
	if(!g) throw LogicError("The graph is a null pointer.");
	const auto v = getNet()->getDG()->findVertex(g);
	if(!v) throw LogicError("Can not find vertex with the graph in the underlying derivation graph.");
	return (*this)[v];
}

int Marking::getNumTokens() const {
	return p->marking.getNumTokens();
}

std::vector<dg::DG::HyperEdge> Marking::getAllEnabled() const {
	const auto inner = p->marking.getAllEnabled();
	const auto &dgHyper = p->net->getDG()->getHyper();
	std::vector<dg::DG::HyperEdge> res;
	res.reserve(inner.size());
	for(const auto e: inner)
		res.push_back(dgHyper.getInterfaceEdge(e));
	return res;
}

std::vector<dg::DG::Vertex> Marking::getNonZeroPlaces() const {
	const auto inner = p->marking.getNonZeroPlaces();
	const auto &dgHyper = p->net->getDG()->getHyper();
	std::vector<dg::DG::Vertex> res;
	res.reserve(inner.size());
	for(const auto v: inner)
		res.push_back(dgHyper.getInterfaceVertex(v));
	return res;
}

std::vector<dg::DG::Vertex> Marking::getEmptyPostPlaces(dg::DG::HyperEdge e) const {
	if(!e) throw LogicError("Can not get empty post places for null edge.");
	if(e.getDG() != getNet()->getDG()) throw LogicError("The edge does not belong to the underlying derivation graph.");
	const auto &dgHyper = p->net->getDG()->getHyper();
	const auto inner = p->marking.getEmptyPostPlaces(dgHyper.getInternalVertex(e));
	std::vector<dg::DG::Vertex> res;
	res.reserve(inner.size());
	for(const auto v: inner)
		res.push_back(dgHyper.getInterfaceVertex(v));
	return res;
}

bool Marking::isEnabled(dg::DG::HyperEdge e) const {
	if(!e) throw LogicError("Can not check if a null edge is enabled.");
	if(e.getDG() != getNet()->getDG()) throw LogicError("The edge does not belong to the underlying derivation graph.");
	return p->marking.isEnabled(p->net->getDG()->getHyper().getInternalVertex(e));
}

void Marking::fire(dg::DG::HyperEdge e) {
	if(!e) throw LogicError("Can not fire a null edge.");
	if(e.getDG() != getNet()->getDG()) throw LogicError("The edge does not belong to the underlying derivation graph.");
	if(!isEnabled(e)) throw LogicError("The edge is not enabled.");
	p->marking.fire(p->net->getDG()->getHyper().getInternalVertex(e));
}

} // namespace mod::causality