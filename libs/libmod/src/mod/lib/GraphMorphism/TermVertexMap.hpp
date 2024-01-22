#ifndef MOD_LIB_GRAPHMORPHISM_TERMVERTEXMAP_HPP
#define MOD_LIB_GRAPHMORPHISM_TERMVERTEXMAP_HPP

#include <mod/Config.hpp>
#include <mod/Error.hpp>
#include <mod/lib/Term/WAM.hpp>

#include <jla_boost/graph/morphism/Concepts.hpp>
#include <jla_boost/graph/morphism/models/PropertyVertexMap.hpp>

// - TermPredConstants (compare terms, variables are equal to everything)
// - ToTermVertexMap
// - Filters for isRenaming and isSpecialisation

// for debugging
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/Term/IO/Write.hpp>

#include <iostream>

namespace mod::lib::GraphMorphism {

// TermPredConstants (compare terms, variables are equal to everything)
//------------------------------------------------------------------------------

template<typename Next>
struct TermPredConstants {
	TermPredConstants(Next next) : next(next) {}

	template<typename VEDom, typename VECodom, typename LabGraphDom, typename LabGraphCodom>
	bool
	operator()(const VEDom &veDom, const VECodom &veCodom, const LabGraphDom &gDom, const LabGraphCodom &gCodom) const {
		const auto &pDom = get_term(gDom);
		const auto &pCodom = get_term(gCodom);
		const auto aDom = get(pDom, veDom);
		const auto aCodom = get(pCodom, veCodom);
		using Handler = typename LabGraphDom::PropTermType::Handler;
		const bool res = Handler::reduce(
				std::logical_and<>(),
				Handler::fmap2(aDom, aCodom, gDom, gCodom,
				               [this, &pDom, &pCodom](std::size_t l, std::size_t r,
				                                      auto &&... args) {
					               // note: the parameter pack _must_ be named because of fucked up rules with varargs
					               // http://lbrandy.com/blog/2013/02/c11s-six-dots/
					               // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0281r0.html
					               return this->compare(l, r, getMachine(pDom),
					                                    getMachine(pCodom));
				               }));
		return res && next(veDom, veCodom, gDom, gCodom);
	}
private:
	bool compare(std::size_t addrLeft, std::size_t addrRight, const Term::Wam &machineLeft,
	             const Term::Wam &machineRight) const {
		assert(stack.empty());
		// maybe_unused to silence warnings on GCC < 9
		// (perhaps this bug? https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85827)
		// TODO: remove attribute when GCC 9 is required
		[[maybe_unused]] constexpr bool DEBUG = false;
		stack.emplace_back(addrLeft, addrRight);
		if constexpr(DEBUG) {
			std::cout << "TermConstEqual:\n";
			Term::Write::wam(machineLeft, Term::getStrings(), IO::Logger(std::cout));
			Term::Write::wam(machineRight, Term::getStrings(), IO::Logger(std::cout));
		}
		while(!stack.empty()) {
			std::size_t l, r;
			std::tie(l, r) = stack.back();
			stack.pop_back();
			if constexpr(DEBUG) std::cout << "comp(" << l << ", " << r << ")\n";
			using AddressType = Term::AddressType;
			using Address = Term::Address;
			using Cell = Term::Cell;
			using CellTag = Term::Cell::Tag;
			Address addrLhs = machineLeft.deref({AddressType::Heap, l});
			Address addrRhs = machineRight.deref({AddressType::Heap, r});
			if constexpr(DEBUG) std::cout << "compDeref(" << addrLhs.addr << ", " << addrRhs.addr << ")\n";
			Cell lhs = machineLeft.getCell(addrLhs);
			Cell rhs = machineRight.getCell(addrRhs);
			// if at least one is a variable we can't decide
			if(lhs.tag == CellTag::REF || rhs.tag == CellTag::REF) {
				if constexpr(DEBUG) std::cout << "maybe, refs involved\n";
				continue;
			}
			// they should be dereferenced
			assert(lhs.tag == CellTag::Structure);
			assert(rhs.tag == CellTag::Structure);
			if(lhs.Structure.arity != rhs.Structure.arity) {
				if constexpr(DEBUG) std::cout << "FALSE, arity\n";
				stack.clear();
				return false;
			}
			if(lhs.Structure.name != rhs.Structure.name) {
				if constexpr(DEBUG) std::cout << "FALSE, name\n";
				stack.clear();
				return false;
			}
			for(std::size_t i = lhs.Structure.arity; i > 0; --i) {
				stack.emplace_back(addrLhs.addr + i, addrRhs.addr + i);
			}
		}
		if constexpr(DEBUG) std::cout << "TRUE\n";
		return true;
	}
private:
	Next next;
	mutable std::vector<std::pair<std::size_t, std::size_t> > stack;
};

template<typename Next = jla_boost::AlwaysTrue>
TermPredConstants<Next> makeTermPredConstants(Next next = jla_boost::AlwaysTrue()) {
	return TermPredConstants<Next>(next);
}

// ToTermVertexMap
//------------------------------------------------------------------------------

namespace GM = jla_boost::GraphMorphism;

struct TermDataT {
};

struct TermData {
	Term::Wam machine;
	Term::MGU mgu;
};

struct TermAssociationHandlerUnify {
	template<typename OuterGraphDom, typename OuterGraphCodom>
	bool operator()(std::size_t l, std::size_t r, const OuterGraphDom &gDom, const OuterGraphCodom &gCodom,
	                Term::Wam &res, Term::MGU &mgu) const {
		constexpr bool DEBUG = false;
		if(DEBUG) {
			auto &s = std::cout;
			s << "TermAssociationHandlerUnify:\n";
			Term::Write::wam(res, Term::getStrings(), IO::Logger(s));
		}
		res.unifyHeapTemp(r, l, mgu);
		if(DEBUG) {
			auto &s = std::cout;
			s << "\tunifyHeapTemp(" << r << ", " << l << ")\n";
			switch(mgu.status) {
			case Term::MGU::Status::Exists:
				s << "\tExists\n";
				break;
			case Term::MGU::Status::Fail:
				s << "\tFail\n";
				break;
			}
		}
		if(mgu.status != Term::MGU::Status::Exists) return false;
		else {
			res.verify();
			return true;
		}
	}
};

template<typename LabGraphDom, typename LabGraphCodom, typename Next>
struct ToTermVertexMap {
	ToTermVertexMap(const LabGraphDom &gDom, const LabGraphCodom &gCodom, Next next)
			: lgDom(gDom), lgCodom(gCodom), next(next) {
		if(!isValid(get_term(gDom))) MOD_ABORT;
		if(!isValid(get_term(gCodom))) MOD_ABORT;
	}

	template<typename VertexMap, typename GraphDom, typename GraphCodom>
	bool operator()(VertexMap &&m, const GraphDom &gDom, const GraphCodom &gCodom) const {
		BOOST_CONCEPT_ASSERT((GM::VertexMapConcept<VertexMap>));
		// Note: GraphMorDom is not necessarily the same as LabGraphDom::GraphType
		//       as LabGraphDom can be a reindexed labelled filtered graph,
		//       and GraphMorDom is the plain filtered graph without reindexing.
		//       Therefore, do not use get_graph(lgDom) and get_graph(lgCodom)!
		const auto &pDomain = get_term(lgDom);
		const auto &pCodomain = get_term(lgCodom);
		assert(isValid(pDomain));
		assert(isValid(pCodomain));

		Term::Wam machine(getMachine(pCodomain));
		machine.setTemp(getMachine(pDomain));
		Term::MGU mgu(machine.getHeap().size());
		using Handler = typename LabGraphDom::PropTermType::Handler;
		for(const auto vDom: asRange(vertices(gDom))) {
			const auto vCodom = get(m, gDom, gCodom, vDom);
			if(vCodom == boost::graph_traits<GraphCodom>::null_vertex()) continue;
			const bool ok = Handler::reduce(
					std::logical_and<>(),
					Handler::fmap2(get(pDomain, vDom), get(pCodomain, vCodom), lgDom, lgCodom,
					               TermAssociationHandlerUnify(), machine, mgu
					));
			if(!ok) return true;
		}
		for(const auto eDom: asRange(edges(gDom))) {
			const auto vDomSrc = source(eDom, gDom);
			const auto vDomTar = target(eDom, gDom);
			const auto vCodomSrc = get(m, gDom, gCodom, vDomSrc);
			const auto vCodomTar = get(m, gDom, gCodom, vDomTar);
			if(vCodomSrc == boost::graph_traits<GraphCodom>::null_vertex()) continue;
			if(vCodomTar == boost::graph_traits<GraphCodom>::null_vertex()) continue;
			const auto peCodom = edge(vCodomSrc, vCodomTar, gCodom);
			if(!peCodom.second) continue; // no edge in the other side
			assert(peCodom.second);
			const auto eCodom = peCodom.first;
			const bool ok = Handler::reduce(
					std::logical_and<>(),
					Handler::fmap2(get(pDomain, eDom), get(pCodomain, eCodom), lgDom, lgCodom,
					               TermAssociationHandlerUnify(), machine, mgu
					));
			if(!ok) return true;
		}
		TermData data{std::move(machine), std::move(mgu)};
		return next(GM::addProp(std::move(m), TermDataT(), std::move(data)), gDom, gCodom);
	}
private:
	const LabGraphDom &lgDom;
	const LabGraphCodom &lgCodom;
	Next next;
};

template<typename LabGraphDom, typename LabGraphCodom, typename Next>
auto makeToTermVertexMap(const LabGraphDom &gDom, const LabGraphCodom &gCodom, Next next) {
	return ToTermVertexMap<LabGraphDom, LabGraphCodom, Next>(gDom, gCodom, next);
}

// Filters for isRenaming and isSpecialisation
//------------------------------------------------------------------------------

struct TermFilterRenaming {
	template<typename VertexMap, typename GraphDom, typename GraphCodom>
	bool operator()(const VertexMap &m, const GraphDom &gDom, const GraphCodom &gCodom) const {
		//		Term::Write::wam(m.machine, Term::getStrings(), std::cout);
		//		Term::Write::mgu(m.machine, m.mgu, Term::getStrings(), std::cout) << "\n";
		const auto &data = get_prop(TermDataT(), m);
		bool res = data.mgu.isRenaming(data.machine);
		//		std::cout << "Result: " << std::boolalpha << res << "\n";
		return res;
	}
};

struct TermFilterSpecialisation {
	template<typename VertexMap, typename GraphDom, typename GraphCodom>
	bool operator()(const VertexMap &m, const GraphDom &gDom, const GraphCodom &gCodom) const {
		//		Term::Write::wam(m.machine, Term::getStrings(), std::cout);
		//		Term::Write::mgu(m.machine, m.mgu, Term::getStrings(), std::cout) << "\n";
		const auto &data = get_prop(TermDataT(), m);
		bool res = data.mgu.isSpecialisation(data.machine);
		//		std::cout << "Result: " << std::boolalpha << res << "\n";
		return res;
	}
};

} // namespace mod::lib::GraphMorphism

#endif // MOD_LIB_GRAPHMORPHISM_TERMVERTEXMAP_HPP