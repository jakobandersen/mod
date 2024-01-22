#include "Read.hpp"

#include <mod/lib/GraphMorphism/Constraints/ShortestPath.hpp>
#include <mod/lib/GraphMorphism/Constraints/VertexAdjacency.hpp>
#include <mod/lib/IO/DFS.hpp>
#include <mod/lib/IO/GML.hpp>
#include <mod/lib/Rules/Real.hpp>
#include <mod/lib/Rules/Properties/Molecule.hpp>
#include <mod/lib/Rules/Properties/Stereo.hpp>
#include <mod/lib/Stereo/Inference.hpp>

#include <gml/parser.hpp>
#include <gml/converter.hpp>
#include <gml/converter_edsl.hpp>

#include <unordered_map>
#include <unordered_set>

namespace mod::lib::Rules::Read {
using lib::IO::Result;
namespace GML = lib::IO::GML;
namespace {

template<typename T>
struct Label {
	std::optional<T> left, context, right;
};

struct VertexLabels {
	bool inLeft = false, inContext = false, inRight = false;
	Label<std::string> string, stereo;
	bool stereoInContext = false;
public:
	lib::DPO::CombinedRule::CombinedVertex cgVertex;
	std::optional<lib::Stereo::Read::ParsedEmbedding> parsedEmbeddingLeft, parsedEmbeddingRight;
};

struct EdgeLabels {
	bool inLeft = false, inContext = false, inRight = false;
	Label<std::string> string, stereo;
	bool stereoInContext = false;
public:
	lib::DPO::CombinedRule::CombinedEdge cgEdge;
};

Result<GML::Rule> parseGML(std::string_view input) {
	GML::Rule rule;
	gml::ast::KeyValue ast;
	try {
		ast = gml::parser::parse(input);
	} catch(const gml::parser::error &e) {
		return Result<>::Error(e.what());
	}
	using namespace gml::converter::edsl;
	auto cVertex = GML::makeVertexConverter(0);
	auto cEdge = GML::makeEdgeConverter(0);
	auto nodeLabels = list<Parent>("nodeLabels")
			(string("label", &GML::AdjacencyConstraint::nodeLabels));
	auto edgeLabels = list<Parent>("edgeLabels")
			(string("label", &GML::AdjacencyConstraint::edgeLabels));
	auto constrainAdj = list<GML::AdjacencyConstraint>("constrainAdj", &GML::Rule::matchConstraints)
			(int_("id", &GML::AdjacencyConstraint::id), 1, 1)
			(string("op", &GML::AdjacencyConstraint::op), 1, 1)
			(int_("count", &GML::AdjacencyConstraint::count), 1, 1)
			(nodeLabels)(edgeLabels);
	auto constrainShortestPath = list<GML::ShortestPathConstraint>("constrainShortestPath",
	                                                               &GML::Rule::matchConstraints)
			(int_("source", &GML::ShortestPathConstraint::source), 1, 1)
			(int_("target", &GML::ShortestPathConstraint::target), 1, 1)
			(string("op", &GML::ShortestPathConstraint::op), 1, 1)
			(int_("length", &GML::ShortestPathConstraint::length), 1, 1);
	auto makeSide = [&](std::string name, GML::Graph GML::Rule::*side) {
		return list<GML::Graph>(name, side)(cVertex)(cEdge);
	};
	auto cRule = list<Parent>("rule")
			(string("ruleID", &GML::Rule::id), 0, 1)
			(string("labelType", &GML::Rule::labelType), 0, 1)
			(makeSide("left", &GML::Rule::left), 0, 1)
			(makeSide("context", &GML::Rule::context), 0, 1)
			(makeSide("right", &GML::Rule::right), 0, 1)
			(constrainAdj)(constrainShortestPath);
	auto iterBegin = &ast;
	auto iterEnd = iterBegin + 1;
	try {
		gml::converter::convert(iterBegin, iterEnd, cRule, rule);
	} catch(const gml::converter::error &e) {
		return Result<>::Error(e.what());
	}
	return rule;
}

Result<> checkGraphDuplicatesAndLoops(const GML::Graph &side, std::string_view name) {
	std::unordered_set<int> vertexIds;
	for(const GML::Vertex &v: side.vertices) {
		if(vertexIds.find(v.id) != end(vertexIds))
			return Result<>::Error("Duplicate vertex " + std::to_string(v.id) + " in " + std::string(name) + " graph.");
		vertexIds.insert(v.id);
	}
	std::set<std::pair<int, int>> edgeIds;
	for(const GML::Edge &e: side.edges) {
		if(e.source == e.target)
			return Result<>::Error(
					"Loop edge (on " + std::to_string(e.source) + ", in " + std::string(name) + ") is not allowed.");
		auto eSorted = std::minmax(e.source, e.target);
		if(edgeIds.find(eSorted) != end(edgeIds))
			return Result<>::Error(
					"Duplicate edge (" + std::to_string(e.source) + ", " + std::to_string(e.target) + ") in "
					+ std::string(name) + " graph.");
		edgeIds.insert(eSorted);
	}
	return Result<>();
}

auto extractDataFromGML(const GML::Rule &rule) {
	std::map<int, VertexLabels> vLabelsFromId;
	std::map<std::pair<int, int>, EdgeLabels> eLabelsFromIds;

	for(const GML::Vertex &vGML: rule.left.vertices) {
		auto &v = vLabelsFromId[vGML.id];
		v.inLeft = true;
		v.string.left = vGML.label;
		v.stereo.left = vGML.stereo;
	}
	for(const GML::Vertex &vGML: rule.context.vertices) {
		auto &v = vLabelsFromId[vGML.id];
		v.inContext = true;
		v.string.context = vGML.label;
		v.stereo.context = vGML.stereo;
	}
	for(const GML::Vertex &vGML: rule.right.vertices) {
		auto &v = vLabelsFromId[vGML.id];
		v.inRight = true;
		v.string.right = vGML.label;
		v.stereo.right = vGML.stereo;
	}
	for(const GML::Edge &eGML: rule.left.edges) {
		auto eSorted = std::minmax(eGML.source, eGML.target);
		auto &e = eLabelsFromIds[eSorted];
		e.inLeft = true;
		e.string.left = eGML.label;
		e.stereo.left = eGML.stereo;
	}
	for(const GML::Edge &eGML: rule.context.edges) {
		auto eSorted = std::minmax(eGML.source, eGML.target);
		auto &e = eLabelsFromIds[eSorted];
		e.inContext = true;
		e.string.context = eGML.label;
		e.stereo.context = eGML.stereo;
	}
	for(const GML::Edge &eGML: rule.right.edges) {
		auto eSorted = std::minmax(eGML.source, eGML.target);
		auto &e = eLabelsFromIds[eSorted];
		e.inRight = true;
		e.string.right = eGML.label;
		e.stereo.right = eGML.stereo;
	}

	return std::pair(std::move(vLabelsFromId), std::move(eLabelsFromIds));
}

struct MatchConstraintConverter {
	MatchConstraintConverter(LabelledRule &dpoResult, const std::map<int, VertexLabels> &vLabelsFromId)
			: dpoResult(dpoResult), vLabelsFromId(vLabelsFromId) {}

	Result<> operator()(const GML::AdjacencyConstraint &cGML) {
		const auto iter = vLabelsFromId.find(cGML.id);
		if(iter == end(vLabelsFromId))
			return Result<>::Error("Error in rule GML. Vertex " + std::to_string(cGML.id) +
			                       " in adjacency constraint does not exist.");
		const auto vConstrained = iter->second.cgVertex;
		lib::GraphMorphism::Constraints::Operator op;
		{
			const auto &s = cGML.op;
			using namespace lib::GraphMorphism::Constraints;
			if(s == "<") op = Operator::LT;
			else if(s == "<=") op = Operator::LEQ;
			else if(s == "=") op = Operator::EQ;
			else if(s == ">=") op = Operator::GEQ;
			else if(s == ">") op = Operator::GT;
			else return Result<>::Error("Error in rule GML. Unknown operator '" + s + "' in adjacency constraint.");
		}
		auto c = std::make_unique<
				lib::GraphMorphism::Constraints::VertexAdjacency<LabelledRule::SideGraphType>
		>(vConstrained, op, cGML.count);
		c->vertexLabels.insert(cGML.nodeLabels.begin(), cGML.nodeLabels.end());
		c->edgeLabels.insert(cGML.edgeLabels.begin(), cGML.edgeLabels.end());
		dpoResult.leftData.matchConstraints.push_back(std::move(c));
		return Result<>();
	}

	Result<> operator()(const GML::ShortestPathConstraint &cGML) {
		const auto iterSrc = vLabelsFromId.find(cGML.source);
		const auto iterTar = vLabelsFromId.find(cGML.target);
		if(iterSrc == end(vLabelsFromId))
			return Result<>::Error("Error in rule GML. Vertex " + std::to_string(cGML.source) +
			                       " in shortest path constraint does not exist.");
		if(iterTar == end(vLabelsFromId))
			return Result<>::Error("Error in rule GML. Vertex " + std::to_string(cGML.target) +
			                       " in shortest path constraint does not exist.");
		const auto vSrc = iterSrc->second.cgVertex;
		const auto vTar = iterTar->second.cgVertex;
		lib::GraphMorphism::Constraints::Operator op;
		{
			const auto &s = cGML.op;
			using namespace lib::GraphMorphism::Constraints;
			if(s == "<") op = Operator::LT;
			else if(s == "<=") op = Operator::LEQ;
			else if(s == "=") op = Operator::EQ;
			else if(s == ">=") op = Operator::GEQ;
			else if(s == ">") op = Operator::GT;
			else
				return Result<>::Error(
						"Error in rule GML. Unknown operator '" + s + "' in shortest path constraint.");
		}
		const auto compSrc = get_component(get_labelled_left(dpoResult))[
				get(boost::vertex_index_t(), get_graph(dpoResult), vSrc)];
		const auto compTar = get_component(get_labelled_left(dpoResult))[
				get(boost::vertex_index_t(), get_graph(dpoResult), vTar)];
		if(compSrc != compTar)
			return Result<>::Error(
					"Error in rule GML. Vertex " + std::to_string(cGML.source) + " and " + std::to_string(cGML.target)
					+ " are in different connected components of the left graph. "
					+ "This is currently not supported for the shortest path constraint.");
		auto c = std::make_unique<
				lib::GraphMorphism::Constraints::ShortestPath<LabelledRule::SideGraphType>
		>(vSrc, vTar, op, cGML.length);
		dpoResult.leftData.matchConstraints.push_back(std::move(c));
		return Result<>();
	}
public:
	LabelledRule &dpoResult;
	const std::map<int, VertexLabels> &vLabelsFromId;
};

} // namespace

Result<Data> gml(lib::IO::Warnings &warnings, std::string_view input, bool printStereoWarnings) {
	auto resRule = parseGML(input);
	if(!resRule) return std::move(resRule); // TODO: remove std::move when C++20/P1825R0 is available
	auto &rule = *resRule;

	if(auto res = checkGraphDuplicatesAndLoops(rule.left, "left"); !res) return res;
	if(auto res = checkGraphDuplicatesAndLoops(rule.context, "context"); !res) return res;
	if(auto res = checkGraphDuplicatesAndLoops(rule.right, "right"); !res) return res;
	auto labelsFromIdPair = extractDataFromGML(rule); // TODO: (C++20) use structured binding directly
	auto &[vLabelsFromId, eLabelsFromIds] = labelsFromIdPair;

	Data data;
	data.rule = LabelledRule();
	data.name = rule.id;
	if(rule.labelType) {
		const std::string &ltString = *rule.labelType;
		if(ltString == "string") data.labelType = LabelType::String;
		else if(ltString == "term") data.labelType = LabelType::Term;
		else return Result<>::Error("Error in rule GML. Unknown labelType '" + ltString + "'.");
	}

	auto &dpoResult = *data.rule;
	auto &rDPO = dpoResult.getRule();
	dpoResult.pString = std::make_unique<PropString>(dpoResult.getRule());
	auto &pString = *dpoResult.pString;

	using CombinedVertex = DPO::CombinedRule::CombinedVertex;
	std::map<CombinedVertex, int> vIdFromCG;
	for(auto &[id, vData]: vLabelsFromId) {
		// First find the right membership:
		// inContext <=> inLeft && inRight
		if(vData.inContext) vData.inLeft = vData.inRight = true;
		else if(vData.inLeft && vData.inRight) vData.inContext = true;

		// Check labels and make (left, right) the correct labels
		if(vData.string.context) {
			if(vData.string.left)
				return Result<>::Error(
						"Error in rule GML. Vertex " + std::to_string(id) + " has a label both in 'context' and 'left'.");
			if(vData.string.right)
				return Result<>::Error(
						"Error in rule GML. Vertex " + std::to_string(id) + " has a label both in 'context' and 'right'.");
			// Note: terms follow the same semantics as string, i.e., the same string in L and R becomes the exact same terms.
			vData.string.left = vData.string.right = vData.string.context;
		}

		// Check that there is a string in left/right when inLeft/inRight
		if(vData.inLeft && !vData.string.left)
			return Result<>::Error("Error in rule GML. Vertex " + std::to_string(id) + " is in L, but has no label.");
		if(vData.inRight && !vData.string.right)
			return Result<>::Error("Error in rule GML. Vertex " + std::to_string(id) + " is in R, but has no label.");

		if(vData.inContext) {
			const auto vK = addVertexK(rDPO);
			pString.addK(vK, *vData.string.left, *vData.string.right);
			vData.cgVertex = get(rDPO.getKtoCG(), getK(rDPO), rDPO.getCombinedGraph(), vK);
		} else if(vData.inLeft) {
			assert(!vData.inRight);
			const auto vL = addVertexL(rDPO);
			pString.addL(vL, *vData.string.left);
			vData.cgVertex = get(rDPO.getLtoCG(), getL(rDPO), rDPO.getCombinedGraph(), vL);
		} else {
			assert(vData.inRight);
			const auto vR = addVertexR(rDPO);
			pString.addR(vR, *vData.string.right);
			vData.cgVertex = get(rDPO.getRtoCG(), getR(rDPO), rDPO.getCombinedGraph(), vR);
		}
		vIdFromCG[vData.cgVertex] = id;
		data.externalToInternalIds[id] = get(boost::vertex_index_t(), rDPO.getCombinedGraph(), vData.cgVertex);
	} // for each vertex

	for(auto &[eIds, eData]: eLabelsFromIds) {
		const auto [src, tar] = eIds;
		if(vLabelsFromId.find(src) == end(vLabelsFromId))
			return Result<>::Error(
					"Error in rule GML. Edge endpoint '" + std::to_string(src) + "' does not exist for edge ("
					+ std::to_string(src) + ", " + std::to_string(tar) + ").");
		if(vLabelsFromId.find(tar) == end(vLabelsFromId))
			return Result<>::Error(
					"Error in rule GML. Edge endpoint '" + std::to_string(tar) + "' does not exist for edge ("
					+ std::to_string(src) + ", " + std::to_string(tar) + ").");
		const CombinedVertex vcSrc = vLabelsFromId[src].cgVertex, vcTar = vLabelsFromId[tar].cgVertex;
		// First find the right membership:
		// inContext <=> inLeft && inRight
		if(eData.inContext) eData.inLeft = eData.inRight = true;
		else if(eData.inLeft && eData.inRight) eData.inContext = true;

		// checking dangling in left/right
		if(eData.inLeft) {
			const auto &g = rDPO.getCombinedGraph();
			if(g[vcSrc].membership == Membership::R)
				return Result<>::Error("Error in rule GML. Edge (" + std::to_string(src) + ", " + std::to_string(tar) +
				                       ") dangling: edge is present in L but endpoint " + std::to_string(src) +
				                       " only present in R.");
			if(g[vcTar].membership == Membership::R)
				return Result<>::Error("Error in rule GML. Edge (" + std::to_string(src) + ", " + std::to_string(tar) +
				                       ") dangling: edge is present in L but endpoint " + std::to_string(tar) +
				                       " only present in R.");
		}
		if(eData.inRight) {
			const auto &g = rDPO.getCombinedGraph();
			if(g[vcSrc].membership == Membership::L)
				return Result<>::Error("Error in rule GML. Edge (" + std::to_string(src) + ", " + std::to_string(tar) +
				                       ") dangling: edge is present in R but endpoint " + std::to_string(src) +
				                       " only present in L.");
			if(g[vcTar].membership == Membership::L)
				return Result<>::Error("Error in rule GML. Edge (" + std::to_string(src) + ", " + std::to_string(tar) +
				                       ") dangling: edge is present in R but endpoint " + std::to_string(tar) +
				                       " only present in L.");
		}

		// Check labels and make (left, right) the correct labels
		if(eData.string.context) {
			if(eData.string.left)
				return Result<>::Error("Error in rule GML. Edge (" + std::to_string(src) + ", " + std::to_string(tar) +
				                       ") has a label both in 'context' and 'left'.");
			if(eData.string.right)
				return Result<>::Error("Error in rule GML. Edge (" + std::to_string(src) + ", " + std::to_string(tar) +
				                       ") has a label both in 'context' and 'right'.");
			// TODO: for term it matters if it's L+R or it's K
			eData.string.left = eData.string.right = eData.string.context;
		}

		// Check that there is a string in left/right when inLeft/inRight
		if(eData.inLeft && !eData.string.left)
			return Result<>::Error("Error in rule GML. Edge (" + std::to_string(src) + ", " + std::to_string(tar) +
			                       ") is in L, but has no label.");
		if(eData.inRight && !eData.string.right)
			return Result<>::Error("Error in rule GML. Edge (" + std::to_string(src) + ", " + std::to_string(tar) +
			                       ") is in R, but has no label.");

		if(eData.inContext) {
			const auto eK = addEdgeK(rDPO, vcSrc, vcTar);
			pString.addK(eK, *eData.string.left, *eData.string.right);
			eData.cgEdge = get(rDPO.getKtoCG(), getK(rDPO), rDPO.getCombinedGraph(), eK);
		} else if(eData.inLeft) {
			assert(!eData.inRight);
			const auto vLSrc = get_inverse(rDPO.getLtoCG(), getL(rDPO), rDPO.getCombinedGraph(), vcSrc);
			const auto vLTar = get_inverse(rDPO.getLtoCG(), getL(rDPO), rDPO.getCombinedGraph(), vcTar);
			const auto eL = addEdgeL(rDPO, vLSrc, vLTar);
			pString.addL(eL, *eData.string.left);
			eData.cgEdge = get(rDPO.getLtoCG(), getL(rDPO), rDPO.getCombinedGraph(), eL);
		} else {
			assert(eData.inRight);
			const auto vRSrc = get_inverse(rDPO.getRtoCG(), getR(rDPO), rDPO.getCombinedGraph(), vcSrc);
			const auto vRTar = get_inverse(rDPO.getRtoCG(), getR(rDPO), rDPO.getCombinedGraph(), vcTar);
			const auto eR = addEdgeR(rDPO, vRSrc, vRTar);
			pString.addR(eR, *eData.string.right);
			eData.cgEdge = get(rDPO.getRtoCG(), getR(rDPO), rDPO.getCombinedGraph(), eR);
		}
	} // for each edge
	// the graph is set, so initialise the component storage
	dpoResult.initComponents();

	// constraints
	for(const GML::MatchConstraint &cGML: rule.matchConstraints) {
		MatchConstraintConverter visitor(dpoResult, vLabelsFromId);
		if(auto res = std::visit(visitor, cGML); !res) return res;
	} // for each constraint

	{ // perhaps we can stop now, if there is not stereo annotation
		bool doStereo = false;
		for(const auto &v: rule.left.vertices) doStereo = doStereo || v.stereo;
		for(const auto &v: rule.context.vertices) doStereo = doStereo || v.stereo;
		for(const auto &v: rule.right.vertices) doStereo = doStereo || v.stereo;
		for(const auto &e: rule.left.edges) doStereo = doStereo || e.stereo;
		for(const auto &e: rule.context.edges) doStereo = doStereo || e.stereo;
		for(const auto &e: rule.right.edges) doStereo = doStereo || e.stereo;
		if(!doStereo) return std::move(data); // TODO: remove std::move when C++20/P1825R0 is available
	}

	// =========================================================================================
	// Stereo
	// =========================================================================================

	for(auto &[id, vData]: vLabelsFromId) {
		// Check labels and make (left, right) the correct labels
		if(vData.stereo.context) {
			if(vData.stereo.left)
				return Result<>::Error(
						"Error in rule GML. Vertex " + std::to_string(id) + " has stereo both in 'context' and 'left'.");
			if(vData.stereo.right)
				return Result<>::Error(
						"Error in rule GML. Vertex " + std::to_string(id) + " has stereo both in 'context' and 'right'.");
			vData.stereo.left = vData.stereo.right = vData.stereo.context;
		}
	}

	for(auto &[eIds, eData]: eLabelsFromIds) {
		const auto [src, tar] = eIds;
		if(eData.stereo.context) {
			if(eData.stereo.left)
				return Result<>::Error("Error in rule GML. Edge (" + std::to_string(src) + ", " + std::to_string(tar) +
				                       ") has stereo both in 'context' and 'left'.");
			if(eData.stereo.right)
				return Result<>::Error("Error in rule GML. Edge (" + std::to_string(src) + ", " + std::to_string(tar) +
				                       ") has stereo both in 'context' and 'right'.");
			eData.stereo.left = eData.stereo.right = eData.stereo.context;
		}
	}

	PropMolecule mol(dpoResult.getRule(), pString); // temporary for doing the inference
	auto molLeft = mol.getLeft();
	auto molRight = mol.getRight();
	auto leftInference = lib::Stereo::Inference(getL(rDPO), molLeft, true);
	auto rightInference = lib::Stereo::Inference(getR(rDPO), molRight, true);
	// Set the explicitly defined edge categories.
	//----------------------------------------------------------------------------
	for(const auto &eIdLabelPair: eLabelsFromIds) {
		const auto &[eIds, labels] = eIdLabelPair;
		const auto handleSide = [&eIdLabelPair, &rDPO](
				const std::optional<std::string> &os, const std::string &side,
				auto &inference, const auto &mSideToCG, const auto &gSide) -> Result<> {
			// TODO: (C++20) use structured binding in loop and capture them
			const auto &[eIds, labels] = eIdLabelPair;
			if(!os) return Result<>();
			const std::string &s = *os;
			if(s.size() != 1)
				return Result<>::Error("Error in stereo data for edge (" + std::to_string(eIds.first)
				                       + ", " + std::to_string(eIds.second) + ") in " + side
				                       + ". Parsing error in stereo data '" + s + "'.");
			lib::Stereo::EdgeCategory cat;
			switch(s.front()) {
			case '*':
				cat = lib::Stereo::EdgeCategory::Any;
				break;
			default:
				return Result<>::Error("Error in stereo data for edge (" + std::to_string(eIds.first) + ", "
				                       + std::to_string(eIds.second) + ") in " + side
				                       + ". Parsing error in stereo data '" + s + "'.");
			}
			const auto eSide = get_inverse(mSideToCG, gSide, rDPO.getCombinedGraph(), labels.cgEdge);
			auto res = inference.assignEdgeCategory(eSide, cat);
			if(!res) {
				res.setError("Error in stereo data for edge (" + std::to_string(eIds.first) + ", "
				             + std::to_string(eIds.second) + ") in " + side + ". " + res.extractError());
				return res;
			}
			return res;
		};
		if(auto res = handleSide(labels.stereo.left, "L", leftInference, rDPO.getLtoCG(), getL(rDPO)); !res) return res;
		if(auto res = handleSide(labels.stereo.right, "R", rightInference, rDPO.getRtoCG(), getR(rDPO)); !res) return res;
	} // for each edge
	// Set the explicitly defined vertex stereo data.
	//----------------------------------------------------------------------------
	const auto &gGeometry = lib::Stereo::getGeometryGraph();
	for(auto &eVIdLabelsPair: vLabelsFromId) {
		auto &[vId, vLabels] = eVIdLabelsPair; // TODO: (C++20) remove, structured binding capture
		// TODO: (C++20) use structured binding in loop
		const auto handleSide = [&](const std::optional<std::string> &os, const std::string &side, auto &inference,
		                            auto &parsedEmbedding, const auto &mSideToCG, const auto &gSide) {
			auto &[vLabelsFromId, eLabelsFromIds] = labelsFromIdPair; // TODO: (C++20) remove, structured binding capture
			const auto &[vId, vLabels] = eVIdLabelsPair;
			if(!os) return Result<>();
			if(auto parsedEmbeddingRes = lib::Stereo::Read::parseEmbedding(*os)) {
				parsedEmbedding = std::move(*parsedEmbeddingRes);
			} else {
				return Result<>::Error(
						"Error in stereo data for vertex " + std::to_string(vId) + " in " + side + ". " +
						parsedEmbeddingRes.extractError());
			}
			const auto &vcg = vLabels.cgVertex;
			const auto vSide = get_inverse(mSideToCG, gSide, rDPO.getCombinedGraph(), vcg);
			// Geometry
			//..........................................................................
			const auto &embGML = *parsedEmbedding;
			if(embGML.geometry) {
				const auto vGeo = gGeometry.findGeometry(*embGML.geometry);
				if(vGeo == gGeometry.nullGeometry())
					return Result<>::Error("Error in stereo data for vertex " + std::to_string(vId) + " in " + side +
					                       ". Invalid geometry '" + *embGML.geometry + "'.");
				if(auto res = inference.assignGeometry(vSide, vGeo); !res) {
					return Result<>::Error(
							"Error in stereo data for vertex " + std::to_string(vId) + " in " + side + ". " +
							res.extractError());
				}
			}
			// Edges
			//..........................................................................
			if(embGML.edges) {
				inference.initEmbedding(vSide);
				for(const auto &e: *embGML.edges) {
					if(const int *idPtr = std::get_if<int>(&e)) {
						const int idNeighbour = *idPtr;
						if(vLabelsFromId.find(idNeighbour) == end(vLabelsFromId))
							return Result<>::Error("Error in graph GML. Neighbour vertex " + std::to_string(idNeighbour) +
							                       " in stereo embedding for vertex "
							                       + std::to_string(vId) + " in " + side + " does not exist.");
						const auto vFromVertexId = [&labelsFromIdPair](int id) {
							auto &[vLabelsFromId, eLabelsFromIds] = labelsFromIdPair; // TODO: (C++20) remove, structured binding capture
							const auto iter = vLabelsFromId.find(id);
							assert(iter != end(vLabelsFromId));
							return iter->second.cgVertex;
						};
						auto epCG = edge(vcg, vFromVertexId(idNeighbour), rDPO.getCombinedGraph());
						if(!epCG.second)
							return Result<>::Error("Error in graph GML. Vertex " + std::to_string(idNeighbour) +
							                       " in stereo embedding for vertex "
							                       + std::to_string(vId) + " in " + side + " is not a neighbour.");
						const auto eSide = get_inverse(mSideToCG, gSide, rDPO.getCombinedGraph(), epCG.first);
						inference.addEdge(vSide, eSide);
					} else if(const char *virtPtr = std::get_if<char>(&e)) {
						switch(*virtPtr) {
						case 'e':
							inference.addLonePair(vSide);
							break;
						case 'r':
							inference.addRadical(vSide);
							break;
						default:
							return Result<>::Error(
									"Error in graph GML. Virtual neighbour in stereo embedding for vertex " +
									std::to_string(vId) + " in " + side + " has unknown type '" + *virtPtr + "'.");
						}
					} else {
						MOD_ABORT; // the parser should know what is allowed
					}
				}
			}
			// Fixation
			//..........................................................................
			if(embGML.fixation) {
				// TODO: expand this when more complicated geometries are implemented
				const bool isFixed = *embGML.fixation;
				if(isFixed) inference.fixSimpleGeometry(vSide);
			}
			return Result<>();
		};
		if(auto res = handleSide(vLabels.stereo.left, "L", leftInference, vLabels.parsedEmbeddingLeft,
		                         rDPO.getLtoCG(), getL(rDPO)); !res)
			return res;
		if(auto res = handleSide(vLabels.stereo.right, "R", rightInference, vLabels.parsedEmbeddingRight,
		                         rDPO.getRtoCG(), getR(rDPO)); !res)
			return res;
	} // for each vertex

	const auto finalize = [&warnings, printStereoWarnings, &rDPO, &vIdFromCG](
			auto &inference, const std::string &side, const auto &gSide, const auto &mSideToCG) {
		return inference.finalize(warnings, printStereoWarnings,
		                          [&rDPO, &vIdFromCG, &side, &gSide, &mSideToCG](lib::DPO::CombinedRule::SideVertex vS) {
			                          const auto v = get(mSideToCG, gSide, rDPO.getCombinedGraph(), vS);
			                          const auto iter = vIdFromCG.find(v);
			                          assert(iter != vIdFromCG.end());
			                          return std::to_string(iter->second) + " in " + side;
		                          });
	};
	if(auto resLeft = finalize(leftInference, "L", getL(rDPO), rDPO.getLtoCG()); !resLeft) return resLeft;
	if(auto resRight = finalize(rightInference, "R", getR(rDPO), rDPO.getRtoCG()); !resRight) return resRight;

	const auto vertexInContext = [&](lib::DPO::CombinedRule::CombinedVertex v) -> bool {
		auto &[vLabelsFromId, eLabelsFromIds] = labelsFromIdPair; // TODO: (C++20) remove, structured binding capture
		const auto idIter = vIdFromCG.find(v);
		assert(idIter != end(vIdFromCG));
		const auto lIter = vLabelsFromId.find(idIter->second);
		assert(lIter != end(vLabelsFromId));
		assert(lIter->second.inContext);
		const auto &stereo = lIter->second.stereo;
		// TODO: this should be looked at at some point
		// if there is any stereo data, maybe we are in context
		if(stereo.left.has_value() || stereo.context.has_value() || stereo.right.has_value())
			return stereo.context.has_value();
		else // otherwise, default to be in context
			return true;
	};

	const auto edgeInContext = [&](lib::DPO::CombinedRule::CombinedEdge e) -> bool {
		auto &[vLabelsFromId, eLabelsFromIds] = labelsFromIdPair; // TODO: (C++20) remove, structured binding capture
		const auto vSrc = source(e, rDPO.getCombinedGraph());
		const auto vTar = target(e, rDPO.getCombinedGraph());
		const auto idSrcIter = vIdFromCG.find(vSrc);
		const auto idTarIter = vIdFromCG.find(vTar);
		assert(idSrcIter != end(vIdFromCG));
		assert(idTarIter != end(vIdFromCG));
		assert(idSrcIter->second < idTarIter->second);
		const auto lIter = eLabelsFromIds.find(std::pair(idSrcIter->second, idTarIter->second));
		assert(lIter != end(eLabelsFromIds));
		assert(lIter->second.inContext);
		const auto &stereo = lIter->second.stereo;
		// TODO: this should be looked at at some point
		// if there is any stereo data, maybe we are in context
		if(stereo.left.has_value() || stereo.context.has_value() || stereo.right.has_value())
			return stereo.context.has_value();
		else // otherwise, default to be in context
			return true;
	};
	dpoResult.pStereo = std::make_unique<PropStereo>(dpoResult.getRule(),
	                                                 std::move(leftInference), std::move(rightInference),
	                                                 vertexInContext, edgeInContext);
	return std::move(data); // TODO: remove std::move when C++20/P1825R0 is available
}

namespace {
//#define MOD_RULEDFS_DEBUG

using CombinedVertex = DPO::CombinedRule::CombinedVertex;

namespace dfsDetail {
using namespace IO::DFS;
using Vertex = IO::DFS::Vertex;
using Edge = IO::DFS::Edge;

using LeftEdgeMapType = std::map<std::pair<CombinedVertex, CombinedVertex>, std::string>;

struct ConvertLeft {
	struct ConvertRes {
		CombinedVertex next;
		bool isRingClosure;
	};

	ConvertLeft(const IO::DFS::Read::RuleResult &res, DPO::CombinedRule &rDPO, PropString &pString)
			: res(res), rDPO(rDPO), pString(pString) {}

	LeftEdgeMapType operator()(Chain &chain) &&{
		const auto sub = (*this)(chain.head, lib::Graph::GraphType::null_vertex());
		CombinedVertex vPrev = sub.next;
		assert(!sub.isRingClosure);
		assert(vPrev != lib::Graph::GraphType::null_vertex());
		for(EVPair &ev: chain.tail) {
			vPrev = (*this)(ev, vPrev);
			assert(vPrev != lib::Graph::GraphType::null_vertex());
		}
		return std::move(edges);
	}

	ConvertRes operator()(Vertex &vertex, CombinedVertex prev) {
		const auto sub = boost::apply_visitor(*this, vertex.vertex);
		assert(!(sub.isRingClosure && prev == lib::Graph::GraphType::null_vertex()));
		const CombinedVertex branchRoot = sub.isRingClosure ? prev : sub.next;
		for(Branch &branch: vertex.branches) {
			CombinedVertex branchPrev = branchRoot;
			for(EVPair &ev: branch.tail) {
				branchPrev = (*this)(ev, branchPrev);
				assert(branchPrev != lib::Graph::GraphType::null_vertex());
			}
		}
		return sub;
	}

	CombinedVertex operator()(EVPair &ev, CombinedVertex prev) {
		const auto res = (*this)(ev.second, prev);
		makeEdge(prev, res.next, ev.first.label);
		if(res.isRingClosure) return prev;
		else return res.next;
	}

	ConvertRes operator()(LabelVertex &vDFS) {
		const auto vL = addVertexL(rDPO);
		pString.addL(vL, vDFS.label);
		const auto v = get(rDPO.getLtoCG(), getL(rDPO), rDPO.getCombinedGraph(), vL);

#ifdef MOD_RULEDFS_DEBUG
		std::cout << "RuleDFS: ConvertLeft, addVertexL(";
		pString.print(std::cout, v);
		std::cout << ")\n";
#endif

		const auto vId = get(boost::vertex_index_t(), rDPO.getCombinedGraph(), v);
		vDFS.gVertexId = vId;

		std::optional<std::string> rightLabel;
		if(vDFS.id && !vDFS.ringClosure) {
			const auto iterRight = res.right.vertexFromId.find(*vDFS.id);
			if(iterRight != res.right.vertexFromId.end()) {
				rightLabel = iterRight->second->label;
				assert(iterRight->second->gVertexId == -1);
				iterRight->second->gVertexId = vId;
			}
		}
		if(rightLabel) {
			const auto vR = promoteVertexL(rDPO, vL);
			pString.promoteL(vL, vR, *rightLabel);
		}
		if(vDFS.ringClosure) {
			const auto vRing = vertex(vDFS.ringClosure->gVertexId, rDPO.getCombinedGraph());
			makeEdge(v, vRing, "-");
		}
		return {v, false};
	}

	ConvertRes operator()(RingClosure &rc) {
		return {vertex(rc.other->gVertexId, rDPO.getCombinedGraph()), true};
	}
private:
	void makeEdge(CombinedVertex vSrc, CombinedVertex vTar, const std::string &label) {
		if(label.empty()) return; // a dot edge for no-edge
		std::pair<CombinedVertex, CombinedVertex> eSorted = std::minmax(vSrc, vTar);
		std::tie(vSrc, vTar) = eSorted;
		assert(edges.find({vSrc, vTar}) == edges.end());
		edges[{vSrc, vTar}] = label;
	}
public:
	const IO::DFS::Read::RuleResult &res;
	DPO::CombinedRule &rDPO;
	PropString &pString;
	LeftEdgeMapType edges;
};

struct ConvertRight {
	struct ConvertRes {
		CombinedVertex next;
		bool isRingClosure;
	};

	ConvertRight(const IO::DFS::Read::RuleResult &res, DPO::CombinedRule &rDPO, PropString &pString,
	             LeftEdgeMapType &leftEdges)
			: res(res), rDPO(rDPO), pString(pString), leftEdges(leftEdges) {}

	void operator()(Chain &chain) &&{
		const auto sub = (*this)(chain.head, lib::Graph::GraphType::null_vertex());
		CombinedVertex vPrev = sub.next;
		assert(!sub.isRingClosure);
		assert(vPrev != lib::Graph::GraphType::null_vertex());
		for(EVPair &ev: chain.tail) {
			vPrev = (*this)(ev, vPrev);
			assert(vPrev != lib::Graph::GraphType::null_vertex());
		}
	}

	ConvertRes operator()(Vertex &vertex, CombinedVertex prev) {
		const auto sub = boost::apply_visitor(*this, vertex.vertex);
		assert(!(sub.isRingClosure && prev == lib::Graph::GraphType::null_vertex()));
		const CombinedVertex branchRoot = sub.isRingClosure ? prev : sub.next;
		for(Branch &branch: vertex.branches) {
			CombinedVertex branchPrev = branchRoot;
			for(EVPair &ev: branch.tail) {
				branchPrev = (*this)(ev, branchPrev);
				assert(branchPrev != lib::Graph::GraphType::null_vertex());
			}
		}
		return sub;
	}

	CombinedVertex operator()(EVPair &ev, CombinedVertex prev) {
		const auto res = (*this)(ev.second, prev);
		makeEdge(prev, res.next, ev.first.label);
		if(res.isRingClosure) return prev;
		else return res.next;
	}

	ConvertRes operator()(LabelVertex &vDFS) {
		// ConvertLeft will set our gVertexId if it is a context vertex
		CombinedVertex v;
		if(vDFS.gVertexId == -1) {
			assert(!vDFS.id || vDFS.ringClosure || res.left.vertexFromId.find(*vDFS.id) == res.left.vertexFromId.end());
			const auto vR = addVertexR(rDPO);
			v = get(rDPO.getRtoCG(), getR(rDPO), rDPO.getCombinedGraph(), vR);
			const auto vId = get(boost::vertex_index_t(), rDPO.getCombinedGraph(), v);
			vDFS.gVertexId = vId;
			pString.addR(vR, vDFS.label);
#ifdef MOD_RULEDFS_DEBUG
			std::cout << "RuleDFS: ConvertRight, add_vertex(";
			pString.print(std::cout, v);
			std::cout << ")\n";
#endif
		} else {
			assert(vDFS.id);
			assert(!vDFS.ringClosure);
			assert(res.left.vertexFromId.find(*vDFS.id) != res.left.vertexFromId.end());
			v = vertex(vDFS.gVertexId, rDPO.getCombinedGraph());
		}
		if(vDFS.ringClosure) {
			const auto vRing = vertex(vDFS.ringClosure->gVertexId, rDPO.getCombinedGraph());
			makeEdge(v, vRing, "-");
		}
		return {v, false};
	}

	ConvertRes operator()(RingClosure &rc) {
		return {vertex(rc.other->gVertexId, rDPO.getCombinedGraph()), true};
	}
private:
	void makeEdge(CombinedVertex vSrc, CombinedVertex vTar, const std::string &label) {
		if(label.empty()) return; // a dot edge for no-edge
		std::pair<CombinedVertex, CombinedVertex> eSorted = std::minmax(vSrc, vTar);
		std::tie(vSrc, vTar) = eSorted;
		const auto iterLeft = leftEdges.find({vSrc, vTar});
		if(iterLeft != leftEdges.end()) {
			const auto eK = addEdgeK(rDPO, vSrc, vTar);
			pString.addK(eK, iterLeft->second, label);
			leftEdges.erase(iterLeft);
#ifdef MOD_RULEDFS_DEBUG
			std::cout << "RuleDFS: ConvertRight, addEdgeK(";
			pString.print(std::cout, eK);
			std::cout << ")\n";
#endif
		} else {
			const auto vRSrc = get_inverse(rDPO.getRtoCG(), getR(rDPO), rDPO.getCombinedGraph(), vSrc);
			const auto vRTar = get_inverse(rDPO.getRtoCG(), getR(rDPO), rDPO.getCombinedGraph(), vTar);
			const auto eR = addEdgeR(rDPO, vRSrc, vRTar);
			pString.addR(eR, label);
#ifdef MOD_RULEDFS_DEBUG
			std::cout << "RuleDFS: ConvertRight, addEdgeR(";
			pString.print(std::cout, get(rDPO.getRtoCG(), getR(rDPO), rDPO.getCombinedGraph(), eR));
			std::cout << ")\n";
#endif
		}
#ifdef MOD_RULEDFS_DEBUG
		pString.print(std::cout << "\tsrc=", vSrc);
		std::cout << '\n';
		pString.print(std::cout << "\ttar=", vTar);
		std::cout << '\n';
		std::cout << std::flush;
#endif
	}
public:
	const IO::DFS::Read::RuleResult &res;
	DPO::CombinedRule &rDPO;
	PropString &pString;
	LeftEdgeMapType &leftEdges;
};

struct ImplicitLeft {
	ImplicitLeft(const IO::DFS::Read::RuleResult &res) : res(res) {}

	Result<> operator()(const Chain &chain) &&{
		auto sub = (*this)(chain.head);
		if(!sub) return sub;
		for(const EVPair &ev: chain.tail) {
			auto sub = (*this)(ev.second);
			if(!sub) return sub;
		}
		return {};
	}

	Result<> operator()(const Vertex &vertex) {
		auto sub = boost::apply_visitor(*this, vertex.vertex);
		if(!sub) return sub;
		for(const Branch &branch: vertex.branches) {
			for(const EVPair &ev: branch.tail) {
				auto sub = (*this)(ev.second);
				if(!sub) return sub;
			}
		}
		return {};
	}

	Result<> operator()(const LabelVertex &vDFS) {
		if(!vDFS.implicit) return {};
		return Result<>::Error("Vertices with implicit hydrogen atoms currently not supported.");
	}

	Result<> operator()(const RingClosure &rc) {
		return {};
	}
public:
	const IO::DFS::Read::RuleResult &res;
};

struct ImplicitRight {
	ImplicitRight(const IO::DFS::Read::RuleResult &res) : res(res) {}

	Result<> operator()(const Chain &chain) &&{
		auto sub = (*this)(chain.head);
		if(!sub) return sub;
		for(const EVPair &ev: chain.tail) {
			auto sub = (*this)(ev.second);
			if(!sub) return sub;
		}
		return {};
	}

	Result<> operator()(const Vertex &vertex) {
		auto sub = boost::apply_visitor(*this, vertex.vertex);
		if(!sub) return sub;
		for(const Branch &branch: vertex.branches) {
			for(const EVPair &ev: branch.tail) {
				auto sub = (*this)(ev.second);
				if(!sub) return sub;
			}
		}
		return {};
	}

	Result<> operator()(const LabelVertex &vDFS) {
		if(!vDFS.implicit) return {};
		return Result<>::Error("Vertices with implicit hydrogen atoms currently not supported.");
	}

	Result<> operator()(const RingClosure &rc) {
		return {};
	}
public:
	const IO::DFS::Read::RuleResult &res;
};

} // namespace dfsDetail
} // namespace

Result<Data> dfs(lib::IO::Warnings &warnings, std::string_view input) {
	auto astRes = lib::IO::DFS::Read::rule(input);
	if(!astRes) return lib::IO::Result<>::Error(astRes.extractError());

	Data data;
	auto rDPO = std::make_unique<lib::DPO::CombinedRule>();
	auto pString = std::make_unique<PropString>(*rDPO);

	std::map<std::pair<CombinedVertex, CombinedVertex>, std::string> leftEdges;
	if(astRes->left.ast)
		leftEdges = dfsDetail::ConvertLeft(*astRes, *rDPO, *pString)(*astRes->left.ast);
	if(astRes->right.ast)
		dfsDetail::ConvertRight(*astRes, *rDPO, *pString, leftEdges)(*astRes->right.ast);
	// add remaining leftEdges
	for(const auto &[ep, label]: leftEdges) {
		const auto vL1 = get_inverse(rDPO->getLtoCG(), getL(*rDPO), rDPO->getCombinedGraph(), ep.first);
		const auto vL2 = get_inverse(rDPO->getLtoCG(), getL(*rDPO), rDPO->getCombinedGraph(), ep.second);
		const auto eL = addEdgeL(*rDPO, vL1, vL2);
		pString->addL(eL, label);
#ifdef MOD_RULEDFS_DEBUG
		std::cout << "RuleDFS: remaining leftEdges, add_edge(";
		pString->print(std::cout, get(rDPO->getLtoCG(), getL(*rDPO), rDPO->getCombinedGraph(), eL));
		std::cout << ")\n";
		pString->print(std::cout << "\tsrc=", ep.first);
		std::cout << '\n';
		pString->print(std::cout << "\ttar=", ep.second);
		std::cout << '\n';
		std::cout << std::flush;
#endif
	}

	if(astRes->left.ast)
		if(auto res = dfsDetail::ImplicitLeft(*astRes)(*astRes->left.ast); !res)
			return res;
	if(astRes->right.ast)
		if(auto res = dfsDetail::ImplicitRight(*astRes)(*astRes->right.ast); !res)
			return res;

	if(astRes->left.ast) {
		for(const auto [id, vDFS]: astRes->left.vertexFromId) {
			assert(vDFS->gVertexId != -1);
			data.externalToInternalIds[id] = vDFS->gVertexId;
		}
	}
	if(astRes->right.ast) {
		for(const auto [id, vDFS]: astRes->right.vertexFromId) {
			assert(vDFS->gVertexId != -1);
			if(auto iter = data.externalToInternalIds.find(id); iter == data.externalToInternalIds.end()) {
				data.externalToInternalIds[id] = vDFS->gVertexId;
			} else {
				assert(iter->second == vDFS->gVertexId);
			}
		}
	}

	data.rule.emplace(std::move(rDPO), std::move(pString), nullptr);
	return std::move(data);  // TODO: remove std::move when C++20/P1825R0 is available
}

} // namespace mod::lib::Rules::Read