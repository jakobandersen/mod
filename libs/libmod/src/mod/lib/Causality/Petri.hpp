#ifndef MOD_LIB_CAUSALITY_PETRI_HPP
#define MOD_LIB_CAUSALITY_PETRI_HPP

#include <mod/lib/DG/Hyper.hpp>

#include <petri/Marking.hpp>

namespace mod::lib::Causality {

struct Net {
	explicit Net(const lib::DG::Hyper &dg);
	~Net();
	const lib::DG::Hyper &getDG() const;
	const petri::Net &getNet() const;
	void syncSize();
	std::vector<lib::DG::HyperVertex> getPostPlaces(lib::DG::HyperVertex e) const;
	petri::Place getPlace(lib::DG::HyperVertex v) const;
	petri::Transition getTransition(lib::DG::HyperVertex e) const;
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
	~Marking();
	const Net &getNet() const;
	const petri::Marking &getMarking() const;
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

} // namespace mod::lib::Causality

#endif // MOD_LIB_CAUSALITY_PETRI_HPP
