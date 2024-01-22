#ifndef MOD_LIB_STEREO_GEOMETRY_GRAPH_HPP
#define MOD_LIB_STEREO_GEOMETRY_GRAPH_HPP

#include <mod/Chem.hpp>
#include <mod/lib/IO/Result.hpp>
#include <mod/lib/Stereo/EdgeCategory.hpp>

#include <boost/graph/adjacency_list.hpp>

#include <functional>
#include <tuple>
#include <vector>

namespace mod::lib::Stereo {
struct Configuration;
struct EmbeddingEdge;
struct Fixation;

enum struct DeductionResult {
	Success, Warning, Error
};

struct GeometryGraph {
	struct VProp {
		using Constructor = std::function<std::unique_ptr<Configuration>(const EmbeddingEdge *, const EmbeddingEdge *,
		                                                                 const Fixation &, std::ostream &)>;
	public:
		VProp() = default; // because BGL requires it
		VProp(const std::string &name);
	public:
		std::string name;
		Constructor constructor;
	};

	struct EProp {
	};
	using GraphType = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, VProp, EProp>;
	using Vertex = boost::graph_traits<GraphType>::vertex_descriptor;
	using Edge = boost::graph_traits<GraphType>::edge_descriptor;

	struct ChemValid {
		AtomId atomId;
		Charge charge;
		bool radical;
		EdgeCategoryCount catCount; // only the bonds, and Undefined must be 0
		std::size_t lonePair;
		Vertex geometry;
	};
private:
	friend const GeometryGraph &getGeometryGraph();
	GeometryGraph();
	GeometryGraph(const GeometryGraph &) = delete;
	GeometryGraph(GeometryGraph &&) = delete;
	GeometryGraph &operator=(const GeometryGraph &) = delete;
	GeometryGraph &operator=(GeometryGraph &&) = delete;
public:
	const GraphType &getGraph() const;
	Vertex findGeometry(const std::string &name) const;
	static Vertex nullGeometry();
public: // deduction
	lib::IO::Result<unsigned char>
	deduceLonePairs(lib::IO::Warnings &warnings, bool printWarnings,
	                const AtomData &ad, const EdgeCategoryCount &catCount, Vertex vGeometry,
	                bool asPattern) const;
	lib::IO::Result<Vertex>
	deduceGeometry(lib::IO::Warnings &warnings, bool printWarnings,
	               const AtomData &ad, const EdgeCategoryCount &catCount,
	               unsigned char numLonePairs, bool asPattern) const;
	lib::IO::Result<std::tuple<Vertex, unsigned char>>
	deduceGeometryAndLonePairs(lib::IO::Warnings &warnings, bool printWarnings,
	                           const AtomData &ad, const EdgeCategoryCount &catCount,
	                           bool asPattern) const;
public: // matching
	bool isAncestorOf(Vertex ancestor, Vertex child) const; // true also if child == ancestor
	Vertex generalize(Vertex a, Vertex b) const;
	Vertex unify(Vertex a, Vertex b) const;
	Vertex pushoutComplement(Vertex a, Vertex b,
	                         Vertex d) const; // a -> b -> d, find c, st. a -> c -> d is the pushout complement
private:
	Vertex addGeometry(VProp &&vProp) const;
	Vertex addChild(Vertex vParent, VProp &&vProp);
private:
	mutable GraphType g;
	mutable std::map<std::string, Vertex> nameToVertex;
public:
	std::vector<ChemValid> chemValids;
public:
	Vertex any;
	Vertex linear;
	Vertex trigonalPlanar;
	Vertex tetrahedral;
};

const GeometryGraph &getGeometryGraph(); // singleton

} // namespace  mod::lib::Stereo

#endif // MOD_LIB_STEREO_GEOMETRY_GRAPH_HPP