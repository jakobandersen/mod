#ifndef PETRI_NET_HPP
#define PETRI_NET_HPP

// define to enable asserts in functions that may be used in inner loops
//#define PETRI_NET_ASSERT

#include <petri/Descriptors.hpp>

#include <boost/graph/adjacency_list.hpp>

#include <memory>
#include <vector>

namespace petri {

struct Net {
	enum class Kind {
		Place, Transition
	};

	struct VProp {
		Kind kind;
		int id;
	};

	using GraphType = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, VProp, int>;
	using Vertex = boost::graph_traits<GraphType>::vertex_descriptor;
	using Edge = boost::graph_traits<GraphType>::edge_descriptor;
public:
	struct PlaceIterator;
	struct PlaceRange;
	struct TransitionIterator;
	struct TransitionRange;
	struct ConsumedIterator;
	struct ConsumedRange;
	struct ProducedIterator;
	struct ProducedRange;
public:
	Net() = default;
public: // readable
	int numPlaces() const;
	int numTransitions() const;
	PlaceRange places() const;
	TransitionRange transitions() const;
	ConsumedRange consumed(Transition t) const;
	ProducedRange produced(Transition t) const;
public:
	const GraphType &getGraph() const;
	Vertex vertexFromTransition(Transition t) const;
	Vertex vertexFromPlace(Place p) const;
	Place placeFromVertex(Vertex v) const;
	int getMaxInWeight() const;
public: // mutable
	Place addPlace();
	Transition addTransition();
	// Requires: p and t to be valid
	// Requires: w to be non-negative
	// Adds w weight to the arc.
	// Returns: the new weight.
	int addArc(Place p, Transition t, int w);
	int addArc(Transition t, Place p, int w);
private:
	static Place makePlace(int i);
	static Transition makeTransition(int i);
private:
	GraphType g;
	int maxInWeight = 0;
	std::vector<Vertex> places_, transitions_;
	std::vector<std::vector<Vertex>> candDisable, candEnable;
};

//==============================================================================
// Implementation
//==============================================================================

struct Net::PlaceIterator : boost::iterator_facade<
/**/ PlaceIterator,
/**/ Place,
/**/ std::random_access_iterator_tag,
/**/ Place
> {
	PlaceIterator() : PlaceIterator(0) {}
	PlaceIterator(int i) : i(i) {}
private:
	friend class boost::iterator_core_access;

	bool equal(PlaceIterator other) const {
		return i == other.i;
	}

	void increment() {
		++i;
	}

	Place dereference() const {
		return Net::makePlace(i);
	}

	// TODO: actually make it random access
private:
	int i;
};

struct Net::PlaceRange {
	explicit PlaceRange(int i) : i(i) {}

	PlaceIterator begin() const {
		return {0};
	}

	PlaceIterator end() const {
		return {i};
	}
private:
	int i;
};

struct Net::TransitionIterator : boost::iterator_facade<
/**/ TransitionIterator,
/**/ Transition,
/**/ std::random_access_iterator_tag,
/**/ Transition
> {
	TransitionIterator() : TransitionIterator(0) {}
	TransitionIterator(int i) : i(i) {}
private:
	friend class boost::iterator_core_access;

	bool equal(TransitionIterator other) const {
		return i == other.i;
	}

	void increment() {
		++i;
	}

	Transition dereference() const {
		return Net::makeTransition(i);
	}

	// TODO: actually make it random access
private:
	int i;
};

struct Net::TransitionRange {
	explicit TransitionRange(int i) : i(i) {}

	TransitionIterator begin() const {
		return {0};
	}

	TransitionIterator end() const {
		return {i};
	}
private:
	int i;
};

//==============================================================================

struct Net::ConsumedIterator : boost::iterator_facade<
/**/ ConsumedIterator,
/**/ std::pair<Place, int>,
/**/ std::random_access_iterator_tag,
/**/ std::pair<Place, int>
> {
	ConsumedIterator() = default;
	ConsumedIterator(const GraphType *g, Vertex t, std::size_t offset) : g(g), t(t), offset(offset) {}
private:
	friend class boost::iterator_core_access;

	bool equal(ConsumedIterator other) const {
		return std::tie(g, t, offset) == std::tie(other.g, other.t, other.offset);
	}

	void increment() {
		++offset;
	}

	std::pair<Place, int> dereference() const {
		auto e = *std::next(in_edges(t, *g).first, offset);
		return {Net::makePlace((*g)[source(e, *g)].id), (*g)[e]};
	}

	// TODO: actually make it random access
private:
	const GraphType *g = nullptr;
	Vertex t = boost::graph_traits<GraphType>::null_vertex();
	std::size_t offset = 0;
};

struct Net::ConsumedRange {
	explicit ConsumedRange(const GraphType *g, Vertex t) : g(g), t(t) {}

	ConsumedIterator begin() const {
		return {g, t, 0};
	}

	ConsumedIterator end() const {
		return {g, t, in_degree(t, *g)};
	}
private:
	const GraphType *g;
	Vertex t;
};

//==============================================================================

struct Net::ProducedIterator : boost::iterator_facade<
/**/ ProducedIterator,
/**/ std::pair<Place, int>,
/**/ std::random_access_iterator_tag,
/**/ std::pair<Place, int>
> {
	ProducedIterator() = default;
	ProducedIterator(const GraphType *g, Vertex t, std::size_t offset) : g(g), t(t), offset(offset) {}
private:
	friend class boost::iterator_core_access;

	bool equal(ProducedIterator other) const {
		return std::tie(g, t, offset) == std::tie(other.g, other.t, other.offset);
	}

	void increment() {
		++offset;
	}

	std::pair<Place, int> dereference() const {
		auto e = *std::next(out_edges(t, *g).first, offset);
		return {Net::makePlace((*g)[target(e, *g)].id), (*g)[e]};
	}

	// TODO: actually make it random access
private:
	const GraphType *g = nullptr;
	Vertex t = boost::graph_traits<GraphType>::null_vertex();
	std::size_t offset = 0;
};

struct Net::ProducedRange {
	explicit ProducedRange(const GraphType *g, Vertex t) : g(g), t(t) {}

	ProducedIterator begin() const {
		return {g, t, 0};
	}

	ProducedIterator end() const {
		return {g, t, out_degree(t, *g)};
	}
private:
	const GraphType *g;
	Vertex t;
};

//==============================================================================

inline int Net::numPlaces() const {
	return places_.size();
}

inline int Net::numTransitions() const {
	return transitions_.size();
}

inline Net::PlaceRange Net::places() const {
	return PlaceRange{numPlaces()};
}

inline Net::TransitionRange Net::transitions() const {
	return TransitionRange{numTransitions()};
}

inline Net::ConsumedRange Net::consumed(Transition t) const {
	return ConsumedRange(&g, vertexFromTransition(t));
}

inline Net::ProducedRange Net::produced(Transition t) const {
	return ProducedRange(&g, vertexFromTransition(t));
}

//==============================================================================

inline const Net::GraphType &Net::getGraph() const {
	return g;
}

inline Net::Vertex Net::vertexFromTransition(Transition t) const {
#ifdef PETRI_NET_ASSERT
	assert(t);
	assert(static_cast<std::size_t>(t.getId()) < transitions_.size());
#endif
	return transitions_[t.getId()];
}

inline Net::Vertex Net::vertexFromPlace(Place p) const {
#ifdef PETRI_NET_ASSERT
	assert(p);
	assert(static_cast<std::size_t> (p.getId()) < places_.size());
#endif
	return places_[p.getId()];
}

inline Place Net::placeFromVertex(Vertex v) const {
#ifdef PETRI_NET_ASSERT
	assert(g[v].kind == Kind::Place);
#endif
	return Place(g[v].id);
}

inline int Net::getMaxInWeight() const {
	return maxInWeight;
}

inline Place Net::addPlace() {
	const int id = places_.size();
	Vertex v = add_vertex({Kind::Place, id}, g);
#ifdef PETRI_NET_ASSERT
	assert(g[v].kind == Kind::Place);
#endif
	places_.push_back(v);
	return Place(id);
}

inline Transition Net::addTransition() {
	const int id = transitions_.size();
	Vertex v = add_vertex({Kind::Transition, id}, g);
#ifdef PETRI_NET_ASSERT
	assert(g[v].kind == Kind::Transition);
#endif
	transitions_.push_back(v);
	return Transition(id);
}

inline int Net::addArc(Place p, Transition t, int w) {
	assert(w >= 0);
	Vertex vp = places_[p.getId()];
	Vertex vt = transitions_[t.getId()];
	assert(g[vp].kind == Kind::Place);
	assert(g[vt].kind == Kind::Transition);
	auto ep = edge(vp, vt, g);
	if(ep.second) {
		int weight = g[ep.first] += w;
		if(weight > maxInWeight) maxInWeight = weight;
		return weight;
	} else {
		add_edge(vp, vt, w, g);
		if(w > maxInWeight) maxInWeight = w;
		return w;
	}
}

inline int Net::addArc(Transition t, Place p, int w) {
	assert(w >= 0);
	Vertex vt = transitions_[t.getId()];
	Vertex vp = places_[p.getId()];
	assert(g[vp].kind == Kind::Place);
	assert(g[vt].kind == Kind::Transition);
	auto ep = edge(vt, vp, g);
	if(ep.second) return g[ep.first] += w;
	else {
		add_edge(vt, vp, w, g);
		return w;
	}
}

//=============================================================================

inline Place Net::makePlace(int i) {
	return Place(i);
}


inline Transition Net::makeTransition(int i) {
	return Transition(i);
}

} // namespace petri

#endif // PETRI_NET_HPP