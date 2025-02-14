#include "Write.hpp"

#include <mod/Post.hpp>
#include <mod/graph/Graph.hpp>
#include <mod/rule/Rule.hpp>
#include <mod/lib/DG/Dump.hpp>
#include <mod/lib/DG/Hyper.hpp>
#include <mod/lib/DG/IO/WriteDetail.hpp>
#include <mod/lib/Graph/MultisetIO.hpp>
#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/Graph/IO/Write.hpp>
#include <mod/lib/GraphMorphism/VF2Finder.hpp>
#include <mod/lib/IO/Config.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/IO/Json.hpp>
#include <mod/lib/Rule/Rule.hpp>
#include <mod/lib/Rule/IO/Write.hpp>

#include <boost/dynamic_bitset.hpp>
#include <boost/lexical_cast.hpp>

#include <fstream>

namespace mod::lib::DG::Write {

nlohmann::json dumpToJson(const NonHyper &dgNonHyper) {
	if(dgNonHyper.getLabelSettings().withStereo)
		throw mod::LogicError("Can not yet dump DGs with stereo data.");

	using VertexKind = HyperVertexKind;
	const auto &dgHyper = dgNonHyper.getHyper();
	const auto &dg = dgHyper.getGraph();

	nlohmann::json j;
	j["version"] = 3;
	j["labelSettings"] = dgNonHyper.getLabelSettings();

	auto jVertices = nlohmann::json::array();
	for(const auto v: asRange(vertices(dg))) {
		if(dg[v].kind != VertexKind::Vertex) continue;
		const auto id = get(boost::vertex_index_t(), dg, v);
		const lib::graph::Graph *g = dg[v].graph;
		assert(g);
		nlohmann::json vertex;
		vertex.push_back(id);
		vertex.push_back(g->getName());
		std::ostringstream ss;
		lib::graph::Write::gml(*g, false, ss);
		vertex.push_back(ss.str());
		jVertices.push_back(std::move(vertex));
	}
	j["vertices"] = std::move(jVertices);


	std::set<const lib::rule::Rule *, lib::rule::LessById> rules;
	for(const auto v: asRange(vertices(dg))) {
		if(dg[v].kind != VertexKind::Edge) continue;
		for(const auto *r: dgHyper.getRulesFromEdge(v))
			rules.insert(r);
	}
	auto jRules = nlohmann::json::array();
	std::unordered_map<const lib::rule::Rule *, int> idFromRule;
	int rId = 0;
	for(const auto *r: rules) {
		std::ostringstream ss;
		rule::Write::gml(*r, false, ss);
		jRules.push_back(ss.str());
		idFromRule.emplace(r, rId);
		++rId;
	}
	j["rules"] = std::move(jRules);

	auto jEdges = nlohmann::json::array();
	for(const auto v: asRange(vertices(dg))) {
		if(dg[v].kind != VertexKind::Edge) continue;
		const auto id = get(boost::vertex_index_t(), dg, v);
		auto jEdge = nlohmann::json::array();
		jEdge.push_back(id);
		auto jSources = nlohmann::json::array();
		for(const auto e: asRange(in_edges(v, dg))) {
			const auto v = source(e, dg);
			const auto id = get(boost::vertex_index_t(), dg, v);
			jSources.push_back(id);
		}
		jEdge.push_back(std::move(jSources));
		auto jTargets = nlohmann::json::array();
		for(const auto e: asRange(out_edges(v, dg))) {
			const auto v = target(e, dg);
			const auto id = get(boost::vertex_index_t(), dg, v);
			jTargets.push_back(id);
		}
		jEdge.push_back(std::move(jTargets));
		auto jRules = nlohmann::json::array();
		for(const auto *r: dgHyper.getRulesFromEdge(v)) {
			const auto iter = idFromRule.find(r);
			assert(iter != end(idFromRule));
			jRules.push_back(iter->second);
		}
		jEdge.push_back(std::move(jRules));

		jEdges.push_back(std::move(jEdge));
	}
	j["edges"] = std::move(jEdges);
	return j;
}

std::string dotNonHyper(const NonHyper &nonHyper) {
	post::FileHandle s(
			IO::makeUniqueFilePrefix() + "dgNonHyper_" + boost::lexical_cast<std::string>(nonHyper.getId()) + ".dot");
	{ // printing
		using Vertex = NonHyperVertex;
		using Edge = NonHyperEdge;
		const NonHyperGraphType &g = nonHyper.getGraph();
		s << "digraph g {" << std::endl;

		for(const auto v: asRange(vertices(g))) {
			const auto id = get(boost::vertex_index_t(), g, v);
			s << id << " [ label=\"" << g[v].graphs << "\" ];\n";
		}

		for(const auto e: asRange(edges(g))) {
			const auto srcId = get(boost::vertex_index_t(), g, source(e, g));
			const auto tarId = get(boost::vertex_index_t(), g, target(e, g));
			s << srcId << " -> " << tarId << " [ label=\"";
			bool first = true;
			for(auto *r: g[e].rules) {
				if(!first) s << ", ";
				first = false;
				s << "r_" << r->getId();
			}
			s << "\" ];\n";
		}
		s << "}\n";
	}
	std::string fileNoExt = s;
	fileNoExt.erase(fileNoExt.end() - 4, fileNoExt.end());
	return fileNoExt;
}

std::string pdfNonHyper(const NonHyper &nonHyper) {
	std::string fileNoExt = dotNonHyper(nonHyper);
	IO::post() << "gv dgNonHyper \"" << fileNoExt << "\" pdf" << std::endl;
	return fileNoExt;
}

//------------------------------------------------------------------------------
// Old/New delimiter
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// TikzPrinter
//------------------------------------------------------------------------------

void TikzPrinter::begin() {
	s << "\\begin{tikzpicture}[remember picture, " << options.tikzpictureOption << "]\n";
	s << "\\input{\\modInputPrefix/" << coords << "}\n";
}

void TikzPrinter::end() {
	s << "\\end{tikzpicture}%\n";
}

void TikzPrinter::comment(const std::string &str) {
	s << "% " << str << "\n";
}

void TikzPrinter::vertex(const std::string &id,
						 const std::string &label,
						 const std::string &image,
						 const std::string &colour) {
	bool haveLabel = label != "";
	bool haveImage = image != "";
	s << "\\node[modStyleDGHyperVertex, at=(v-coord-" << id << ")";
	if(!colour.empty()) s << ", draw=" << colour;
	s << "] (v-" << id << ") {";
	if(haveImage) {
		if(options.withInlineGraphs) {
			s << "{\\input{\\modInputPrefix/" << image << "}}";
		} else {
			s << "\\includegraphics[scale=\\modDGHyperImageScale] {\\modInputPrefix/" << image << "}";
		}
	}
	if(haveImage && haveLabel) s << "\\\\";
	if(haveLabel) {
		s << "{";
		if(options.labelsAsLatexMath) s << IO::asLatexMath(label);
		else s << IO::escapeForLatex(label);
		s << "}";
	}
	s << "};\n";
}

void TikzPrinter::vertexHidden(const std::string &id, bool large) {
	s << "\\node[modStyleDGHyperVertexHidden";
	if(large) s << "Large";
	s << ", at=(v-coord-" << id << ")] (v-" << id << ") {};\n";
}

void TikzPrinter::transitVertex(const std::string &idHost,
								const std::string &idTransit,
								const std::string &angle,
								const std::string &label) {
	s << "\\node[modStyleDGTransitVertex, at=(v-" << idHost << "." << angle << ")";
	if(!label.empty()) s << ", label=" << angle << ":{" << label << "}";
	s << "] (v-" << idTransit << ") {};\n";
}

void TikzPrinter::hyperEdge(const std::string &id, const std::string &label, const std::string &colour) {
	s << "\\node[modStyleDGHyperEdge, at=(v-coord-" << id << ")";
	if(!colour.empty()) s << ", draw=" << colour;
	s << "] (v-" << id << ") {";
	if(label != "") {
		if(options.labelsAsLatexMath) s << IO::asLatexMath(label);
		else s << IO::escapeForLatex(label);
	}
	s << "};\n";
}

void TikzPrinter::connector(const std::string &idTail,
							const std::string &idHead,
							const std::string &colour,
							int num, int maxNum) {
	if(maxNum > 1) {
		s << "\\pgfmathparse{";
		int position = num - maxNum / 2 - 1;
		s << position << "*\\modDGHyperConnectorBend";
		if(maxNum % 2 == 0) s << " + 0.5*\\modDGHyperConnectorBend";
		s << "}\n\\pgfmathsetmacro{\\modDGHyperConnectorResultAngle}{\\pgfmathresult}\n";
	}
	s << "\\path[modStyleDGHyperConnector";
	if(!colour.empty()) s << ", draw=" << colour;
	s << "] (v-" << idTail << ") to";
	if(maxNum > 1) s << "[bend right=\\modDGHyperConnectorResultAngle]";
	s << " (v-" << idHead << ");\n";
}

void TikzPrinter::tailConnector(const std::string &idVertex,
								const std::string &idHyperEdge,
								const std::string &colour,
								int num, int maxNum) {
	connector(idVertex, idHyperEdge, colour, num, maxNum);
}

void TikzPrinter::headConnector(const std::string &idHyperEdge,
								const std::string &idVertex,
								const std::string &colour,
								int num, int maxNum) {
	connector(idHyperEdge, idVertex, colour, num, maxNum);
}

void TikzPrinter::shortcutEdge(const std::string &idTail,
							   const std::string &idHead,
							   const std::string &label,
							   const std::string &colour,
							   bool hasReverse) {
	s << "\\path[modStyleDGHyperConnector";
	if(!colour.empty()) s << ", draw=" << colour;
	s << "] (v-" << idTail << ") to";
	if(hasReverse) s << "[modStyleDGHyperHasReverseShortcut]";
	s << " node[auto, swap] {";
	if(label != "") {
		if(options.labelsAsLatexMath) s << IO::asLatexMath(label);
		else s << IO::escapeForLatex(label);
	}
	s << "} (v-" << idHead << ");\n";
}

void TikzPrinter::transitEdge(const std::string &idTail,
							  const std::string &idHead,
							  const std::string &label,
							  const std::string &colour) {
	s << "\\path[modStyleDGHyperConnector";
	if(!colour.empty()) s << ", draw=" << colour;
	s << "] (v-" << idTail << ") to";
	s << " node[auto, swap] {";
	if(label != "") {
		if(options.labelsAsLatexMath) s << IO::asLatexMath(label);
		else s << IO::escapeForLatex(label);
	}
	s << "} (v-" << idHead << ");\n";
}

std::function<std::string(const Hyper &, HyperVertex, Options::DupVertex, const std::string &)>
TikzPrinter::getImageCreator() {
	return [this](const Hyper &dg, HyperVertex v, Options::DupVertex vDup, const std::string &id) -> std::string {
		if(this->options.imageOverwrite) {
			auto [file, cmd] = this->options.imageOverwrite(v, options.dupGraph[vDup].dupNum, dg);
			if(!file.empty()) {
				if(!cmd.empty()) lib::IO::post() << cmd << '\n';
				return file;
			}
		}
		const auto &g = *dg.getGraph()[v].graph;
		const auto doIt = [&](const auto &gOpts) {
			if(this->options.withInlineGraphs) {
				return lib::graph::Write::tikz(g, gOpts, true, "v-" + id + "-");
			} else {
				return lib::graph::Write::pdf(g, gOpts);
			}
		};
		if(!options.rotationOverwrite && !options.mirrorOverwrite) {
			return doIt(graphOptions);
		} else {
			auto gOpts = graphOptions;
			const auto gAPI = g.getAPIReference();
			if(options.rotationOverwrite)
				gOpts.Rotation(options.rotationOverwrite(gAPI));
			if(options.mirrorOverwrite)
				gOpts.Mirror(options.mirrorOverwrite(gAPI));
			return doIt(gOpts);
		}
	};
}

//------------------------------------------------------------------------------
// Options
//------------------------------------------------------------------------------

std::pair<int, Options::DupVertex> Options::inDegreeVisible(DupVertex e, const Hyper &dg) const {
	const auto &g = dg.getGraph();
	DupVertex vDupFirst = g.null_vertex();
	int numVisible = 0;
	for(DupVertex vDupIn: asRange(inv_adjacent_vertices(e, dupGraph))) {
		Vertex vIn = dupGraph[vDupIn].v;
		bool isVisible = isVertexVisible(vIn, dg);
		if(!isVisible) continue;
		numVisible++;
		if(vDupFirst == g.null_vertex()) vDupFirst = vDupIn;
	}
	return std::make_pair(numVisible, vDupFirst);
}

std::pair<int, Options::DupVertex> Options::outDegreeVisible(DupVertex e, const Hyper &dg) const {
	const auto &g = dg.getGraph();
	DupVertex vDupFirst = g.null_vertex();
	int numVisible = 0;
	for(DupVertex vDupOut: asRange(adjacent_vertices(e, dupGraph))) {
		Vertex vOut = dupGraph[vDupOut].v;
		bool isVisible = isVertexVisible(vOut, dg);
		if(!isVisible) continue;
		numVisible++;
		if(vDupFirst == g.null_vertex()) vDupFirst = vDupOut;
	}
	return std::make_pair(numVisible, vDupFirst);
}

bool Options::isShortcutEdge(DupVertex e,
							 const Hyper &dg,
							 int inDegreeVisible,
							 int outDegreeVisible) const {
	const auto &g = dg.getGraph();
	Vertex v = dupGraph[e].v;
	return (withShortcutEdges && in_degree(v, g) == 1 && out_degree(v, g) == 1 && inDegreeVisible > 0 &&
			outDegreeVisible > 0)
		   || (withShortcutEdgesAfterVisibility && inDegreeVisible == 1 && outDegreeVisible == 1);
}

std::string Options::vDupToId(DupVertex vDup, const Hyper &dg) const {
	const auto dupNum = dupGraph[vDup].dupNum;
	const auto v = dupGraph[vDup].v;
	const auto vId = get(boost::vertex_index_t(), dg.getGraph(), v);
	return toStr(vId) + '-' + toStr(dupNum);
}

//------------------------------------------------------------------------------
// Data
//------------------------------------------------------------------------------

Data::Data(const Hyper &dg) : dg(dg) {
	for(Vertex v: asRange(vertices(dg.getGraph()))) {
		if(dg.getGraph()[v].kind != HyperVertexKind::Edge) continue;
		std::unordered_map<int, Connections> val;
		Connections c(in_degree(v, dg.getGraph()), out_degree(v, dg.getGraph()));
		val.emplace(0, std::move(c));
		connections.emplace(v, std::move(val));
	}
}

bool Data::makeDuplicate(Vertex e, int eDup) {
	assert(dg.getGraph()[e].kind == HyperVertexKind::Edge);
	const auto iterEdgeDups = connections.find(e);
	assert(iterEdgeDups != end(connections));
	const auto iterEdgeCons = iterEdgeDups->second.find(eDup);
	if(iterEdgeCons != end(iterEdgeDups->second)) return false;
	std::unordered_map<int, Connections> val;
	Connections c(in_degree(e, dg.getGraph()), out_degree(e, dg.getGraph()));
	iterEdgeDups->second.insert(std::make_pair(eDup, std::move(c)));
	return true;
}

bool Data::removeDuplicate(Vertex e, int eDup) {
	assert(dg.getGraph()[e].kind == HyperVertexKind::Edge);
	auto iterEdgeDups = connections.find(e);
	assert(iterEdgeDups != end(connections));
	const auto count = iterEdgeDups->second.erase(eDup);
	return count != 0;
}

namespace {

void reconnectCommon(const Hyper &dgHyper,
					 Data::ConnectionsStore &connections,
					 Vertex v, int eDup,
					 Vertex headOrTail, int vDupTar, int vDupSrc,
					 bool isTail) {
	//	std::cout << "reconnect(" << std::boolalpha << isTail << ", " << v << ", " << eDup << ", " << headOrTail << ", " << vDupTar << ", " << vDupSrc << ")" << std::endl;
	const auto &dg = dgHyper.getGraph();
	assert(dg[v].kind == HyperVertexKind::Edge);
	assert(dg[headOrTail].kind == HyperVertexKind::Vertex);
	const auto iterEdgeDups = connections.find(v);
	assert(iterEdgeDups != end(connections));
	const auto iterEdgeCons = iterEdgeDups->second.find(eDup);
	if(iterEdgeCons == end(iterEdgeDups->second))
		throw LogicError("Hyperedge duplicate does not exist.");
	int offset = [v, headOrTail, isTail, &dg]() {
		if(isTail) {
			const auto vs = inv_adjacent_vertices(v, dg);
			const auto iter = std::find(vs.first, vs.second, headOrTail);
			return iter - vs.first;
		} else {
			const auto vs = adjacent_vertices(v, dg);
			const auto iter = std::find(vs.first, vs.second, headOrTail);
			return iter - vs.first;
		}
	}();
	Data::Connections &cons = iterEdgeCons->second;
	auto &dupNums = isTail ? cons.tail : cons.head;
	if(offset == dupNums.size()) {
		std::stringstream ss;
		if(isTail) ss << "Tail";
		else ss << "Head";
		ss << " duplicate " << vDupSrc << " does not exist. Duplicates are:";
		throw FatalError(ss.str());
	}
	dupNums[offset] = vDupTar;
}

} // namespace

void Data::reconnectSource(Vertex v, int eDup, Vertex tail, int vDupTar, int vDupSrc) {
	reconnectCommon(dg, connections, v, eDup, tail, vDupTar, vDupSrc, true);
}

void Data::reconnectTarget(Vertex v, int eDup, Vertex head, int vDupTar, int vDupSrc) {
	reconnectCommon(dg, connections, v, eDup, head, vDupTar, vDupSrc, false);
}

void Data::removeVertexIfDegreeZero(Vertex v) {
	assert(dg.getGraph()[v].kind == HyperVertexKind::Vertex);
	removedIfDegreeZero.insert(v);
}

void Data::compile(Options &options) const {
	using DupVertex = Options::DupVertex;
	const auto &dg = this->dg.getGraph();
	auto &dupGraph = options.dupGraph;
	assert(num_vertices(dupGraph) == 0);
	std::vector<std::map<int, DupVertex>> duplicates(num_vertices(dg));
	auto idx = get(boost::vertex_index_t(), dg);
	// collect duplicates, set all to null_vertex
	for(const auto &eDupsCons: connections) {
		Vertex e = eDupsCons.first;
		for(const auto &eDupCons: eDupsCons.second) {
			duplicates[idx[e]][eDupCons.first] = dupGraph.null_vertex();
			for(int i = 0; i < in_degree(e, dg); i++) {
				Vertex vIn = *(inv_adjacent_vertices(e, dg).first + i);
				int vDup = eDupCons.second.tail[i];
				duplicates[idx[vIn]][vDup] = dupGraph.null_vertex();
			}
			for(int i = 0; i < out_degree(e, dg); i++) {
				Vertex vOut = *(adjacent_vertices(e, dg).first + i);
				int vDup = eDupCons.second.head[i];
				duplicates[idx[vOut]][vDup] = dupGraph.null_vertex();
			}
		}
	}
	// create vertices
	for(Vertex v: asRange(vertices(dg))) {
		if(dg[v].kind != HyperVertexKind::Vertex) continue;
		int vId = idx[v];
		const auto &dups = duplicates[vId];
		if(dups.empty() &&
		   removedIfDegreeZero.find(v) == removedIfDegreeZero.end()) { // happens when a vertex has degree 0
			DupVertex vDup = add_vertex(dupGraph);
			dupGraph[vDup].v = v;
			dupGraph[vDup].dupNum = 0;
		} else {
			for(auto &pDup: duplicates[vId]) {
				DupVertex vDup = add_vertex(dupGraph);
				pDup.second = vDup;
				dupGraph[vDup].v = v;
				dupGraph[vDup].dupNum = pDup.first;
			}
		}
	}
	// create edges
	for(Vertex v: asRange(vertices(dg))) {
		if(dg[v].kind != HyperVertexKind::Edge) continue;
		int vId = idx[v];
		for(auto &pDup: duplicates[vId]) {
			DupVertex vDup = add_vertex(dupGraph);
			pDup.second = vDup;
			dupGraph[vDup].v = v;
			dupGraph[vDup].dupNum = pDup.first;
		}
	}
	// create connectors
	for(const auto &eDupsCons: connections) {
		Vertex e = eDupsCons.first;
		for(const auto &eDupCons: eDupsCons.second) {
			DupVertex eDup = duplicates[idx[e]][eDupCons.first];
			for(int i = 0; i < in_degree(e, dg); i++) {
				Vertex vIn = *(inv_adjacent_vertices(e, dg).first + i);
				int vDupNum = eDupCons.second.tail[i];
				DupVertex vDupIn = duplicates[idx[vIn]][vDupNum];
				add_edge(vDupIn, eDup, dupGraph);
			}
			for(int i = 0; i < out_degree(e, dg); i++) {
				Vertex vOut = *(adjacent_vertices(e, dg).first + i);
				int vDupNum = eDupCons.second.head[i];
				DupVertex vDupOut = duplicates[idx[vOut]][vDupNum];
				add_edge(eDup, vDupOut, dupGraph);
			}
		}
	}
	if(options.dupVertexLabel) MOD_ABORT;
	else options.dupVertexLabel = this->dupVertexLabel;

	if(options.dupVertexColour) MOD_ABORT;
	else options.dupVertexColour = this->dupVertexColour;
}

//------------------------------------------------------------------------------
// Printer
//------------------------------------------------------------------------------

std::pair<std::string, std::string> Printer::printHyper(
		const Data &data, const IO::Graph::Write::Options &graphOptions) {
	Options options = prePrint(data);
	const auto files = pdf(data.dg, options, graphOptions);
	postPrint();
	return files;
}

void Printer::pushSuffix(const std::string suffix) {
	suffixes.push_back(suffix);
}

void Printer::popSuffix() {
	assert(!suffixes.empty());
	suffixes.pop_back();
}

void Printer::pushVertexVisible(std::function<bool(Vertex, const Hyper &)> f) {
	vertexVisibles.push_back(f);
}

void Printer::popVertexVisible() {
	assert(!vertexVisibles.empty());
	vertexVisibles.pop_back();
}

bool Printer::hasVertexVisible() const {
	return !vertexVisibles.empty();
}

void Printer::pushEdgeVisible(std::function<bool(Vertex, const Hyper &)> f) {
	edgeVisibles.push_back(f);
}

void Printer::popEdgeVisible() {
	assert(!edgeVisibles.empty());
	edgeVisibles.pop_back();
}

bool Printer::hasEdgeVisible() const {
	return !edgeVisibles.empty();
}

void Printer::pushVertexLabel(std::function<std::string(Vertex, const Hyper &)> f) {
	vertexLabels.push_back(f);
}

void Printer::popVertexLabel() {
	assert(!vertexLabels.empty());
	vertexLabels.pop_back();
}

bool Printer::hasVertexLabel() const {
	return !vertexLabels.empty();
}

void Printer::pushEdgeLabel(std::function<std::string(Vertex, const Hyper &)> f) {
	edgeLabels.push_back(f);
}

void Printer::popEdgeLabel() {
	assert(!edgeLabels.empty());
	edgeLabels.pop_back();
}

bool Printer::hasEdgeLabel() const {
	return !edgeLabels.empty();
}

void Printer::pushVertexColour(std::function<std::string(Vertex, const Hyper &)> f, bool extendToEdges) {
	vertexColour.emplace_back(f, extendToEdges);
}

void Printer::popVertexColour() {
	assert(!vertexColour.empty());
	vertexColour.pop_back();
}

bool Printer::hasVertexColour() const {
	return !vertexColour.empty();
}

void Printer::pushEdgeColour(std::function<std::string(Vertex, const Hyper &)> f) {
	edgeColour.push_back(f);
}

void Printer::popEdgeColour() {
	assert(!edgeColour.empty());
	edgeColour.pop_back();
}

bool Printer::hasEdgeColour() const {
	return !edgeColour.empty();
}

void Printer::setRotationOverwrite(std::function<int(std::shared_ptr<mod::graph::Graph>)> f) {
	baseOptions.rotationOverwrite = f;
}

void Printer::setMirrorOverwrite(std::function<bool(std::shared_ptr<mod::graph::Graph>)> f) {
	baseOptions.mirrorOverwrite = f;
}

void Printer::setImageOverwrite(std::function<std::pair<std::string, std::string>(
		Vertex v, int dupNum, const Hyper &)> f) {
	baseOptions.imageOverwrite = f;
}

void Printer::setGraphvizPrefix(const std::string &prefix) {
	baseOptions.graphvizPrefix = prefix;
}

const std::string &Printer::getGraphvizPrefix() const {
	return baseOptions.graphvizPrefix;
}

void Printer::setTikzpictureOption(const std::string &option) {
	baseOptions.tikzpictureOption = option;
}

const std::string &Printer::getTikzpictureOption() const {
	return baseOptions.tikzpictureOption;
}

Options Printer::prePrint(const Data &data) {
	Options options = baseOptions;
	data.compile(options);
	if(withGraphName) {
		pushVertexLabel([](Vertex v, const Hyper &dg) -> std::string {
			const auto &g = dg.getGraph();
			assert(g[v].kind == HyperVertexKind::Vertex);
			return g[v].graph->getName();
		});
	}
	if(withRuleId) { // we want the rule id before its name
		pushEdgeLabel([this](Vertex v, const Hyper &dg) -> std::string {
			assert(dg.getGraph()[v].kind == HyperVertexKind::Edge);
			std::string res;
			bool first = true;
			for(auto *r: dg.getRulesFromEdge(v)) {
				if(!first) res += this->edgeLabelSep;
				first = false;
				res += "r_{";
				res += boost::lexical_cast<std::string>(r->getId());
				res += '}';
			}
			return res;
		});
	}
	if(withRuleName) {
		pushEdgeLabel([this](Vertex v, const Hyper &dg) -> std::string {
			assert(dg.getGraph()[v].kind == HyperVertexKind::Edge);
			std::string res;
			bool first = true;
			for(auto *r: dg.getRulesFromEdge(v)) {
				if(!first) res += this->edgeLabelSep;
				first = false;
				res += r->getName();
			}
			return res;
		});
	}

	setup(options);
	return options;
}

void Printer::postPrint() {
	if(withRuleId) popEdgeLabel();
	if(withRuleName) popEdgeLabel();
	if(withGraphName) popVertexLabel();
}

void Printer::setup(Options &options) {
	for(const auto &s: suffixes) options.suffix += s;

	options.vertexVisible = [this](Vertex v, const Hyper &dg) {
		for(const auto &f: vertexVisibles) {
			if(!f(v, dg)) return false;
		}
		return true;
	};

	options.hyperedgeVisible = [this](Vertex v, const Hyper &dg) {
		for(const auto &f: edgeVisibles) {
			if(!f(v, dg)) return false;
		}
		return true;
	};

	options.vertexLabel = [this](Vertex v, const Hyper &dg) {
		std::vector<std::string> labels;
		labels.reserve(vertexLabels.size());
		for(const auto &vl: vertexLabels) {
			std::string label = vl(v, dg);
			if(!label.empty())
				labels.push_back(std::move(label));
		}
		std::string label;
		if(!labels.empty())
			label += std::move(labels.front());
		for(int i = 1; i < labels.size(); ++i) {
			label += vertexLabelSep;
			label += std::move(labels[i]);
		}
		return label;
	};
	options.hyperedgeLabel = [this](Vertex v, const Hyper &dg) {
		std::vector<std::string> labels;
		labels.reserve(edgeLabels.size());
		for(const auto &vl: edgeLabels) {
			std::string label = vl(v, dg);
			if(!label.empty())
				labels.push_back(std::move(label));
		}
		std::string label;
		if(!labels.empty())
			label += std::move(labels.front());
		for(int i = 1; i < labels.size(); ++i) {
			label += edgeLabelSep;
			label += std::move(labels[i]);
		}
		return label;
	};

	options.vertexColour = [this](Vertex v, const Hyper &dg) {
		for(const auto &fp: vertexColour) {
			std::string colour = fp.first(v, dg);
			if(!colour.empty()) return colour;
		}
		return std::string();
	};

	auto getRawHyperEdgeColour = [this](Vertex v, const Hyper &dg) {
		assert(dg.getGraph()[v].kind == HyperVertexKind::Edge);
		for(const auto &f: edgeColour) {
			std::string colour = f(v, dg);
			if(!colour.empty()) return colour;
		}
		return std::string();
	};

	auto getExtendColour = [this](Vertex v, const Hyper &dg) {
		assert(dg.getGraph()[v].kind == HyperVertexKind::Vertex);
		for(const auto &fp: vertexColour) {
			if(!fp.second) continue;
			std::string colour = fp.first(v, dg);
			if(!colour.empty()) return colour;
		}
		return std::string();
	};

	auto getHyperedgeExtendColour = [getExtendColour](Vertex v, const Hyper &dg) {
		assert(dg.getGraph()[v].kind == HyperVertexKind::Edge);
		std::string tailColour, headColour;
		for(Vertex vAdj: asRange(inv_adjacent_vertices(v, dg.getGraph()))) {
			tailColour = getExtendColour(vAdj, dg);
			if(!tailColour.empty()) break;
		}
		for(Vertex vAdj: asRange(adjacent_vertices(v, dg.getGraph()))) {
			headColour = getExtendColour(vAdj, dg);
			if(!headColour.empty()) break;
		}
		if(!tailColour.empty() && !headColour.empty()) return headColour;
		else return std::string();
	};

	options.hyperedgeColour = [getRawHyperEdgeColour, getHyperedgeExtendColour](Vertex v, const Hyper &dg) {
		assert(dg.getGraph()[v].kind == HyperVertexKind::Edge);
		std::string colour = getRawHyperEdgeColour(v, dg);
		if(!colour.empty()) return colour;
		return getHyperedgeExtendColour(v, dg);
	};
	options.tailColour = [getRawHyperEdgeColour, getExtendColour,
			getHyperedgeExtendColour](Vertex v, Vertex e, const Hyper &dg) {
		assert(dg.getGraph()[v].kind == HyperVertexKind::Vertex);
		assert(dg.getGraph()[e].kind == HyperVertexKind::Edge);
		std::string colour = getRawHyperEdgeColour(e, dg);
		if(!colour.empty()) return colour;
		colour = getHyperedgeExtendColour(e, dg);
		if(colour.empty()) return std::string();
		else return getExtendColour(v, dg);
	};
	options.headColour = [getRawHyperEdgeColour, getExtendColour,
			getHyperedgeExtendColour](Vertex e, Vertex v, const Hyper &dg) {
		assert(dg.getGraph()[v].kind == HyperVertexKind::Vertex);
		assert(dg.getGraph()[e].kind == HyperVertexKind::Edge);
		std::string colour = getRawHyperEdgeColour(e, dg);
		if(!colour.empty()) return colour;
		colour = getHyperedgeExtendColour(e, dg);
		if(colour.empty()) return std::string();
		else return getExtendColour(v, dg);
	};
}

//------------------------------------------------------------------------------
// Algorithms
//------------------------------------------------------------------------------

void generic(const Hyper &dg, const Options &options, SyntaxPrinter &print) {
	using DupVertex = Options::DupVertex;
	using DupEdge = Options::DupEdge;
	const auto imageCreator = print.getImageCreator();
	print.begin();

	// vertices
	detail::forEachVertex(dg, options, print, true, [&dg, &options, &print, imageCreator](DupVertex vDup) {
		Vertex v = options.dupGraph[vDup].v;
		std::string id = options.vDupToId(vDup, dg);
		std::string label = options.getVertexLabel(dg, vDup);
		std::string image;
		if(options.withGraphImages) image = imageCreator(dg, v, vDup, id);
		std::string colour = options.getVertexColour(vDup, dg);
		print.vertex(id, label, image, colour);
	});

	// edges
	detail::forEachExplicitHyperEdge(dg, options, print, [&dg, &options, &print](DupVertex vDup) {
		Vertex v = options.dupGraph[vDup].v;
		std::string id = options.vDupToId(vDup, dg);
		std::string label = options.getHyperedgeLabel(v, dg);
		std::string colour = options.getHyperedgeColour(v, dg);
		print.hyperEdge(id, label, colour);
	});

	// connectors
	auto fTail = [&dg, &options, &print](DupVertex vDup, DupVertex vDupAdj, int count, int maxCount) {
		Vertex v = options.dupGraph[vDup].v;
		Vertex vAdj = options.dupGraph[vDupAdj].v;

		std::string id = options.vDupToId(vDup, dg);
		std::string idAdj = options.vDupToId(vDupAdj, dg);
		std::string tailColour = options.getTailColour(vAdj, v, dg);
		for(int i = 1; i <= count; i++)
			print.tailConnector(idAdj, id, tailColour, i, maxCount);
	};
	auto fHead = [&dg, &options, &print](DupVertex vDup, DupVertex vDupAdj, int count, int maxCount) {
		Vertex v = options.dupGraph[vDup].v;
		Vertex vAdj = options.dupGraph[vDupAdj].v;

		std::string id = options.vDupToId(vDup, dg);
		std::string idAdj = options.vDupToId(vDupAdj, dg);
		std::string headColour = options.getHeadColour(v, vAdj, dg);
		for(int i = 1; i <= count; i++)
			print.headConnector(id, idAdj, headColour, i, maxCount);
	};
	auto fShortcut = [&dg, &options, &print](DupVertex vDup, DupVertex vDupTail, DupVertex vDupHead, bool hasReverse) {
		Vertex v = options.dupGraph[vDup].v;
		std::string idTail = options.vDupToId(vDupTail, dg);
		std::string idHead = options.vDupToId(vDupHead, dg);
		std::string label = options.getHyperedgeLabel(v, dg);
		std::string colour = options.getHyperedgeColour(v, dg);
		print.shortcutEdge(idTail, idHead, label, colour, hasReverse);
	};
	detail::forEachConnector(dg, options, print, fTail, fHead, fShortcut);
	if(options.auxPrinter) options.auxPrinter(dg, options, print);
	print.end();
}

std::string dot(const Hyper &dg, const Options &options, const IO::Graph::Write::Options &graphOptions) {
	struct DotPrinter : SyntaxPrinter {
		DotPrinter(std::string file, std::string prefix, const IO::Graph::Write::Options &graphOptions)
				: SyntaxPrinter(file), prefix(prefix), graphOptions(graphOptions) {}

		std::string getName() const override {
			return "dot";
		}

		virtual void begin() override {
			s << "digraph g {\n";
			if(!prefix.empty()) s << prefix << '\n';
		}

		virtual void end() override {
			s << "}\n";
		}

		virtual void comment(const std::string &str) override {
			s << "// " << str << '\n';
		}

		virtual void vertex(const std::string &id,
							const std::string &label,
							const std::string &image,
							const std::string &colour) override {
			s << '"' << id << "\" [ shape=ellipse label=\"" << label << "\"";
			if(!image.empty()) s << " image=\"" << image << "\"";
			if(!colour.empty()) s << " color=\"" << colour << "\"";
			s << " ];\n";
		}

		virtual void vertexHidden(const std::string &id, bool large) override {
			s << '"' << id << "\" [ shape=point style=invis label=\"\" ];\n";
		}

		virtual void hyperEdge(const std::string &id, const std::string &label, const std::string &colour) override {
			s << '"' << id << "\" [ shape=box label=\"" << label << "\"";
			if(!colour.empty()) s << "color=\"" << colour << "\"";
			s << " ];\n";
		}

		void connector(const std::string &idSrc,
					   const std::string &idTarget,
					   const std::string &colour,
					   int num, int maxNum) {
			// dot handles parallel edges fine, just print the data
			s << '"' << idSrc << "\" -> \"" << idTarget << "\" [";
			if(!colour.empty()) s << "color=\"" << colour << "\"";
			s << " ];\n";
		}

		virtual void tailConnector(const std::string &idVertex,
								   const std::string &idHyperEdge,
								   const std::string &colour,
								   int num, int maxNum) override {
			connector(idVertex, idHyperEdge, colour, num, maxNum);
		}

		virtual void headConnector(const std::string &idHyperEdge,
								   const std::string &idVertex,
								   const std::string &colour,
								   int num, int maxNum) override {
			connector(idHyperEdge, idVertex, colour, num, maxNum);
		}

		virtual void shortcutEdge(const std::string &idTail,
								  const std::string &idHead,
								  const std::string &label,
								  const std::string &colour,
								  bool hasReverse) override {
			s << '"' << idTail << "\" -> \"" << idHead << "\" [ label=\"" << label << "\"";
			if(!colour.empty()) s << " color=\"" << colour << "\"";
			s << " ];\n";
		}

		virtual std::function<std::string(const Hyper &,
										  HyperVertex, Options::DupVertex,
										  const std::string &)> getImageCreator() override {
			return [this](const Hyper &dg, HyperVertex v, Options::DupVertex vDup,
						  const std::string &id) -> std::string {
				const auto &g = *dg.getGraph()[v].graph;
				return lib::graph::Write::svg(g, graphOptions);
			};
		}

	public:
		std::string prefix;
		const IO::Graph::Write::Options &graphOptions;
	};
	std::string file = IO::makeUniqueFilePrefix();
	file += "dg_" + boost::lexical_cast<std::string>(dg.getNonHyper().getId()) + "_";
	file += options;
	file += ".dot";
	DotPrinter printer(file, options.graphvizPrefix, graphOptions);
	generic(dg, options, printer);
	return file;
}

std::string coords(const Hyper &dg, const Options &options, const IO::Graph::Write::Options &graphOptions) {
	std::string fileNoExt = dot(dg, options, graphOptions);
	fileNoExt.erase(end(fileNoExt) - 4, end(fileNoExt));
	IO::post() << "coordsFromGV dgHyper  \"" << fileNoExt << "\"" << std::endl;
	return fileNoExt + "_coord.tex";
}

std::pair<std::string, std::string> tikz(const Hyper &dg, const Options &options,
										 const IO::Graph::Write::Options &graphOptions) {
	std::string fileCoordsExt = coords(dg, options, graphOptions);
	std::string file = IO::makeUniqueFilePrefix();
	file += "dg_" + boost::lexical_cast<std::string>(dg.getNonHyper().getId()) + "_";
	file += options;
	file += ".tex";
	TikzPrinter printer(file, fileCoordsExt, options, graphOptions);
	generic(dg, options, printer);
	return std::make_pair(file, fileCoordsExt);
}

std::string pdfFromDot(const Hyper &dg, const Options &options,
					   const IO::Graph::Write::Options &graphOptions) {
	std::string fileNoExt = dot(dg, options, graphOptions);
	fileNoExt.erase(end(fileNoExt) - 4, end(fileNoExt));
	IO::post() << "gv dgHyper \"" << fileNoExt << "\" pdf" << std::endl;
	return fileNoExt + ".pdf";
}

std::pair<std::string, std::string> pdf(const Hyper &dg, const Options &options,
										const IO::Graph::Write::Options &graphOptions) {
	auto tikzFiles = tikz(dg, options, graphOptions);
	std::string fileNoExt = tikzFiles.first.substr(0, tikzFiles.first.length() - 4);
	std::string fileCoordsNoExt = tikzFiles.second.substr(0, tikzFiles.second.length() - 4);
	IO::post() << "compileTikz \"" << fileNoExt << "\" \"" << fileCoordsNoExt << "\"";
	if(options.withInlineGraphs) IO::post() << " 3";
	IO::post() << std::endl;
	return {fileNoExt + ".pdf", tikzFiles.second};
}

std::pair<std::string, std::string> summary(const Data &data, Printer &printer,
											const IO::Graph::Write::Options &graphOptions) {
	const auto files = printer.printHyper(data, graphOptions);
	std::string fileNoExt = files.first;
	const auto &dg = data.dg;
	fileNoExt.erase(end(fileNoExt) - 4, end(fileNoExt));
	IO::post() << "summaryDGHyper \"dg_" << dg.getNonHyper().getId() << "\" \"" << fileNoExt << "\"" << std::endl;
	return files;
}

std::string summaryNonHyper(const NonHyper &dg) {
	std::string file = pdfNonHyper(dg);
	IO::post() << "summaryDGNonHyper \"dg_" << dg.getId() << "\" \"" << file << "\"" << std::endl;
	return file;
}

// -----------------------------------------------------------------------------------

nlohmann::json dumpDigest(const HyperGraphType &dg) {
	nlohmann::json res;
	// apparently it 53 bits is the maximum that is generally supported in some JSON implementations,
	// so 32 is the maximum block size we can use
	boost::dynamic_bitset<uint32_t> isVertex(num_vertices(dg));
	for(const auto v: asRange(vertices(dg)))
		isVertex[get(boost::vertex_index_t(), dg, v)] = dg[v].kind == HyperVertexKind::Vertex;
	auto bitmap = nlohmann::json::array();
	boost::to_block_range(isVertex, std::back_inserter(bitmap));
	res["numUDGVertices"] = num_vertices(dg);
	res["isVertex"] = std::move(bitmap);
	return res;
}

} // namespace mod::lib::DG::Write