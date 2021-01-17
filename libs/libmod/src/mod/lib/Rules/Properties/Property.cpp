#include "Property.hpp"

#include <iostream>

namespace mod::lib::Rules::detail {

void PropVerify(const void *g, const void *gOther,
                std::size_t nGraph, std::size_t nOther,
                std::size_t mGraph, std::size_t mOther) {
	if(g != gOther) {
		std::cout << "Different graphs: g = " << (std::uintptr_t) g
		          << ", &this->g = " << (std::uintptr_t) gOther << std::endl;
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

} // namespace mod::lib::Rules::detail