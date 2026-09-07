#ifndef MOD_LIB_CAUSALITY_PETRI_HPP
#define MOD_LIB_CAUSALITY_PETRI_HPP

#include <mod/lib/DG/Hyper.hpp>

#include <petri/Marking.hpp>

namespace mod::lib::Causality {

struct Net {
	explicit Net(const lib::DG::Hyper &dg);
	~Net() = default;
	const lib::DG::Hyper &getDG() const { return dg; }
	const petri::Net &getNet() const { return net; }
	void syncSize();
	std::vector<lib::DG::HyperVertex> getPostPlaces(lib::DG::HyperVertex e) const;
	petri::Place getPlace(lib::DG::HyperVertex v) const;
	petri::Transition getTransition(lib::DG::HyperVertex e) const;
	lib::DG::HyperVertex getVertex(petri::Place p) const;
private:
	friend class Marking;
	const lib::DG::Hyper &dg;
	petri::Net net;
	// maps vHyperId -> Place/Transition, so not all are valid
	std::vector<petri::Place> placeMap;
	std::vector<petri::Transition> transitionMap;
	// maps Place/Transition id to vHyper
	std::vector<lib::DG::HyperVertex> places, transitions;
};

struct Marking {
	explicit Marking(const Net &net);
	~Marking() = default;
	const Net &getNet() const { return net; }
	const petri::Marking &getMarking() const { return m; }
	void syncSize();
	int add(lib::DG::HyperVertex v, int c);
	int remove(lib::DG::HyperVertex v, int c);
	int getNumTokens() const;
	int operator[](lib::DG::HyperVertex v) const;
	std::vector<lib::DG::HyperVertex> getAllEnabled() const;
	std::vector<lib::DG::HyperVertex> getNonZeroPlaces() const;
	std::vector<lib::DG::HyperVertex> getEmptyPostPlaces(lib::DG::HyperVertex e) const;
	bool isEnabled(lib::DG::HyperVertex e) const;
	// Require: enabled
	void fire(lib::DG::HyperVertex e);
private:
	const Net &net;
	petri::Marking m;
};

// ============================================================================

inline petri::Place Net::getPlace(lib::DG::HyperVertex v) const {
#ifdef MOD_LIB_CAUSALITY_PETRI_ASSERT
	assert(dg.getGraph()[v].kind == lib::DG::HyperVertexKind::Vertex);
#endif
	const auto id = get(boost::vertex_index_t(), dg.getGraph(), v);
	petri::Place p = placeMap[id];
#ifdef MOD_LIB_CAUSALITY_PETRI_ASSERT
	assert(p);
#endif
	return p;
}

inline petri::Transition Net::getTransition(lib::DG::HyperVertex e) const {
#ifdef MOD_LIB_CAUSALITY_PETRI_ASSERT
	assert(dg.getGraph()[e].kind == lib::DG::HyperVertexKind::Edge);
#endif
	const auto id = get(boost::vertex_index_t(), dg.getGraph(), e);
	petri::Transition t = transitionMap[id];
#ifdef MOD_LIB_CAUSALITY_PETRI_ASSERT
	assert(t);
#endif
	return t;
}

inline lib::DG::HyperVertex Net::getVertex(petri::Place p) const {
#ifdef MOD_LIB_CAUSALITY_PETRI_ASSERT
	assert(p);
#endif
	return places[p.getId()];
}

// ============================================================================

inline bool Marking::isEnabled(lib::DG::HyperVertex e) const {
	const auto t = net.getTransition(e);
	return m.isEnabled(t);
}

} // namespace mod::lib::Causality

#endif // MOD_LIB_CAUSALITY_PETRI_HPP
