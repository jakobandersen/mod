#include "Read.hpp"

#include <iomanip>

//#define BOOST_SPIRIT_X3_DEBUG
//
//namespace std {
//
//std::ostream &operator<<(std::ostream &s, const std::optional<std::string> &os) {
//	if(!os) return s << "nullopt";
//	else return s << *os;
//}
//
//} // namespace std

#include <mod/graph/Graph.hpp>
#include <mod/rule/Rule.hpp>
#include <mod/lib/DG/Dump.hpp>
#include <mod/lib/DG/NonHyperBuilder.hpp>
#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/Graph/Properties/Stereo.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/IO/Config.hpp>
#include <mod/lib/IO/Json.hpp>
#include <mod/lib/IO/Parsing.hpp>

#include <boost/dynamic_bitset.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/spirit/home/x3/auxiliary/attr.hpp>
#include <boost/spirit/home/x3/auxiliary/eps.hpp>
#include <boost/spirit/home/x3/char/char.hpp>
#include <boost/spirit/home/x3/char/char_class.hpp>
#include <boost/spirit/home/x3/directive/lexeme.hpp>
#include <boost/spirit/home/x3/numeric/uint.hpp>
#include <boost/spirit/home/x3/operator/alternative.hpp>
#include <boost/spirit/home/x3/operator/and_predicate.hpp>
#include <boost/spirit/home/x3/operator/difference.hpp>
#include <boost/spirit/home/x3/operator/list.hpp>
#include <boost/spirit/home/x3/operator/optional.hpp>
#include <boost/spirit/home/x3/operator/plus.hpp>
#include <boost/spirit/home/x3/string/literal_string.hpp>

#include <string>

BOOST_FUSION_ADAPT_STRUCT(mod::lib::DG::Read::AbstractDerivation,
                          (std::optional<std::string>, id)
		                          (mod::lib::DG::Read::AbstractDerivation::List, left)
		                          (bool, reversible)
		                          (mod::lib::DG::Read::AbstractDerivation::List, right)
)

namespace mod::lib::DG::Read {
namespace {
namespace parser {

const auto identifier = x3::rule<struct identifier, std::string>("identifier")
		                        = x3::lexeme[+(x3::ascii::char_ - x3::ascii::space)];
const auto derivationId = x3::rule<struct derivationId, std::string>("derivation ID")
		                          = x3::lexeme["#" > identifier];
const auto coef = x3::rule<struct coef, unsigned int>("coefficient")
		                  = x3::lexeme[x3::uint_ >> &x3::ascii::space] | (x3::eps >> x3::attr(1u));
const auto element = x3::rule<struct element, std::pair<unsigned int, std::string> >("element")
		                     = coef >> identifier;
const auto side = x3::rule<struct side, std::vector<std::pair<unsigned int, std::string>>>("side")
		                  = element % '+';
const auto arrow = x3::rule<struct arrow, bool>("-> or <=>")
		                   = ("->" >> x3::attr(false)) | ("<=>" >> x3::attr(true));
const auto derivation = x3::rule<struct arrow, AbstractDerivation>("derivation")
		                        = (-derivationId >> side) > arrow > side; // parens due to Clang warning
const auto derivations = +derivation;

} // namespace parser
} // namespace

std::optional<nlohmann::json> loadDump(const std::string &file, std::ostream &err) {
	boost::iostreams::mapped_file_source ifs(file);
	std::vector<std::uint8_t> data(ifs.begin(), ifs.end());
	auto jOpt = IO::readJson(data, err);
	if(!jOpt) return {};
	static const nlohmann::json schema = R"({
		"$schema": "http://json-schema.org/draft-07/schema#",
		"type": "object",
		"properties": {
			"version": {"type": "integer"},
			"labelSettings": {"type": "object"},
			"vertices": {"type": "array", "items": {
				"type": "array",
				"items": [
					{"type": "integer", "description": "id"},
					{"type": "string",  "description": "graphName"},
					{"type": "string",  "description": "graphGML"}
				]
			}},
			"rules": {"type": "array", "items": {"type": "string"}},
			"edges": {"type": "array", "items": {
				"type": "array",
				"items": [
					{"type": "integer",                             "description": "id"},
					{"type": "array", "items": {"type": "integer"}, "description": "sources"},
					{"type": "array", "items": {"type": "integer"}, "description": "targets"}
				]
			}}
		},
		"required": ["version", "labelSettings", "vertices"]
	})"_json;
	// TODO: the labelSettings part should be validated as well
	static const nlohmann::json_schema::json_validator validator(schema);
	auto &j = *jOpt;
	if(!IO::validateJson(j, validator, err, "Data does not conform to schema:"))
		return {};

	if(j["version"].get<int>() != 3) {
		err << "Unknown DG dump version.";
		return {};
	}

	return jOpt;
}

std::unique_ptr<NonHyper> dump(const std::vector<std::shared_ptr<mod::graph::Graph>> &graphDatabase,
                               const std::vector<std::shared_ptr<mod::rule::Rule>> &ruleDatabase,
                               const std::string &file,
                               IsomorphismPolicy graphPolicy,
                               std::ostream &err, int verbosity) {
	boost::iostreams::mapped_file_source ifs;
	try {
		ifs.open(file);
	} catch(const BOOST_IOSTREAMS_FAILURE &e) {
		err << "Could not open file '" << file << "':\n" << e.what();
		return {};
	}
	// the newer old versions have a version tag
	if(ifs.size() > 8 && std::string(ifs.begin(), ifs.begin() + 8) == "version:") {
		ifs.close();
		return Dump::load(graphDatabase, ruleDatabase, file, err);
	}
	// but the older old versions do not
	if(ifs.size() > 12 && std::string(ifs.begin(), ifs.begin() + 12) == "numVertices:") {
		ifs.close();
		return Dump::load(graphDatabase, ruleDatabase, file, err);
	}
	ifs.close();
	auto jOpt = loadDump(file, err);
	if(!jOpt) return {};
	auto &j = *jOpt;

	LabelSettings labelSettings = from_json(j["labelSettings"]);
	auto dgInternal = std::make_unique<NonHyperBuilder>(labelSettings, graphDatabase, graphPolicy);
	{ // construction
		auto b = dgInternal->build(nullptr, nullptr);
		auto res = b.trustLoadDump(std::move(j), ruleDatabase, err, verbosity);
		if(!res) return {};
	}
	return std::unique_ptr<NonHyper>(dgInternal.release());
}

std::optional<std::vector<AbstractDerivation>> abstract(const std::string &s, std::ostream &err) {
	auto iterStart = s.begin(), iterEnd = s.end();
	std::vector<AbstractDerivation> derivations;
	try {
		lib::IO::parse(iterStart, iterEnd, parser::derivations, derivations, true, x3::ascii::space);
	} catch(const lib::IO::ParsingError &e) {
		err << e.msg;
		return {};
	}
	return derivations;
}

// -----------------------------------------------------------------------------------

bool dumpDigest(const HyperGraphType &dg, const nlohmann::json &j, std::ostream &err, const std::string &errType) {
	static const nlohmann::json schema = R"({
		"$schema": "http://json-schema.org/draft-07/schema#",
		"type": "object",
		"properties": {
			"numUDGVertices": {"type": "integer"},
			"isVertex": {"type": "array", "items": {"type": "integer"}}
		},
		"required": ["numUDGVertices", "isVertex"]
	})"_json;
	static const nlohmann::json_schema::json_validator validator(schema);
	if(!IO::validateJson(j, validator, err, "Data does not conform to schema:"))
		return false;
	const int numUDGVertices = j["numUDGVertices"];
	if(num_vertices(dg) != numUDGVertices) {
		err << "DG size mismatch. Given DG has " << num_vertices(dg)
		    << " vertices and edges, but the " << errType << " is based on one with "
		    << numUDGVertices << ".";
		return false;
	}
	const auto &bitmap = j["isVertex"];
	std::vector<uint32_t> blocks;
	blocks.reserve(bitmap.size());
	for(const auto &b: bitmap)
		blocks.push_back(b.get<uint32_t>());
	boost::dynamic_bitset<uint32_t> isVertex(num_vertices(dg));
	boost::from_block_range(blocks.begin(), blocks.end(), isVertex);
	assert(isVertex.size() >= num_vertices(dg));
	isVertex.resize(num_vertices(dg));
	for(const auto v: asRange(vertices(dg))) {
		const auto isVertexActual = dg[v].kind == DG::HyperVertexKind::Vertex;
		const auto vId = get(boost::vertex_index_t(), dg, v);
		if(isVertex[vId] != isVertexActual) {
			err << "DG mismatch at ";
			if(isVertexActual) err << "vertex";
			else err << "edge";
			err << " " << vId << ".";
			return false;
		}
	}
	return true;
}

std::optional<HyperVertex>
vertexOrEdge(const HyperGraphType &dg, std::size_t id, std::ostream &err, const std::string &errPrefix) {
	if(id >= num_vertices(dg)) {
		err << errPrefix << " ID for vertex or edge is out of range (id="
		    << id << ", last=" << num_vertices(dg) << ").";
		return {};
	}
	const auto vs = vertices(dg);
	const auto v = *std::next(vs.first, id);
	assert(get(boost::vertex_index_t(), dg, v) == id);
	return v;
}

std::optional<HyperVertex>
vertex(const HyperGraphType &dg, std::size_t id, std::ostream &err, const std::string &errPrefix) {
	auto vOpt = vertexOrEdge(dg, id, err, errPrefix);
	if(vOpt && dg[*vOpt].kind != HyperVertexKind::Vertex) {
		err << errPrefix << " ID does not represent a vertex (id=" << id << ").";
		return {};
	}
	return vOpt;
}

std::optional<HyperVertex>
edge(const HyperGraphType &dg, std::size_t id, std::ostream &err, const std::string &errPrefix) {
	auto vOpt = vertexOrEdge(dg, id, err, errPrefix);
	if(vOpt && dg[*vOpt].kind != HyperVertexKind::Edge) {
		err << errPrefix << " ID does not represent a edge (id=" << id << ").";
		return {};
	}
	return vOpt;
}

} // namespace mod::lib::DG::Read