#include <mod/py/Common.hpp>

#include <mod/causality/EventTrace.hpp>
#include <mod/causality/EventTracePrinter.hpp>
#include <mod/causality/Petri.hpp>

namespace mod::causality::Py {
namespace {

EventTrace::Event EventTrace_getitem(const EventTrace &et, int index) {
	if(index < 0) index = et.size() + index;
	return et[index];
}

// based on https://www.boost.org/doc/libs/1_53_0/libs/python/doc/v2/faq.html#custom_string
struct Action_to_python {
	static PyObject *convert(const Action &a) {
		py::object obj = std::visit([](const auto &a) { return py::object(a); }, a);
		return py::incref(obj.ptr());
	}
};

template<typename T>
void EventTrace_add(EventTrace &t, double time, const T &a) {
	return t.add(time, a);
}

PyObject *EventTrace_Event_action(const EventTrace::Event &e) {
	return Action_to_python::convert(e.action);
}

} // namespace

void EventTrace_doExport() {
	// rst: .. class:: causality.EdgeAction
	// rst:
	py::class_<EdgeAction>("EdgeAction", py::no_init)
			// rst:		.. method:: __init__()
			// rst:		            __init__(e)
			.def(py::init<>())
			.def(py::init<dg::DG::HyperEdge>())
			.def(py::self == py::self)
			.def(py::self != py::self)
			.def(str(py::self))
					// rst:		.. attribute:: edge
					// rst:
					// rst:			(Read-only) the hyperedge for the action.
					// rst:			May not be null.
					// rst:
					// rst:			:type: DG.HyperEdge
			.def_readonly("edge", &EdgeAction::edge)
					// rst:		.. function:: void applyTo(m) const
					// rst:
					// rst:			Perform ``m.fire(edge)``.
					// rst:
					// rst:			:param Marking m: the marking to apply the action to.
			.def("applyTo", &EdgeAction::applyTo);
	// rst: .. class:: causality.InputAction
	// rst:
	py::class_<InputAction>("InputAction", py::no_init)
			// rst:		.. method:: __init__()
			// rst:		            __init__(v)
			.def(py::init<>())
			.def(py::init<dg::DG::Vertex>())
			.def(py::self == py::self)
			.def(py::self != py::self)
			.def(str(py::self))
					// rst:		.. attribute:: vertex
					// rst:
					// rst:			(Read-only) the vertex being injected.
					// rst:			May not be null.
					// rst:
					// rst:			:type: DG.Vertex
			.def_readonly("vertex", &InputAction::vertex)
					// rst:		.. function:: void applyTo(m) const
					// rst:
					// rst:			Perform `m.add(vertex, 1)`.
					// rst:
					// rst:			:param Marking m: the marking to apply the action to.
			.def("applyTo", &InputAction::applyTo);
	// rst: .. class:: causality.OutputAction
	// rst:
	py::class_<OutputAction>("OutputAction", py::no_init)
			// rst:		.. method:: __init__()
			// rst:		            __init__(v)
			.def(py::init<>())
			.def(py::init<dg::DG::Vertex>())
			.def(py::self == py::self)
			.def(py::self != py::self)
			.def(str(py::self))
					// rst:		.. attribute:: vertex
					// rst:
					// rst:			(Read-only) the vertex being extracted.
					// rst:			May not be null.
					// rst:
					// rst:			:type: DG.Vertex
			.def_readonly("vertex", &OutputAction::vertex)
					// rst:		.. function:: void applyTo(m) const
					// rst:
					// rst:			Perform `m.remove(vertex, 1)`.
					// rst:
					// rst:			:param Marking m: the marking to apply the action to.
			.def("applyTo", &OutputAction::applyTo);

	py::to_python_converter<Action, Action_to_python>();

	// rst: .. class:: causality.EventTrace
	// rst:
	// rst:		A representation of a trace of events starting from an initial marking.
	// rst:		An event is either a reaction/hyperedge being performed
	// rst:		or a transfer of tokens out or in to the system.
	// rst:
	py::scope EventTraceClass = py::class_<EventTrace>("EventTrace", py::no_init)
			// rst:		.. method:: __init__(initialState)
			// rst:
			// rst:			Construct an empty event trace with an initial state.
			// rst:			A copy of the given marking is being stored.
			// rst:
			// rst:			:param Marking initialState: the marking to used as the initial state.
			.def(py::init<const Marking &>())
					// rst:		.. attribute:: initialState
					// rst:
					// rst:			(Read-only) Read a copy of the initial state for this event trace.
					// rst:
					// rst:			:type: Marking
			.add_property("initialState", &EventTrace::getInitialState)
					// rst:		.. method:: __len__()
					// rst:
					// rst:			:returns: the number of events in the trace.
					// rst:			:rtype: int
			.def("__len__", &EventTrace::size)
					// rst:		.. attribute:: time
					// rst:
					// rst:			(Read-only) the time of the latest added event, or 0 if ``len(self) == 0``.
					// rst:
					// rst:			:type: float
			.add_property("time", &EventTrace::getTime)
			.def("__iter__", py::iterator<EventTrace>())
					// rst:		.. method:: __getitem__()
					// rst:
					// rst:			:returns: the ``i``\ th event in the trace.
					// rst:			:rtype: Event
					// rst:			:raises: :class:`LogicError` if ``i`` is out of bounds.
			.def("__getitem__", EventTrace_getitem)
			.def(str(py::self))
					// rst:		.. method:: __eq__()
					// rst:
					// rst:			Calls ``syncSize()`` on the internal initial state and the underlying net.
			.def(py::self == py::self)
					// rst:		.. method:: add(time, action)
					// rst:
					// rst:			Append an event to the trace.
					// rst:
					// rst:			:param float time: the time associated with the event.
					// rst:			:param action: the action of the event.
					// rst:			:type action: EdgeAction or InputAction or OutputAction
					// rst:			:raises: :class:`LogicError` if the time is less than the current time of the trace.
					// rst:			:raises: :class:`LogicError` if the action has null descriptors.
					// rst:			:raises: :class:`LogicError` if the descriptors in the action does not belong to the underlying derivation graph.
			.def("add", &EventTrace_add<EdgeAction>)
			.def("add", &EventTrace_add<InputAction>)
			.def("add", &EventTrace_add<OutputAction>)
					// rst:		.. method:: print(printer=EventTracePrinter())
					// rst:
					// rst:			Create a plot of the counts throughout the event trace at each time point.
					// rst:
					// rst:			:returns: the filename for the PDF that will be compiled by the post processor.
					// rst:			:rtype: str
			.def("print", static_cast<std::string (EventTrace::*)(const EventTracePrinter&) const>(&EventTrace::print))
					// rst:		.. method:: dump()
					// rst:		            dump(filename)
					// rst:
					// rst:			Dump the initial marking and list of events to a file. This file can be loaded in again.
					// rst:
					// rst:			:param str filename: the name of the file to save the dump to.
					// rst:				If non is given an auto-generated name in the ``out/`` folder is used.
					// rst:				If an empty string is given, it is treated as if non is given.
					// rst:
					// rst:				.. note:: The filename is being used literally, i.e., it is not being
					// rst:					prefixed according to the current script location as input filenames are.
					// rst:			:returns: the filename with the dumped model.
					// rst:			:rtype: str
			.def("dump", static_cast<std::string (EventTrace::*)() const>(&EventTrace::dump))
			.def("dump", static_cast<std::string (EventTrace::*)(const std::string &) const>(&EventTrace::dump))
					// rst:		.. staticmethod:: load(dg, f)
					// rst:		                  load(net, f)
					// rst:
					// rst:			Load an event trace dump.
					// rst:
					// rst:			:param DG dg: a matching derivation graph for the loaded event trace.
					// rst:				A :class:`~causality.Net` is being constructed based on this DG,
					// rst:				i.e., it is equivalent to ``EventTrace(Net(dg), f)``.
					// rst:			:param Net net: a Petri net where the underlying derivation graph matches the loaded event trace.
					// rst:			:type f: str or CWDPath
					// rst:			:param f: an event trace dump file to load.
					// rst:			:type f: str or CWDPath
					// rst:			:returns: the loaded event trace.
					// rst:			:rtype: EventTrace
					// rst:			:raises: :class:`LogicError` if ``dg``/``net`` is ``None``.
					// rst:			:raises: :class:`InputError` on bad data or if the given derivation graph does not match the data.
			.def("load", static_cast<EventTrace (*)(std::shared_ptr<dg::DG>, const std::string &)>(&EventTrace::load))
			.def("load",
			     static_cast<EventTrace (*)(std::shared_ptr<Net>, const std::string &)>(&EventTrace::load))
			.staticmethod("load");

	// rst: .. class:: causality.EventTrace.Event
	// rst:
	// rst:		A representation of an event in an event trace.
	// rst:
	py::class_<EventTrace::Event>("Event", py::no_init)
			.def(py::self == py::self)
			.def(py::self != py::self)
			.def(str(py::self))
					// rst:		.. attribute:: time
					// rst:
					// rst:			(Read-only) the associated time for the event.
					// rst:
					// rst:			:type: float
			.def_readonly("time", &EventTrace::Event::time)
					// rst:		.. attribute:: action
					// rst:
					// rst:			(Read-only) the action of the event.
					// rst:
					// rst:			:type: EdgeAction or InputAction or OutputAction
			.add_property("action", &EventTrace_Event_action);
}

} // namespace mod::causality::Py