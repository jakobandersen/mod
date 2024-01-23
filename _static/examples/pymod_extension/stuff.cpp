#include "stuff.hpp"

namespace awesome {

std::shared_ptr<mod::graph::Graph> doStuff() {
	auto g = mod::graph::Graph::fromSMILES("CCO");
	g->setName("Ethanol");
	return g;
}

} // namespace awesome
