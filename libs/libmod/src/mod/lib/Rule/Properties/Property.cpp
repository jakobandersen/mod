#include "Property.hpp"

#include <iostream>

namespace mod::lib::rule::detail {

void PropVerify(const lib::DPO::CombinedRule &rule,
                std::size_t vLSize, std::size_t vRSize,
                std::size_t eLSize, std::size_t eRSize) {
	const auto printStuff = [&]() {
		std::cout << "|V(CG)| = " << num_vertices(rule.getCombinedGraph()) << '\t'
		          << "|E(CG)| = " << num_edges(rule.getCombinedGraph()) << std::endl;
		std::cout << "|V(L)| = " << num_vertices(getL(rule)) << '\t'
		          << "|E(L)| = " << num_edges(getL(rule)) << std::endl;
		std::cout << "|V(K)| = " << num_vertices(getK(rule)) << '\t'
		          << "|E(K)| = " << num_edges(getK(rule)) << std::endl;
		std::cout << "|V(R)| = " << num_vertices(getR(rule)) << '\t'
		          << "|E(R)| = " << num_edges(getR(rule)) << std::endl;
		std::cout << "|PV(L)| = " << vLSize << '\t'
		          << "|PE(L)| = " << eLSize << std::endl;
		std::cout << "|PV(R)| = " << vRSize << '\t'
		          << "|PE(R)| = " << eRSize << std::endl;
	};
	if(num_vertices(getL(rule)) != vLSize) {
		std::cout << "Different |V(L)|:\n";
		printStuff();
		MOD_ABORT;
	}
	if(num_vertices(getR(rule)) != vRSize) {
		std::cout << "Different |V(R)|:\n";
		printStuff();
		MOD_ABORT;
	}
	if(num_edges(getL(rule)) != eLSize) {
		std::cout << "Different |E(L)|:\n";
		printStuff();
		MOD_ABORT;
	}
	if(num_edges(getR(rule)) != eRSize) {
		std::cout << "Different |E(R)|:\n";
		printStuff();
		MOD_ABORT;
	}
}

} // namespace mod::lib::rule::detail