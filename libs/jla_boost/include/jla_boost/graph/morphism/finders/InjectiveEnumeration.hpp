#ifndef JLA_BOOST_GRAPH_MORPHISM_FINDERS_INJECTIVEENUMERATION_HPP
#define JLA_BOOST_GRAPH_MORPHISM_FINDERS_INJECTIVEENUMERATION_HPP

#include <jla_boost/graph/morphism/models/Vector.hpp>
#include <jla_boost/graph/morphism/PropertyTags.hpp>

#ifdef MORPHISM_INJECTIVE_ENUMERATION_DEBUG

#include <iostream>

#endif

namespace jla_boost::GraphMorphism {
using namespace boost; // TODO: remvoe

// The stack is partitioned into 3 sections, in the following order from the bottom:
// 1: The unchecked matches.
//    The vertices mapped by the user, which may not abide the defined constraints.
// 2: The checked matches.
//    The vertices mapped by the user, which fulfill the defined constraints.
//    preTryPush, prePop
// 3: The dynamic match.
//    The vertices mapped by the enumeration algorithm.
//    tryPush, pop

template<typename Derived, typename GraphLeft, typename GraphRight, typename EdgePred, typename VertexPred>
struct InjectiveEnumerationState {
	using VertexLeft = typename boost::graph_traits<GraphLeft>::vertex_descriptor;
	using VertexRight = typename boost::graph_traits<GraphRight>::vertex_descriptor;
	using EdgeLeft = typename boost::graph_traits<GraphLeft>::edge_descriptor;
	using EdgeRight = typename boost::graph_traits<GraphRight>::edge_descriptor;
public:
	static VertexLeft vNullLeft() {
		return boost::graph_traits<GraphLeft>::null_vertex();
	}

	static VertexRight vNullRight() {
		return boost::graph_traits<GraphRight>::null_vertex();
	}
protected:
	InjectiveEnumerationState(const GraphLeft &gLeft, const GraphRight &gRight, EdgePred edgePred, VertexPred vertexPred)
			: gLeft(gLeft), gRight(gRight), edgePred(edgePred), vertexPred(vertexPred),
			// alg state
			  m(gLeft, gRight), mInverse(gRight, gLeft) {}
private:
	void pushImpl(VertexLeft vLeft, VertexRight vRight) {
		assert(rightFromLeft(vLeft) == vNullRight());
		assert(leftFromRight(vRight) == vNullLeft());
		put(m, gLeft, gRight, vLeft, vRight);
		put(mInverse, gRight, gLeft, vRight, vLeft);
		stack.emplace_back(vLeft, vRight);
	}

	std::pair<VertexLeft, VertexRight> popNoAsserts() {
		auto p = stack.back();
		put(m, gLeft, gRight, p.first, vNullRight());
		put(mInverse, gRight, gLeft, p.second, vNullLeft());
		stack.pop_back();
		return p;
	}
public:
	std::size_t getPreForcedStackSize() const {
		return uncheckedEnd;
	}

	std::size_t getPreStackSize() const {
		return checkedEnd;
	}

	std::size_t getTotalStackSize() const {
		return stack.size();
	}

	VertexRight rightFromLeft(VertexLeft vLeft) const {
		return get(m, gLeft, gRight, vLeft);
	}

	VertexLeft leftFromRight(VertexRight vRight) const {
		return get(mInverse, gRight, gLeft, vRight);
	}

	auto getVertexMap() const {
		return makeInvertibleVertexMapAdaptor(std::cref(m), std::cref(mInverse));
	}

	auto getSizedVertexMap() const {
		return addProp(getVertexMap(), PreImageSizeT(), this->getTotalStackSize());
	}

	void preForcePush(VertexLeft vLeft, VertexRight vRight) {
		assert(uncheckedEnd == checkedEnd); // don't do this after preTryPush
		assert(stack.size() == uncheckedEnd); // or during enumeration
		pushImpl(vLeft, vRight);
		++checkedEnd;
		++uncheckedEnd;
	}

	std::pair<VertexLeft, VertexRight> preForcePop() {
		assert(stack.size() == uncheckedEnd); // don't do this during enumeration
		assert(uncheckedEnd == checkedEnd); // and only if not preTryPush has succeeded
		assert(!stack.empty()); // and of course only if there is anything
		--uncheckedEnd;
		--checkedEnd;
		return popNoAsserts();
	}

	// returns true iff the push succeeded
	bool preTryPush(VertexLeft vLeft, VertexRight vRight) {
		assert(stack.size() == checkedEnd); // don't do this during enumeration
		const bool res = tryPush(vLeft, vRight);
		if(res) ++checkedEnd;
		return res;
	}

	std::pair<VertexLeft, VertexRight> prePop() {
		assert(stack.size() == checkedEnd); // don't do this during enumeration
		assert(stack.size() > uncheckedEnd); // and only if preTryPush has been used
		--checkedEnd;
		return popNoAsserts();
	}

	// returns true iff the push succeeded
	bool tryPush(VertexLeft vLeft, VertexRight vRight) {
		assert(rightFromLeft(vLeft) == vNullRight());
		assert(leftFromRight(vRight) == vNullLeft());
#ifdef MORPHISM_INJECTIVE_ENUMERATION_DEBUG
		std::cout << indent() << "tryPush(" << vLeft << ", " << vRight << "):" << std::endl;
#endif
		if(!vertexPred(vLeft, vRight)) {
#ifdef MORPHISM_INJECTIVE_ENUMERATION_DEBUG
			std::cout << indent(1) << "failed: vertexPred" << std::endl;
#endif
			return false;
		}

		bool res = static_cast<Derived &>(*this).visit_tryPush(vLeft, vRight);
		if(!res) {
#ifdef MORPHISM_INJECTIVE_ENUMERATION_DEBUG
			std::cout << indent(1) << "failed: visit_tryPush" << std::endl;
#endif
			return false;
		}
#ifdef MORPHISM_INJECTIVE_ENUMERATION_DEBUG
		std::cout << indent(1) << "success" << std::endl;
#endif
		// now actually do the push
		pushImpl(vLeft, vRight);
		return true;
	}

	std::pair<VertexLeft, VertexRight> pop() {
		assert(stack.size() > checkedEnd);
		auto res = popNoAsserts();
#ifdef MORPHISM_INJECTIVE_ENUMERATION_DEBUG
		std::cout << indent() << "pop(" << res.first << ", " << res.second << ")" << std::endl;
#endif
		return res;
	}
protected: // args
	const GraphLeft &gLeft;
	const GraphRight &gRight;
	EdgePred edgePred;
	VertexPred vertexPred;
private: // alg state
	// we do very specific puts, so we manage the two directions manually
	VectorVertexMap<GraphLeft, GraphRight> m;
	VectorVertexMap<GraphRight, GraphLeft> mInverse;
	std::vector<std::pair<VertexLeft, VertexRight>> stack;
	std::size_t uncheckedEnd = 0;
	std::size_t checkedEnd = 0;
protected:
#ifdef MORPHISM_INJECTIVE_ENUMERATION_DEBUG
	int debug_indent = 0;
	std::string indent(int extra = 0) {
		return std::string((debug_indent + extra) * 2, ' ');
	}
#endif
};

} // namespace jla_boost::GraphMorphism

#endif // JLA_BOOST_GRAPH_MORPHISM_FINDERS_INJECTIVEENUMERATION_HPP