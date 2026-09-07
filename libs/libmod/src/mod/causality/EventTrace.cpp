#include "EventTrace.hpp"

#include <mod/Error.hpp>
#include <mod/causality/EventTracePrinter.hpp>
#include <mod/causality/Petri.hpp>
#include <mod/lib/Causality/EventTrace.hpp>
#include <mod/lib/Causality/IO/Write.hpp>
#include <mod/lib/IO/IO.hpp>

namespace mod::causality {

void EdgeAction::applyTo(Marking &m) const {
	m.fire(edge);
}

bool operator==(const EdgeAction &a, const EdgeAction &b) {
	return a.edge == b.edge;
}

bool operator!=(const EdgeAction &a, const EdgeAction &b) {
	return !(a == b);
}

std::ostream &operator<<(std::ostream &s, const EdgeAction &e) {
	return s << "EdgeAction{" << e.edge << "}";
}

// ----------------------------------------------------------------------------

void OutputAction::applyTo(Marking &m) const {
	m.remove(vertex, 1);
}

bool operator==(const OutputAction &a, const OutputAction &b) {
	return std::tie(a.vertex) == std::tie(b.vertex);
}

bool operator!=(const OutputAction &a, const OutputAction &b) {
	return !(a == b);
}

std::ostream &operator<<(std::ostream &s, const OutputAction &e) {
	return s << "OutputAction{" << e.vertex << "}";
}

// ----------------------------------------------------------------------------

void InputAction::applyTo(Marking &m) const {
	m.add(vertex, 1);
}

bool operator==(const InputAction &a, const InputAction &b) {
	return std::tie(a.vertex) == std::tie(b.vertex);
}

bool operator!=(const InputAction &a, const InputAction &b) {
	return !(a == b);
}

std::ostream &operator<<(std::ostream &s, const InputAction &e) {
	return s << "InputAction{" << e.vertex << "}";
}

// ############################################################################
// ############################################################################

bool operator==(const EventTrace::Event &a, const EventTrace::Event &b) {
	return std::tie(a.time, a.action) == std::tie(b.time, b.action);
}

bool operator!=(const EventTrace::Event &a, const EventTrace::Event &b) {
	return !(a == b);
}

std::ostream &operator<<(std::ostream &s, const EventTrace::Event &e) {
	s << "Event{" << e.time << ", ";
	std::visit([&s](const auto &a) { s << a; }, e.action);
	return s << "}";
}

// ############################################################################

struct EventTrace::Pimpl {
	std::shared_ptr<Net> net; // to keep it alive
	lib::Causality::EventTrace t;
};

// ############################################################################

EventTrace::iterator::iterator() = default;
EventTrace::iterator::iterator(const EventTrace *t, int offset) : t(t), offset(offset) {}

EventTrace::Event EventTrace::iterator::dereference() const {
	const auto &es = t->p->t.getEvents();
	assert(offset >= 0 && offset < es.size());
	struct Visitor {
		Visitor(const lib::DG::Hyper &dg_) : dg_(dg_) {}

		Action operator()(const lib::Causality::EdgeAction &a) const {
			return EdgeAction{dg_.getInterfaceEdge(a.e)};
		}

		Action operator()(const lib::Causality::InputAction &a) const {
			return InputAction{dg_.getInterfaceVertex(a.v)};
		}

		Action operator()(const lib::Causality::OutputAction &a) const {
			return OutputAction{dg_.getInterfaceVertex(a.v)};
		}
	public:
		const lib::DG::Hyper &dg_;
	};
	return Event{es[offset].time, std::visit(Visitor(t->p->net->getDG()->getHyper()), es[offset].action)};
}

bool EventTrace::iterator::equal(iterator other) const {
	return std::tie(t, offset) == std::tie(other.t, other.offset);
}

void EventTrace::iterator::increment() {
	++offset;
}

void EventTrace::iterator::advance(int n) {
	offset += n;
}

// ############################################################################

EventTrace::EventTrace(std::shared_ptr<Net> net, lib::Causality::EventTrace trace) {
	p.reset(new Pimpl{net, std::move(trace)});
}

EventTrace::EventTrace(const Marking &initialState)
		: EventTrace(initialState.getNet(),
		             lib::Causality::EventTrace(
				             initialState.getNet()->getDG()->getHyper(),
				             initialState.getMarking()
		             )) {}

EventTrace::~EventTrace() = default;
EventTrace::EventTrace(EventTrace &&) = default;
EventTrace &EventTrace::operator=(EventTrace &&) = default;

EventTrace::EventTrace(const EventTrace &other) {
	p.reset(new Pimpl(*other.p));
}

EventTrace &EventTrace::operator=(const EventTrace &other) {
	if(&other != this)
		p.reset(new Pimpl(*other.p));
	return *this;
}

lib::Causality::EventTrace &EventTrace::getEventTrace() {
	return p->t;
}

const lib::Causality::EventTrace &EventTrace::getEventTrace() const {
	return p->t;
}

std::shared_ptr<dg::DG> EventTrace::getDG() const {
	return p->net->getDG();
}

Marking EventTrace::getInitialState() const {
	p->t.syncSize();
	return Marking(p->net, p->t.getInitialState());
}

int EventTrace::size() const {
	return p->t.getEvents().size();
}

double EventTrace::getTime() const {
	if(p->t.getEvents().size() == 0) return 0;
	else return (p->t.getEvents().end() - 1)->time;
}

EventTrace::const_iterator EventTrace::begin() const {
	return const_iterator(this, 0);
}

EventTrace::const_iterator EventTrace::end() const {
	return const_iterator(this, this->size());
}

EventTrace::Event EventTrace::operator[](int i) const {
	if(i < 0 || i >= size()) throw LogicError("Offset is out of bounds.");
	return begin()[i];
}

std::ostream &operator<<(std::ostream &s, const EventTrace &t) {
	s << "EventTrace{" << t.getInitialState() << ", ";
	s << "[";
	bool first = true;
	for(const auto &e: t) {
		if(!first)
			s << ", ";
		first = false;
		s << e;
	}
	return s << "]}";
}

bool operator==(const EventTrace &a, const EventTrace &b) {
	if(a.p->net != b.p->net) return false;
	a.p->net->syncSize();
	b.p->net->syncSize();
	a.p->t.syncSize();
	b.p->t.syncSize();
	return a.p->t == b.p->t;
}

void EventTrace::add(double time, const Action &action) {
	struct Visitor {
		lib::Causality::Action operator()(const EdgeAction &a) const {
			if(!a.edge) throw LogicError("Can not add edge event with null edge.");
			if(a.edge.getDG() != dg)
				throw LogicError("The hyperedge in the edge action does not belong to the underlying derivation graph.");
			return lib::Causality::EdgeAction{dg->getHyper().getInternalVertex(a.edge)};
		}

		lib::Causality::Action operator()(const InputAction &a) const {
			if(!a.vertex) throw LogicError("Can not add input event with null vertex.");
			if(a.vertex.getDG() != dg)
				throw LogicError("The vertex in the event action does not belong to the underlying derivation graph.");
			return lib::Causality::InputAction{dg->getHyper().getInternalVertex(a.vertex)};
		}

		lib::Causality::Action operator()(const OutputAction &a) const {
			if(!a.vertex) throw LogicError("Can not add output event with null vertex.");
			if(a.vertex.getDG() != dg)
				throw LogicError("The vertex in the output action does not belong to the underlying derivation graph.");
			return lib::Causality::OutputAction{dg->getHyper().getInternalVertex(a.vertex)};
		}
	public:
		std::shared_ptr<dg::DG> dg;
	};
	if(time < getTime())
		throw LogicError("Can not add event with time " + std::to_string(time) + " which is less than the current time "
		                 + std::to_string(getTime()) + ".");
	p->t.add({time, std::visit(Visitor{p->net->getDG()}, action)});
}

std::string EventTrace::print() const {
	return print(EventTracePrinter());
}

std::string EventTrace::print(const EventTracePrinter &printer) const {
	p->t.syncSize();
	return lib::Causality::Write::summaryEventTrace(p->t, printer.getPrinter());
}

std::string EventTrace::dump() const {
	return dump("");
}

std::string EventTrace::dump(const std::string &filename) const {
	if(filename.empty()) {
		std::string name = lib::IO::makeUniqueFilePrefix() + "EventTrace.eventTrace";
		lib::IO::writeJsonFile(name, p->t.dump());
		return name;
	} else {
		lib::IO::writeJsonFile(filename, p->t.dump());
		return filename;
	}
}

EventTrace EventTrace::load(std::shared_ptr<dg::DG> dg_, const std::string &file) {
	if(!dg_) throw LogicError("The derivation graph is a null pointer.");
	return load(Net::make(dg_), file);
}

EventTrace EventTrace::load(std::shared_ptr<Net> net, const std::string &file) {
	if(!net) throw LogicError("The Petri net is a null pointer.");

	std::ostringstream err;
	auto inner = lib::Causality::EventTrace::load(net->getNet(), file, err);
	if(!inner) throw InputError("EventTrace load error: " + err.str());
	return EventTrace(net, std::move(*inner));
}

} // namespace mod::causality