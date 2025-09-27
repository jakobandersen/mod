#ifndef MOD_LIB_CAUSALITY_EVENTTRACE_HPP
#define MOD_LIB_CAUSALITY_EVENTTRACE_HPP

#include <mod/lib/Causality/Petri.hpp>
#include <mod/lib/DG/GraphDecl.hpp>
#include <mod/lib/IO/Json.hpp>

#include <variant>

namespace mod::lib::DG {
struct Hyper;
} // namespace mod::lib::DG
namespace mod::lib::Causality {

struct EdgeAction {
	lib::DG::HyperVertex e;
};

struct InputAction {
	lib::DG::HyperVertex v;
};

struct OutputAction {
	lib::DG::HyperVertex v;
};

using Action = std::variant<EdgeAction, InputAction, OutputAction>;

struct EventTrace {
	struct Event {
		double time;
		Action action;
	public:
		friend bool operator==(const Event &a, const Event &b);
	};
public:
	EventTrace(const lib::DG::Hyper &dg, Marking initialState);
	void syncSize() const;
	const Marking &getInitialState() const;
public:
	// pre: based on same DG
	// pre: the proper synxSize() calls should have been made
	friend bool operator==(const EventTrace &a, const EventTrace &b);
public:
	nlohmann::json dump() const;
	static std::optional<EventTrace> load(const Net &net, const std::string &file, std::ostream &err);
public:
	void add(Event e);
	const std::vector<Event> &getEvents() const;
public:
	const lib::DG::Hyper &dg;
private:
	mutable Marking initialState;
	std::vector<Event> events;
};

} // namespace mod::lib::Causality

#endif // MOD_LIB_CAUSALITY_EVENTTRACE_HPP
