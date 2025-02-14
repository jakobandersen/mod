#ifndef MOD_LIG_STEREO_IO_WRITECONFIGURATION_HPP
#define MOD_LIG_STEREO_IO_WRITECONFIGURATION_HPP

#include <mod/Post.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/IO/GraphWrite.hpp>
#include <mod/lib/IO/GraphWriteGeneric.hpp>
#include <mod/lib/Stereo/Configuration/Configuration.hpp>

#include <jla_boost/graph/EdgeIndexedAdjacencyList.hpp>

#include <map>
#include <string>
#include <vector>

namespace mod::lib::Stereo::Write {

template<typename GraphInner>
std::string coords(const GraphInner &gStereo, const Configuration &conf, const std::string &name,
                   std::map<typename boost::graph_traits<GraphInner>::vertex_descriptor, int> &vMap) {
	post::FileHandle s(IO::makeUniqueFilePrefix() + name + "_coord.tex");
	std::vector<std::size_t> vIds(num_vertices(gStereo));
	using SVertex = typename boost::graph_traits<GraphInner>::vertex_descriptor;
	for(SVertex vStereo: asRange(vertices(gStereo))) {
		auto vId = get(boost::vertex_index_t(), gStereo, vStereo);
		auto iter = vMap.find(vStereo);
		assert(iter != end(vMap));
		if(iter->second == -1) vIds[vIds.size() - 1] = vId;
		else vIds[iter->second] = vId;
	}
	conf.printCoords(s, vIds);
	return s;
}

template<typename GraphPrint, typename Depict, typename ShownId>
std::pair<std::string, std::string>
tikz(const GraphPrint &g, typename boost::graph_traits<GraphPrint>::vertex_descriptor v,
     const Configuration &conf, const std::string &name, const Depict &depict,
     const IO::Graph::Write::Options &options, ShownId shownId) {
	const bool printLonePairs = true;
	using GVertex = typename boost::graph_traits<GraphPrint>::vertex_descriptor;
	using GEdge = typename boost::graph_traits<GraphPrint>::edge_descriptor;

	using GraphStereo = jla_boost::EdgeIndexedAdjacencyList<boost::undirectedS>;
	using SVertex = boost::graph_traits<GraphStereo>::vertex_descriptor;
	using SEdge = boost::graph_traits<GraphStereo>::edge_descriptor;

	// we make a new graph with copies and then the extra lone pairs
	GraphStereo gStereo;
	std::map<SVertex, int> vMap; // the order id, though -1 => the center
	SVertex vCenter = add_vertex(gStereo);
	vMap[vCenter] = -1;
	std::map<std::pair<SVertex, SVertex>, IO::Graph::Write::EdgeFake3DType> edgeDepiction;
	for(std::size_t i = 0; i < conf.degree(); ++i) {
		SVertex vStereo = add_vertex(gStereo);
		vMap[vStereo] = i;
		add_edge(vCenter, vStereo, gStereo);
		auto edge3Dtype = conf.getEdgeDepiction(i);
		edgeDepiction[std::make_pair(vCenter, vStereo)] = edge3Dtype;
		edgeDepiction[std::make_pair(vStereo, vCenter)] = IO::Graph::Write::invertEdgeFake3DType(edge3Dtype);
	}

	std::string coordFile = coords(gStereo, conf, name, vMap);
	post::FileHandle s(IO::makeUniqueFilePrefix() + name + ".tex");

	struct DepictorAndAdvOptions {
		DepictorAndAdvOptions(const GraphPrint &gOuter, GVertex vOuterCenter, const GraphStereo &g,
		                      const Depict &depict, bool printLonePairs, const std::map<SVertex, int> &vMap,
		                      const Configuration &conf,
		                      const std::map<std::pair<SVertex, SVertex>, IO::Graph::Write::EdgeFake3DType> &edgeDepiction,
		                      ShownId shownId)
				: gOuter(gOuter), vOuterCenter(vOuterCenter),
				  nullVertexOuter(boost::graph_traits<GraphPrint>::null_vertex()), gInner(g),
				  depict(depict), printLonePairs(printLonePairs), vMap(vMap), conf(conf), edgeDepiction(edgeDepiction),
				  shownId(shownId) {}

		GVertex getOuterVertexFromInnerVertex(SVertex vInner) const {
			auto iter = vMap.find(vInner);
			assert(iter != end(vMap));
			if(iter->second == -1) return vOuterCenter;
			const auto &emb = conf.begin()[iter->second];
			if(emb.type != EmbeddingEdge::Type::Edge) return nullVertexOuter;
			auto eOuter = emb.getEdge(vOuterCenter, gOuter);
			return target(eOuter, gOuter);
		}

		GEdge getOuterEdgeFromInnerEdge(SEdge eInner) const {
			GVertex vSrcOuter = getOuterVertexFromInnerVertex(source(eInner, gInner));
			GVertex vTarOuter = getOuterVertexFromInnerVertex(target(eInner, gInner));
			assert(vSrcOuter != nullVertexOuter);
			assert(vTarOuter != nullVertexOuter);
			auto p = edge(vSrcOuter, vTarOuter, gOuter);
			assert(p.second);
			return p.first;
		}

		unsigned char getAtomId(SVertex vInner) const {
			GVertex vOuter = getOuterVertexFromInnerVertex(vInner);
			if(vOuter == nullVertexOuter) return AtomIds::Invalid;
			else return depict.getAtomId(vOuter);
		}

		Isotope getIsotope(SVertex vInner) const {
			GVertex vOuter = getOuterVertexFromInnerVertex(vInner);
			if(vOuter == nullVertexOuter) return Isotope();
			else return depict.getIsotope(vOuter);
		}

		signed char getCharge(SVertex vInner) const {
			GVertex vOuter = getOuterVertexFromInnerVertex(vInner);
			if(vOuter == nullVertexOuter) return 0;
			else return depict.getCharge(vOuter);
		}

		bool getRadical(SVertex vInner) const {
			GVertex vOuter = getOuterVertexFromInnerVertex(vInner);
			if(vOuter == nullVertexOuter) return false;
			else return depict.getRadical(vOuter);
		}

		BondType getBondData(SEdge eInner) const {
			GVertex vSrcOuter = getOuterVertexFromInnerVertex(source(eInner, gInner));
			GVertex vTarOuter = getOuterVertexFromInnerVertex(target(eInner, gInner));
			if(vSrcOuter == nullVertexOuter || vTarOuter == nullVertexOuter) return BondType::Invalid;
			else return depict.getBondData(getOuterEdgeFromInnerEdge(eInner));
		}

		AtomData operator()(SVertex v) const {
			GVertex vOuter = getOuterVertexFromInnerVertex(v);
			if(vOuter == nullVertexOuter) return AtomData();
			else return depict(vOuter);
		}

		BondType operator()(SEdge eInner) const {
			GVertex vSrcOuter = getOuterVertexFromInnerVertex(source(eInner, gInner));
			GVertex vTarOuter = getOuterVertexFromInnerVertex(target(eInner, gInner));
			if(vSrcOuter == nullVertexOuter || vTarOuter == nullVertexOuter) return BondType::Invalid;
			else return depict(getOuterEdgeFromInnerEdge(eInner));
		}

		std::string getVertexLabelNoIsotopeChargeRadical(SVertex vInner) const {
			GVertex vOuter = getOuterVertexFromInnerVertex(vInner);
			if(vOuter != nullVertexOuter)return depict.getVertexLabelNoIsotopeChargeRadical(vOuter);
			auto iter = vMap.find(vInner);
			assert(iter != end(vMap));
			assert(iter->second != -1);
			const auto &emb = conf.begin()[iter->second];
			switch(emb.type) {
			case EmbeddingEdge::Type::Edge:
				MOD_ABORT;
			case EmbeddingEdge::Type::LonePair:
				return "e";
			case EmbeddingEdge::Type::Radical:
				return "r";
			}
			MOD_ABORT;
		}

		std::string getEdgeLabel(SEdge eInner) const {
			GVertex vSrcOuter = getOuterVertexFromInnerVertex(source(eInner, gInner));
			GVertex vTarOuter = getOuterVertexFromInnerVertex(target(eInner, gInner));
			if(vSrcOuter == nullVertexOuter || vTarOuter == nullVertexOuter) return "";
			else return depict.getEdgeLabel(getOuterEdgeFromInnerEdge(eInner));
		}

		bool hasImportantStereo(SVertex vInner) const {
			return true;
		}

		bool getHasCoordinates() const {
			return false;
		}

		double getX(SVertex v, bool b) const {
			return 0;
		}

		double getY(SVertex v, bool b) const {
			return 0;
		}

		bool isVisible(SVertex v) const {
			if(printLonePairs) return true;
			else
				MOD_ABORT;
			return true;
		}

		std::string getColour(SVertex) const {
			return "";
		}

		std::string getColour(SEdge) const {
			return "";
		}

		std::string getShownId(SVertex vInner) const {
			GVertex vOuter = getOuterVertexFromInnerVertex(vInner);
			if(vOuter == nullVertexOuter) MOD_ABORT;
			else return boost::lexical_cast<std::string>(shownId(gOuter, vOuter));
		}

		bool overwriteWithIndex(SVertex vInner) const {
			auto vOuter = getOuterVertexFromInnerVertex(vInner);
			return vOuter == nullVertexOuter;
		}

		IO::Graph::Write::EdgeFake3DType getEdgeFake3DType(SEdge eInner, bool withHydrogen) const {
			auto iter = edgeDepiction.find(std::make_pair(source(eInner, gInner), target(eInner, gInner)));
			assert(iter != end(edgeDepiction));
			return iter->second;
		}

		std::string getRawStereoString(SVertex vInner) const {
			return "";
		}

		std::string getPrettyStereoString(SVertex vInner) const {
			return "";
		}

		std::string getStereoString(SEdge eInner) const {
			return "";
		}

		std::string getEdgeAnnotation(SEdge eInner) const {
			SVertex vSrcInner = source(eInner, gInner), vTarInner = target(eInner, gInner);
			auto iterSrc = vMap.find(vSrcInner), iterTar = vMap.find(vTarInner);
			assert(iterSrc != end(vMap));
			assert(iterTar != end(vMap));
			assert(iterSrc->second == -1 || iterTar->second == -1);
			bool swapped = false;
			if(iterSrc->second != -1) {
				std::swap(vSrcInner, vTarInner);
				std::swap(iterSrc, iterTar);
				swapped = true;
			}
			std::string res;
			// if the edge category does not correspond to the bond type, then print it
			const EmbeddingEdge &emb = conf.begin()[iterTar->second];
			if(emb.type == EmbeddingEdge::Type::Edge) {
				auto eOuter = getOuterEdgeFromInnerEdge(eInner);
				auto eCatFromBt = bondTypeToEdgeCategory(depict.getBondData(eOuter));
				if(eCatFromBt != emb.cat) {
					res += " node[auto] {";
					res += boost::lexical_cast<std::string>(emb.cat);
					res += "}";
				}
			}

			// print the offset and whatever the configuration wants
			res += " node[auto, pos=";
			if(swapped) res += "0.25";
			else res += "0.75";
			res += "] {{\\tiny ";
			res += boost::lexical_cast<std::string>(iterTar->second);
			res += conf.getEdgeAnnotation(iterTar->second);
			res += "}} ";
			return res;
		}

		bool disallowHydrogenCollapse(SVertex) const {
			return false;
		}

		std::string getOpts(SVertex v) const {
			return std::string();
		}
	public:
		int getOutputId(SVertex vInner) const {
			return get(boost::vertex_index_t(), gInner, vInner);
		}
	private:
		const GraphPrint &gOuter;
		GVertex vOuterCenter, nullVertexOuter;
		const GraphStereo &gInner;
		const Depict &depict;
		bool printLonePairs;
		const std::map<SVertex, int> &vMap;
		const Configuration &conf;
		const std::map<std::pair<SVertex, SVertex>, IO::Graph::Write::EdgeFake3DType> &edgeDepiction;
		ShownId shownId;
	} depictAndAdvOptions(g, v, gStereo, depict, printLonePairs, vMap, conf, edgeDepiction, shownId);
	auto bonusWriter = [&](std::ostream &s) {
	};
	lib::IO::Graph::Write::tikz(s, options, gStereo, depictAndAdvOptions, coordFile, depictAndAdvOptions, bonusWriter,
	                            "");
	return std::pair<std::string, std::string>(s, coordFile);
}

template<typename Graph, typename Depict, typename ShownId>
std::string pdf(const Graph &g, typename boost::graph_traits<Graph>::vertex_descriptor v,
                const Configuration &conf, const std::string &name, const Depict &depict,
                const lib::IO::Graph::Write::Options &options, ShownId shownId) {
	const auto p = tikz(g, v, conf, name, depict, options, shownId);
	std::string fileTikz = p.first, fileCoords = p.second;
	std::string fileNoExt = fileTikz.substr(0, fileTikz.size() - 4);
	std::string fileCoordsNoExt = fileCoords.substr(0, fileCoords.size() - 4);
	IO::post() << "compileTikz \"" << fileNoExt << "\" \"" << fileCoordsNoExt << "\"" << std::endl;
	return fileNoExt + ".pdf";
}

} // namespace mod::lib::Stereo::Write

#endif // MOD_LIG_STEREO_IO_WRITECONFIGURATION_HPP
