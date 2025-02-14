#ifndef MOD_DG_VERTEXMAPPER_HPP
#define MOD_DG_VERTEXMAPPER_HPP

#include <mod/dg/GraphInterface.hpp>
#include <mod/graph/Union.hpp>
#include <mod/rule/GraphInterface.hpp>
#include <mod/VertexMap.hpp>

#include <boost/iterator/iterator_facade.hpp>

namespace mod::graph {
struct Union;
} // mod::graph
namespace mod::dg {
// rst-class: dg::VertexMapper
// rst:
// rst:		A class for enumerating vertex maps for a given :class:`dg::DG::HyperEdge`.
// rst:
// rst:		For the given hyperedge, collect the graphs associated with
// rst:		respectively the source and target vertices, and create the disjoint union of those graphs.
// rst:		Let the result be the graphs :math:`G` and :math:`H`, available via `getLeft()` and `getRight()` respectively.
// rst:		Then each rule :math:`p = (L\leftarrow K\rightarrow R)` associated with the hyperedge,
// rst:		generate direct derivations :math:`\require{mathtools} G\xRightarrow{p, m} H'`.
// rst:		An isomorphism :math:`H'\rightarrow H` is then found to ensure we have generated the correct product.
// rst:		Each result can be described in the following commutative diagram.
// rst:
// rst:		.. tikz:: A diagram describing each result generated by the vertex mapper.
// rst:			A consists of a double-pushout diagram for a direct derivation :math:`G\xRightarrow{p, m} H'`
// rst:			and an isomorphism :math:`H'\rightarrow H`.
// rst:
// rst:			\matrix[matrix of math nodes, row sep=4em, column sep=4em]{
// rst:				|(L)| L		\& |(K)| K		\& |(R)| R	\\
// rst:				|(G)| G		\& |(D)| D		\& |(H')| H\smash{'}	\& |(H)| H	\\
// rst:			};
// rst:			\foreach \s/\t in {K/L, K/R, K/D} {
// rst:				\draw[->] (\s) to (\t);
// rst:			}
// rst:			\draw[->] (L) to node[auto] {$m$} (G);
// rst:			\draw[->] (D) to node[above] {$g\vphantom{h}$} (G);
// rst:			\draw[->] (D) to node[above] {$h\vphantom{g}$} (H');
// rst:			\draw[->] (R) to node[auto] {$a$} (H');
// rst:			\draw[->] (H') to node[auto] {$b$} (H);
// rst:			\draw[->] (R) to node[auto] {$m' = b\circ a$} (H);
// rst:
// rst:		Each result is available in the form of three vertex maps:
// rst:
// rst:		- `Result::map` (:math:`V(G) \rightarrow V(H)`): the vertex map of the derivation,
// rst:		  which maps vertices of the input graph :math:`G` to the product graph :math:`H`.
// rst:		  The map is defined as the composition :math:`b\circ h\circ g^{-1}`.
// rst:		  Note that if the rule :math:`p` either creates or removes vertices, then the map is partial.
// rst:		  As all morphisms are injective, the vertex map is as well.
// rst:		- `Result::match` (:math:`m\colon L\rightarrow G`): the match morphism.
// rst:		- `Result::comatch` (:math:`m'\colon L\rightarrow H`): the comatch morphism.
// rst:		  It is defined as the composition :math:`b\circ a`.
// rst:
// rst:		The vertex mapper can be configured in two ways via the constructor:
// rst:
// rst:		- ``upToIsomorphismGDH``: this controls which spans :math:`G\leftarrow D\rightarrow H'` and match morphisms :math:`m\colon L\rightarrow G`
// rst:		  are enumerated. When set `true` only a single representative of the span is generated per isomorphism class.
// rst:		- ``rightLimit``: this controls the amount of isomorphisms :math:`b\colon H'\rightarrow H` are generated.
// rst:
// rst:		For example:
// rst:
// rst:		- If you just want a single result, then use ``upToIsomorphismGDH = true`` and ``rightLimit = 1``.
// rst:		- If you want all different vertex maps :math:`V(G)\rightarrow V(H)`,
// rst:		  then use ``upToIsomorphismGDH = true`` and ``rightLimit`` set to some arbitrary high value, e.g., :math:`2^{30}`.
// rst:		- If you are interested in all the different ways the rule can be matched to generate this direct derivation,
// rst:		  but do not care about the specific vertex map :math:`V(G)\rightarrow V(H)`,
// rst:		  then use ``upToIsomorphismGDH = false`` and ``rightLimit = 1``.
// rst:		- And finally, if you want all possible results,
// rst:		  then use ``upToIsomorphismGDH = false`` and set ``rightLimit`` to some high value, e.g., :math:`2^{30}`.
// rst:
// rst-class-start:
struct MOD_DECL VertexMapper {
	// rst: .. type:: Map = VertexMap<graph::Union, graph::Union>
	// rst:
	// rst:		The type of the vertex map :math:`V(G) \rightarrow V(H)`.
	using Map = VertexMap<graph::Union, graph::Union>;
	// rst: .. type:: Match = VertexMap<rule::Rule::LeftGraph, graph::Union>
	// rst:
	// rst:		The type of the vertex map :math:`V(L) \rightarrow V(G)`,
	// rst:		i.e., the match morphism :math:`m`.
	using Match = VertexMap<rule::Rule::LeftGraph, graph::Union>;
	// rst: .. type:: Comatch = VertexMap<rule::Rule::RightGraph, graph::Union>
	// rst:
	// rst:		The type of the vertex map :math:`V(R) \rightarrow V(H)`,
	// rst:		i.e., the co-match morphism :math:`m'`.
	using Comatch = VertexMap<rule::Rule::RightGraph, graph::Union>;
	// rst: .. class:: Result
	// rst:
	// rst:		The value type returned for each result.
	// rst:
	struct Result {
		// rst: 	.. var:: std::shared_ptr<rule::Rule> r
		// rst:
		// rst: 		The rule used to generate the result.
		std::shared_ptr<rule::Rule> r;
		// rst: 	.. var:: Map map
		// rst:
		// rst: 		The vertex map :math:`V(G) \rightarrow V(H)`.
		Map map;
		// rst: 	.. var:: Match match
		// rst:
		// rst: 		The vertex map :math:`V(L) \rightarrow V(G)`.
		Match match;
		// rst: 	.. var:: Comatch comatch
		// rst:
		// rst: 		The vertex map :math:`V(R) \rightarrow V(H)`.
		Comatch comatch;
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
	// rst:               VertexMapper(DG::HyperEdge e, bool upToIsomorphismGDH, int rightLimit, int verbosity)
	// rst:
	// rst:		Construct a vertex map holder, and immediately calculate vertex maps for the derivations underlying the
	// rst:		given hyperedge.
	// rst:
	// rst:		:param e: the hyperedge to construct vertex maps for.
	// rst:		:param upToIsomorphismGDH: whether to only enumerate spans :math:`G \leftarrow D\rightarrow H'` up to isomorphism, all :math:`m`, or just those such that all bottom spans
	// rst:			:math:`(G\leftarrow D\rightarrow H)` up to isomorphism are generated.
	// rst:			Defaults to `true`.
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
	VertexMapper(DG::HyperEdge e, bool upToIsomorphismGDH, int rightLimit, int verbosity);
	~VertexMapper();
	// rst: .. function:: DG::HyperEdge getEdge() const
	// rst:
	// rst:		:returns: the hyperedge for which the mapper calculates vertex maps.
	DG::HyperEdge getEdge() const;
	// rst: .. function:: graph::Union getLeft() const
	// rst:               graph::Union getRight() const
	// rst:
	// rst:		:returns: the disjoint union of graphs from respectively the source and target vertices of the hyperedge.
	// rst:			That is, the graphs :math:`G` and :math:`H`.
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
