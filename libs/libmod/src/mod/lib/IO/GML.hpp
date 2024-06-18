#ifndef MOD_LIB_IO_GML_HPP
#define MOD_LIB_IO_GML_HPP

#include <mod/lib/Stereo/IO/Read.hpp>

#include <gml/converter_edsl.hpp>

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace mod::lib::IO::GML {

struct Vertex {
	int id;
	std::optional<std::string> label;
	std::optional<std::string> stereo;
public:
	std::optional<lib::Stereo::Read::ParsedEmbedding> parsedEmbedding;
};

struct Edge {
	friend std::ostream &operator<<(std::ostream &s, const Edge &e);
public:
	int source, target;
	std::optional<std::string> label;
	std::optional<std::string> stereo;
};

struct Graph {
	std::vector<Vertex> vertices;
	std::vector<Edge> edges;
};

struct AdjacencyConstraint {
	int id;
	std::string op;
	unsigned int count;
	std::vector<std::string> nodeLabels, edgeLabels;
};

struct LabelAnyConstraint {
	std::string label;
	std::vector<std::string> labels;
};

struct ShortestPathConstraint {
	int source, target;
	std::string op;
	int length;
};

using MatchConstraint = std::variant<AdjacencyConstraint, LabelAnyConstraint, ShortestPathConstraint>;

struct Rule {
	std::optional<std::string> id;
	std::optional<std::string> labelType;
	Graph left, context, right;
	std::vector<MatchConstraint> matchConstraints;
};

//------------------------------------------------------------------------------

static const auto makeVertexConverter = [](std::size_t lowerBound) {
	using namespace gml::converter::edsl;
	auto cStereo = string("stereo", &Vertex::stereo);
	return list<Vertex>("node", &Graph::vertices)
			(int_("id", &Vertex::id), 1, 1)
			(string("label", &Vertex::label), lowerBound, 1)
			(cStereo, 0, 1);
};

static const auto makeEdgeConverter = [](std::size_t lowerBound) {
	using namespace gml::converter::edsl;
	auto cStereo = string("stereo", &Edge::stereo);
	return list<Edge>("edge", &Graph::edges)
			(int_("source", &Edge::source), 1, 1)
			(int_("target", &Edge::target), 1, 1)
			(string("label", &Edge::label), lowerBound, 1)
			(cStereo, 0, 1);
};

} // namespace mod::lib::IO::GML

#endif // MOD_LIB_IO_GML_HPP