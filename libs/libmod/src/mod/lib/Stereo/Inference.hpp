#ifndef MOD_LIB_STEREO_INFERENCE_HPP
#define MOD_LIB_STEREO_INFERENCE_HPP

#include <mod/Error.hpp>

#include <mod/lib/IO/Result.hpp>
#include <mod/lib/Stereo/EdgeCategory.hpp>
#include <mod/lib/Stereo/EmbeddingEdge.hpp>
#include <mod/lib/Stereo/GeometryGraph.hpp>
#include <mod/lib/Stereo/Configuration/Configuration.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <boost/graph/graph_traits.hpp>
#include <boost/lexical_cast.hpp>

namespace mod::lib::Stereo {

struct InferenceVertexData {
	GeometryGraph::Vertex vGeometry = GeometryGraph::nullGeometry();
	int nextAvailableVirtual;
	std::vector<EmbeddingEdge> edges;
	bool explicitEmbedding = false;
	Fixation fix = Fixation::free();
public:
	std::unique_ptr<Configuration> configuration;
};

struct InferenceEdgeData {
	EdgeCategorySubset valid = EdgeCategorySubset().all();
	EdgeCategory finalCategory; // will be set by finalize
};

template<typename Graph, typename PropMolecule>
struct Inference {
	using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
	using Edge = typename boost::graph_traits<Graph>::edge_descriptor;
public:
	Inference(const Graph &g, PropMolecule &&) = delete;

	Inference(const Graph &g, const PropMolecule &pMolecule, bool asPattern)
			: g(g), pMolecule(pMolecule), asPattern(asPattern),
			  vertexData(num_vertices(g)), edgeData(num_edges(g)) {
		for(const auto v: asRange(vertices(g)))
			vertexData[get(boost::vertex_index_t(), g, v)].nextAvailableVirtual = out_degree(v, g);
	}

	lib::IO::Result<> assignGeometry(Vertex v, GeometryGraph::Vertex vGeometry) {
		assert(vGeometry != GeometryGraph::nullGeometry());
		const auto vId = get(boost::vertex_index_t(), g, v);
		auto &data = vertexData[vId];
		if(data.vGeometry != GeometryGraph::nullGeometry())
			return lib::IO::Result<>::Error("Geometry already assigned.");
		data.vGeometry = vGeometry;
		return {};
	}

	lib::IO::Result<> assignEdgeCategory(Edge e, EdgeCategory cat) {
		const auto eId = get(boost::edge_index_t(), g, e);
		auto &data = edgeData[eId];
		if(!data.valid(cat)) {
			return lib::IO::Result<>::Error(
					"Can not assign '" + boost::lexical_cast<std::string>(cat)
					+ "'. Not a valid possibility (valid="
					+ boost::lexical_cast<std::string>(data.valid) + ").");
		}
		data.valid = EdgeCategorySubset() + cat;
		return {};
	}

	void initEmbedding(Vertex v) {
		const auto vId = get(boost::vertex_index_t(), g, v);
		auto &data = vertexData[vId];
		data.explicitEmbedding = true;
	}

	void addEdge(Vertex v, Edge eOut) {
		const auto vId = get(boost::vertex_index_t(), g, v);
		auto &data = vertexData[vId];
		data.explicitEmbedding = true;
		const auto outRange = out_edges(v, g);
		const auto iter = std::find(outRange.first, outRange.second, eOut);
		assert(iter != outRange.second);
		// Add the edge with undefined category. It will be assigned properly in finalization.
		data.edges.emplace_back(std::distance(outRange.first, iter), EmbeddingEdge::Type::Edge, EdgeCategory::Undefined);
	}

	void addLonePair(Vertex v) {
		const auto vId = get(boost::vertex_index_t(), g, v);
		auto &data = vertexData[vId];
		data.explicitEmbedding = true;
		const auto offset = data.nextAvailableVirtual;
		++data.nextAvailableVirtual;
		data.edges.emplace_back(offset, EmbeddingEdge::Type::LonePair, EdgeCategory::Single);
	}

	void addRadical(Vertex v) {
		const auto vId = get(boost::vertex_index_t(), g, v);
		auto &data = vertexData[vId];
		data.explicitEmbedding = true;
		const auto offset = data.nextAvailableVirtual;
		++data.nextAvailableVirtual;
		data.edges.emplace_back(offset, EmbeddingEdge::Type::Radical, EdgeCategory::Single);
	}

	void fixSimpleGeometry(Vertex v) {
		const auto vId = get(boost::vertex_index_t(), g, v);
		auto &data = vertexData[vId];
		assert(data.explicitEmbedding); // the embedding should have been initialised at this point
		data.fix = Fixation::simpleFixed();
	}

	template<typename VertexPrinter>
	lib::IO::Result<> finalize(lib::IO::Warnings &warnings, bool printWarnings, VertexPrinter vertexPrinter) {
		const auto &geo = getGeometryGraph();
		const auto &gGeometry = geo.getGraph();
		assert(!hasFinalized);
		// Finalize edge categories.
		//--------------------------------------------------------------------------
		for(const auto e: asRange(edges(g))) {
			const auto eId = get(boost::edge_index_t(), g, e);
			auto &data = edgeData[eId];
			const EdgeCategorySubset &eCat = data.valid;
			if(eCat.count() == 0) MOD_ABORT; // should have been handled earlier
			if(eCat.count() == 1) {
				data.finalCategory = eCat.selectFirst();
			} else {
				// assume chemical
				const auto eCatChem = bondTypeToEdgeCategory(pMolecule[e]);
				if(!eCat(eCatChem)) {
					MOD_ABORT; // TODO: implement
				}
				data.finalCategory = eCatChem;
			}
		}

		// Assign edge categories to embedding edges.
		//--------------------------------------------------------------------------
		for(const auto v: asRange(vertices(g))) {
			const auto vId = get(boost::vertex_index_t(), g, v);
			for(auto &emb: vertexData[vId].edges) {
				if(emb.type != EmbeddingEdge::Type::Edge) continue;
				const auto e = emb.getEdge(v, g);
				emb.cat = edgeData[get(boost::edge_index_t(), g, e)].finalCategory;
			}
		}

		for(const auto v: asRange(vertices(g))) {
			auto res = finalizeVertex(warnings, printWarnings, v, vertexPrinter);
			if(!res) return res;
		}

		// Construct the configurations.
		//--------------------------------------------------------------------------
		for(const auto v: asRange(vertices(g))) {
			const auto vId = get(boost::vertex_index_t(), g, v);
			auto &data = vertexData[vId];
			assert(data.vGeometry != GeometryGraph::nullGeometry());
			assert(!data.configuration);
			std::stringstream ssErr;
			data.configuration = gGeometry[data.vGeometry].constructor(
					data.edges.data(), data.edges.data() + data.edges.size(), data.fix, ssErr);
			if(!data.configuration)
				return lib::IO::Result<>::Error("Error in configuration construction for vertex " + vertexPrinter(v) + ".\n"
				                                + ssErr.str());
		}
		hasFinalized = true;
		return {};
	}

private:
	EdgeCategoryCount addEdgesFromGraph(Vertex v) {
		const auto vId = get(boost::vertex_index_t(), g, v);
		auto &data = vertexData[vId];
		const auto d = out_degree(v, g);
		assert(data.edges.empty());
		data.edges.reserve(d);
		std::size_t offset = 0;
		EdgeCategoryCount catCount;
		for(const auto eOut: asRange(out_edges(v, g))) {
			data.edges.emplace_back(offset, EmbeddingEdge::Type::Edge,
			                        edgeData[get(boost::edge_index_t(), g, eOut)].finalCategory);
			++catCount[edgeData[get(boost::edge_index_t(), g, eOut)].finalCategory];
			++offset;
		}
		return catCount;
	}

	void addLonePairs(Vertex v, std::size_t n) {
		auto vId = get(boost::vertex_index_t(), g, v);
		auto &data = vertexData[vId];
		data.edges.reserve(data.edges.size() + n);
		for(std::size_t i = 0; i < n; ++i) {
			data.edges.emplace_back(data.edges.size(), EmbeddingEdge::Type::LonePair, EdgeCategory::Single);
		}
	}

	template<typename VertexPrinter>
	lib::IO::Result<> finalizeVertex(lib::IO::Warnings &warnings, bool printWarnings,
	                                 Vertex v, VertexPrinter vertexPrinter) {
		const auto &geo = getGeometryGraph();
		const auto vId = get(boost::vertex_index_t(), g, v);
		auto &data = vertexData[vId];
		AtomData ad = pMolecule[v];
		EdgeCategoryCount catCount;

		bool explicitGeometry = data.vGeometry != GeometryGraph::nullGeometry();
		bool explicitEmbedding = data.explicitEmbedding;

		std::size_t numLonePairs = 0;
		bool radical = false;
		if(!explicitEmbedding) {
			catCount = addEdgesFromGraph(v);
			// TODO: shouldn't we deduce radical?
		} else {
			// check if all real edges are there, once
			std::vector<bool> offsetsUsed(data.edges.size(), false);
			int edgeCount = 0;
			for(const auto &emb: data.edges) {
				if(emb.type != EmbeddingEdge::Type::Edge) continue;
				assert(emb.offset < out_degree(v, g));
				if(offsetsUsed[emb.offset])
					return lib::IO::Result<>::Error(
							"Duplicate edge in stereo embedding for vertex " + vertexPrinter(v) + ".");
				offsetsUsed[emb.offset] = true;
				++edgeCount;
			}
			if(edgeCount != out_degree(v, g)) {
				assert(edgeCount < out_degree(v, g));
				return lib::IO::Result<>::Error(
						"Too few edges in stereo embedding for vertex " + vertexPrinter(v) + ". Got " +
						std::to_string(edgeCount)
						+ " edges, but the degree is " + std::to_string(out_degree(v, g)) + "."
				);
			}

			// and now count stuff
			for(const auto &emb: data.edges) {
				assert(emb.offset < data.edges.size());
				switch(emb.type) {
				case EmbeddingEdge::Type::Edge:
					++catCount[emb.cat];
					break;
				case EmbeddingEdge::Type::LonePair:
					assert(
							emb.offset >= out_degree(v, g)); // should not happen, [0, d[ are reserved for the real edges
					++numLonePairs;
					break;
				case EmbeddingEdge::Type::Radical:
					assert(
							emb.offset >= out_degree(v, g)); // should not happen, [0, d[ are reserved for the real edges
					if(radical)
						return lib::IO::Result<>::Error(
								"Multiple radicals in stereo embedding for vertex " + vertexPrinter(v) + ".");
					radical = true;
					break;
				}
			}
		}
		if(explicitGeometry && explicitEmbedding) {
			// the user has spoken
		} else if(explicitGeometry && !explicitEmbedding) {
			if(auto res = geo.deduceLonePairs(warnings, printWarnings, ad, catCount, data.vGeometry, asPattern))
				numLonePairs = *res;
			else return std::move(res);
		} else if(!explicitGeometry && explicitEmbedding) {
			if(auto res = geo.deduceGeometry(warnings, printWarnings, ad, catCount, numLonePairs, asPattern))
				data.vGeometry = *res;
			else return std::move(res);
		} else {
			assert(!explicitGeometry && !explicitEmbedding);
			if(auto res = geo.deduceGeometryAndLonePairs(warnings, printWarnings, ad, catCount, asPattern))
				std::tie(data.vGeometry, numLonePairs) = *res;
			else return std::move(res);
		}
		if(!explicitEmbedding) addLonePairs(v, numLonePairs);
		return {};
	}

public: // if hasFinalized
	std::unique_ptr<Configuration> extractConfiguration(Vertex v) {
		assert(hasFinalized);
		const auto vId = get(boost::vertex_index_t(), g, v);
		assert(vertexData[vId].configuration);
		return std::move(vertexData[vId].configuration);
	}

	EdgeCategory getEdgeCategory(Edge e) const {
		assert(hasFinalized);
		const auto eId = get(boost::edge_index_t(), g, e);
		assert(eId < num_edges(g));
		return edgeData[eId].finalCategory;
	}

public:
	const Graph &g;
	const PropMolecule &pMolecule;
	const bool asPattern;
private:
	bool hasFinalized = false;
	std::vector<InferenceVertexData> vertexData;
	std::vector<InferenceEdgeData> edgeData;
};

} // namespace mod::lib::Stereo

#endif // MOD_LIB_STEREO_INFERENCE_HPP