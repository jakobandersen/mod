#ifndef MOD_DG_VERTEXMAPPER_HPP
#define MOD_DG_VERTEXMAPPER_HPP

#include <mod/dg/GraphInterface.hpp>
#include <mod/graph/Union.hpp>
#include <mod/VertexMap.hpp>

#include <boost/iterator/iterator_adaptor.hpp>

namespace mod::graph {
struct Union;
} // mod::graph
namespace mod::dg {
// rst-class: dg::VertexMapper
// rst:
// rst:		A class for enumerating all valid vertex maps for a given :class:`dg::DG::HyperEdge`.
// rst:		That is, for such a hyperedge, collect the graphs associated with
// rst:		respectively the source and target vertices, and create the disjoint union of those graphs.
// rst:		Let the result be the graphs :math:`G'` and :math:`H'`.
// rst:		Then each rule :math:`p = (L\leftarrow K\rightarrow R)` associated with the hyperedge,
// rst:		generate direct derivations :math:`G\overset{p, m}{\Rightarrow} H`
// rst:		where :math:`G` is isomorphic to :math:`G'` and :math:`H` is isomorphic to :math:`H'`.
// rst:
// rst:		Each recorded vertex map is a map :math:`V(G) \rightarrow V(H)`.
// rst:		Those maps are available in this class.
// rst:
// rst-class-start:
struct MOD_DECL VertexMapper {
	// rst: .. type:: Map = VertexMap<graph::Union, graph::Union>;
	// rst:
	// rst:		The vertex map type.
	using Map = VertexMap<graph::Union, graph::Union>;
	// rst: .. class:: Result
	// rst:
	// rst:		The value type returned for each vertex map.
	// rst:		the rule used to generate the map, and then the actual map.
	// rst:
	struct Result {
		// rst: 	.. var:: std::shared_ptr<rule::Rule> r
		// rst:
		// rst: 		The rule used to generate the map.
		std::shared_ptr<rule::Rule> r;
		// rst: 	.. var:: Map map
		// rst:
		// rst: 		The actual vertex map :math:`V(G) \rightarrow V(H)`.
		Map map;
	};
public:
	// rst: .. class:: iterator
	// rst:
	// rst:		A random-access iterator over :type:`Result`\ s.
	struct iterator : public boost::iterator_facade<iterator, Result, std::random_access_iterator_tag, Result> {
		iterator() = default;
	private:
		friend class VertexMapper;
		iterator(const VertexMapper *owner, std::size_t i) : owner(owner), i(i) {}
	private:
		friend class boost::iterator_core_access;
		value_type dereference() const;
		bool equal(const iterator &other) const { return std::tie(owner, i) == std::tie(other.owner, other.i); }
		void increment() { ++i; }
		void decrement() { --i; }
		void advance(iterator::difference_type n) { i += n; }
		iterator::difference_type distance_to(const iterator &other) const { return other.i - i; }
	private:
		const VertexMapper *owner = nullptr;
		std::size_t i = 0;
	};
	// rst: .. type:: const_iterator = iterator
	using const_iterator = iterator;
public:
	// rst: .. function:: VertexMapper(DG::HyperEdge e)
	// rst:               VertexMapper(DG::HyperEdge e, bool upToIsomorphismG, int leftLimit, int rightLimit, int verbosity)
	// rst:
	// rst:		Construct a vertex map holder. It will immediately calculate all vertex maps for the derivations underlying the
	// rst:		given hyperedge.
	// rst:		By default all maps :math:`V(G) \rightarrow V(H)` are enumerated.
	// rst:		To only enumerate a singular vertex map per rule, use `upToIsomorphismG = true` and `leftLimit = rightLimit = 1`.
	// rst:
	// rst:		:param e: the hyperedge to construct vertex maps for.
	// rst:		:param upToIsomrophismG: whether to enumerate all :math:`m`, or just those such that all bottom spans
	// rst:			:math:`(G\leftarrow D\rightarrow H)` up to isomorphism are generated.
	// rst:			Defaults to `true`.
	// rst:		:param leftLimit: after bottom span generation, find this many isomorphisms back to the sources of the hyperedge.
	// rst:			Defaults to 1.
	// rst:		:param rightLimit: after bottom span generation, find this many isomorphisms back to the targets of the hyperedge.
	// rst:			Defaults to :math:`2^{30}`.
	// rst:		:param verbosity: the level of debug information to print. Defaults to 0.
	// rst:
	// rst:			- 0 (or less): print no information.
	// rst:			- 1: print debug information within the vertex mapping, but not debug information related to rule composition.
	// rst:			- 10: also print information for morphism generation for rule composition.
	// rst:			- 20: also print rule composition information.
	// rst:
	// rst:		:throws: :class:`LogicError` if `!e`.
	VertexMapper(DG::HyperEdge e);
	VertexMapper(DG::HyperEdge e, bool upToIsomorphismG, int leftLimit, int rightLimit, int verbosity);
	~VertexMapper();
	// rst: .. function:: DG::HyperEdge getEdge() const
	// rst:
	// rst:		:returns: the hyperedge for which the mapper calculates vertex maps.
	DG::HyperEdge getEdge() const;
	// rst: .. function:: graph::Union getLeft() const
	// rst:               graph::Union getRight() const
	// rst:
	// rst:		:returns: the disjoint union of graphs from respectively the source and target vertices of the hyperedge.
	// rst:			That is, the graphs :math:`G` and :math:`H` that are the domain and codomain graphs of the calculated
	// rst:			vertex maps.
	graph::Union getLeft() const;
	graph::Union getRight() const;
	// rst: .. function:: const_iterator begin() const
	// rst:               const_iterator end() const
	// rst:
	// rst:		:returns: iterators for the range of vertex maps calculated by the mapper.
	const_iterator begin() const;
	const_iterator end() const;
	// rst: .. function:: std::size_t size() const
	// rst:
	// rst:		:returns: `end() - begin()`
	std::size_t size() const;
	// rst: .. function:: Result operator[](std::size_t i) const
	// rst:
	// rst:		:returns: `begin()[i]`
	// rst:		:throws: :class:`LogicError` if `i >= size()`.
	Result operator[](std::size_t i) const;
private:
	struct Pimpl;
	std::shared_ptr<Pimpl> p;
};
// rst-class-end:

} // namespace mod::dg

#endif // MOD_DG_VERTEXMAPPER_HPP
