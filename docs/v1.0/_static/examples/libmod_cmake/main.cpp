#include <mod/graph/Graph.hpp>

#include <iostream>

int main() {
	auto g = mod::graph::Graph::fromDFS("[T]");
	std::cout << "Graph name:      " << g->getName() << "\n";
}