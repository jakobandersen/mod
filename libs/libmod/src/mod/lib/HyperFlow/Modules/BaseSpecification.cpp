#include "Base.hpp"

#include <mod/Function.hpp>
#include <mod/lib/DG/Hyper.hpp>
#include <mod/lib/DG/IO/Read.hpp>
#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/HyperFlow/Model.hpp>
#include <mod/lib/HyperFlow/Specification.hpp>
#include <mod/lib/IO/Json.hpp>

#include <cassert>
#include <ostream>

namespace mod::lib::HyperFlow {
using HyperVertex = lib::DG::HyperVertex;

BaseSpecification::BaseSpecification(Specification &owner, bool setDefaults)
		: SpecificationModule(owner) {}

std::string BaseSpecification::getName() const {
	return "Base";
}

void BaseSpecification::addDefaultObjective(hyperflow::LinExp &exp) const {
	exp += hyperflow::vars::edgeFlow;
	exp += hyperflow::vars::inFlow;
}

void BaseSpecification::addSource(lib::DG::HyperVertex v) {
	assert(owner.dgHyper.getGraph()[v].kind == lib::DG::HyperVertexKind::Vertex);
	sources.insert(v);
}

const std::set<lib::DG::HyperVertex> &BaseSpecification::getSources() const {
	return sources;
}

void BaseSpecification::addSink(lib::DG::HyperVertex v) {
	assert(owner.dgHyper.getGraph()[v].kind == lib::DG::HyperVertexKind::Vertex);
	sinks.insert(v);
}

const std::set<lib::DG::HyperVertex> &BaseSpecification::getSinks() const {
	return sinks;
}

void BaseSpecification::exclude(lib::DG::HyperVertex v) {
	assert(owner.dgHyper.getGraph()[v].kind == lib::DG::HyperVertexKind::Vertex);
	excluded.insert(v);
}

const std::set<lib::DG::HyperVertex> &BaseSpecification::getExcluded() const {
	return excluded;
}

void BaseSpecification::separateIOInternalTransit(lib::DG::HyperVertex v) {
	verticesWithForcedTransitSeparation.insert(v);
}

const std::set<lib::DG::HyperVertex> &BaseSpecification::getSeparatedIOInternalTransit() const {
	return verticesWithForcedTransitSeparation;
}

void BaseSpecification::listImpl(std::ostream &s) const {
	s << "Sources:  ";
	for(HyperVertex v : getSources()) {
		const lib::graph::Graph *g = owner.dgHyper.getGraph()[v].graph;
		s << "'" << g->getName() << "' ";
	}
	s << "\n";
	s << "Sinks:    ";
	for(HyperVertex v : getSinks()) {
		const lib::graph::Graph *g = owner.dgHyper.getGraph()[v].graph;
		s << "'" << g->getName() << "' ";
	}
	s << "\n";
	s << "Excluded: ";
	for(HyperVertex v : getExcluded()) {
		const lib::graph::Graph *g = owner.dgHyper.getGraph()[v].graph;
		s << "'" << g->getName() << "' ";
	}
	s << "\n";
	s << "SeparatedIOInternalTransit: ";
	for(HyperVertex v : getSeparatedIOInternalTransit()) {
		const lib::graph::Graph *g = owner.dgHyper.getGraph()[v].graph;
		s << "'" << g->getName() << "' ";
	}
	s << "\n";
	s << "Allow hyper-loops: " << std::boolalpha << allowHyperLoops << "\n";

	s << "AllowReversal:     " << std::boolalpha << allowReversal << "\n";
	s << "AllowIOReversal:   " << std::boolalpha << allowIOReversal << "\n";
	s << "Relaxed: " << std::boolalpha << relaxed << "\n";
}

nlohmann::json BaseSpecification::dumpImpl() const {
	nlohmann::json j;
	j["version"] = 1;
	const auto dgIdx = get(boost::vertex_index_t(), owner.dgHyper.getGraph());
	{
		std::vector<lib::DG::HyperVertex> sources;
		sources.assign(begin(getSources()), end(getSources()));
		std::sort(begin(sources), end(sources));
		auto &s = j["sources"] = nlohmann::json::array();
		for(const auto v : sources) s.push_back(dgIdx[v]);
	}
	{
		std::vector<lib::DG::HyperVertex> sinks;
		sinks.assign(begin(getSinks()), end(getSinks()));
		std::sort(begin(sinks), end(sinks));
		auto &s = j["sinks"] = nlohmann::json::array();
		for(const auto v : sinks) s.push_back(dgIdx[v]);
	}
	{
		std::vector<lib::DG::HyperVertex> excluded;
		excluded.assign(begin(getExcluded()), end(getExcluded()));
		std::sort(begin(excluded), end(excluded));
		auto &s = j["excluded"] = nlohmann::json::array();
		for(const auto v : excluded) s.push_back(dgIdx[v]);
	}
	{
		std::vector<lib::DG::HyperVertex> separated;
		separated.assign(begin(getSeparatedIOInternalTransit()), end(getSeparatedIOInternalTransit()));
		std::sort(begin(separated), end(separated));
		auto &s = j["separatedIOInternalTransit"] = nlohmann::json::array();
		for(const auto v : separated) s.push_back(dgIdx[v]);
	}
	j["allowHyperLoops"] = allowHyperLoops;
	j["allowReversal"] = allowReversal;
	j["allowIOReversal"] = allowIOReversal;
	j["relaxed"] = relaxed;
	return j;
}

bool BaseSpecification::loadImpl(const nlohmann::json &j, std::ostream &err) {
	static const auto schema = R"({
		"$schema": "http://json-schema.org/draft-07/schema#",
		"type": "object",
		"properties": {
			"version": {"type": "integer", "minimum": 1, "maximum": 1},
			"sources":
				{"type": "array", "items": {"type": "integer", "minimum": 0}},
			"sinks":
				{"type": "array", "items": {"type": "integer", "minimum": 0}},
			"excluded":
				{"type": "array", "items": {"type": "integer", "minimum": 0}},
			"separatedIOInternalTransit":
				{"type": "array", "items": {"type": "integer", "minimum": 0}},
			"allowHyperLoops": {"type": "boolean"},
			"allowReversal": {"type": "boolean"},
			"allowIOReversal": {"type": "boolean"},
			"relaxed": {"type": "boolean"}
		},
		"required": ["version"]
	})"_json;
	static const nlohmann::json_schema::json_validator validator(schema);
	if(!IO::validateJson(j, validator, err, "Base module specification data does not conform to schema:"))
		return false;
	const auto &dg = owner.dgHyper.getGraph();
	for(const int id : j["sources"]) {
		const auto vOpt = lib::DG::Read::vertex(dg, id, err, "Base specification source error.");
		if(!vOpt) return false;
		addSource(*vOpt);
	}
	for(const int id : j["sinks"]) {
		const auto vOpt = lib::DG::Read::vertex(dg, id, err, "Base specification sink error.");
		if(!vOpt) return false;
		addSink(*vOpt);
	}
	for(const int id : j["excluded"]) {
		const auto vOpt = lib::DG::Read::vertex(dg, id, err, "Base specification excluded error.");
		if(!vOpt) return false;
		exclude(*vOpt);
	}
	for(const int id : j["separatedIOInternalTransit"]) {
		const auto vOpt = lib::DG::Read::vertex(dg, id, err, "Base specification separatedIOInternalTransit error.");
		if(!vOpt) return false;
		separateIOInternalTransit(*vOpt);
	}

	allowHyperLoops = j["allowHyperLoops"];
	allowReversal = j["allowReversal"];
	allowIOReversal = j["allowIOReversal"];
	relaxed = j["relaxed"];
	return true;
}

Transits BaseSpecification::getTransits() const {
	Transits rs;
	for(const auto vHyper : verticesWithForcedTransitSeparation)
		lib::DG::Expanded::addIOInverseTransits(owner.dgHyper, vHyper, rs.expressed);
	if(!allowReversal) lib::DG::Expanded::addInternalInverseTransits(owner.dgHyper, rs.deleted);
	if(!allowIOReversal) lib::DG::Expanded::addIOInverseTransits(owner.dgHyper, rs.deleted);
	return rs;
}

std::unique_ptr<ModelModule> BaseSpecification::createModel(Model &owner) const {
	return std::make_unique<BaseModel>(owner, *this);
}

} // namespace mod::lib::HyperFlow
