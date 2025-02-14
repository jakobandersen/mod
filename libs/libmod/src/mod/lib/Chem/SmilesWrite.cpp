// Originally the code came from GGL, but has now been substantially rewritten.
// See http://www.opensmiles.org/opensmiles.html for specification.
// Note though, that this is still the original CANGEN algorithm which is wrong.

//#define DO_DEBUG

#include "Smiles.hpp"

#include <mod/Error.hpp>
#include <mod/lib/Chem/MoleculeUtil.hpp>
#include <mod/lib/Graph/Properties/Molecule.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/IO/IO.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <cassert>
#include <iostream>

namespace mod::lib::Chem {
namespace {
using Vertex = lib::graph::Vertex;
using Edge = lib::graph::Edge;
using VSizeType = lib::graph::GraphType::vertices_size_type;

const unsigned int primesLength = 1001;
const unsigned int primes[primesLength] = {1, 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67,
                                           71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149,
                                           151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229,
                                           233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313,
                                           317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
                                           419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499,
                                           503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601,
                                           607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691,
                                           701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809,
                                           811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881, 883, 887, 907,
                                           911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013,
                                           1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087, 1091, 1093,
                                           1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171, 1181, 1187, 1193,
                                           1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289,
                                           1291, 1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381, 1399,
                                           1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459, 1471, 1481, 1483,
                                           1487, 1489, 1493, 1499, 1511, 1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571,
                                           1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657, 1663,
                                           1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747, 1753, 1759,
                                           1777, 1783, 1787, 1789, 1801, 1811, 1823, 1831, 1847, 1861, 1867, 1871, 1873,
                                           1877, 1879, 1889, 1901, 1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987,
                                           1993, 1997, 1999, 2003, 2011, 2017, 2027, 2029, 2039, 2053, 2063, 2069, 2081,
                                           2083, 2087, 2089, 2099, 2111, 2113, 2129, 2131, 2137, 2141, 2143, 2153, 2161,
                                           2179, 2203, 2207, 2213, 2221, 2237, 2239, 2243, 2251, 2267, 2269, 2273, 2281,
                                           2287, 2293, 2297, 2309, 2311, 2333, 2339, 2341, 2347, 2351, 2357, 2371, 2377,
                                           2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423, 2437, 2441, 2447, 2459, 2467,
                                           2473, 2477, 2503, 2521, 2531, 2539, 2543, 2549, 2551, 2557, 2579, 2591, 2593,
                                           2609, 2617, 2621, 2633, 2647, 2657, 2659, 2663, 2671, 2677, 2683, 2687, 2689,
                                           2693, 2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741, 2749, 2753, 2767, 2777,
                                           2789, 2791, 2797, 2801, 2803, 2819, 2833, 2837, 2843, 2851, 2857, 2861, 2879,
                                           2887, 2897, 2903, 2909, 2917, 2927, 2939, 2953, 2957, 2963, 2969, 2971, 2999,
                                           3001, 3011, 3019, 3023, 3037, 3041, 3049, 3061, 3067, 3079, 3083, 3089, 3109,
                                           3119, 3121, 3137, 3163, 3167, 3169, 3181, 3187, 3191, 3203, 3209, 3217, 3221,
                                           3229, 3251, 3253, 3257, 3259, 3271, 3299, 3301, 3307, 3313, 3319, 3323, 3329,
                                           3331, 3343, 3347, 3359, 3361, 3371, 3373, 3389, 3391, 3407, 3413, 3433, 3449,
                                           3457, 3461, 3463, 3467, 3469, 3491, 3499, 3511, 3517, 3527, 3529, 3533, 3539,
                                           3541, 3547, 3557, 3559, 3571, 3581, 3583, 3593, 3607, 3613, 3617, 3623, 3631,
                                           3637, 3643, 3659, 3671, 3673, 3677, 3691, 3697, 3701, 3709, 3719, 3727, 3733,
                                           3739, 3761, 3767, 3769, 3779, 3793, 3797, 3803, 3821, 3823, 3833, 3847, 3851,
                                           3853, 3863, 3877, 3881, 3889, 3907, 3911, 3917, 3919, 3923, 3929, 3931, 3943,
                                           3947, 3967, 3989, 4001, 4003, 4007, 4013, 4019, 4021, 4027, 4049, 4051, 4057,
                                           4073, 4079, 4091, 4093, 4099, 4111, 4127, 4129, 4133, 4139, 4153, 4157, 4159,
                                           4177, 4201, 4211, 4217, 4219, 4229, 4231, 4241, 4243, 4253, 4259, 4261, 4271,
                                           4273, 4283, 4289, 4297, 4327, 4337, 4339, 4349, 4357, 4363, 4373, 4391, 4397,
                                           4409, 4421, 4423, 4441, 4447, 4451, 4457, 4463, 4481, 4483, 4493, 4507, 4513,
                                           4517, 4519, 4523, 4547, 4549, 4561, 4567, 4583, 4591, 4597, 4603, 4621, 4637,
                                           4639, 4643, 4649, 4651, 4657, 4663, 4673, 4679, 4691, 4703, 4721, 4723, 4729,
                                           4733, 4751, 4759, 4783, 4787, 4789, 4793, 4799, 4801, 4813, 4817, 4831, 4861,
                                           4871, 4877, 4889, 4903, 4909, 4919, 4931, 4933, 4937, 4943, 4951, 4957, 4967,
                                           4969, 4973, 4987, 4993, 4999, 5003, 5009, 5011, 5021, 5023, 5039, 5051, 5059,
                                           5077, 5081, 5087, 5099, 5101, 5107, 5113, 5119, 5147, 5153, 5167, 5171, 5179,
                                           5189, 5197, 5209, 5227, 5231, 5233, 5237, 5261, 5273, 5279, 5281, 5297, 5303,
                                           5309, 5323, 5333, 5347, 5351, 5381, 5387, 5393, 5399, 5407, 5413, 5417, 5419,
                                           5431, 5437, 5441, 5443, 5449, 5471, 5477, 5479, 5483, 5501, 5503, 5507, 5519,
                                           5521, 5527, 5531, 5557, 5563, 5569, 5573, 5581, 5591, 5623, 5639, 5641, 5647,
                                           5651, 5653, 5657, 5659, 5669, 5683, 5689, 5693, 5701, 5711, 5717, 5737, 5741,
                                           5743, 5749, 5779, 5783, 5791, 5801, 5807, 5813, 5821, 5827, 5839, 5843, 5849,
                                           5851, 5857, 5861, 5867, 5869, 5879, 5881, 5897, 5903, 5923, 5927, 5939, 5953,
                                           5981, 5987, 6007, 6011, 6029, 6037, 6043, 6047, 6053, 6067, 6073, 6079, 6089,
                                           6091, 6101, 6113, 6121, 6131, 6133, 6143, 6151, 6163, 6173, 6197, 6199, 6203,
                                           6211, 6217, 6221, 6229, 6247, 6257, 6263, 6269, 6271, 6277, 6287, 6299, 6301,
                                           6311, 6317, 6323, 6329, 6337, 6343, 6353, 6359, 6361, 6367, 6373, 6379, 6389,
                                           6397, 6421, 6427, 6449, 6451, 6469, 6473, 6481, 6491, 6521, 6529, 6547, 6551,
                                           6553, 6563, 6569, 6571, 6577, 6581, 6599, 6607, 6619, 6637, 6653, 6659, 6661,
                                           6673, 6679, 6689, 6691, 6701, 6703, 6709, 6719, 6733, 6737, 6761, 6763, 6779,
                                           6781, 6791, 6793, 6803, 6823, 6827, 6829, 6833, 6841, 6857, 6863, 6869, 6871,
                                           6883, 6899, 6907, 6911, 6917, 6947, 6949, 6959, 6961, 6967, 6971, 6977, 6983,
                                           6991, 6997, 7001, 7013, 7019, 7027, 7039, 7043, 7057, 7069, 7079, 7103, 7109,
                                           7121, 7127, 7129, 7151, 7159, 7177, 7187, 7193, 7207, 7211, 7213, 7219, 7229,
                                           7237, 7243, 7247, 7253, 7283, 7297, 7307, 7309, 7321, 7331, 7333, 7349, 7351,
                                           7369, 7393, 7411, 7417, 7433, 7451, 7457, 7459, 7477, 7481, 7487, 7489, 7499,
                                           7507, 7517, 7523, 7529, 7537, 7541, 7547, 7549, 7559, 7561, 7573, 7577, 7583,
                                           7589, 7591, 7603, 7607, 7621, 7639, 7643, 7649, 7669, 7673, 7681, 7687, 7691,
                                           7699, 7703, 7717, 7723, 7727, 7741, 7753, 7757, 7759, 7789, 7793, 7817, 7823,
                                           7829, 7841, 7853, 7867, 7873, 7877, 7879, 7883, 7901, 7907, 7919};

unsigned int prime(unsigned int number) {
	// check if already precalculated
	if(number < primesLength) return primes[number];

	// set to last of precalculated primes
	unsigned int prime = primes[primesLength - 1];
	unsigned int primeSqrt = 0;
	unsigned int count = primesLength - 1;
	bool prm = false;

	while(count < number) {
		prime += 2;
		prm = true;
		primeSqrt = (unsigned int) sqrt(prime);
		// check if we can divide by known primes
		for(unsigned int i = 2; prm && i < primesLength && primes[i] <= primeSqrt; ++i)
			if(prime % primes[i] == 0) prm = false;
		// check if we can divide by an odd number > known primes
		for(unsigned int n = primes[primesLength - 1] + 2; prm && n <= primeSqrt; n += 2)
			if((prime % n) == 0) prm = false;
		// if prime
		if(prm) count++;
	}
	return prime;
}

unsigned long long int
combineNeighbours(Vertex v, const std::vector<unsigned long long int> &ranks, const lib::graph::GraphType &g,
                  const lib::graph::PropMolecule &pMol) {
	// The bondFactor is an attempt to propagate the bond type.
	unsigned long long int combinedRank = 1;
	for(Edge eOut: asRange(out_edges(v, g))) {
		Vertex vAdj = target(eOut, g);
		unsigned int rankPrime = prime(ranks[get(boost::vertex_index_t(), g, vAdj)]);
		//		auto bt = pMol(eOut);
		//		unsigned int bondFactor = 1;
		//		switch(bt) {
		//		case BondType::Single: bondFactor = 1;
		//			break;
		//		case BondType::Double: bondFactor = 2;
		//			break;
		//		case BondType::Triple: bondFactor = 3;
		//			break;
		//		case BondType::Aromatic: bondFactor = 4;
		//			break;
		//		case BondType::Invalid: MOD_ABORT;
		//		}
		combinedRank *= rankPrime; // * bondFactor;
	}
	return combinedRank;
}

bool isCollapsibleHydrogen(Vertex v, const lib::graph::GraphType &g, const lib::graph::PropMolecule &pMol) {
	return isCleanHydrogen(pMol[v])
	       && degree(v, g) == 1
	       && pMol[*out_edges(v, g).first] == BondType::Single
	       && pMol[*adjacent_vertices(v, g).first].getAtomId() != AtomIds::Invalid; // not an abstract
}

struct AuxData {
	AuxData(const lib::graph::GraphType &g, const lib::graph::PropMolecule &pMol)
			: g(g), pMol(pMol),
			  numSingle(num_vertices(g), 0),
			  numDouble(num_vertices(g), 0),
			  numTriple(num_vertices(g), 0),
			  numAromatic(num_vertices(g), 0),
			  numHydrogen(num_vertices(g), 0) {
		for(Edge e: asRange(edges(g))) {
			Vertex vSrc = source(e, g), vTar = target(e, g);
			VSizeType vSrcId = get(boost::vertex_index_t(), g, vSrc), vTarId = get(boost::vertex_index_t(), g, vTar);
			switch(pMol[e]) {
			case BondType::Invalid:
				MOD_ABORT;
			case BondType::Single:
				numSingle[vSrcId]++;
				numSingle[vTarId]++;
				if(isCollapsibleHydrogen(vSrc, g, pMol)) numHydrogen[vTarId]++;
				if(isCollapsibleHydrogen(vTar, g, pMol)) numHydrogen[vSrcId]++;
				break;
			case BondType::Aromatic:
				numAromatic[vSrcId]++;
				numAromatic[vTarId]++;
				break;
			case BondType::Double:
				numDouble[vSrcId]++;
				numDouble[vTarId]++;
				break;
			case BondType::Triple:
				numTriple[vSrcId]++;
				numTriple[vTarId]++;
				break;
			}
		}
	}

	unsigned char getValenceNoAromatic(Vertex v) const {
		VSizeType vId = get(boost::vertex_index_t(), g, v);
		return numSingle[vId] + numDouble[vId] * 2 + numTriple[vId] * 3;
	}

	unsigned char getNumSingle(Vertex v) const {
		return numSingle[get(boost::vertex_index_t(), g, v)];
	}

	unsigned char getNumDouble(Vertex v) const {
		return numDouble[get(boost::vertex_index_t(), g, v)];
	}

	unsigned char getNumTriple(Vertex v) const {
		return numTriple[get(boost::vertex_index_t(), g, v)];
	}

	unsigned char getNumAromatic(Vertex v) const {
		return numAromatic[get(boost::vertex_index_t(), g, v)];
	}

	unsigned char getNumHydrogen(Vertex v) const {
		return numHydrogen[get(boost::vertex_index_t(), g, v)];
	}

	VSizeType getInvariant(Vertex v) const {
		if(isCleanHydrogen(pMol[v])) return 0;
		else {
			VSizeType degree = out_degree(v, g);
			VSizeType valenceWithAromatic =
					getValenceNoAromatic(v)
					+ (numAromatic[get(boost::vertex_index_t(), g, v)] > 0 ? 1 : 0)
					+ numAromatic[get(boost::vertex_index_t(), g, v)];
			VSizeType hydrogenDegree = numHydrogen[get(boost::vertex_index_t(), g, v)];
			const auto charge = pMol[v].getCharge();
			const auto radical = pMol[v].getRadical();
			assert(pMol[v].getAtomId() != AtomIds::Invalid);
			return 100000000 * (degree - hydrogenDegree)
			       + 1000000 * (valenceWithAromatic - hydrogenDegree)
			       + 10000 * pMol[v].getAtomId()
			       + 1000 * (charge == 0 ? 0 : charge < 0 ? 1 : 2)
			       + 100 * std::abs(charge)
			       + 10 * (radical ? 1 : 0)
			       + hydrogenDegree;
		}
	}
private:
	const lib::graph::GraphType &g;
	const lib::graph::PropMolecule &pMol;
	std::vector<unsigned char> numSingle, numDouble, numTriple, numAromatic, numHydrogen;
};

struct SmilesWriter {
	int getRank(int idx) {
		return (*ranks)[idx];
	}

	SmilesWriter(const lib::graph::GraphType &g,
	             const lib::graph::PropString &pString,
	             const lib::graph::PropMolecule &pMol,
	             const std::vector<int> *ranks,
	             bool withIds)
			: g(g), pString(pString), pMol(pMol), auxData(g, pMol), visited(num_vertices(g), false), ranks(ranks),
			  withIds(withIds), nextOrder(1), vOrder(num_vertices(g), 0), ringIds(num_vertices(g)),
			  branches(num_vertices(g)) {}

	lib::IO::Result<SmilesWriteResult> generate() {
#ifdef DO_DEBUG
		std::cout << "Generating SMILES string" << std::endl;
#endif
		const auto idx = get(boost::vertex_index_t(), g);

		if(!pMol.getHasOnlyChemicalBonds()) {
			MOD_ABORT; // can not get to here currently, as the canonicalization algorithm can't handle non-chemical bonds
		}
		for(const auto v: asRange(vertices(g))) {
			if(pMol[v].getAtomId() != AtomIds::Invalid) continue;
			int bracketCount = 0;
			const auto &str = pString[v];
			const auto error = [idx, v, &str](std::string msg, const auto iter) {
				msg += " in vertex label of vertex ";
				msg += std::to_string(idx[v]);
				msg += ":\n   ";
				msg += str;
				msg += "\n   ";
				msg += std::string(iter - str.begin(), '-');
				msg += '^';
				return lib::IO::Result<>::Error(msg);
			};
			for(auto iter = str.begin(); iter != str.end(); ++iter) {
				const char c = *iter;
				switch(c) {
				case ':':
					return error("Colon character", iter);
				case '[':
					++bracketCount;
					break;
				case ']':
					if(bracketCount == 0)
						return error("Unbalanced square brackets", iter);
					--bracketCount;
					break;
				}
			}
			if(bracketCount != 0)
				return error("Open square bracket", str.end());
		}

		// handle annoying special cases
		if(num_vertices(g) == 0) return SmilesWriteResult{output, isAbstract};
		else if(num_vertices(g) == 1) {
			getLabel(*vertices(g).first);
			return SmilesWriteResult{output, isAbstract};
		} else if(num_vertices(g) == 2) {
			// check for H_2
			auto vIter = vertices(g).first;
			Vertex v1 = *vIter, v2 = *(vIter + 1);
			if(isCleanHydrogen(pMol[v1]) && isCleanHydrogen(pMol[v2])) {
				assert(num_edges(g) == 1);
				Edge e = *edges(g).first;
				if(pMol[e] == BondType::Single) {
					if(withIds) {
						output = "[H:" + std::to_string(idx[v1]) + "][H:" + std::to_string(idx[v2]) + "]";
					} else {
						output = "[H][H]";
					}
					return SmilesWriteResult{output, isAbstract};
				}
			}
		}

		// ignore pure hydrogens as explicit vertices, i.e., set them to already visited
		if(!withIds) {
			for(Vertex v: asRange(vertices(g))) {
				if(isCollapsibleHydrogen(v, g, pMol)) {
					visited[idx[v]] = true;
				}
#ifdef DO_DEBUG
				std::cout << "Visited? " << idx[v] << ": " << pString[v] << ", " << std::boolalpha << visited[idx[v]]
						  << std::endl;
#endif
			}
		}

		Vertex start;
		std::vector<int> cangenRanks;
		if(ranks) {
			//  find the minimum vertex by rank, which has not been visited (i.e., is not a clean hydrogen)
			const auto vStartIter = std::min_element(vertices(g).first, vertices(g).second,
			                                         [this, &idx](Vertex a, Vertex b) {
				                                         const auto ia = idx[a], ib = idx[b];
				                                         const auto va = visited[ia], vb = visited[ib];
				                                         if(va) return false;
				                                         if(vb) return true;
				                                         return getRank(ia) < getRank(ib);
			                                         });
			assert(vStartIter != vertices(g).second);
			start = *vStartIter;
		} else {
			if(!pMol.getIsMolecule())
				return lib::IO::Result<>::Error(
						"Can not use the broken CANGEN canonicalization algorithm with non-molecules.");
			const auto r = canonicalise();
			std::copy(r.begin(), r.end(), std::back_inserter(cangenRanks));
			ranks = &cangenRanks;
			const auto vStartIter = std::find_if(vertices(g).first, vertices(g).second, [this, &idx](Vertex v) {
				return getRank(idx[v]) == 1;
			});
			assert(vStartIter != vertices(g).second);
			start = *vStartIter;
		}

		for(Vertex v: asRange(vertices(g))) {
			if(visited[idx[v]]) continue;
			// in general, there will be quite few ring closures, right?
			//			ringClosures[idx[v]].reserve(degree(v, graph));
			branches[idx[v]].reserve(degree(v, g));
		}
		// note that preBuildDFS takes visited by non-const reference, and will change it, so give it a copy

		preBuildVisited = visited;
		unsigned int conservativeCharCount = preBuildDFS(start, start);
		output.reserve(conservativeCharCount);
		assignRingIds();
		buildDFS(start, start, (Edge()));
#ifdef DO_DEBUG
		std::cout << "Final SMILES string: " << output << std::endl;
#endif
		return SmilesWriteResult{output, isAbstract};
	}

	std::vector<unsigned long long int> canonicalise() {
		const auto idx = get(boost::vertex_index_t(), g);
		VSizeType numNonVisitedVertices = 0; // the number of vertices except for clen hydrogens
#ifdef DO_DEBUG
		std::cout << "new canonicalise" << std::endl;
#endif
		// record data for the initial invariants (and for the subsequent string generation)
		for(Vertex v: asRange(vertices(g))) {
			if(visited[v]) continue;
			numNonVisitedVertices++;
		}
		assert(numNonVisitedVertices > 0);

		// The rank/invariant for each vertex. We actually only need it for non-hydrogens, but we want constant time lookup
		std::vector<unsigned long long int> vertexValue(num_vertices(g), 0);
		// Non-hydrogen vertices, sorted by vertexValue
		std::vector<Vertex> sortedVertices;
		sortedVertices.reserve(numNonVisitedVertices);

		for(Vertex v: asRange(vertices(g))) {
			vertexValue[idx[v]] = auxData.getInvariant(v);
			//		if(vertexValue[idx[v]] != nodeData[idx[v]].getInvariant()) {
			//			std::cout << "Diff invariant for vertex " << idx[v] << " with label '" << vname(v) << "'" << std::endl;
			//			std::cout << "\tnew: " << vertexValue[idx[v]] << std::endl;
			//			std::cout << "\told: " << nodeData[idx[v]].getInvariant() << std::endl;
			//			std::cout << "\tadjacency:" << std::endl;
			//			for(Edge e : asRange(out_edges(v, g))) {
			//				std::cout << "\t\t" << bondToString(molState(e)) << getAtomList()[molState(target(e, g)).getAtomId()] << std::endl;
			//			}
			//		}
			if(!visited[idx[v]]) sortedVertices.push_back(v);
		}

		// set consecutive ranks starting from 1
		std::sort(sortedVertices.begin(), sortedVertices.end(),
		          [&vertexValue, &idx](Vertex u, Vertex v) {
			          return vertexValue[idx[u]] < vertexValue[idx[v]];
		          });
#ifdef DO_DEBUG
		std::cout << "\tinitial values" << std::endl;
		for(Vertex v: sortedVertices) {
			std::cout << "\t\t" << idx[v] << " '" << symbolFromAtomId(pMol[v].getAtomId()) << (int) pMol[v].getCharge()
					  << "' " << vertexValue[idx[v]] << std::endl;
		}
#endif

		VSizeType rank = 1;
		unsigned long long int prevValue = vertexValue[idx[sortedVertices.front()]];
		vertexValue[idx[sortedVertices.front()]] = rank;
		for(VSizeType i = 1; i < sortedVertices.size(); i++) {
			unsigned long long int &thisValue = vertexValue[idx[sortedVertices[i]]];
			if(thisValue != prevValue) rank++;
			prevValue = thisValue;
			thisValue = rank;
		}

#ifdef DO_DEBUG
		std::cout << "\tinitial ranks" << std::endl;
		for(Vertex v: sortedVertices) {
			std::cout << "\t\t" << idx[v] << " '" << symbolFromAtomId(pMol[v].getAtomId()) << (int) pMol[v].getCharge()
					  << "' " << vertexValue[idx[v]] << std::endl;
		}
#endif

		// iterate rank setup until each node has a unique rank
		while(vertexValue[idx[sortedVertices.back()]] != numNonVisitedVertices) { // still tied ranks
			std::vector<unsigned long long int> prevVertexValue = vertexValue; // copy is needed so we can do swaps in inner loop
			do { // change vertex value using neighbours
				swap(prevVertexValue, vertexValue); // relies on the initial copying outside the loop
				// calculate new ranks based on multiples of prime numbers
				for(Vertex v: sortedVertices) {
					assert(!visited[v]);
					vertexValue[idx[v]] = combineNeighbours(v, prevVertexValue, g, pMol);
				}

				// sort the vertices according to the vertexValue, but retain stability with prevVertexValue
				std::sort(sortedVertices.begin(), sortedVertices.end(),
				          [&prevVertexValue, &vertexValue, &idx](Vertex u, Vertex v) {
					          if(prevVertexValue[idx[u]] != prevVertexValue[idx[v]])
						          return prevVertexValue[idx[u]] < prevVertexValue[idx[v]];
					          else return vertexValue[idx[u]] < vertexValue[idx[v]];
				          }
				);
				// assign previous ranks, but possibly break ties with the new vertex values
				VSizeType rank = 1;
				unsigned long long int prevValue = vertexValue[idx[sortedVertices.front()]];
				vertexValue[idx[sortedVertices.front()]] = rank;
				for(unsigned int i = 1; i < sortedVertices.size(); i++) {
					unsigned long long int &thisValue = vertexValue[idx[sortedVertices[i]]];
					if(prevVertexValue[idx[sortedVertices[i - 1]]] != prevVertexValue[idx[sortedVertices[i]]]) rank++;
					else if(prevValue != thisValue) rank++; // yay, tie breaking
					prevValue = thisValue;
					thisValue = rank;
				}
				// TODO: this assertion should not be needed, right? How can be get fewer ranks?
				assert(vertexValue[idx[sortedVertices.back()]] >= prevVertexValue[idx[sortedVertices.back()]]);
#ifdef DO_DEBUG
				std::cout << "\tinner loop ranks" << std::endl;
				for(Vertex v: sortedVertices) {
					std::cout << "\t\t" << idx[v] << " '" << symbolFromAtomId(pMol[v].getAtomId())
							  << (int) pMol[v].getCharge() << "' " << vertexValue[idx[v]] << std::endl;
				}
#endif
			} while(vertexValue[idx[sortedVertices.back()]] != numNonVisitedVertices // continue if we still have ties
			        && vertexValue[idx[sortedVertices.back()]] >
			           prevVertexValue[idx[sortedVertices.back()]]); // continue if we broke any ties

			// still some nodes have equal rank
			if(vertexValue[idx[sortedVertices.back()]] != numNonVisitedVertices) {
				// break a the "smallest" tie arbitrarily
				bool doTieBreak = true;
				unsigned long long int prevValue = vertexValue[idx[sortedVertices.front()]];
				unsigned int rank_newCode = 1;
				vertexValue[idx[sortedVertices.front()]] = rank_newCode;
				for(unsigned int i = 1; i < sortedVertices.size(); i++) {
					unsigned long long int &thisValue = vertexValue[idx[sortedVertices[i]]];
					if(thisValue != prevValue) rank_newCode++;
					else if(doTieBreak) {
						rank_newCode++;
						doTieBreak = false;
					}
					prevValue = thisValue;
					thisValue = rank_newCode;
				}
			}
#ifdef DO_DEBUG
			std::cout << "\tafter tie break ranks" << std::endl;
			for(Vertex v: sortedVertices) {
				std::cout << "\t\t" << idx[v] << " '" << symbolFromAtomId(pMol[v].getAtomId())
						  << (int) pMol[v].getCharge() << "' " << vertexValue[idx[v]] << std::endl;
			}
#endif
		}
		return vertexValue;
	}

	unsigned int preBuildDFS(Vertex v, Vertex p) {
		auto idx = get(boost::vertex_index_t(), g);
#ifdef DO_DEBUG
		std::cout << "preBuildDFS: " << idx[v] << std::endl;
#endif
		assert(!preBuildVisited[idx[v]]);

		preBuildVisited[idx[v]] = true;
		vOrder[idx[v]] = nextOrder++;
		unsigned int charCount = 1; // lower bound on number of characters in final atom label
		std::vector<Edge> edgesNotVisited, edgesVisited;

		for(Edge e: asRange(out_edges(v, g))) {
			if(!withIds && isCollapsibleHydrogen(target(e, g), g, pMol)) continue;
			if(!preBuildVisited[target(e, g)]) edgesNotVisited.push_back(e);
			else edgesVisited.push_back(e);
		}
		const auto comp = [this, &idx](Edge e1, Edge e2) {
			// prefer double and triple bonds, otherwise use ranks
			auto bt1 = pMol[e1];
			auto bt2 = pMol[e2];
			bool isMulti1 = bt1 == BondType::Double || bt1 == BondType::Triple;
			bool isMulti2 = bt2 == BondType::Double || bt2 == BondType::Triple;
			if(isMulti1 != isMulti2) return isMulti2;
			return getRank(idx[target(e1, g)]) < getRank(idx[target(e2, g)]);
		};
		std::sort(edgesNotVisited.begin(), edgesNotVisited.end(), comp);
		std::sort(edgesVisited.begin(), edgesVisited.end(), comp);

		for(Edge e: edgesNotVisited) {
			Vertex tar = target(e, g);
			if(!preBuildVisited[tar]) {
				branches[idx[v]].push_back(e);
				charCount += preBuildDFS(tar, v); //branch
			} else { // open ring
				if(tar != p) {
					ringClosures.emplace_back(e, 0);
					charCount++;
				}
			}
		}
		for(Edge e: edgesVisited) {
			Vertex tar = target(e, g);
			if(tar != p) charCount++;
		}
		return charCount;
	}

	void assignRingIds() {
		// for now, mimic the old behaviour
		auto idx = get(boost::vertex_index_t(), g);
		std::sort(begin(ringClosures), end(ringClosures),
		          [this, idx](const std::pair<Edge, unsigned char> &p1, const std::pair<Edge, unsigned char> &p2) {
			          Vertex vSrc1 = source(p1.first, g), vTar1 = target(p1.first, g);
			          Vertex vSrc2 = source(p2.first, g), vTar2 = target(p2.first, g);
			          if(vOrder[idx[vSrc1]] != vOrder[idx[vSrc2]]) return vOrder[idx[vSrc1]] < vOrder[idx[vSrc2]];
			          return vOrder[idx[vTar1]] < vOrder[idx[vTar2]];
			          // prefer double and triple bonds, otherwise use ranks
			          auto bt1 = pMol[p1.first];
			          auto bt2 = pMol[p2.first];
			          bool isMulti1 = bt1 == BondType::Double || bt1 == BondType::Triple;
			          bool isMulti2 = bt2 == BondType::Double || bt2 == BondType::Triple;
			          if(isMulti1 != isMulti2) return isMulti2;
			          return getRank(idx[vTar1]) < getRank(idx[vTar2]);
		          });
		if(ringClosures.size() >= 100) {
			std::cout << "ERROR in SMILES writing. Too many ring closures with current scheme." << std::endl;
			MOD_ABORT;
		}
		for(unsigned int i = 0; i < ringClosures.size(); i++)
			ringClosures[i].second = i + 1; // start with 1 instead of 0
		auto ringClosuresCopy = ringClosures;
		std::sort(begin(ringClosuresCopy), end(ringClosuresCopy),
		          [this, idx](const std::pair<Edge, unsigned char> &p1, const std::pair<Edge, unsigned char> &p2) {
			          Vertex vSrc1 = source(p1.first, g), vTar1 = target(p1.first, g);
			          Vertex vSrc2 = source(p2.first, g), vTar2 = target(p2.first, g);
			          if(vOrder[idx[vTar1]] != vOrder[idx[vTar2]]) return vOrder[idx[vTar1]] < vOrder[idx[vTar2]];
			          // prefer double and triple bonds, otherwise use ranks
			          auto bt1 = pMol[p1.first];
			          auto bt2 = pMol[p2.first];
			          bool isMulti1 = bt1 == BondType::Double || bt1 == BondType::Triple;
			          bool isMulti2 = bt2 == BondType::Double || bt2 == BondType::Triple;
			          if(isMulti1 != isMulti2) return isMulti2;
			          return getRank(idx[vSrc1]) < getRank(idx[vSrc2]);
		          });
		// open before close
		for(const auto &p: ringClosures) ringIds[idx[source(p.first, g)]].emplace_back(BondType::Invalid, p.second);
		for(const auto &p: ringClosuresCopy) ringIds[idx[target(p.first, g)]].emplace_back(pMol[p.first], p.second);
	}

	void buildDFS(Vertex v, Vertex p, Edge vp) {
		const auto idx = get(boost::vertex_index_t(), g);
#ifdef DO_DEBUG
		std::cout << "buildDFS(v" << idx[v] << ", p=" << idx[p] << "), initial: " << output << std::endl;
#endif
		assert(!visited[v]);
		visited[v] = true;
		if(v != p) {
			bool printBond = false;
			auto btvp = pMol[vp];
			if(btvp != BondType::Single) printBond = true;
			else {
				// TODO: if v and p will be marked aromatic, then we should print the bond anyway
			}
			if(printBond) output += bondToChar(btvp);
		}

		getLabel(v);
#ifdef DO_DEBUG
		std::cout << "buildDFS(v" << idx[v] << ", p=" << idx[p] << "), after label: " << output << std::endl;
#endif
		//		{ // debug vOrder
		//			output += "<" + std::to_string(vOrder[idx[v]]) + ">";
		//		}

		for(const auto &p: ringIds[idx[v]]) {
			bool printBond = false;
			if(p.first != BondType::Invalid) {
				if(p.first != BondType::Single) printBond = true;
				else {
					// TODO: if both target and source will be marked aromatic, then we should print the bond anyway
				}
				if(printBond) output += bondToChar(p.first);
			}

			if(p.second < 10) output += '0' + p.second;
			else {
				output += '%';
				output += std::to_string((int) p.second);
			}
		}

		// all except the last branches should be en parentheses
		for(unsigned int i = 0; i + 1 < branches[idx[v]].size(); i++) {
			Edge e = branches[idx[v]][i];
			Vertex tar = target(e, g);
			output += '(';
			buildDFS(tar, v, e); //branch
			output += ')';
		}
		if(!branches[idx[v]].empty()) {
			Edge e = branches[idx[v]].back();
			Vertex tar = target(e, g);
			buildDFS(tar, v, e);
		}
#ifdef DO_DEBUG
		std::cout << "buildDFS(v" << idx[v] << ", p=" << idx[p] << "), end: " << output << std::endl;
#endif
	}

	void getLabel(Vertex v) {
		bool hasBrackets = true;
		if(pMol[v].getCharge() == 0 && !pMol[v].getRadical() && pMol[v].getIsotope() == Isotope()
		   && isInSmilesOrganicSubset(pMol[v].getAtomId())
		   && isValenceForOrganicSubsetNormal(v)) {
			// if the hydrogen count is such that it can be deduced, then remove brackets
			hasBrackets = false;
		}
		if(withIds) hasBrackets = true;
		if(!hasBrackets) {
			appendSymbolFromAtomId(output, pMol[v].getAtomId());
			return;
		}
		assert(hasBrackets);

		output += '[';
		if(pMol[v].getAtomId() == AtomIds::Invalid) {
			output += pString[v];
			isAbstract = true;
		} else {
			if(pMol[v].getIsotope() != Isotope())
				output += std::to_string(pMol[v].getIsotope());
			appendSymbolFromAtomId(output, pMol[v].getAtomId());
			unsigned char numH = auxData.getNumHydrogen(v);
			if(numH > 0 && !withIds) {
				output += 'H';
				if(numH > 1) {
					if(numH >= 10) MOD_ABORT;
					output += '0' + numH;
				}
			}
			const auto charge = pMol[v].getCharge();
			if(charge != 0) {
				output += charge > 0 ? '+' : '-';
				if(std::abs(charge) > 1) {
					if(std::abs(charge) > 9) MOD_ABORT;
					output += ('0' + std::abs(charge));
				}
			}
			if(pMol[v].getRadical()) {
				output += '.';
			}
		}
		if(withIds) {
			output += ':';
			output += std::to_string(get(boost::vertex_index_t(), g, v));
		}
		output += ']';
	}

	bool isValenceForOrganicSubsetNormal(Vertex v) {
		//==========================================================================
		// WARNING: keep in sync with the implicit hydrogen adder
		//==========================================================================
		// this is intended to check if one of the organic subset atoms can be written without brackets
		assert(pMol[v].getCharge() == 0);
		// B, C, N, O, P, S, F, Cl, Br, I
		// B			3
		// C			4
		// N			3, 5, {{-, :, :}}, {{-, -, =}}, {{:, :, :}}
		// O			2
		// P			3, 5
		// S			2, 4, 6, {{:, :}}
		// halogens		1
		using namespace AtomIds;
		using BondVector = std::tuple<unsigned char, unsigned char, unsigned char, unsigned char>;
		BondVector numBonds{auxData.getNumSingle(v), auxData.getNumDouble(v), auxData.getNumTriple(v),
		                    auxData.getNumAromatic(v)};
		unsigned char valence = auxData.getValenceNoAromatic(v) + auxData.getNumAromatic(v);
		if(auxData.getNumAromatic(v) > 0) valence++;
		unsigned char hCount = auxData.getNumHydrogen(v);
		switch(pMol[v].getAtomId()) {
		case Boron:
			if(valence == 3) return true;
			else if(valence > 3) return hCount == 0;
			else return false;
		case Carbon:
			if(valence == 4) return true;
			else if(valence > 4) return hCount == 0;
			else return false;
		case Nitrogen:
			if(numBonds == BondVector(1, 0, 0, 2) && hCount == 0) return true;
			if(numBonds == BondVector(2, 1, 0, 0) && hCount == 0) return true;
			if(numBonds == BondVector(0, 0, 0, 3) && hCount == 0) return true;
			[[fallthrough]];
		case Phosphorus:
			switch(valence) {
			case 3:
				return true;
			case 5:
				return hCount < 2;
			default:
				return valence > 5 && hCount == 0;
			}
		case Oxygen:
			if(valence == 2) return true;
			else return valence > 2 && hCount == 0;
		case Sulfur:
			if(numBonds == BondVector(0, 0, 0, 2) && hCount == 0) return true;
			switch(valence) {
			case 2:
				return true;
			case 4:
			case 6:
				return hCount < 2;
			default:
				return valence > 6 && hCount == 0;
			}
		case Fluorine:
		case Chlorine:
		case Bromine:
		case Iodine:
			if(valence == 1) return true;
			return valence > 1 && hCount == 0;
		default:
			MOD_ABORT;
		}
	}
private:
	const lib::graph::GraphType &g;
	const lib::graph::PropString &pString;
	const lib::graph::PropMolecule &pMol;
	const AuxData auxData;
	std::vector<bool> visited, preBuildVisited;
	const std::vector<int> *ranks;
	const bool withIds;
	VSizeType nextOrder;
	std::vector<VSizeType> vOrder;
	std::vector<std::pair<Edge, unsigned char>> ringClosures;
	std::vector<std::vector<std::pair<BondType, unsigned char>>> ringIds;
	std::vector<std::vector<Edge>> branches;
	std::string output;
	bool isAbstract = false;
};

} // namespace

lib::IO::Result<SmilesWriteResult>
getSmiles(const lib::graph::GraphType &g,
          const lib::graph::PropString &pString,
          const lib::graph::PropMolecule &pMol,
          const std::vector<int> *ranks, bool withIds) {
	SmilesWriter writer(g, pString, pMol, ranks, withIds);
	return writer.generate();
}

} // namespace mod::lib::Chem