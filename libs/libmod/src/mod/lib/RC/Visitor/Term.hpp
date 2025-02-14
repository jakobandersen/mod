#ifndef MOD_LIB_RC_VISITOR_TERM_HPP
#define MOD_LIB_RC_VISITOR_TERM_HPP

#include <mod/lib/GraphMorphism/TermVertexMap.hpp>
#include <mod/lib/RC/Visitor/Compound.hpp>
#include <mod/lib/Rule/Properties/Term.hpp>
#include <mod/lib/Term/WAM.hpp>
#include <mod/lib/Term/IO/Write.hpp>

namespace mod::lib::RC::Visitor {

static constexpr std::size_t TERM_MAX = std::numeric_limits<std::size_t>::max();

struct Term {
	using Membership = lib::DPO::Membership;
	using AddressType = lib::Term::AddressType;
	using Cell = lib::Term::Cell;
	using CellTag = lib::Term::Cell::Tag;
public:
	Term(const lib::rule::LabelledRule &rFirst, const lib::rule::LabelledRule &rSecond)
			: rFirst(rFirst), rSecond(rSecond) {}

	template<bool Verbose, typename InvertibleVertexMap, typename Result>
	bool init(IO::Logger logger, const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
	          InvertibleVertexMap &match, Result &result) {
		assert(&dpoFirst == &rFirst.getRule());
		assert(&dpoSecond == &rSecond.getRule());

		auto &data = get_prop(GraphMorphism::TermDataT(), match);
		auto &machine = data.machine;
		machine.verify();
		result.pTerm = std::make_unique<typename Result::PropTermType>(*result.rDPO, std::move(machine));
		if(Verbose) {
			logger.indent() << "New machine:\n";
			++logger.indentLevel;
			lib::Term::Write::wam(getMachine(*result.pTerm), lib::Term::getStrings(), logger);
		}
		return true;
	}

	template<bool Verbose, typename InvertibleVertexMap, typename Result>
	bool finalize(IO::Logger logger, const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
	              InvertibleVertexMap &match, Result &result) {
		result.pTerm->verify();
		return true;
	}
public:
	template<bool Verbose, typename InvertibleVertexMap, typename Result, typename VertexFirst, typename VertexResult>
	void copyVertexFirst(IO::Logger logger,
	                     const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
	                     const InvertibleVertexMap &match,
	                     const Result &result,
	                     const VertexFirst &vFirst, const VertexResult &vResult) {
		assert(result.pTerm);
		const auto &pFirst = *rFirst.pTerm;
		auto &pResult = *result.pTerm;
		auto m = membership(rFirst, vFirst);
		assert(m == result.rDPO->getCombinedGraph()[vResult].membership);
		switch(m) {
		case Membership::L:
			pResult.add(vResult, pFirst.getLeft()[vFirst], TERM_MAX);
			break;
		case Membership::R:
			pResult.add(vResult, TERM_MAX, pFirst.getRight()[vFirst]);
			break;
		case Membership::K:
			pResult.add(vResult, pFirst.getLeft()[vFirst], pFirst.getRight()[vFirst]);
			break;
		}
		if(Verbose) {
			logger.indent() << "Current machine:\n";
			++logger.indentLevel;
			lib::Term::Write::wam(getMachine(*result.pTerm), lib::Term::getStrings(), logger);
		}
	}

	template<bool Verbose, typename InvertibleVertexMap, typename Result, typename VertexSecond, typename VertexResult>
	void copyVertexSecond(IO::Logger logger,
	                      const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
	                      const InvertibleVertexMap &match,
	                      const Result &result,
	                      const VertexSecond &vSecond, const VertexResult &vResult) {
		assert(result.pTerm);
		const auto &pSecond = *rSecond.pTerm;
		auto &pResult = *result.pTerm;
		auto m = membership(rSecond, vSecond);
		assert(m == result.rDPO->getCombinedGraph()[vResult].membership);
		if(m != Membership::L) fixSecondTerm<Verbose>(logger, pSecond.getRight()[vSecond], result);
		if(m != Membership::R) fixSecondTerm<Verbose>(logger, pSecond.getLeft()[vSecond], result);
		switch(m) {
		case Membership::L:
			pResult.add(vResult, deref(pSecond.getLeft()[vSecond], result), TERM_MAX);
			break;
		case Membership::R:
			pResult.add(vResult, TERM_MAX, deref(pSecond.getRight()[vSecond], result));
			break;
		case Membership::K:
			pResult.add(vResult, deref(pSecond.getLeft()[vSecond], result), deref(pSecond.getRight()[vSecond], result));
			break;
		}
		if(Verbose) {
			logger.indent() << "Current machine:\n";
			++logger.indentLevel;
			lib::Term::Write::wam(getMachine(*result.pTerm), lib::Term::getStrings(), logger);
		}
	}

	template<bool Verbose, typename InvertibleVertexMap, typename Result, typename EdgeFirst, typename EdgeResult>
	void copyEdgeFirst(IO::Logger logger,
	                   const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
	                   const InvertibleVertexMap &match,
	                   const Result &result,
	                   const EdgeFirst &eFirst, const EdgeResult &eResult) {
		// the membership of e may be different from eResult
		assert(result.pTerm);
		const auto &pFirst = *rFirst.pTerm;
		auto &pResult = *result.pTerm;
		auto m = result.rDPO->getCombinedGraph()[eResult].membership;
		switch(m) {
		case Membership::L:
			pResult.add(eResult, pFirst.getLeft()[eFirst], TERM_MAX);
			break;
		case Membership::R:
			pResult.add(eResult, TERM_MAX, pFirst.getRight()[eFirst]);
			break;
		case Membership::K:
			pResult.add(eResult, pFirst.getLeft()[eFirst], pFirst.getRight()[eFirst]);
			break;
		}
		if(Verbose) {
			logger.indent() << "Current machine:\n";
			++logger.indentLevel;
			lib::Term::Write::wam(getMachine(*result.pTerm), lib::Term::getStrings(), logger);
		}
	}

	template<bool Verbose, typename InvertibleVertexMap, typename Result, typename EdgeSecond, typename EdgeResult>
	void copyEdgeSecond(IO::Logger logger,
	                    const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
	                    const InvertibleVertexMap &match,
	                    const Result &result,
	                    const EdgeSecond &eSecond, const EdgeResult &eResult) {
		// the membership of e may be different from eResult
		assert(result.pTerm);
		const auto &pSecond = *rSecond.pTerm;
		auto &pResult = *result.pTerm;
		auto m = result.rDPO->getCombinedGraph()[eResult].membership;
		if(m != Membership::L) fixSecondTerm<Verbose>(logger, pSecond.getRight()[eSecond], result);
		if(m != Membership::R) fixSecondTerm<Verbose>(logger, pSecond.getLeft()[eSecond], result);
		switch(m) {
		case Membership::L:
			pResult.add(eResult, deref(pSecond.getLeft()[eSecond], result), TERM_MAX);
			break;
		case Membership::R:
			pResult.add(eResult, TERM_MAX, deref(pSecond.getRight()[eSecond], result));
			break;
		case Membership::K:
			pResult.add(eResult, deref(pSecond.getLeft()[eSecond], result), deref(pSecond.getRight()[eSecond], result));
			break;
		}
		if(Verbose) {
			logger.indent() << "Current machine:\n";
			++logger.indentLevel;
			lib::Term::Write::wam(getMachine(*result.pTerm), lib::Term::getStrings(), logger);
		}
	}
public:
	template<typename InvertibleVertexMap, typename Result, typename VertexFirst>
	void printVertexFirst(const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
	                      const InvertibleVertexMap &match,
	                      const Result &result,
	                      std::ostream &s, const VertexFirst &vFirst) {
		rFirst.pTerm->print(s, vFirst);
	}

	template<typename InvertibleVertexMap, typename Result, typename VertexSecond>
	void printVertexSecond(const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
	                       const InvertibleVertexMap &match,
	                       const Result &result,
	                       std::ostream &s, const VertexSecond &vSecond) {
		rSecond.pTerm->print(s, vSecond);
	}

	template<typename InvertibleVertexMap, typename Result, typename VertexResult>
	void printVertexResult(const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
	                       const InvertibleVertexMap &match,
	                       const Result &result,
	                       std::ostream &s, const VertexResult &vResult) {
		result.pTerm->print(s, vResult);
	}

	template<typename InvertibleVertexMap, typename Result, typename EdgeFirst>
	void printEdgeFirst(const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
	                    const InvertibleVertexMap &match,
	                    const Result &result,
	                    std::ostream &s, const EdgeFirst &eFirst) {
		rFirst.pTerm->print(s, eFirst);
	}

	template<typename InvertibleVertexMap, typename Result, typename EdgeSecond>
	void printEdgeSecond(const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
	                     const InvertibleVertexMap &match,
	                     const Result &result,
	                     std::ostream &s, const EdgeSecond &eSecond) {
		rSecond.pTerm->print(s, eSecond);
	}
public:
	template<bool Verbose, typename InvertibleVertexMap, typename Result, typename VertexResult, typename VertexSecond>
	void composeVertexRvsLR(IO::Logger logger,
									const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
	                        const InvertibleVertexMap &match,
	                        const Result &result,
	                        VertexResult vResult, VertexSecond vSecond) {
		//   -> a | a -> b, maybe a == b
		auto addr = rSecond.pTerm->getRight()[vSecond];
		fixSecondTerm<Verbose>(logger, addr, result);
		result.pTerm->setRight(vResult, deref(addr, result));
		//   -> b
	}

	template<bool Verbose, typename InvertibleVertexMap, typename Result, typename VertexResult, typename VertexSecond>
	void composeVertexLRvsL(IO::Logger logger,
									const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
	                        const InvertibleVertexMap &match,
	                        const Result &result,
	                        VertexResult vResult, VertexSecond vSecond) {
		// vFirst is CONTEXT, so do nothing
		// a -> a | a ->
		// b -> a | a ->
		// to
		// a ->
		// b ->
	}

	template<bool Verbose, typename InvertibleVertexMap, typename Result, typename VertexResult, typename VertexSecond>
	void composeVertexLRvsLR(IO::Logger logger,
									 const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
	                         const InvertibleVertexMap &match,
	                         const Result &result,
	                         VertexResult vResult, VertexSecond vSecond) {
		// the left label of vResult is ok, but the right label might have to change
		// a != b, a != c, b =? c
		// a -> a | a -> a
		// a -> a | a -> c
		// b -> a | a -> a
		// b -> a | a -> c
		auto addr = rSecond.pTerm->getRight()[vSecond];
		fixSecondTerm<Verbose>(logger, addr, result);
		result.pTerm->setRight(vResult, deref(addr, result));
	}
public:
	template<bool Verbose, typename InvertibleVertexMap, typename Result, typename EdgeResult, typename EdgeSecond>
	void
	setEdgeResultRightFromSecondRight(IO::Logger logger,
												 const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
	                                  const InvertibleVertexMap &match, const Result &result,
	                                  EdgeResult eResult, EdgeSecond eSecond) {
		auto addr = rSecond.pTerm->getRight()[eSecond];
		fixSecondTerm<Verbose>(logger, addr, result);
		result.pTerm->setRight(eResult, deref(addr, result));
	}
private:
	template<bool Verbose, typename Result>
	void fixSecondTerm(IO::Logger logger, std::size_t addr, Result &result) {
		auto &m = getMachine(*result.pTerm);
		m.verify();
		if(Verbose) {
			logger.indent() << "Copy " << addr << '\n';
			++logger.indentLevel;
			lib::Term::Write::wam(m, lib::Term::getStrings(), logger);
			--logger.indentLevel;
		}
		m.copyFromTemp(addr);
		if(Verbose) {
			logger.indent() << "After copy " << addr << '\n';
			++logger.indentLevel;
			lib::Term::Write::wam(m, lib::Term::getStrings(), logger);
			--logger.indentLevel;
		}
		m.verify();
	}

	template<typename Result>
	std::size_t deref(std::size_t addrTemp, Result &result) {
		getMachine(*result.pTerm).verify();
		auto addr = getMachine(*result.pTerm).deref({AddressType::Temp, addrTemp});
		assert(addr.type == AddressType::Heap);
		return addr.addr;
	}
private:
	const lib::rule::LabelledRule &rFirst;
	const lib::rule::LabelledRule &rSecond;
};

} // namespace mod::lib::RC::Visitor

#endif // MOD_LIB_RC_VISITOR_TERM_HPP
