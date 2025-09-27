#include <petri/Marking.hpp>
#include <petri/Net.hpp>

#include <iostream>

int main() {
	using namespace petri;
	Net net;
	Place tail = net.addPlace();
	Place head = net.addPlace();
	Transition t = net.addTransition();
	net.addArc(tail, t, 2);
	net.addArc(t, head, 1);
	std::cout << "|P|: " << net.numPlaces() << "\n";
	std::cout << "|T|: " << net.numPlaces() << "\n";
	for(Place p : net.places())
		std::cout << "place: " << p.getId() << std::endl;
	for(Transition t : net.transitions())
		std::cout << "transition: " << t.getId() << std::endl;

	// TODO: test much more
	// TODO: test iterators for random access
}
