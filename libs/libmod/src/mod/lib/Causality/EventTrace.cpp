#include "EventTrace.hpp"

#include <mod/lib/DG/IO/Read.hpp>
#include <mod/lib/DG/IO/Write.hpp>

#include <boost/iostreams/device/mapped_file.hpp>

namespace mod::lib::Causality {
namespace {

struct CompareVisitor {
	bool operator()(const EdgeAction &a, const EdgeAction &b) const {
		return a.e == b.e;
	}

	bool operator()(const InputAction &a, const InputAction &b) const {
		return a.v == b.v;
	}

	bool operator()(const OutputAction &a, const OutputAction &b) const {
		return a.v == b.v;
	}

	template<typename T, typename U>
	std::enable_if_t<!std::is_same_v < T, U>, bool>
	operator()(const T &a, const U &b) const {
		return false;
	}
};

} // namespace

bool operator==(const EventTrace::Event &a, const EventTrace::Event &b) {
	if(a.time != b.time) return false;
	return std::visit(CompareVisitor(), a.action, b.action);
}

EventTrace::EventTrace(const lib::DG::Hyper &dg, Marking initialState)
		: dg(dg), initialState(initialState) {}

void EventTrace::syncSize() const {
	initialState.syncSize();
}

const Marking &EventTrace::getInitialState() const {
	return initialState;
}

bool operator==(const EventTrace &a, const EventTrace &b) {
	assert(&a.dg == &b.dg);
	if(a.initialState.getMarking() != b.initialState.getMarking()) return false;
	return a.events == b.events;
}

void EventTrace::add(Event e) {
	events.push_back(std::move(e));
}

const std::vector <EventTrace::Event> &EventTrace::getEvents() const {
	return events;
}

nlohmann::json EventTrace::dump() const {
	nlohmann::json j;
	j["version"] = 1;
	j["dgData"] = lib::DG::Write::dumpDigest(dg.getGraph());

	const auto dgIdx = get(boost::vertex_index_t(), dg.getGraph());

	{ // initialState
		auto ps = nlohmann::json::array();
		for(const auto v: initialState.getNonZeroPlaces())
			ps.push_back(nlohmann::json::array({dgIdx[v], initialState[v]}));
		j["initialState"] = std::move(ps);
	}

	{
		auto es = nlohmann::json::array();
		struct Visitor {
			int operator()(const EdgeAction &a) const {
				return idx[a.e];
			}

			int operator()(const InputAction &a) const {
				return -idx[a.v] - 1;
			}

			int operator()(const OutputAction &a) const {
				return idx[a.v];
			}
		public:
			decltype(dgIdx) idx;
		};
		for(const auto &e: events)
			es.push_back(nlohmann::json::array({e.time, std::visit(Visitor{dgIdx}, e.action)}));
		j["events"] = std::move(es);
	}
	return j;
}

std::optional <EventTrace> EventTrace::load(const Net &net, const std::string &file, std::ostream &err) {
	boost::iostreams::mapped_file_source ifs;
	try {
		ifs.open(file);
	} catch(const BOOST_IOSTREAMS_FAILURE &e) {
		err << "Could not open file '" << file << "':\n" << e.what();
		return {};
	}
	std::vector <std::uint8_t> data(ifs.begin(), ifs.end());
	auto jOpt = IO::readJson(data, err);
	if(!jOpt) return {};
	auto &j = *jOpt;

	static const nlohmann::json schema = R"({
	    "$schema": "http://json-schema.org/draft-07/schema#",
	    "type": "object",
	    "properties": {
	        "version": {"type": "integer", "minimum": 1, "maximum": 1},
	        "initialState": {"type": "array", "items": {"type": "array", "prefixItems": [
	            {"type": "integer", "minimum": 0, "description": "vertex ID"},
	            {"type": "integer", "minimum": 0, "description": "token count"}
	        ], "minItems": 2, "maxItems": 2}},
	        "events": {"type": "array", "items": {"type": "array", "prefixItems": [
	            {"type": "number",  "description": "event time"},
	            {"type": "integer", "description": "action encoding"}
	        ], "minItems": 2, "maxItems": 2}}
	    },
	    "required": ["version", "dgData", "initialState", "events"]
	})"_json;
	static const nlohmann::json_schema::json_validator validator(schema);
	if(!IO::validateJson(j, validator, err, "Data does not conform to schema:"))
		return {};

	assert(j["version"].get<int>() == 1);

	const auto &dgHyper = net.getDG();
	const auto &dg = dgHyper.getGraph();

	if(!lib::DG::Read::dumpDigest(dg, j["dgData"], err, "event trace"))
		return {};

	Marking initialState(net);
	for(const auto &jtc: j["initialState"]) {
		assert(jtc.size() == 2);
		const std::size_t jv = jtc[0];
		const int jc = jtc[1];
		assert(jc >= 0);
		const auto vOpt = lib::DG::Read::vertex(dg, jv, err, "Initial state vertex error.");
		if(!vOpt) return {};
		initialState.add(*vOpt, jc);
	}

	EventTrace trace(dgHyper, std::move(initialState));
	for(const auto &je: j["events"]) {
		assert(je.size() == 2);
		const double time = je[0];
		const int action = je[1];
		if(action >= 0) {
			const auto vOpt = lib::DG::Read::vertexOrEdge(dg, action, err, "Event edge or output action error.");
			if(!vOpt) return {};
			if(dg[*vOpt].kind == lib::DG::HyperVertexKind::Edge) {
				trace.add(Event{time, EdgeAction{*vOpt}});
			} else {
				trace.add(Event{time, OutputAction{*vOpt}});
			}
		} else {
			const auto vOpt = lib::DG::Read::vertex(dg, -action - 1, err, "Event input action error.");
			if(!vOpt) return {};
			trace.add(Event{time, InputAction{*vOpt}});
		}
	}

	return trace;
}

} // namespace mod::lib::Causality