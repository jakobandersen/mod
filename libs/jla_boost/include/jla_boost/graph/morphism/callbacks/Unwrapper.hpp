#ifndef JLA_BOOST_GRAPH_MORPHISM_CALLBACKS_UNWRAPPER_HPP
#define JLA_BOOST_GRAPH_MORPHISM_CALLBACKS_UNWRAPPER_HPP

// Forward a reinterpreted morphism where the domain and codomain graphs have
// been unwrapped.

#include <jla_boost/graph/AdaptorTraits.hpp>
#include <jla_boost/graph/morphism/Concepts.hpp>

namespace jla_boost::GraphMorphism {

template<typename Next>
struct UnwrapperBoth {
	UnwrapperBoth(Next next) : next(next) {}

	template<typename VertexMap>
	bool operator()(VertexMap &&m,
	                const typename VertexMapTraits<VertexMap>::GraphDom &gDom,
	                const typename VertexMapTraits<VertexMap>::GraphCodom &gCodom) const {
		BOOST_CONCEPT_ASSERT((VertexMapConcept<VertexMap>));
		using GraphDom = typename VertexMapTraits<VertexMap>::GraphDom;
		using GraphCodom = typename VertexMapTraits<VertexMap>::GraphCodom;
		using GraphDomUnwrapped = typename GraphAdaptorTraits<GraphDom>::type;
		using GraphCodomUnwrapped = typename GraphAdaptorTraits<GraphCodom>::type;
		const GraphDomUnwrapped &gDomUnwrapped = GraphAdaptorTraits<GraphDom>::unwrap(gDom);
		const GraphCodomUnwrapped &gCodomUnwrapped = GraphAdaptorTraits<GraphCodom>::unwrap(gCodom);
		return next(VertexMapTraits<VertexMap>::template reinterpret<GraphDomUnwrapped, GraphCodomUnwrapped>(
				            std::forward<VertexMap>(m),
				            gDom, gCodom, gDomUnwrapped, gCodomUnwrapped),
		            gDomUnwrapped, gCodomUnwrapped);
	}
private:
	Next next;
};

template<typename Next>
UnwrapperBoth<Next> unwrapBoth(Next next) {
	return UnwrapperBoth<Next>(next);
}

template<typename Next>
struct UnwrapperDom {
	UnwrapperDom(Next next) : next(next) {}

	template<typename VertexMap>
	bool operator()(VertexMap &&m,
	                const typename VertexMapTraits<VertexMap>::GraphDom &gDom,
	                const typename VertexMapTraits<VertexMap>::GraphCodom &gCodom) const {
		BOOST_CONCEPT_ASSERT((VertexMapConcept<VertexMap>));
		using GraphDom = typename VertexMapTraits<VertexMap>::GraphDom;
		using GraphCodom = typename VertexMapTraits<VertexMap>::GraphCodom;
		using GraphDomUnwrapped = typename GraphAdaptorTraits<GraphDom>::type;
		const GraphDomUnwrapped &gDomUnwrapped = GraphAdaptorTraits<GraphDom>::unwrap(gDom);
		return next(VertexMapTraits<VertexMap>::template reinterpret<GraphDomUnwrapped, GraphCodom>(
				            std::forward<VertexMap>(m),
				            gDom, gCodom, gDomUnwrapped, gCodom),
		            gDomUnwrapped, gCodom);
	}
private:
	Next next;
};

template<typename Next>
UnwrapperDom<Next> unwrapDom(Next next) {
	return UnwrapperDom<Next>(next);
}

template<typename Next>
struct UnwrapperCodom {
	UnwrapperCodom(Next next) : next(next) {}

	template<typename VertexMap>
	bool operator()(VertexMap &&m,
	                const typename VertexMapTraits<VertexMap>::GraphDom &gDom,
	                const typename VertexMapTraits<VertexMap>::GraphCodom &gCodom) const {
		BOOST_CONCEPT_ASSERT((VertexMapConcept<VertexMap>));
		using GraphDom = typename VertexMapTraits<VertexMap>::GraphDom;
		using GraphCodom = typename VertexMapTraits<VertexMap>::GraphCodom;
		using GraphCodomUnwrapped = typename GraphAdaptorTraits<GraphCodom>::type;
		const GraphCodomUnwrapped &gCodomUnwrapped = GraphAdaptorTraits<GraphCodom>::unwrap(gCodom);
		return next(VertexMapTraits<VertexMap>::template reinterpret<GraphDom, GraphCodomUnwrapped>(
				            std::forward<VertexMap>(m),
				            gDom, gCodom, gDom, gCodomUnwrapped),
		            gDom, gCodomUnwrapped);
	}
private:
	Next next;
};

template<typename Next>
UnwrapperCodom<Next> unwrapCodom(Next next) {
	return UnwrapperCodom<Next>(next);
}

} // namespace jla_boost::GraphMorphism

#endif // JLA_BOOST_GRAPH_MORPHISM_CALLBACKS_UNWRAPPER_HPP