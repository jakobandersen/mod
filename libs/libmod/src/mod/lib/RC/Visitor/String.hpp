#ifndef MOD_LIB_RC_VISITOR_STRING_HPP
#define MOD_LIB_RC_VISITOR_STRING_HPP

#include <mod/lib/RC/Visitor/Compound.hpp>

#include <mod/lib/Rule/Properties/String.hpp>

namespace mod::lib::RC::Visitor {

struct String {
	using Membership = lib::DPO::Membership;
public:
	String(const lib::rule::LabelledRule &rFirst, const lib::rule::LabelledRule &rSecond)
			: rFirst(rFirst), rSecond(rSecond) {}

	template<bool Verbose, typename InvertibleVertexMap, typename Result>
	bool init(IO::Logger logger, const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
	          const InvertibleVertexMap &match, Result &result) {
		assert(&dpoFirst == &rFirst.getRule());
		assert(&dpoSecond == &rSecond.getRule());

		result.pString = std::make_unique<typename Result::PropStringType>(*result.rDPO);
		return true;
	}

	template<bool Verbose, typename InvertibleVertexMap, typename Result>
	bool finalize(IO::Logger logger, const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
	              InvertibleVertexMap &match, Result &result) {
		result.pString->verify();
		return true;
	}
public:
	template<bool Verbose, typename InvertibleVertexMap, typename Result, typename VertexFirst, typename VertexResult>
	void copyVertexFirst(IO::Logger logger,
								const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
								const InvertibleVertexMap &match, const Result &result,
								const VertexFirst &vFirst, const VertexResult &vResult) {
		copyVertex(rFirst, result, vFirst, vResult, *rFirst.pString);
	}

	template<bool Verbose, typename InvertibleVertexMap, typename Result, typename VertexSecond, typename VertexResult>
	void copyVertexSecond(IO::Logger logger,
								 const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
								 const InvertibleVertexMap &match, const Result &result,
								 const VertexSecond &vSecond, const VertexResult &vResult) {
		copyVertex(rSecond, result, vSecond, vResult, *rSecond.pString);
	}

	template<bool Verbose, typename InvertibleVertexMap, typename Result, typename EdgeFirst, typename EdgeResult>
	void copyEdgeFirst(IO::Logger logger,
							 const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
							 const InvertibleVertexMap &match, const Result &result,
							 const EdgeFirst &eFirst, const EdgeResult &eResult) {
		copyEdge(rFirst, result, eFirst, eResult, *rFirst.pString);
	}

	template<bool Verbose, typename InvertibleVertexMap, typename Result, typename EdgeSecond, typename EdgeResult>
	void copyEdgeSecond(IO::Logger logger,
							  const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
							  const InvertibleVertexMap &match, const Result &result,
							  const EdgeSecond &eSecond, const EdgeResult &eResult) {
		copyEdge(rSecond, result, eSecond, eResult, *rSecond.pString);
	}
public:
	template<typename InvertibleVertexMap, typename Result, typename VertexFirst>
	void printVertexFirst(const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
								 const InvertibleVertexMap &match, const Result &result,
								 std::ostream &s, const VertexFirst &vFirst) {
		rFirst.pString->print(s, vFirst);
	}

	template<typename InvertibleVertexMap, typename Result, typename VertexSecond>
	void printVertexSecond(const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
								  const InvertibleVertexMap &match, const Result &result,
								  std::ostream &s, const VertexSecond &vSecond) {
		rSecond.pString->print(s, vSecond);
	}

	template<typename InvertibleVertexMap, typename Result, typename VertexResult>
	void printVertexResult(const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
								  const InvertibleVertexMap &match, const Result &result,
								  std::ostream &s, const VertexResult &vResult) {
		result.pString->print(s, vResult);
	}

	template<typename InvertibleVertexMap, typename Result, typename EdgeFirst>
	void printEdgeFirst(const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
							  const InvertibleVertexMap &match, const Result &result,
							  std::ostream &s, const EdgeFirst &eFirst) {
		rFirst.pString->print(s, eFirst);
	}

	template<typename InvertibleVertexMap, typename Result, typename EdgeSecond>
	void printEdgeSecond(const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
								const InvertibleVertexMap &match, const Result &result,
								std::ostream &s, const EdgeSecond &eSecond) {
		rSecond.pString->print(s, eSecond);
	}

public:
	template<bool Verbose, typename InvertibleVertexMap, typename Result, typename VertexResult, typename VertexSecond>
	void composeVertexRvsLR(IO::Logger logger,
									const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
									const InvertibleVertexMap &match, const Result &result,
									VertexResult vResult, VertexSecond vSecond) {
		//   -> a | a -> b, maybe a == b
		result.pString->setRight(vResult, rSecond.pString->getRight()[vSecond]);
		//   -> b
	}

	template<bool Verbose, typename InvertibleVertexMap, typename Result, typename VertexResult, typename VertexSecond>
	void composeVertexLRvsL(IO::Logger logger,
									const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
									const InvertibleVertexMap &match, const Result &result,
									VertexResult vResult, VertexSecond vSecond) {
		// vFirst is CONTEXT, so do nothing
		// a -> a | a ->
		// b -> a | a ->
		//		stateNew->setLeft(vNew, stateNew->getLeft()[vNew]);
		// a ->
		// b ->
	}

	template<bool Verbose, typename InvertibleVertexMap, typename Result, typename VertexResult, typename VertexSecond>
	void composeVertexLRvsLR(IO::Logger logger,
									 const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
									 const InvertibleVertexMap &match, const Result &result,
									 VertexResult vResult, VertexSecond vSecond) {
		// the left label of vResult is ok, but the right label might have to change
		// a != b, a != c, b =? c
		// a -> a | a -> a
		// a -> a | a -> c
		// b -> a | a -> a
		// b -> a | a -> c
		result.pString->setRight(vResult, rSecond.pString->getRight()[vSecond]);
	}
public:
	template<bool Verbose, typename InvertibleVertexMap, typename Result, typename EdgeResult, typename EdgeSecond>
	void
	setEdgeResultRightFromSecondRight(IO::Logger logger,
												 const lib::DPO::CombinedRule &dpoFirst, const lib::DPO::CombinedRule &dpoSecond,
												 const InvertibleVertexMap &match, const Result &result,
												 EdgeResult eResult, EdgeSecond eSecond) {
		result.pString->setRight(eResult, rSecond.pString->getRight()[eSecond]);
	}
private:
	template<typename RuleFrom, typename Result, typename VertexFrom, typename VertexResult, typename Prop>
	void copyVertex(const RuleFrom &rFrom, const Result &result, VertexFrom vFrom, VertexResult vResult,
						 const Prop &pFrom) {
		assert(result.pString);
		auto &pString = *result.pString;
		auto m = membership(rFrom, vFrom);
		assert(m == result.rDPO->getCombinedGraph()[vResult].membership);
		switch(m) {
		case Membership::L:
			pString.add(vResult, pFrom.getLeft()[vFrom], "");
			break;
		case Membership::R:
			pString.add(vResult, "", pFrom.getRight()[vFrom]);
			break;
		case Membership::K:
			pString.add(vResult, pFrom.getLeft()[vFrom], pFrom.getRight()[vFrom]);
			break;
		}
	}

	template<typename RuleFrom, typename Result, typename EdgeFrom, typename EdgeResult, typename Prop>
	void copyEdge(const RuleFrom &rFrom, const Result &result, EdgeFrom eFrom, EdgeResult eResult, const Prop &pFrom) {
		// the membership of eFrom may be different from eResult
		assert(result.pString);
		auto &pString = *result.pString;
		auto m = result.rDPO->getCombinedGraph()[eResult].membership;
		switch(m) {
		case Membership::L:
			pString.add(eResult, pFrom.getLeft()[eFrom], "");
			break;
		case Membership::R:
			pString.add(eResult, "", pFrom.getRight()[eFrom]);
			break;
		case Membership::K:
			pString.add(eResult, pFrom.getLeft()[eFrom], pFrom.getRight()[eFrom]);
			break;
		}
	}
private:
	const lib::rule::LabelledRule &rFirst;
	const lib::rule::LabelledRule &rSecond;
};

} // namespace mod::lib::RC::Visitor

#endif // MOD_LIB_RC_VISITOR_STRING_HPP