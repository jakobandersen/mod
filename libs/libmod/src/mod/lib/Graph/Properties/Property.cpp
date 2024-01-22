#include "Property.hpp"

#include <iostream>

namespace mod::lib::Graph::detail {

void PropVerify(const GraphType *g, const GraphType *gOther,
                std::size_t nGraph, std::size_t nOther,
                std::size_t mGraph, std::size_t mOther) {
	if(g != gOther) {
		std::cout << "Different graphs: g = " << g << ", &this->g = " << gOther << std::endl;
		MOD_ABORT;
	}
	if(nGraph != nOther) {
		std::cout << "Different sizes: num_vertices(this->g) = " << nGraph
		          << ", vertexLabels.size() = " << nOther << std::endl;
		MOD_ABORT;
	}
	if(mGraph != mOther) {
		std::cout << "Different sizes: num_edges(this->g) = " << mGraph
		          << ", edgeLabels.size() = " << mOther << std::endl;
		MOD_ABORT;
	}
}

} // namespace mod::lib::Graph::detail