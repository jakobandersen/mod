#include "GeometryGraph.hpp"

#include <mod/Chem.hpp>
#include <mod/Error.hpp>
#include <mod/lib/Chem/MoleculeUtil.hpp>
#include <mod/lib/Stereo/EdgeCategory.hpp>

#include <mod/lib/Stereo/Configuration/Configuration.hpp>
#include <mod/lib/Stereo/Configuration/Any.hpp>
#include <mod/lib/Stereo/Configuration/Linear.hpp>
#include <mod/lib/Stereo/Configuration/TrigonalPlanar.hpp>
#include <mod/lib/Stereo/Configuration/Tetrahedral.hpp>

#include <memory>

namespace mod {
namespace lib {
namespace Stereo {
namespace {

// Based on std::integer_sequence in libstdc++
// TODO: simplify when C++14 is used.

template<std::size_t ...Is>
struct IndexSequence {
	using next = IndexSequence<Is..., sizeof...(Is)>;
};

template<std::size_t N>
struct MakeIndexSequence {
	using type = typename MakeIndexSequence<N - 1>::type::next;
};

template<>
struct MakeIndexSequence<0> {
	using type = IndexSequence<>;
};

template<std::size_t ...Is>
std::array<EmbeddingEdge, sizeof...(Is)> toArrayHelper(IndexSequence<Is...>, const EmbeddingEdge *b) {
	return std::array<EmbeddingEdge, sizeof...(Is)>({b[Is]...});
}

template<std::size_t N>
std::array<EmbeddingEdge, N> toArray(const EmbeddingEdge *b, const EmbeddingEdge *e) {
	assert(e - b == N);
	constexpr std::size_t M = N; // because of https://gcc.gnu.org/bugzilla/show_bug.cgi?id=57887
	return toArrayHelper(typename MakeIndexSequence<M>::type(), b);
}

} // namespace

// VProp
//------------------------------------------------------------------------------

GeometryGraph::VProp::VProp(const std::string &name) : name(name) {}

// GeometryGraph
//------------------------------------------------------------------------------

GeometryGraph::GeometryGraph() {
	any = addGeometry(VProp("any"));
	g[any].constructor = [this](const EmbeddingEdge *b, const EmbeddingEdge *e, const Fixation &fix, std::ostream &err) {
		return std::make_unique<Any>(*this, b, e);
	};
	linear = addChild(any, VProp("linear"));
	g[linear].constructor = [this](const EmbeddingEdge *b, const EmbeddingEdge *e, const Fixation &fix,
	                               std::ostream &err) {
		constexpr std::size_t N = 2;
		if(e - b != N) {
			err << "Can not create linear geometry for vertex with degree " << (e - b) << ", must be 2.";
			return std::unique_ptr<Linear>();
		}
		return std::make_unique<Linear>(*this, toArray<N>(b, e));
	};
	trigonalPlanar = addChild(any, VProp("trigonalPlanar"));
	g[trigonalPlanar].constructor = [this](const EmbeddingEdge *b, const EmbeddingEdge *e, const Fixation &fix,
	                                       std::ostream &err) {
		constexpr std::size_t N = 3;
		if(e - b != N) {
			err << "Can not create trigonalPlanar geometry for vertex with degree " << (e - b) << ", must be 3.";
			return std::unique_ptr<TrigonalPlanar>();
		}
		bool fixed;
		if(fix != Fixation::free() && fix != Fixation::simpleFixed()) MOD_ABORT; // TODO: error
		fixed = fix.asSimple();
		return std::make_unique<TrigonalPlanar>(*this, toArray<N>(b, e), fixed);
	};
	tetrahedral = addChild(any, VProp("tetrahedral"));
	g[tetrahedral].constructor = [this](const EmbeddingEdge *b, const EmbeddingEdge *e, const Fixation &fix,
	                                    std::ostream &err) {
		constexpr std::size_t N = 4;
		if(e - b != N) {
			err << "Can not create tetrahedral geometry for vertex with degree " << (e - b) << ", must be 4.";
			return std::unique_ptr<Tetrahedral>();
		}
		bool fixed;
		if(fix != Fixation::free() && fix != Fixation::simpleFixed()) MOD_ABORT; // TODO: error
		fixed = fix.asSimple();
		return std::make_unique<Tetrahedral>(*this, toArray<N>(b, e), fixed);
	};

#define V(atomId, charge, radical, single, double_, triple, aromatic, lonePair, geometry)\
   do {                                                                         \
      EdgeCategoryCount catCount;                                               \
      catCount[EdgeCategory::Single] = single;                                  \
      catCount[EdgeCategory::Double] = double_;                                 \
      catCount[EdgeCategory::Triple] = triple;                                  \
      catCount[EdgeCategory::Aromatic] = aromatic;                              \
      chemValids.push_back(ChemValid{AtomId(atomId), Charge(charge), radical, catCount, lonePair, geometry}); \
   } while(false)
	using namespace mod::AtomIds;
	V(H, 1, false, 0, 0, 0, 0, 0, any);
	V(H, 0, false, 1, 0, 0, 0, 0, any);

	V(C, 0, false, 1, 0, 1, 0, 0, linear);
	V(C, 0, false, 0, 2, 0, 0, 0, linear);
	V(C, 0, false, 2, 1, 0, 0, 0, trigonalPlanar);
	V(C, 0, false, 1, 0, 0, 2, 0, trigonalPlanar);
	V(C, 0, false, 0, 1, 0, 2, 0, trigonalPlanar);
	V(C, 0, false, 0, 0, 0, 3, 0, trigonalPlanar);
	V(C, 0, false, 4, 0, 0, 0, 0, tetrahedral);

	//	V(N, 0, false, 2, 1, 0, 0, 0, trigonalPlanar); // it was R-NOOH, not normal
	V(N, 0, false, 1, 1, 0, 0, 1, trigonalPlanar);
	V(N, 0, false, 1, 0, 0, 2, 0, trigonalPlanar);
	V(N, 0, false, 0, 1, 0, 2, 0, trigonalPlanar); // TODO: is this correct?
	V(N, 0, false, 0, 0, 0, 3, 0, trigonalPlanar);
	V(N, 0, false, 0, 0, 0, 2, 1, trigonalPlanar);
	V(N, 1, false, 2, 1, 0, 0, 0, trigonalPlanar); // TODO: is this correct? should it be this geometry?
	V(N, 1, false, 1, 0, 0, 2, 0, trigonalPlanar);
	V(N, 0, false, 3, 0, 0, 0, 1, tetrahedral);
	V(N, 1, false, 4, 0, 0, 0, 0, tetrahedral);

	//V(O, 0, false, 2, 0, 0, 0, 2, tetrahedral);
	//V(O, 0, false, 0, 1, 0, 0, 2, trigonalPlanar);
	//V(O, -1, false, 1, 0, 0, 0, 3, tetrahedral);

	V(P, 0, false, 3, 1, 0, 0, 0, tetrahedral);

	//	V(S, 0, false, 0, 1, 0, 0, 2, trigonalPlanar); // TODO: check
	//	V(S, 0, false, 0, 0, 0, 2, 1, trigonalPlanar); // TODO: check
	//	V(S, 0, false, 2, 2, 0, 0, 0, tetrahedral);
	//	V(S, 0, false, 2, 0, 0, 0, 2, tetrahedral);
	//	V(S, 1, false, 3, 0, 0, 0, 1, tetrahedral);
}

const GeometryGraph::GraphType &GeometryGraph::getGraph() const {
	return g;
}

GeometryGraph::Vertex GeometryGraph::findGeometry(const std::string &name) const {
	auto iter = nameToVertex.find(name);
	if(iter == end(nameToVertex)) return nullGeometry();
	else return iter->second;
}

GeometryGraph::Vertex GeometryGraph::nullGeometry() {
	return boost::graph_traits<GraphType>::null_vertex();
}

lib::IO::Result<unsigned char>
GeometryGraph::deduceLonePairs(lib::IO::Warnings &warnings, bool printWarnings,
                               const AtomData &ad, const EdgeCategoryCount &catCount,
                               Vertex vGeometry, bool asPattern) const {
	auto atomId = ad.getAtomId();
	auto charge = ad.getCharge();
	auto radical = ad.getRadical();
	if(catCount[EdgeCategory::Undefined] > 0) MOD_ABORT; // wait for test case
	if(atomId == AtomIds::Invalid || catCount[EdgeCategory::Undefined] > 0)
		return 0;
	assert(catCount[EdgeCategory::Undefined] == 0);
	std::vector<ChemValid> viables;
	if(asPattern) {
		// find all matches where we may be missing some neighbours
		std::copy_if(chemValids.begin(), chemValids.end(), std::back_inserter(viables), [&](const ChemValid &cv) {
			return cv.atomId == atomId && cv.charge == charge && cv.radical == radical
			       && componentWiseLEQ(catCount, cv.catCount)
			       && cv.geometry == vGeometry;
		});
	} else {
		// find all exact matches
		std::copy_if(chemValids.begin(), chemValids.end(), std::back_inserter(viables), [&](const ChemValid &cv) {
			return cv.atomId == atomId && cv.charge == charge && cv.radical == radical
			       && cv.catCount == catCount
			       && cv.geometry == vGeometry;
		});
	}

	if(viables.empty()) {
		std::stringstream msg;
		msg << "No viable configurations for " << ad << " with bonds " << catCount << ", in geometry '"
		    << g[vGeometry].name << "'.";
		warnings.add(msg.str(), printWarnings);
		return 0;
	}
	if(viables.size() > 1) {
		std::stringstream msg;
		msg << "Ambiguous deduction for " << ad << " with bonds " << catCount << ", in geometry '" << g[vGeometry].name
		    << "'. Matches are:\n";
		for(const auto &v: viables) {
			msg << "\t" << AtomData(v.atomId, v.charge, v.radical);
			if(v.catCount.sum() > 0) msg << ", " << v.catCount;
			if(v.lonePair > 0) msg << ", e = " << v.lonePair;
			msg << ", geometry = " << g[v.geometry].name << "\n";
		}
		return lib::IO::Result<>::Error(msg.str());
	}
	assert(viables.size() == 1);
	return viables.front().lonePair;
}

lib::IO::Result<GeometryGraph::Vertex>
GeometryGraph::deduceGeometry(lib::IO::Warnings &warnings, bool printWarnings,
                              const AtomData &ad, const EdgeCategoryCount &catCount,
                              unsigned char numLonePairs, bool asPattern) const {
	auto atomId = ad.getAtomId();
	auto charge = ad.getCharge();
	auto radical = ad.getRadical();
	if(catCount[EdgeCategory::Undefined] > 0) MOD_ABORT; // wait for test case
	if(atomId == AtomIds::Invalid || catCount[EdgeCategory::Undefined] > 0)
		return any;
	assert(catCount[EdgeCategory::Undefined] == 0);
	std::vector<ChemValid> viables;
	// TODO: should there be a difference when it's a pattern? we know all the neighbours.
	//	if(asPattern) {
	//		MOD_ABORT;
	//	} else {
	// find all exact matches
	std::copy_if(chemValids.begin(), chemValids.end(), std::back_inserter(viables), [&](const ChemValid &cv) {
		return cv.atomId == atomId && cv.charge == charge && cv.radical == radical
		       && cv.catCount == catCount && cv.lonePair == numLonePairs;
	});
	//	}
	if(viables.empty()) {
		std::stringstream msg;
		msg << "No viable geometries for " << ad;
		if(catCount.sum() > 0) msg << " with bonds " << catCount;
		else msg << " without bonds";
		msg << ", and ";
		if(numLonePairs > 0) msg << numLonePairs << " lone pairs.";
		else msg << " no lone pairs.";
		warnings.add(msg.str(), printWarnings);
		return any;
	}
	if(viables.size() > 1) {
		std::stringstream msg;
		msg << "Ambiguous deduction for " << ad << " with bonds " << catCount << ", and " << numLonePairs
		    << " lone pairs.Matches are:\n";
		for(auto &v: viables) {
			msg << "\t" << AtomData(v.atomId, v.charge, v.radical);
			if(v.catCount.sum() > 0) msg << ", " << v.catCount;
			msg << ", geometry = " << g[v.geometry].name << "\n";
		}
		return lib::IO::Result<>::Error(msg.str());
	}
	assert(viables.size() == 1);
	return viables.front().geometry;
}

lib::IO::Result<std::tuple<GeometryGraph::Vertex, unsigned char>>
GeometryGraph::deduceGeometryAndLonePairs(lib::IO::Warnings &warnings, bool printWarnings,
                                          const AtomData &ad, const EdgeCategoryCount &catCount,
                                          bool asPattern) const {
	using Res = std::tuple<GeometryGraph::Vertex, unsigned char>;
	auto atomId = ad.getAtomId();
	auto charge = ad.getCharge();
	auto radical = ad.getRadical();
	if(atomId == AtomIds::Invalid || catCount[EdgeCategory::Undefined] > 0)
		return Res{any, 0};
	assert(catCount[EdgeCategory::Undefined] == 0);
	std::vector<ChemValid> viables;
	//	if(asPattern && false) { // for now only exact matches
	//		// find all matches where we may be missing some neighbours
	//		std::copy_if(chemValids.begin(), chemValids.end(), std::back_inserter(viables), [&](const ChemValid & cv) {
	//			return cv.atomId == atomId && cv.charge == charge && cv.radical == radical
	//					&& componentWiseLEQ(catCount, cv.catCount);
	//		});
	//	} else {
	// find all exact matches
	std::copy_if(chemValids.begin(), chemValids.end(), std::back_inserter(viables), [&](const ChemValid &cv) {
		return cv.atomId == atomId && cv.charge == charge && cv.radical == radical
		       && cv.catCount == catCount;
	});
	//	}
	if(viables.empty()) {
		bool doWarn = true;
		if(asPattern) {
			// if it could match some viable, then don't warn
			std::copy_if(chemValids.begin(), chemValids.end(), std::back_inserter(viables), [&](const ChemValid &cv) {
				return cv.atomId == atomId && cv.charge == charge && cv.radical == radical
				       && componentWiseLEQ(catCount, cv.catCount);
			});
			if(!viables.empty()) doWarn = false;
		}
		if(doWarn) {
			std::stringstream msg;
			msg << "No viable geometries for " << ad;
			if(catCount.sum() > 0) msg << " with bonds " << catCount << ".";
			else msg << " without bonds.";
			warnings.add(msg.str(), printWarnings);
		}
		return Res{any, 0};
	}
	if(viables.size() > 1) {
		if(asPattern) {
			// TODO: be more clever, maybe?
			return Res{any, 0};
		}
		std::stringstream msg;
		msg << "Ambiguous deduction for " << ad << " with bonds " << catCount << ". Matches are:\n";
		for(auto &v: viables) {
			msg << "\t" << AtomData(v.atomId, v.charge, v.radical);
			if(v.catCount.sum() > 0) msg << ", " << v.catCount;
			if(v.lonePair > 0) msg << ", e = " << v.lonePair;
			msg << ", geometry = " << g[v.geometry].name << "\n";
		}
		return lib::IO::Result<>::Error(msg.str());
	}
	assert(viables.size() == 1);
	return Res{viables.front().geometry, viables.front().lonePair};
}

bool GeometryGraph::isAncestorOf(Vertex ancestor, Vertex child) const {
	while(true) {
		if(child == ancestor) return true;
		if(in_degree(child, g) == 0) return false;
		child = *inv_adjacent_vertices(child, g).first;
	}
}

GeometryGraph::Vertex GeometryGraph::generalize(Vertex a, Vertex b) const {
	const auto level = [this](Vertex v) {
		int i = 0;
		for(; in_degree(v, g) != 0; v = *inv_adjacent_vertices(v, g).first)
			++i;
		return i;
	};
	auto aLevel = level(a);
	auto bLevel = level(b);
	for(; aLevel > bLevel; --aLevel) {
		a = *inv_adjacent_vertices(a, g).first;
	}
	for(; bLevel > aLevel; --bLevel) {
		b = *inv_adjacent_vertices(b, g).first;
	}
	while(a != b) {
		a = *inv_adjacent_vertices(a, g).first;
		b = *inv_adjacent_vertices(b, g).first;
	}
	return a;
}

GeometryGraph::Vertex GeometryGraph::unify(Vertex a, Vertex b) const {
	if(isAncestorOf(a, b)) return b;
	else if(isAncestorOf(b, a)) return a;
	else return nullGeometry();
}

GeometryGraph::Vertex GeometryGraph::pushoutComplement(Vertex a, Vertex b, Vertex d) const {
	// TODO: is this right?
	if(b == d) return a;
	else return d;
}

GeometryGraph::Vertex GeometryGraph::addGeometry(VProp &&vProp) const {
	Vertex v = add_vertex(std::move(vProp), g);
	assert(nameToVertex.find(g[v].name) == end(nameToVertex));
	nameToVertex[g[v].name] = v;
	return v;
}

GeometryGraph::Vertex GeometryGraph::addChild(Vertex vParent, VProp &&vProp) {
	Vertex v = addGeometry(std::move(vProp));
	add_edge(vParent, v, g);
	return v;
}

//------------------------------------------------------------------------------

const GeometryGraph &getGeometryGraph() {
	static GeometryGraph g;
	return g;
}

} // namespace Stereo
} // namespace lib
} // namespace mod