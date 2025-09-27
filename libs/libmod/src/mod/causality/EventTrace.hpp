#ifndef MOD_CAUSALITY_EVENTTRACE_HPP
#define MOD_CAUSALITY_EVENTTRACE_HPP

#include <mod/BuildConfig.hpp>
#include <mod/causality/ForwardDecl.hpp>
#include <mod/dg/DG.hpp>
#include <mod/dg/GraphInterface.hpp>

#include <boost/iterator/iterator_facade.hpp>

#include <variant>

namespace mod::causality {

// rst: .. type:: causality::Action = std::variant<causality::EdgeAction, causality::InputAction, causality::OutputAction>

// rst:
// rst: .. class:: causality::EdgeAction
// rst:
struct MOD_DECL EdgeAction {
	// rst: 	.. function:: EdgeAction()
	// rst: 	              EdgeAction(dg::DG::HyperEdge edge)
	EdgeAction() = default;
	EdgeAction(dg::DG::HyperEdge edge) : edge(edge) {}
	// rst: 	.. function:: void applyTo(Marking &m) const
	// rst:
	// rst:			Perform `m.fire(edge)`.
	void applyTo(Marking &m) const;
public:
	MOD_DECL friend bool operator==(const EdgeAction &a, const EdgeAction &b);
	MOD_DECL friend bool operator!=(const EdgeAction &a, const EdgeAction &b);
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const EdgeAction &e);
public:
	// rst: 	.. var:: dg::DG::HyperEdge edge
	// rst:
	// rst:			May not be a null edge.
	dg::DG::HyperEdge edge;
};

// rst:
// rst: .. class:: causality::InputAction
// rst:
struct MOD_DECL InputAction {
	// rst: 	.. function:: InputAction()
	// rst: 	              InputAction(dg::DG::Vertex vertex)
	InputAction() = default;
	InputAction(dg::DG::Vertex vertex) : vertex(vertex) {}
	// rst: 	.. function:: void applyTo(Marking &m) const
	// rst:
	// rst:			Perform `m.add(vertex, 1)`.
	void applyTo(Marking &m) const;
public:
	MOD_DECL friend bool operator==(const InputAction &a, const InputAction &b);
	MOD_DECL friend bool operator!=(const InputAction &a, const InputAction &b);
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const InputAction &e);
public:
	// rst: 	.. var:: dg::DG::Vertex vertex
	// rst:
	// rst:			May not be a null vertex.
	dg::DG::Vertex vertex;
};

// rst:
// rst: .. class:: causality::OutputAction
// rst:
struct MOD_DECL OutputAction {
	// rst: 	.. function:: OutputAction()
	// rst: 	              OutputAction(dg::DG::Vertex vertex)
	OutputAction() = default;
	OutputAction(dg::DG::Vertex vertex) : vertex(vertex) {}
	// rst: 	.. function:: void applyTo(Marking &m) const
	// rst:
	// rst:			Perform `m.remove(vertex, 1)`.
	void applyTo(Marking &m) const;
public:
	MOD_DECL friend bool operator==(const OutputAction &a, const OutputAction &b);
	MOD_DECL friend bool operator!=(const OutputAction &a, const OutputAction &b);
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const OutputAction &e);
public:
	// rst: 	.. var:: dg::DG::Vertex vertex
	// rst:
	// rst:			May not be a null vertex.
	dg::DG::Vertex vertex;
};

// the rst is on top
using Action = std::variant<EdgeAction, InputAction, OutputAction>;

// rst:
// rst-class: causality::EventTrace
// rst:
// rst:		A helper class to hold an initial state with a subsequent trace of :class:`Event`\ s.
// rst:
// rst-class-start:
struct MOD_DECL EventTrace {
	// rst-nested: causality::EventTrace::Event
	// rst-nested-start:
	struct Event {
		// rst:		.. var:: double time
		double time;
		// rst:		.. var:: Action action
		Action action;
	public:
		MOD_DECL friend bool operator==(const Event &a, const Event &b);
		MOD_DECL friend bool operator!=(const Event &a, const Event &b);
		MOD_DECL friend std::ostream &operator<<(std::ostream &s, const Event &e);
	};
	// rst-nested-end:
	// rst: .. type:: iterator
	// rst:           const_iterator = iterator
	struct iterator : boost::iterator_facade<iterator, Event, std::random_access_iterator_tag, Event> {
		iterator();
		iterator(const EventTrace *t, int offset);
	private:
		friend class boost::iterator_core_access;
		Event dereference() const;
		bool equal(iterator other) const;
		void increment();
		// TODO: implement the rest to make it a true random-access iterator
		void advance(int n);
	private:
		const EventTrace *t;
		int offset;
	};
	using const_iterator = iterator;
private:
	EventTrace(std::shared_ptr<Net> net, lib::Causality::EventTrace trace);
public:
	// rst: .. function:: EventTrace(Marking initialState)
	// rst:
	// rst:		Construct an empty event trace with an initial state.
	// rst:		A copy of the given marking is being stored.
	EventTrace(const Marking &initialState);
	~EventTrace();
	EventTrace(EventTrace &&);
	EventTrace &operator=(EventTrace &&);
	EventTrace(const EventTrace &);
	EventTrace &operator=(const EventTrace &);
public:
	// rst: .. function:: Marking getInitialState() const
	// rst:
	// rst:		:returns: a copy of the initial state for this event trace.
	Marking getInitialState() const;
	// rst: .. function:: int size() const
	// rst:
	// rst:		:returns: the number of events in the trace.
	int size() const;
	// rst: .. function:: double getTime() const
	// rst:
	// rst:		:returns: the time of the latest added event, or 0 if `size() == 0`.
	double getTime() const;
	// rst: .. function:: const_iterator begin() const
	// rst:               const_iterator end() const
	// rst:
	// rst:		:returns: the begin/end iterator for the range of events in the trace.
	const_iterator begin() const;
	const_iterator end() const;
	// rst: .. function:: Event operator[](int i) const
	// rst:
	// rst:		:returns: the `i`\ th event in the trace.
	// rst:		:throws: :class:`LogicError` if `i` is out of bounds.
	Event operator[](int i) const;
	// rst: .. function:: friend std::ostream &operator<<(std::ostream &s, const EventTrace &t)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const EventTrace &t);
	// rst: .. function:: friend bool operator==(const EventTrace &a, const EventTrace &b)
	// rst:
	// rst:		Calls ``syncSize()`` on the internal initial state and the underlying net.
	MOD_DECL friend bool operator==(const EventTrace &a, const EventTrace &b);
public:
	// rst: .. function:: void add(double time, Action action)
	// rst:
	// rst:		Append an event to the trace.
	// rst:
	// rst:		:throws: :class:`LogicError` if the time is less than the current time of the trace.
	// rst:		:throws: :class:`LogicError` if the action has null descriptors.
	// rst:		:throws: :class:`LogicError` if the descriptors in the action does not belong to the underlying derivation graph.
	void add(double time, const Action &action);
public:
	// rst: .. function:: std::string print() const
	// rst:               std::string print(const EventTracePrinter &printer) const
	// rst:
	// rst:		Create a plot of the counts throughout the event trace at each time point.
	// rst:
	// rst:		:returns: the filename for the PDF that will be compiled by the post processor.
	std::string print() const;
	std::string print(const EventTracePrinter &printer) const;
	// rst: .. function:: std::string dump() const
	// rst:               std::string dump(const std::string &filename) const
	// rst:
	// rst:		Dump the initial marking and list of events to a file. This file can be loaded in again.
	// rst:
	// rst:		:param filename: the name of the file to save the dump to.
	// rst:			If non is given an auto-generated name in the ``out/`` folder is used.
	// rst:			If an empty string is given, it is treated as if non is given.
	// rst:		:returns: the filename with the dumped trace.
	std::string dump() const;
	std::string dump(const std::string &filename) const;
private:
	struct Pimpl;
	std::unique_ptr<Pimpl> p;
public:
	// rst: .. function:: static EventTrace load(std::shared_ptr<dg::DG> dg_, const std::string &file)
	// rst:               static EventTrace load(std::shared_ptr<Net> net, const std::string &file)
	// rst:
	// rst:		The version taking a derivation graph will first create a Petri net and call the second version,
	// rst:		i.e., it is equivalent to `load(Net::make(dg_), file)`.
	// rst:
	// rst:		:returns: an event trace loaded from the given file.
	// rst:			The given derivation graph or the underlying derivation graph of the given net
	// rst:			must match the derivation graph originally used to create the dump.
	// rst:		:throws: :class:`LogicError` if `dg_`/`net` is a null pointer.
	// rst:		:throws: :class:`InputError` on bad data or if the given derivation graph does not match the data.
	static EventTrace load(std::shared_ptr<dg::DG> dg_, const std::string &file);
	static EventTrace load(std::shared_ptr<Net> net, const std::string &file);
};
// rst-class-end:

} // namespace mod::causality

#endif // MOD_CAUSALITY_EVENTTRACE_HPP