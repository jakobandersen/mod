#include "Read.hpp"

#include <mod/Config.hpp>
#include <mod/lib/Algorithm/ConnectedComponents.hpp>
#include <mod/lib/Chem/MDL.hpp>
#include <mod/lib/Chem/MoleculeUtil.hpp>
#include <mod/lib/Chem/Smiles.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/Graph/Properties/Molecule.hpp>
#include <mod/lib/Graph/Properties/Stereo.hpp>
#include <mod/lib/IO/DFS.hpp>
#include <mod/lib/IO/GML.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/Stereo/GeometryGraph.hpp>
#include <mod/lib/Stereo/Inference.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <gml/parser.hpp>
#include <gml/converter.hpp>
#include <gml/converter_edsl.hpp>

namespace mod::lib::Graph::Read {

namespace GML = lib::IO::GML;
using lib::IO::Result;

Data::Data() = default;

Data::Data(Data &&other) : g(std::move(other.g)), pString(std::move(other.pString)), pStereo(std::move(other.pStereo)),
                           externalToInternalIds(std::move(other.externalToInternalIds)) {}

Data::~Data() {
	if(std::uncaught_exceptions() != 0) return;
	if(g) MOD_ABORT;
	if(pString) MOD_ABORT;
	if(pStereo) MOD_ABORT;
}

void Data::reset() {
	pStereo.reset();
	pString.reset();
	g.reset();
}

Result<std::vector<Data>> gml(lib::IO::Warnings &warnings, std::string_view src, bool printStereoWarnings) {
	GML::Graph gGML;
	{
		gml::ast::KeyValue ast;
		try {
			ast = gml::parser::parse(src);
		} catch(const gml::parser::error &e) {
			return lib::IO::Result<>::Error(e.what());
		}
		using namespace gml::converter::edsl;
		auto cVertex = GML::makeVertexConverter(1);
		auto cEdge = GML::makeEdgeConverter(1);
		auto cGraph = list<Parent>("graph")(cVertex)(cEdge);
		auto iterBegin = &ast;
		auto iterEnd = iterBegin + 1;
		try {
			gml::converter::convert(iterBegin, iterEnd, cGraph, gGML);
		} catch(const gml::converter::error &e) {
			return Result<>::Error(e.what());
		}
	}

	std::sort(begin(gGML.vertices), end(gGML.vertices), [](const GML::Vertex &v1, const GML::Vertex &v2) -> bool {
		return v1.id < v2.id;
	});

	// Check that is mathematically is a graph

	std::unordered_map<int, int> globalIddFromExtID;
	{
		int i = 0;
		for(const auto &vGML: gGML.vertices) {
			if(globalIddFromExtID.find(vGML.id) != end(globalIddFromExtID))
				return Result<>::Error("Vertex id " + std::to_string(vGML.id) + " used multiple times.");
			globalIddFromExtID.emplace(vGML.id, i++);
		}
	}

	for(const auto &eGML: gGML.edges) {
		if(eGML.source == eGML.target)
			return Result<>::Error("Loop edge (on " + std::to_string(eGML.source) + ") is not allowed.");
		if(globalIddFromExtID.find(eGML.source) == end(globalIddFromExtID))
			return Result<>::Error("Source " + std::to_string(eGML.source) + " does not exist in '"
			                       + boost::lexical_cast<std::string>(eGML) + "'.");
		if(globalIddFromExtID.find(eGML.target) == end(globalIddFromExtID))
			return Result<>::Error("Target " + std::to_string(eGML.target) + " does not exist in '"
			                       + boost::lexical_cast<std::string>(eGML) + "'.");
	}

	// Now we can calculate connected components and start converting data

	ConnectedComponents components(gGML.vertices.size());
	for(const auto &e: gGML.edges) {
		const auto iterSrc = globalIddFromExtID.find(e.source);
		const auto iterTar = globalIddFromExtID.find(e.target);
		components.join(iterSrc->second, iterTar->second);
	}
	const auto numComponents = components.finalize();

	std::vector<Data> datas(numComponents);
	std::vector<std::unordered_map<lib::Graph::Vertex, int>> extIDFromVertex(numComponents);
	for(auto &d: datas) {
		d.g = std::make_unique<lib::Graph::GraphType>();
		d.pString = std::make_unique<lib::Graph::PropString>(*d.g);
	}

	const auto atError = [&datas](std::string msg) -> Result<> {
		for(auto &d: datas) d.reset();
		return Result<>::Error(std::move(msg));
	};

	for(const auto &vGML: gGML.vertices) {
		const auto comp = components[globalIddFromExtID.find(vGML.id)->second];
		auto &g = *datas[comp].g;
		const auto v = add_vertex(g);
		assert(vGML.label);
		datas[comp].pString->addVertex(v, *vGML.label);
		datas[comp].externalToInternalIds.emplace(vGML.id, get(boost::vertex_index_t(), g, v));
		extIDFromVertex[comp].emplace(v, vGML.id);
	}
	const auto vFromVertexId = [&](int id) {
		const auto globalIDIter = globalIddFromExtID.find(id);
		assert(globalIDIter != end(globalIddFromExtID));
		const auto comp = components[globalIDIter->second];
		const auto vIdIter = datas[comp].externalToInternalIds.find(id);
		assert(vIdIter != end(datas[comp].externalToInternalIds));
		return std::pair(comp, vertex(vIdIter->second, *datas[comp].g));
	};
	for(const auto &eGML: gGML.edges) {
		const auto [comp, vSrc] = vFromVertexId(eGML.source);
		const auto [compTar, vTar] = vFromVertexId(eGML.target);
		assert(comp == compTar);
		auto &g = *datas[comp].g;
		const auto eQuery = edge(vSrc, vTar, g);
		if(eQuery.second)
			return atError("Duplicate edge with source " + std::to_string(eGML.source)
			               + " and target " + std::to_string(eGML.target) + ".");
		const auto e = add_edge(vSrc, vTar, g);
		assert(eGML.label);
		datas[comp].pString->addEdge(e.first, *eGML.label);
	}

	bool doStereo = false;
	for(const auto &vGML: gGML.vertices) doStereo = doStereo || vGML.stereo;
	for(const auto &eGML: gGML.edges) doStereo = doStereo || eGML.stereo;
	if(!doStereo) return std::move(datas); // TODO: remove std::move when C++20/P1825R0 is available
	// Stereo
	//============================================================================
	std::vector<lib::Graph::PropMolecule> molStates;
	std::vector<lib::Stereo::Inference<lib::Graph::GraphType, lib::Graph::PropMolecule>> stereoInferences;
	molStates.reserve(numComponents);
	stereoInferences.reserve(numComponents);
	for(int i = 0; i != numComponents; ++i) {
		const auto &g = *datas[i].g;
		molStates.emplace_back(g, *datas[i].pString);
		stereoInferences.push_back(lib::Stereo::Inference(g, molStates.back(), false));
	}

	const auto &gGeometry = lib::Stereo::getGeometryGraph();
	// Set the explicitly defined edge categories.
	//----------------------------------------------------------------------------
	for(const auto &eGML: gGML.edges) {
		const auto [comp, vSrc] = vFromVertexId(eGML.source);
		const auto [compTar, vTar] = vFromVertexId(eGML.target);
		assert(comp == compTar);
		const auto &g = *datas[comp].g;
		const auto ePair = edge(vSrc, vTar, g);
		assert(ePair.second);
		if(!eGML.stereo) continue;
		const std::string &s = *eGML.stereo;
		if(s.size() != 1)
			return atError(
					"Error in stereo data for edge (" + std::to_string(eGML.source)
					+ ", " + std::to_string(eGML.target) + "). Parsing error in stereo data '" + s + "'.");
		lib::Stereo::EdgeCategory cat;
		switch(s.front()) {
		case '*':
			cat = lib::Stereo::EdgeCategory::Any;
			break;
		default:
			return atError("Error in stereo data for edge (" + std::to_string(eGML.source)
			               + ", " + std::to_string(eGML.target) + "). Parsing error in stereo data '" + s + "'.");
		}
		if(auto res = stereoInferences[comp].assignEdgeCategory(ePair.first, cat); !res)
			return atError("Error in stereo data for edge (" + std::to_string(eGML.source)
			               + ", " + std::to_string(eGML.target) + "). "
			               + res.extractError());
	}
	// Set the explicitly stereo data.
	//----------------------------------------------------------------------------
	for(auto &vGML: gGML.vertices) {
		if(!vGML.stereo) continue;
		const auto [comp, v] = vFromVertexId(vGML.id);
		if(auto res = lib::Stereo::Read::parseEmbedding(*vGML.stereo)) {
			vGML.parsedEmbedding = std::move(*res);
		} else {
			return atError("Error in stereo data for vertex " + std::to_string(vGML.id) + ". "
			               + res.extractError());
		}
		// Geometry
		//..........................................................................
		const auto &embGML = *vGML.parsedEmbedding;
		if(embGML.geometry) {
			const auto vGeo = gGeometry.findGeometry(*embGML.geometry);
			if(vGeo == gGeometry.nullGeometry())
				return atError("Error in stereo data for vertex " + std::to_string(vGML.id)
				               + ". Invalid geometry '" + *embGML.geometry + "'.");
			if(auto res = stereoInferences[comp].assignGeometry(v, vGeo); !res)
				return atError("Error in stereo data for vertex " + std::to_string(vGML.id) + ". "
				               + res.extractError());
		}
		// Edges
		//..........................................................................
		if(embGML.edges) {
			stereoInferences[comp].initEmbedding(v);
			for(const auto &e: *embGML.edges) {
				if(const int *idPtr = std::get_if<int>(&e)) {
					const auto extIDNeighbour = *idPtr;
					if(globalIddFromExtID.find(extIDNeighbour) == end(globalIddFromExtID))
						return atError("Neighbour vertex " + std::to_string(extIDNeighbour)
						               + " in stereo embedding for vertex " + std::to_string(vGML.id) + " does not exist.");
					const auto [compNeighbour, vNeighbour] = vFromVertexId(extIDNeighbour);
					const auto ePair = edge(v, vNeighbour, *datas[comp].g);
					if(!ePair.second)
						return atError("Error in graph GML. Vertex " + std::to_string(extIDNeighbour) +
						               " in stereo embedding for vertex " + std::to_string(vGML.id) + " is not a neighbour.");
					assert(compNeighbour == comp);
					stereoInferences[comp].addEdge(v, ePair.first);
				} else if(const char *virtPtr = std::get_if<char>(&e)) {
					switch(*virtPtr) {
					case 'e':
						stereoInferences[comp].addLonePair(v);
						break;
					case 'r':
						stereoInferences[comp].addRadical(v);
						break;
					default:
						MOD_ABORT; // the parser should know what is allowed, nope, not any more
//						return Result<>::Error(
//								"Error in graph GML. Virtual neighbour in stereo embedding for vertex " +
//								std::to_string(vId) + " in " + side + " has unknown type '" + *virtPtr + "'.");
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
			if(isFixed) stereoInferences[comp].fixSimpleGeometry(v);
		}
	} // end of explicit stereo data

	for(int comp = 0; comp != numComponents; ++comp) {
		// TODO: the warning should only be printed once, instead of for each connected component
		auto stereoResult = stereoInferences[comp].finalize(
				warnings, printStereoWarnings, [comp, &extIDFromVertex](lib::Graph::Vertex v) {
					const auto iter = extIDFromVertex[comp].find(v);
					assert(iter != extIDFromVertex[comp].end());
					return std::to_string(iter->second);
				});
		if(!stereoResult)
			return atError(stereoResult.extractError());
		datas[comp].pStereo = std::make_unique<lib::Graph::PropStereo>(*datas[comp].g, std::move(stereoInferences[comp]));
	}
	return std::move(datas); // TODO: remove std::move when C++20/P1825R0 is available
}

namespace {
namespace dfsDetail {
using namespace IO::DFS;
using Vertex = IO::DFS::Vertex;
using Edge = IO::DFS::Edge;

using GVertex = lib::Graph::Vertex;
using GEdge = lib::Graph::Edge;

struct JoinConnected {
	using ConvertRes = std::pair<const LabelVertex *, bool>;
public:
	JoinConnected(ConnectedComponents &components) : components(components) {}

	void operator()(const Chain &chain) {
		auto [prev, isRingClosure] = (*this)(chain.head, nullptr);
		assert(!isRingClosure);
		assert(prev);
		for(const EVPair &ev: chain.tail) {
			prev = (*this)(ev, prev);
			assert(prev);
		}
	}

	ConvertRes operator()(const Vertex &vertex, const LabelVertex *prev) {
		const auto [subNext, subIsRingClosure] = boost::apply_visitor(*this, vertex.vertex);
		assert(!(subIsRingClosure && prev == nullptr));
		const auto *branchRoot = subIsRingClosure ? prev : subNext;
		for(const Branch &branch: vertex.branches) {
			const auto *branchPrev = branchRoot;
			for(const EVPair &ev: branch.tail) {
				branchPrev = (*this)(ev, branchPrev);
				assert(branchPrev);
			}
		}
		return {subNext, subIsRingClosure};
	}

	const LabelVertex *operator()(const EVPair &ev, const LabelVertex *prev) {
		const auto [next, isRingClosure] = (*this)(ev.second, prev);
		join(prev, next, ev.first.label);
		if(isRingClosure) return prev;
		else return next;
	}

	ConvertRes operator()(const LabelVertex &vertex) {
		if(vertex.ringClosure)
			join(&vertex, vertex.ringClosure, "-");
		return {&vertex, false};
	}

	ConvertRes operator()(const RingClosure &vertex) {
		return {vertex.other, true};
	}

private:
	void join(const LabelVertex *src, const LabelVertex *tar, const std::string &label) {
		if(label.empty()) return; // a dot edge for no-edge
		components.join(src->connectedComponentID, tar->connectedComponentID);
	}

private:
	ConnectedComponents &components;
};

struct Converter {
	struct ConvertRes {
		int component;
		GVertex next;
		bool isRingClosure;
	};
public:
	Converter(std::vector<std::unique_ptr<lib::Graph::GraphType>> &gPtrs,
	          std::vector<std::unique_ptr<lib::Graph::PropString>> &pStringPtrs,
	          const ConnectedComponents &components)
			: gPtrs(gPtrs), pStringPtrs(pStringPtrs), components(components) {}

	void operator()(Chain &chain) {
		const auto sub = (*this)(chain.head, -1, lib::Graph::GraphType::null_vertex());
		int component = sub.component;
		GVertex vPrev = sub.next;
		assert(!sub.isRingClosure);
		assert(vPrev != lib::Graph::GraphType::null_vertex());
		for(EVPair &ev: chain.tail) {
			std::tie(component, vPrev) = (*this)(ev, component, vPrev);
			assert(vPrev != lib::Graph::GraphType::null_vertex());
		}
	}

	ConvertRes operator()(Vertex &vertex, int component, GVertex prev) {
		const auto sub = boost::apply_visitor(*this, vertex.vertex);
		assert(!(sub.isRingClosure && prev == lib::Graph::GraphType::null_vertex()));
		const GVertex branchRoot = sub.isRingClosure ? prev : sub.next;
		const int branchRootComponent = sub.isRingClosure ? component : sub.component;
		for(Branch &branch: vertex.branches) {
			int componentPrev = branchRootComponent;
			GVertex branchPrev = branchRoot;
			for(EVPair &ev: branch.tail) {
				std::tie(componentPrev, branchPrev) = (*this)(ev, componentPrev, branchPrev);
				assert(branchPrev != lib::Graph::GraphType::null_vertex());
			}
		}
		return sub;
	}

	std::pair<int, GVertex> operator()(EVPair &ev, int component, GVertex prev) {
		const auto res = (*this)(ev.second, component, prev);
		makeEdge(component, prev, res.component, res.next, ev.first.label);
		if(res.isRingClosure) return {component, prev};
		else return {res.component, res.next};
	}

	ConvertRes operator()(LabelVertex &vDFS) {
		const int component = components[vDFS.connectedComponentID];
		const GVertex v = add_vertex(*gPtrs[component]);
		pStringPtrs[component]->addVertex(v, vDFS.label);
		if(vDFS.ringClosure) {
			const auto componentRing = components[vDFS.ringClosure->connectedComponentID];
			const auto vRing = vertex(vDFS.ringClosure->gVertexId, *gPtrs[componentRing]);
			makeEdge(component, v, componentRing, vRing, "-");
		}
		vDFS.gVertexId = get(boost::vertex_index_t(), *gPtrs[component], v);
		return {component, v, false};
	}

	ConvertRes operator()(RingClosure &rc) {
		const int component = components[rc.other->connectedComponentID];
		return {component, vertex(rc.other->gVertexId, *gPtrs[component]), true};
	}

private:
	void makeEdge(int srcComponent, GVertex vSrc, int tarComponent, GVertex vTar, const std::string &label) {
		if(label.empty()) return; // a dot edge for no-edge
		assert(srcComponent == tarComponent);
		assert(vSrc != vTar);
		assert(!edge(vSrc, vTar, *gPtrs[srcComponent]).second);
		std::pair<GEdge, bool> e = add_edge(vSrc, vTar, *gPtrs[srcComponent]);
		assert(e.second);
		pStringPtrs[srcComponent]->addEdge(e.first, label);
	}

private:
	std::vector<std::unique_ptr<lib::Graph::GraphType>> &gPtrs;
	std::vector<std::unique_ptr<lib::Graph::PropString>> &pStringPtrs;
	const ConnectedComponents &components;
};

struct ImplicitHydrogenAdder {
	ImplicitHydrogenAdder(std::vector<std::unique_ptr<lib::Graph::GraphType>> &gPtrs,
	                      std::vector<std::unique_ptr<lib::Graph::PropString>> &pStringPtrs,
	                      const ConnectedComponents &components)
			: gPtrs(gPtrs), pStringPtrs(pStringPtrs), components(components) {}

	void operator()(const Chain &chain) {
		(*this)(chain.head);
		for(const auto &ev: chain.tail) (*this)(ev.second);
	}

	void operator()(const Vertex &vertex) {
		boost::apply_visitor(*this, vertex.vertex);
		for(const auto &b: vertex.branches)
			for(const auto &ev: b.tail) (*this)(ev.second);
	}

	void operator()(const RingClosure &) {}

	void operator()(const LabelVertex &vDFS) {
		if(!vDFS.implicit) return;
		// we can only add hydrogens if all incident edges are valid bonds
		const int component = components[vDFS.connectedComponentID];
		auto &g = *gPtrs[component];
		const auto gVertex = vertex(vDFS.gVertexId, g);
		auto &pString = *pStringPtrs[component];
		for(auto eOut: asRange(out_edges(gVertex, g)))
			if(lib::Chem::decodeEdgeLabel(pString[eOut]) == BondType::Invalid)
				return;
		const auto atomId = lib::Chem::atomIdFromSymbol(vDFS.label);
		const auto iter = std::find(begin(lib::Chem::getSmilesOrganicSubset()),
		                            end(lib::Chem::getSmilesOrganicSubset()), atomId);
		if(iter == end(lib::Chem::getSmilesOrganicSubset()))
			MOD_ABORT;
		const auto hydrogenAdder = [](lib::Graph::GraphType &g, lib::Graph::PropString &pString,
		                              lib::Graph::Vertex p) {
			const GVertex v = add_vertex(g);
			pString.addVertex(v, "H");
			const GEdge e = add_edge(v, p, g).first;
			pString.addEdge(e, "-");
		};
		lib::Chem::addImplicitHydrogens(g, pString, gVertex, atomId, hydrogenAdder);
	}

private:
	std::vector<std::unique_ptr<lib::Graph::GraphType>> &gPtrs;
	std::vector<std::unique_ptr<lib::Graph::PropString>> &pStringPtrs;
	const ConnectedComponents &components;
};

} // namespace dfsDetail
} // namespace

Result<std::vector<Data>> dfs(lib::IO::Warnings &warnings, std::string_view src) {
	auto astRes = lib::IO::DFS::Read::graph(src);
	if(!astRes) return lib::IO::Result<>::Error(astRes.extractError());
	auto &[astPtr, numVertices, vertexFromId] = *astRes;
	auto &ast = *astPtr;

	ConnectedComponents components(numVertices);
	(dfsDetail::JoinConnected(components)(ast));
	const int numComponents = components.finalize();
	for(int i = 0; i != numVertices; ++i) {
		assert(components[i] >= 0);
		assert(components[i] < numComponents);
	}

	std::vector<std::unique_ptr<lib::Graph::GraphType>> gPtrs(numComponents);
	std::vector<std::unique_ptr<lib::Graph::PropString>> pStringPtrs(numComponents);
	for(int i = 0; i != numComponents; ++i) {
		gPtrs[i] = std::make_unique<lib::Graph::GraphType>();
		pStringPtrs[i] = std::make_unique<lib::Graph::PropString>(*gPtrs[i]);
	}

	dfsDetail::Converter(gPtrs, pStringPtrs, components)(ast);
	dfsDetail::ImplicitHydrogenAdder(gPtrs, pStringPtrs, components)(ast);
	std::vector<Read::Data> datas(numComponents);
	for(int i = 0; i != numComponents; ++i) {
		assert(gPtrs[i]);
		datas[i].g = std::move(gPtrs[i]);
		datas[i].pString = std::move(pStringPtrs[i]);
	}
	for(auto &&vp: vertexFromId) {
		const int component = components[vp.second->connectedComponentID];
		datas[component].externalToInternalIds[vp.first] = vp.second->gVertexId;
	}
	return std::move(datas); // TODO: remove std::move when C++20/P1825R0 is available
}

Result<std::vector<Data>> smiles(lib::IO::Warnings &warnings, std::string_view src, bool printStereoWarnings,
                                 bool allowAbstract, SmilesClassPolicy classPolicy) {
	return lib::Chem::readSmiles(warnings, printStereoWarnings, src, allowAbstract, classPolicy);
}

Result<std::vector<Data>> MDLMOL(lib::IO::Warnings &warnings, std::string_view src, const MDLOptions &options) {
	return lib::Chem::readMDLMOL(warnings, src, options);
}

Result<std::vector<std::vector<Data>>>
MDLSD(lib::IO::Warnings &warnings, std::string_view src, const MDLOptions &options) {
	return lib::Chem::readMDLSD(warnings, src, options);
}

} // namespace mod::lib::Graph::Read