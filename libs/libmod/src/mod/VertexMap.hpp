#ifndef MOD_VERTEXMAP_HPP
#define MOD_VERTEXMAP_HPP

#include <mod/Error.hpp>
#include <mod/GraphConcepts.hpp>

#include <ostream>

namespace mod {

// rst-class: template<typename Domain, typename Codomain> VertexMap
// rst:
// rst:	A class template used to represent general vertex maps.
// rst:
// rst-class-start:
template<typename Domain, typename Codomain>
struct VertexMap {
	BOOST_CONCEPT_ASSERT((mod::concepts::LabelledGraph<Domain>));
	BOOST_CONCEPT_ASSERT((mod::concepts::LabelledGraph<Codomain>));
public:
	// rst: .. type:: DomainHandle = GraphHandle<Domain>
	// rst:           CodomainHandle = GraphHandle<Codomain>
	using DomainHandle = GraphHandle<Domain>;
	using CodomainHandle = GraphHandle<Codomain>;
	// rst: .. type:: DomVertex = typename Domain::Vertex
	// rst:           CodomVertex = typename Codomain::Vertex
	using DomVertex = typename Domain::Vertex;
	using CodomVertex = typename Codomain::Vertex;
public:
	VertexMap(DomainHandle dom, CodomainHandle codom,
			  std::function<CodomVertex(DomVertex)> forward,
			  std::function<DomVertex(CodomVertex)> backward)
			: dom(dom), codom(codom), forward(forward), backward(backward) {}

	// rst: .. function:: friend std::ostream &operator<<(std::ostream &s, const VertexMap &m)
	friend std::ostream &operator<<(std::ostream &s, const VertexMap &m) {
		return s << "VertexMap{" << getGraphFromHandle(m.getDomain())
				 << ", " << getGraphFromHandle(m.getCodomain()) << "}";
	}

	// rst: .. function:: DomainHandle getDomain() const
	// rst:               CodomainHandle getCodomain() const
	// rst:
	// rst:		:returns: the domain and codomain graphs for the vertex map.
	DomainHandle getDomain() const { return dom; }
	CodomainHandle getCodomain() const { return codom; }
	// rst: .. function:: CodomVertex operator[](DomVertex v) const
	// rst:
	// rst:		:returns: the image of the given domain vertex.
	// rst:			May return a null vertex if the map is partial.
	// rst:		:throws: :class:`LogicError` if `!v`.
	// rst:		:throws: :class:`LogicError` if `v.getGraph() != getDomain()`.
	CodomVertex operator[](DomVertex v) const {
		if(!v) throw LogicError("Can not map null vertex.");
		if(v.getGraph() != getDomain()) throw LogicError("Vertex does not belong to the domain graph.");
		return forward(v);
	}

	// rst: .. function:: DomVertex getInverse(CodomVertex v) const
	// rst:
	// rst:		:returns: the domain vertex that maps to the given codomain vertex.
	// rst:			May return a null vertex if non exist.
	// rst:		:throws: :class:`LogicError` if `!v`.
	// rst:		:throws: :class:`LogicError` if `v.getGraph() != getCodomain()`.
	DomVertex getInverse(CodomVertex v) const {
		if(!v) throw LogicError("Can not map null vertex.");
		if(v.getGraph() != getCodomain()) throw LogicError("Vertex does not belong to the codomain graph.");
		return backward(v);
	}
private:
	DomainHandle dom;
	CodomainHandle codom;
	std::function<CodomVertex(DomVertex)> forward;
	std::function<DomVertex(CodomVertex)> backward;
};
// rst-class-end:

} // namespace mod

#endif // MOD_VERTEXMAP_HPP