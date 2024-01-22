#ifndef JLA_BOOST_GRAPH_MORPHISM_MODELS_PROPERTY_VERTEX_MAP_HPP
#define JLA_BOOST_GRAPH_MORPHISM_MODELS_PROPERTY_VERTEX_MAP_HPP

// A wrapper that attaches arbitrary tagged data to a vertex map implementation.

#include <jla_boost/graph/morphism/Concepts.hpp>

#include <graph_canon/tagged_list.hpp>

namespace jla_boost {
namespace GraphMorphism {
using graph_canon::tagged_element;
using graph_canon::tagged_list;

template<typename VertexMap, typename ...Props>
struct PropertyVertexMap {
	BOOST_CONCEPT_ASSERT((VertexMapConcept<VertexMap>));
public: // VertexMap
	using GraphDom = typename VertexMapTraits<VertexMap>::GraphDom;
	using GraphCodom = typename VertexMapTraits<VertexMap>::GraphCodom;
	using Storable = typename VertexMapTraits<VertexMap>::Storable;

	template<typename GraphDomU, typename GraphCodomU>
	static auto reinterpret(PropertyVertexMap &&m, const GraphDom &gDom, const GraphCodom &gCodom,
			const GraphDomU &gDomReinterpreted, const GraphCodomU &gCodomReinterpreted) {
		auto innerReinterpreted = VertexMapTraits<VertexMap>::template reinterpret<GraphDomU, GraphCodomU>
				(std::move(m.m), gDom, gCodom, gDomReinterpreted, gCodomReinterpreted);
		return PropertyVertexMap<decltype(innerReinterpreted), Props...>(std::move(innerReinterpreted), std::move(m.props));
	}

	template<typename Trans>
	static auto transform(Trans trans, PropertyVertexMap &&m, const GraphDom &gDom, const GraphCodom &gCodom) {
		auto innerTransformed = VertexMapTraits<VertexMap>::transform(trans, std::move(m.m), gDom, gCodom);
		return PropertyVertexMap<decltype(innerTransformed), Props...>(std::move(innerTransformed), std::move(m.props));
	}
public:

	PropertyVertexMap(VertexMap m, tagged_list<Props...> props) : m(std::move(m)), props(std::move(props)) { }

	friend typename boost::graph_traits<GraphCodom>::vertex_descriptor
	get(const PropertyVertexMap &m, const GraphDom &gDom, const GraphCodom &gCodom,
			typename boost::graph_traits<GraphDom>::vertex_descriptor v) {
		return get(m.m, gDom, gCodom, v);
	}

	friend typename boost::graph_traits<GraphDom>::vertex_descriptor
	get_inverse(const PropertyVertexMap &m, const GraphDom &gDom, const GraphCodom &gCodom,
			typename boost::graph_traits<GraphCodom>::vertex_descriptor v) {
		return get_inverse(m.m, gDom, gCodom, v);
	}
public:
// see https://www.spinics.net/lists/fedora-devel/msg312638.html
#pragma GCC diagnostic push
#if __GNUC__ >= 13
#pragma GCC diagnostic ignored "-Wdangling-reference"
#endif
	template<typename Tag, typename = typename std::enable_if<(1 <= graph_canon::meta::size<typename graph_canon::detail::tagged_list_matches<Tag, Props...>::type>::value)>::type>
	friend decltype(auto) get_prop(Tag&&, PropertyVertexMap &m) {
		return get(Tag(), m.props);
	}

	template<typename Tag, typename = typename std::enable_if<(1 <= graph_canon::meta::size<typename graph_canon::detail::tagged_list_matches<Tag, Props...>::type>::value)>::type>
	friend decltype(auto) get_prop(Tag&&, const PropertyVertexMap &m) {
		return get(Tag(), m.props);
	}
#pragma GCC diagnostic pop
public:
	VertexMap m;
	tagged_list<Props...> props;
};

namespace detail {

template<typename VertexMap>
struct addPropImpl {

	template<typename Tag, typename Value>
	static auto add(VertexMap &&m, Tag, Value &&v) {
		using E = tagged_element<Tag, Value>;
		using M = PropertyVertexMap<VertexMap, E>;
		return M(std::forward<VertexMap>(m), graph_canon::make_tagged_list(E{std::forward<Value>(v)}));
	}
};

template<typename VertexMap, typename ...Props>
struct addPropImpl<PropertyVertexMap<VertexMap, Props...> > {

	template<typename Tag, typename Value>
	static auto add(PropertyVertexMap<VertexMap, Props...> &&m, Tag, Value &&v) {
		using E = tagged_element<Tag, Value>;
		using M = PropertyVertexMap<VertexMap, E, Props...>;
		using Concat = graph_canon::tagged_list_concat<E, tagged_list<Props...> >;
		return M(std::move(m.m), Concat::make(E{std::forward<Value>(v)}, std::move(m.props)));
	}
};

} // namespace detail

template<typename VertexMap, typename Tag, typename Value>
auto addProp(VertexMap &&m, Tag, Value &&v) {
	return detail::addPropImpl<VertexMap>::add(std::forward<VertexMap>(m), Tag(), std::forward<Value>(v));
}

} // namespace GraphMorphism
} // namespace jla_boost

#endif /* JLA_BOOST_GRAPH_MORPHISM_MODELS_PROPERTY_VERTEX_MAP_HPP */