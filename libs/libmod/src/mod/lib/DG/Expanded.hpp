#ifndef MOD_LIB_DG_EXPANDED_HPP
#define MOD_LIB_DG_EXPANDED_HPP

#include <mod/lib/DG/GraphDecl.hpp>

#include <unordered_map>

namespace mod::lib::DG {
struct Hyper;

struct Transit {
	// eIn == null_vertex => eIn is the input edge
	// eOut == null_vertex => eOut is the output edge
	HyperVertex eIn, v, eOut;

	friend bool operator==(Transit a, Transit b) {
		return std::tie(a.v, a.eIn, a.eOut) == std::tie(b.v, b.eIn, b.eOut);
	}

	friend bool operator<(Transit a, Transit b) {
		return std::tie(a.v, a.eIn, a.eOut) < std::tie(b.v, b.eIn, b.eOut);
	}
};

} // namespace mod::lib::DG

template<>
struct std::hash<mod::lib::DG::Transit> {
	std::size_t operator()(mod::lib::DG::Transit r) const;
};

namespace mod::lib::DG {

struct Expanded {
	Expanded(const Expanded &) = delete;
	Expanded &operator=(const Expanded &) = delete;
public:
	using TransitSet = std::unordered_set<Transit>;

	struct VertexData {
		// the first vertex is the catch-all vertex if the flag is set
		bool hasInCatchAll, hasOutCatchAll;
		std::vector<ExpandedVertex> inVertices, outVertices;
		ExpandedVertex inputEdge, outputEdge;
	};
public:
	// all expressedTransits and deletedTransits will be translated to actual transit edges
	// though with the deletedTransits missing
	Expanded(const Hyper &dgHyper, TransitSet expressedTransits, TransitSet deletedTransits);
	const ExpandedGraphType &getGraph() const;
	ExpandedVertex getExpandedFromHyperEdge(HyperVertex e) const;
	const VertexData &getVertexData(HyperVertex v) const;
	bool isCatchAll(ExpandedVertex v) const;
	bool hasIOInternalTransitSeparated(HyperVertex v) const;
public: // helpers for creating sets of transit edges
	static void addAllTransits(const Hyper &dgHyper, TransitSet &c);
	static void addAllTransits(const Hyper &dgHyper, HyperVertex v, TransitSet &c);
	static void addNonInverseTransits(const Hyper &dgHyper, TransitSet &c);
	static void addNonInverseTransits(const Hyper &dgHyper, HyperVertex v, TransitSet &c);
	static void addInternalInverseTransits(const Hyper &dgHyper, TransitSet &c);
	static void addInternalInverseTransits(const Hyper &dgHyper, HyperVertex v, TransitSet &c);
	static void addIOInverseTransits(const Hyper &dgHyper, TransitSet &c);
	static void addIOInverseTransits(const Hyper &dgHyper, HyperVertex v, TransitSet &c);
public:
	const Hyper &dgHyper;
private:
	ExpandedGraphType graph;
	std::unordered_map<HyperVertex, ExpandedVertex> hyperEdgeToExpanded;
	std::unordered_map<HyperVertex, VertexData> vertexData;
	std::unordered_map<Transit, ExpandedVertex> transitMap; // will be null_vertex if the transit is deleted
};

} // namespace mod::lib::DG

#endif // MOD_LIB_DG_EXPANDED_HPP