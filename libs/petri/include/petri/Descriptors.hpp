#ifndef PETRI_DESCRIPTORS_HPP
#define PETRI_DESCRIPTORS_HPP

#include <functional>

namespace petri {
struct Marking;
struct Net;

struct Place {
	Place() : id(-1) {}
private:
	friend struct Marking;
	friend struct Net;
	explicit Place(int id) : id(id) {}
public:
	int getId() const {
		return id;
	}

	explicit operator bool() const {
		return *this != Place();
	}

	friend bool operator==(Place a, Place b) {
		return a.id == b.id;
	}

	friend bool operator!=(Place a, Place b) {
		return !(a == b);
	}

	friend bool operator<(Place a, Place b) {
		return a.id < b.id;
	}

	friend std::size_t hash_value(Place p) {
		return p.id;
	}
private:
	int id;
};

struct Transition {
	Transition() : id(-1) {}
private:
	friend struct Marking;
	friend struct Net;
	explicit Transition(int id) : id(id) {}
public:
	int getId() const {
		return id;
	}

	explicit operator bool() const {
		return *this != Transition();
	}

	friend bool operator==(Transition a, Transition b) {
		return a.id == b.id;
	}

	friend bool operator!=(Transition a, Transition b) {
		return !(a == b);
	}

	friend bool operator<(Transition a, Transition b) {
		return a.id < b.id;
	}

	friend std::size_t hash_value(Transition t) {
		return t.id;
	}
private:
	int id;
};

} // namespace petri
namespace std {

template<>
struct hash<petri::Place> {
	auto operator()(petri::Place p) const {
		return hash_value(p);
	}
};

template<>
struct hash<petri::Transition> {
	auto operator()(petri::Transition t) const {
		return hash_value(t);
	}
};

} // namespace std

#endif // PETRI_DESCRIPTORS_HPP