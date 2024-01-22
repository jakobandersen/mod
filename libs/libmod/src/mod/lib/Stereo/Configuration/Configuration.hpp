#ifndef MOD_LIB_STEREO_CONFIGURATION_CONFIGURATION_HPP
#define MOD_LIB_STEREO_CONFIGURATION_CONFIGURATION_HPP

#include <mod/lib/Stereo/EmbeddingEdge.hpp>
#include <mod/lib/Stereo/GeometryGraph.hpp>

#include <iosfwd>

namespace mod::lib::IO::Graph::Write {
enum struct EdgeFake3DType;
} // namespace mod::lib::IO::Graph::Write
namespace mod::lib::Stereo {

struct Fixation {
	explicit Fixation(bool f); // simple
	bool asSimple() const;
	friend bool operator==(const Fixation &lhs, const Fixation &rhs);
	friend bool operator!=(const Fixation &lhs, const Fixation &rhs);
	friend std::ostream &operator<<(std::ostream &s, const Fixation &f);
private:
	bool simple;
public:
	static Fixation free();
	static Fixation simpleFixed();
};

struct Configuration {
	Configuration(const Configuration&) = delete;
	Configuration &operator=(const Configuration&) = delete;
protected:
	explicit Configuration(GeometryGraph::Vertex vGeometry, const EmbeddingEdge *first, const EmbeddingEdge *last);
public:
	virtual std::unique_ptr<Configuration> cloneFree(const GeometryGraph &g) const = 0;
	virtual std::unique_ptr<Configuration> clone(const GeometryGraph &g, const std::vector<std::size_t> &offsetMap) const = 0;
	virtual ~Configuration();
	GeometryGraph::Vertex getGeometryVertex() const;
	virtual Fixation getFixation() const;
	virtual const EmbeddingEdge *begin() const = 0;
	virtual const EmbeddingEdge *end() const = 0;
	std::size_t degree() const;

	int getNumLonePairs() const {
		return numLonePairs;
	}

	bool getHasRadical() const {
		return hasRadical;
	}
public: // checking
	// pre: dynamic type of this and other is the same
	virtual bool localPredIso(const Configuration &) const {
		return true;
	}

	// pre: dynamic type of this and other is the same
	virtual bool localPredSpec(const Configuration &) const {
		return true;
	}

	// TODO: this should be a kind of vtable constant
	virtual bool morphismStaticOk() const {
		return true;
	}

	// E.g., influenced by fixedness. (TODO: only fixedness? then we could maybe pull it into the base class)
	virtual bool morphismDynamicOk() const {
		return true;
	}

	// pre: dynamic type of this and cCodom is the same
	// perm:
	// - low numbers are actual mappings
	// - -1 means "to be mapped", e.g., for radicals and lone pairs
	// Semantically: is there a valid completion of the given partial permutation that makes the two structures isomorphic.
	virtual bool morphismIso(const Configuration &cCodom, std::vector<std::size_t> &perm) const = 0;
	// pre: the dynamic type of this must be an ancestor or the same as of cCodom
	virtual bool morphismSpec(const Configuration &cCodom, std::vector<std::size_t> &perm) const = 0;
public: // all these are implemented in IO
	virtual IO::Graph::Write::EdgeFake3DType getEdgeDepiction(std::size_t i) const;
	virtual void printCoords(std::ostream &s, const std::vector<std::size_t> &vIds) const = 0;
	virtual std::string getEdgeAnnotation(std::size_t i) const;
public:
	std::string asRawString(std::function<std::size_t(const EmbeddingEdge&)> getNeighbourId) const;
	std::string asPrettyString(std::function<std::size_t(const EmbeddingEdge&)> getNeighbourId) const;
private:
	virtual std::string asRawStringImpl(std::function<std::size_t(const EmbeddingEdge&)> getNeighbourId) const;
	virtual std::pair<std::string, bool> asPrettyStringImpl(std::function<std::size_t(const EmbeddingEdge&)> getNeighbourId) const = 0;
private:
	GeometryGraph::Vertex vGeometry; // this is essentially a type id
	int numLonePairs;
	bool hasRadical;
};

struct DynamicDegree : Configuration {
protected:
	DynamicDegree(GeometryGraph::Vertex vGeometry, const EmbeddingEdge *b, const EmbeddingEdge *e);
public:
	virtual const EmbeddingEdge *begin() const override final {
		return edges.data();
	}

	virtual const EmbeddingEdge *end() const override final {
		return edges.data() + edges.size();
	}
protected:
	std::vector<EmbeddingEdge> edges;
};

template<std::size_t d>
struct StaticDegree : Configuration {
protected:
	StaticDegree(GeometryGraph::Vertex vGeometry, const std::array<EmbeddingEdge, d> &edges)
	: Configuration(vGeometry, edges.begin(), edges.end()), edges(edges) { }
public:

	virtual const EmbeddingEdge *begin() const override final {
		return edges.data();
	}

	virtual const EmbeddingEdge *end() const override final {
		return edges.data() + edges.size();
	}
protected:
	std::array<EmbeddingEdge, d> edges;
};

} // namespace mod::lib::Stereo

#endif // MOD_LIB_STEREO_CONFIGURATION_CONFIGURATION_HPP