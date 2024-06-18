#include "Term.hpp"

#include <mod/lib/Graph/LabelledGraph.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/IO/ParsingError.hpp>
#include <mod/lib/Term/IO/Read.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

namespace mod::lib::Graph {

PropTerm::PropTerm(const GraphType &g, const PropString &pString, const StringStore &stringStore) : Base(g) {
	// take every unique string and parse it
	std::unordered_map<std::string, std::size_t> labelToAddress;
	lib::Term::RawAppendStore varToAddr;
	const auto handleLabel = [&stringStore, &labelToAddress, &varToAddr, this](
			const std::string &label) -> std::size_t {
		// if there is a * in the label, then we can not just use label caching
		if(label.find('*') == std::string::npos) {
			auto iter = labelToAddress.find(label);
			if(iter != end(labelToAddress))
				return iter->second;
		}
		lib::Term::RawTerm rawTerm;
		try {
			rawTerm = Term::Read::rawTerm(label, stringStore);
		} catch(const lib::IO::ParsingError &e) {
			parsingError = e.msg;
			return std::numeric_limits<std::size_t>::max();
		}
		lib::Term::Address addr = lib::Term::append(machine, rawTerm, varToAddr);
		labelToAddress[label] = addr.addr;
		return addr.addr;
	};
	this->vertexState.resize(num_vertices(g));
	for(Vertex v: asRange(vertices(g))) {
		std::size_t vId = get(boost::vertex_index_t(), *this->g, v);
		assert(vId < this->vertexState.size());
		this->vertexState[vId] = handleLabel(pString[v]);
	}
	this->edgeState.reserve(num_edges(g));
	for(Edge e: asRange(edges(g))) {
		assert(get(boost::edge_index_t(), g, e) == this->edgeState.size());
		this->edgeState.push_back(handleLabel(pString[e]));
	}
	for(const auto &c: machine.getHeap()) {
		if(c.tag == Term::Cell::Tag::REF) {
			hasVariables = true;
			break;
		}
	}
	verify(this->g);
}

bool PropTerm::getHasVariables() const {
	return hasVariables;
}

const std::string &PropTerm::getParsingError() const {
	assert(!isValid(*this));
	return *parsingError;
}

bool isValid(const PropTerm &p) {
	return !p.parsingError.has_value();
}

const lib::Term::Wam &getMachine(const PropTerm &p) {
	return p.machine;
}

} // namespace mod::lib::Graph