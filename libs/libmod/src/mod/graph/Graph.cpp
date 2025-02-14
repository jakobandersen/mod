#include "Graph.hpp"

#include <mod/Error.hpp>
#include <mod/Function.hpp>
#include <mod/VertexMap.hpp>
#include <mod/graph/Automorphism.hpp>
#include <mod/graph/GraphInterface.hpp>
#include <mod/graph/Printer.hpp>
#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/Graph/IO/DepictionData.hpp>
#include <mod/lib/Graph/IO/Read.hpp>
#include <mod/lib/Graph/IO/Write.hpp>
#include <mod/lib/Graph/Properties/Molecule.hpp>
#include <mod/lib/Graph/Properties/Stereo.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/Graph/Properties/Term.hpp>

#include <boost/iostreams/device/mapped_file.hpp>

#include <cassert>
#include <iostream>

namespace mod::graph {

// Graph
//------------------------------------------------------------------------------

struct Graph::ExternalData {
	std::map<int, std::size_t> externalToInternalIds;
	std::vector<std::pair<std::string, bool>> warnings;
};

Graph::Graph(std::unique_ptr<lib::graph::Graph> g) : g(std::move(g)) {}

Graph::~Graph() = default;

std::size_t Graph::getId() const {
	return g->getId();
}

std::ostream &operator<<(std::ostream &s, const Graph &g) {
	return s << "'" << g.g->getName() << "'";
}

lib::graph::Graph &Graph::getGraph() const {
	return *g;
}

//------------------------------------------------------------------------------

std::size_t Graph::numVertices() const {
	return num_vertices(g->getGraph());
}

Graph::VertexRange Graph::vertices() const {
	return VertexRange(g->getAPIReference());
}

std::size_t Graph::numEdges() const {
	return num_edges(g->getGraph());
}

Graph::EdgeRange Graph::edges() const {
	return EdgeRange(g->getAPIReference());
}

//------------------------------------------------------------------------------

Graph::AutGroup Graph::aut(LabelSettings labelSettings) const {
	return AutGroup(g->getAPIReference(), labelSettings);
}

//------------------------------------------------------------------------------

std::pair<std::string, std::string> Graph::print() const {
	Printer first;
	Printer second;
	second.setMolDefault();
	return print(first, second);
}

std::pair<std::string, std::string> Graph::print(const Printer &first, const Printer &second) const {
	return lib::graph::Write::summary(*g, first.getOptions(), second.getOptions());
}

void Graph::printTermState() const {
	lib::graph::Write::termState(*g);
}

std::string Graph::getGMLString(bool withCoords) const {
	if(withCoords && !g->getDepictionData().getHasCoordinates())
		throw LogicError("Coordinates are not available for this graph (" + getName() + ").");
	std::stringstream ss;
	lib::graph::Write::gml(*g, withCoords, ss);
	return ss.str();
}

std::string Graph::printGML(bool withCoords) const {
	if(withCoords && !g->getDepictionData().getHasCoordinates())
		throw LogicError("Coordinates are not available for this graph (" + getName() + ").");
	return lib::graph::Write::gml(*g, withCoords);
}

const std::string &Graph::getName() const {
	return g->getName();
}

void Graph::setName(std::string name) const {
	g->setName(name);
}

const std::string &Graph::getSmiles() const {
	return g->getSmiles(false);
}

const std::string &Graph::getSmilesWithIds() const {
	return g->getSmiles(true);
}

const std::string &Graph::getGraphDFS() const {
	return g->getGraphDFS().first;
}

const std::string &Graph::getGraphDFSWithIds() const {
	return g->getGraphDFSWithIds();
}

const std::string &Graph::getLinearEncoding() const {
	if(g->getMoleculeState().getIsMolecule()) return g->getSmiles(false);
	else return g->getGraphDFS().first;
}

bool Graph::getIsMolecule() const {
	return g->getMoleculeState().getIsMolecule();
}

double Graph::getEnergy() const {
	if(getIsMolecule()) return g->getMoleculeState().getEnergy();
	else return std::numeric_limits<double>::quiet_NaN();
}

void Graph::cacheEnergy(double value) const {
	if(getIsMolecule()) g->getMoleculeState().cacheEnergy(value);
	else throw LogicError("Graph::cacheEnergy: Caching of energy failed. '" + getName() + "' is not a molecule.\n");
}

double Graph::getExactMass() const {
	if(!getIsMolecule()) throw LogicError("Can not get exact mass of a non-molecule.");
	return g->getMoleculeState().getExactMass();
}

unsigned int Graph::vLabelCount(const std::string &label) const {
	return g->getVertexLabelCount(label);
}

unsigned int Graph::eLabelCount(const std::string &label) const {
	return g->getEdgeLabelCount(label);
}

namespace {

void checkTermParsing(const lib::graph::Graph &g, LabelSettings ls) {
	if(ls.type == LabelType::Term) {
		const auto &term = get_term(g.getLabelledGraph());
		if(!isValid(term)) {
			std::string msg = "Parsing failed for graph '" + g.getName() + "'. " + term.getParsingError();
			throw TermParsingError(std::move(msg));
		}
	}
}

} // namespace

std::size_t
Graph::isomorphism(std::shared_ptr<graph::Graph> codomain, std::size_t maxNumMatches,
                   LabelSettings labelSettings) const {
	if(!codomain) throw LogicError("codomain is null.");
	checkTermParsing(*g, labelSettings);
	checkTermParsing(*codomain->g, labelSettings);
	return lib::graph::Graph::isomorphism(*g, *codomain->g, maxNumMatches, labelSettings);
}

std::size_t
Graph::monomorphism(std::shared_ptr<graph::Graph> codomain, std::size_t maxNumMatches,
                    LabelSettings labelSettings) const {
	if(!codomain) throw LogicError("codomain is null.");
	checkTermParsing(*g, labelSettings);
	checkTermParsing(*codomain->g, labelSettings);
	return lib::graph::Graph::monomorphism(*g, *codomain->g, maxNumMatches, labelSettings);
}

void Graph::enumerateIsomorphisms(std::shared_ptr<Graph> codomain,
                                  std::shared_ptr<Function<bool(VertexMap<Graph, Graph>)>> callback,
                                  LabelSettings labelSettings) const {
	if(!codomain) throw LogicError("codomain is null.");
	if(!callback) throw LogicError("callback is null.");
	checkTermParsing(*g, labelSettings);
	checkTermParsing(*codomain->g, labelSettings);
	return lib::graph::Graph::enumerateIsomorphisms(*g, *codomain->g, toStdFunction(callback), labelSettings);
}

void Graph::enumerateMonomorphisms(std::shared_ptr<Graph> codomain,
                                   std::shared_ptr<Function<bool(VertexMap<Graph, Graph>)>> callback,
                                   LabelSettings labelSettings) const {
	if(!codomain) throw LogicError("codomain is null.");
	if(!callback) throw LogicError("callback is null.");
	checkTermParsing(*g, labelSettings);
	checkTermParsing(*codomain->g, labelSettings);
	return lib::graph::Graph::enumerateMonomorphisms(*g, *codomain->g, toStdFunction(callback), labelSettings);
}

std::shared_ptr<Graph> Graph::makePermutation() const {
	auto gPerm = create(std::make_unique<lib::graph::Graph>(lib::graph::makePermutation(*g)));
	gPerm->setName(getName() + " perm");
	return gPerm;
}

void Graph::setImage(std::shared_ptr<mod::Function<std::string()>> image) {
	g->getDepictionData().setImage(image);
}

std::shared_ptr<mod::Function<std::string()> > Graph::getImage() const {
	return g->getDepictionData().getImage();
}

void Graph::setImageCommand(std::string cmd) {
	g->getDepictionData().setImageCommand(cmd);
}

std::string Graph::getImageCommand() const {
	return g->getDepictionData().getImageCommand();
}

void Graph::instantiateStereo() const {
	get_stereo(g->getLabelledGraph());
}

Graph::Vertex Graph::getVertexFromExternalId(int id) const {
	if(!externalData || externalData->externalToInternalIds.empty()) return Vertex();
	const auto iter = externalData->externalToInternalIds.find(id);
	if(iter == end(externalData->externalToInternalIds)) return Vertex();
	return Vertex(g->getAPIReference(), iter->second);
}

int Graph::getMinExternalId() const {
	if(!externalData || externalData->externalToInternalIds.empty()) return 0;
	return externalData->externalToInternalIds.begin()->first;
}

int Graph::getMaxExternalId() const {
	if(!externalData || externalData->externalToInternalIds.empty()) return 0;
	return (--externalData->externalToInternalIds.end())->first;
}

std::vector<std::pair<std::string, bool>> Graph::getLoadingWarnings() const {
	if(!externalData) throw LogicError("Can not get loading warnings. No data from external loading stored.");
	return externalData->warnings;
}

//------------------------------------------------------------------------------
// Static
//------------------------------------------------------------------------------

namespace {

std::shared_ptr<Graph>
makeGraphFromData(lib::graph::Read::Data data, std::vector<std::pair<std::string, bool>> warnings) {
	auto gInternal = std::make_unique<lib::graph::Graph>(
			std::move(data.g), std::move(data.pString), std::move(data.pStereo));
	std::shared_ptr<Graph> g = Graph::create(std::move(gInternal),
	                                         std::move(data.externalToInternalIds),
	                                         std::move(warnings));
	return g;
}

std::shared_ptr<Graph>
handleLoadedGraph(std::vector<lib::graph::Read::Data> data, lib::IO::Warnings warnings,
                  const std::string &type, const std::string &source) {
	if(data.size() != 1) {
		if(data.empty()) {
			throw InputError("Error in loading " + type + " from " + source + ".\nthe graph is empty.");
		} else {
			throw InputError("Error in loading " + type + " from " + source
			                 + ".\nthe graph is not connected (" + std::to_string(data.size()) + " components).");
		}
	}
	return makeGraphFromData(std::move(data.front()), warnings.extractWarnings());
}

std::vector<std::shared_ptr<Graph>>
handleLoadedGraphs(std::vector<lib::graph::Read::Data> data, lib::IO::Warnings warnings,
                   const std::string &type, const std::string &source) {
	// the warnings are copied into each graph
	const auto warningList = warnings.extractWarnings();
	std::vector<std::shared_ptr<Graph>> res;
	res.reserve(data.size());
	for(auto &d: data)
		res.push_back(makeGraphFromData(std::move(d), warningList));
	return res;
}

std::vector<std::shared_ptr<Graph>>
handleLoadedGraphVector(std::vector<std::vector<lib::graph::Read::Data>> data, lib::IO::Warnings warnings,
                        const std::string &type, const std::string &source) {
	std::vector<std::shared_ptr<Graph>> res;
	res.reserve(data.size());
	for(auto &ds: data)
		res.push_back(handleLoadedGraph(std::move(ds), lib::IO::Warnings(warnings), type, source));
	return res;
}

std::vector<std::vector<std::shared_ptr<Graph>>>
handleLoadedGraphsVector(std::vector<std::vector<lib::graph::Read::Data>> data, lib::IO::Warnings warnings,
                         const std::string &type, const std::string &source) {
	std::vector<std::vector<std::shared_ptr<Graph>>> res;
	res.reserve(data.size());
	for(auto &ds: data)
		res.push_back(handleLoadedGraphs(std::move(ds), lib::IO::Warnings(warnings), type, source));
	return res;
}

boost::iostreams::mapped_file_source openFile(const std::string &file, const std::string &type) {
	boost::iostreams::mapped_file_source ifs;
	try {
		ifs.open(file);
	} catch(const BOOST_IOSTREAMS_FAILURE &e) {
		throw InputError("Could not open " + type + " '" + file + "':\n" + e.what());
	}
	if(!ifs) throw InputError("Could not open " + type + " '" + file + "'.\n");
	return ifs;
}


template<bool IsFile, typename FParse, typename FProcess, typename ...ParseArgs>
auto load(const std::string &data, const std::string &type, FParse parse, FProcess process, ParseArgs &&... parseArgs) {
	lib::IO::Warnings warnings;
	auto parsedData = [&]() {
		if constexpr(IsFile) {
			auto ifs = openFile(data, type + " file");
			return parse(warnings, {ifs.begin(), ifs.size()}, std::forward<ParseArgs>(parseArgs)...);
		} else {
			return parse(warnings, data, std::forward<ParseArgs>(parseArgs)...);
		}
	}();
	std::cout << warnings << std::flush;
	std::string source = IsFile ? type + " file '" + data + "'" : "inline " + type + " string";
	if(!parsedData) throw InputError("Error in loading from " + source + ".\n" + parsedData.extractError());
	return process(std::move(*parsedData), std::move(warnings), type, source);
}

} // namespace

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

std::shared_ptr<Graph> Graph::fromGMLString(const std::string &data, bool printStereoWarnings) {
	return load<false>(data, "GML", &lib::graph::Read::gml, &handleLoadedGraph, printStereoWarnings);
}

std::shared_ptr<Graph> Graph::fromGMLFile(const std::string &file, bool printStereoWarnings) {
	return load<true>(file, "GML", &lib::graph::Read::gml, &handleLoadedGraph, printStereoWarnings);
}

std::vector<std::shared_ptr<Graph>> Graph::fromGMLStringMulti(const std::string &data, bool printStereoWarnings) {
	return load<false>(data, "GML", &lib::graph::Read::gml, &handleLoadedGraphs, printStereoWarnings);
}

std::vector<std::shared_ptr<Graph>> Graph::fromGMLFileMulti(const std::string &file, bool printStereoWarnings) {
	return load<true>(file, "GML", &lib::graph::Read::gml, &handleLoadedGraphs, printStereoWarnings);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

std::shared_ptr<Graph> Graph::fromDFS(const std::string &data) {
	return load<false>(data, "GraphDFS", &lib::graph::Read::dfs, &handleLoadedGraph);
}

std::vector<std::shared_ptr<Graph>> Graph::fromDFSMulti(const std::string &data) {
	return load<false>(data, "GraphDFS", &lib::graph::Read::dfs, &handleLoadedGraphs);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

std::shared_ptr<Graph> Graph::fromSMILES(const std::string &smiles) {
	return Graph::fromSMILES(smiles, false, SmilesClassPolicy::NoneOnDuplicate, true);
}

std::shared_ptr<Graph>
Graph::fromSMILES(const std::string &smiles, bool allowAbstract,
                  SmilesClassPolicy classPolicy, bool printStereoWarnings) {
	return load<false>(smiles, "SMILES", &lib::graph::Read::smiles, &handleLoadedGraph,
	                   printStereoWarnings, allowAbstract, classPolicy);
}

std::vector<std::shared_ptr<Graph>> Graph::fromSMILESMulti(const std::string &smiles) {
	return fromSMILESMulti(smiles, false, SmilesClassPolicy::NoneOnDuplicate, true);
}

std::vector<std::shared_ptr<Graph>>
Graph::fromSMILESMulti(const std::string &smiles, bool allowAbstract,
                       SmilesClassPolicy classPolicy, bool printStereoWarnings) {
	return load<false>(smiles, "SMILES", &lib::graph::Read::smiles, &handleLoadedGraphs,
	                   printStereoWarnings, allowAbstract, classPolicy);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

std::shared_ptr<Graph> Graph::fromMOLString(const std::string &data, const MDLOptions &options) {
	return load<false>(data, "MOL", &lib::graph::Read::MDLMOL, &handleLoadedGraph, options);
}

std::shared_ptr<Graph> Graph::fromMOLFile(const std::string &file, const MDLOptions &options) {
	return load<true>(file, "MOL", &lib::graph::Read::MDLMOL, &handleLoadedGraph, options);
}

std::vector<std::shared_ptr<Graph>> Graph::fromMOLStringMulti(const std::string &data, const MDLOptions &options) {
	return load<false>(data, "MOL", &lib::graph::Read::MDLMOL, &handleLoadedGraphs, options);
}

std::vector<std::shared_ptr<Graph>> Graph::fromMOLFileMulti(const std::string &file, const MDLOptions &options) {
	return load<true>(file, "MOL", &lib::graph::Read::MDLMOL, &handleLoadedGraphs, options);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

std::vector<std::shared_ptr<Graph>> Graph::fromSDString(const std::string &data, const MDLOptions &options) {

	return load<false>(data, "SD", &lib::graph::Read::MDLSD, &handleLoadedGraphVector, options);
}

std::vector<std::shared_ptr<Graph>> Graph::fromSDFile(const std::string &file, const MDLOptions &options) {
	return load<true>(file, "SD", &lib::graph::Read::MDLSD, &handleLoadedGraphVector, options);
}


std::vector<std::vector<std::shared_ptr<Graph>>>
Graph::fromSDStringMulti(const std::string &data, const MDLOptions &options) {
	return load<false>(data, "SD", &lib::graph::Read::MDLSD, &handleLoadedGraphsVector, options);
}

std::vector<std::vector<std::shared_ptr<Graph>>>
Graph::fromSDFileMulti(const std::string &file, const MDLOptions &options) {
	return load<true>(file, "SD", &lib::graph::Read::MDLSD, &handleLoadedGraphsVector, options);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

std::shared_ptr<Graph> Graph::create(std::unique_ptr<lib::graph::Graph> g) {
	if(!g) MOD_ABORT;
	std::shared_ptr<Graph> wrapped = std::shared_ptr<Graph>(new Graph(std::move(g)));
	wrapped->g->setAPIReference(wrapped);
	return wrapped;
}

std::shared_ptr<Graph> Graph::create(std::unique_ptr<lib::graph::Graph> g,
                                     std::map<int, std::size_t> externalToInternalIds,
                                     std::vector<std::pair<std::string, bool>> warnings) {
	auto wrapped = create(std::move(g));
	auto externalData = std::make_unique<ExternalData>();
	externalData->externalToInternalIds = std::move(externalToInternalIds);
	externalData->warnings = std::move(warnings);
	wrapped->externalData = std::move(externalData);
	return wrapped;
}

} // namespace mod::graph