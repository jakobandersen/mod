#ifndef MOD_CAUSALITY_DGPETRI_HPP
#define MOD_CAUSALITY_DGPETRI_HPP

#include <mod/BuildConfig.hpp>
#include <mod/causality/ForwardDecl.hpp>
#include <mod/dg/DG.hpp>

namespace mod::causality {

// rst:
// rst:	.. versionadded:: 1.1
// rst:
// rst: .. class:: causality::Net
// rst:
// rst:		Adaptation of a derivation graph into a Petri net.
// rst:		Importantly, if the underlying derivation graph is enlarged then
// rst:		:func:`syncSize` must be called before calling certain other
// rst:		methods on the object.
// rst:
class MOD_DECL Net {
	explicit Net(std::shared_ptr<dg::DG> dg_);
public:
	~Net();
	// rst: 	.. function:: std::shared_ptr<dg::DG> getDG() const
	// rst:
	// rst:			:returns: the underlying derivation graph.
	std::shared_ptr<dg::DG> getDG() const;
	const lib::Causality::Net &getNet() const;
	// rst: 	.. function:: friend std::ostream &operator<<(std::ostream &s, const Net &net)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const Net &net);
	// rst: 	.. function:: void syncSize()
	// rst:
	// rst:			Enlarges the internal data structures to the current size of the underlying derivation graph.
	void syncSize();
	// rst: 	.. function:: std::vector<dg::DG::Vertex> getPostPlaces(dg::DG::HyperEdge e) const
	// rst:
	// rst:			:returns: a list of unique targets for the given hyperedge.
	// rst:
	// rst:			Requires :func:`syncSize` to have been called since the last time the underlying derivation graph has changed size.
	std::vector<dg::DG::Vertex> getPostPlaces(dg::DG::HyperEdge e) const;
public:
	// rst: 	.. function:: static std::shared_ptr<Net> make(std::shared_ptr<dg::DG> dg_)
	// rst:
	// rst:			Calls :func:`syncSize`.
	// rst:
	// rst:			:returns: a new Petri net, adapting the given derivation graph.
	// rst:			:throws: :class:`LogicError` if `dg_` is null.
	// rst:			:throws: :class:`LogicError` if neither `dg_->hasActiveBuilder()` nor `dg_->isLocked()`.
	static std::shared_ptr<Net> make(std::shared_ptr<dg::DG> dg_);
private:
	struct Pimpl;
	std::unique_ptr<Pimpl> p;
};

// rst:
// rst-class: causality::Marking
// rst:
// rst:		Representation of a marking on a Petri net.
// rst:		Importantly, if the underlying derivation graph is enlarged then
// rst:		:func:`syncSize` must be called before calling certain other
// rst:		methods on the object.
// rst:
// rst-class-start:
struct MOD_DECL Marking {
	// rst: .. function:: explicit Marking(std::shared_ptr<Net> net)
	// rst:
	// rst:		:throws: :class:`LogicError` if `net` is null.
	explicit Marking(std::shared_ptr<Net> net);
	// pre: &net->getNet() == &marking.getNet()
	explicit Marking(std::shared_ptr<Net> net, const lib::Causality::Marking &marking);
	~Marking();
	Marking(Marking&&);
	Marking &operator=(Marking&&);
	Marking(const Marking&);
	Marking &operator=(const Marking&);
	// rst: .. function:: std::shared_ptr<Net> getNet() const
	// rst:
	// rst:		:returns: the underlying Petri net.
	std::shared_ptr<Net> getNet() const;
	const lib::Causality::Marking &getMarking() const;
	// rst: .. function:: void syncSize() const
	// rst:
	// rst:		Enlarges the internal data structures to the current size of the underlying derivation graph.
	// rst:
	// rst:		Calls `getNet().syncSize()`.
	void syncSize() const;
	// rst: .. function:: friend std::ostream &operator<<(std::ostream &s, const Marking &m)
	// rst:
	// rst:		Calls `syncSize()`.
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const Marking &m);
	// rst: .. function:: friend bool operator==(const Marking &a, const Marking &b)
	// rst:               friend bool operator!=(const Marking &a, const Marking &b)
	// rst:
	// rst:		May call `a.syncSize()` and `b.syncSize()`.
	MOD_DECL friend bool operator==(const Marking &a, const Marking &b);
	MOD_DECL friend bool operator!=(const Marking &a, const Marking &b);
	// rst: .. function:: int add(dg::DG::Vertex v, int c)
	// rst:               int add(std::shared_ptr<graph::Graph> g, int c)
	// rst:
	// rst:		Add `c` tokens on the place `v`.
	// rst:		The graph version is equivalent to ``add(getNet()->getDG()->findVertex(g), c)``.
	// rst:
	// rst:		:returns: the new total token count on `v`.
	// rst:		:throws: :class:`LogicError` if `!v`.
	// rst:		:throws: :class:`LogicError` if `v.getDG() != getNet()->getDG()`.
	// rst:		:throws: :class:`LogicError` if `c < 0`.
	// rst:		:throws: :class:`LogicError` if `!g`.
	// rst:		:throws: :class:`LogicError` if `!getNet()->getDG()->findVertex(g)`.
	// rst:
	// rst:		Requires :func:`syncSize` to have been called since the last time the underlying derivation graph has changed size.
	int add(dg::DG::Vertex v, int c);
	int add(std::shared_ptr<graph::Graph> g, int c);
	// rst: .. function:: int remove(dg::DG::Vertex v, int c)
	// rst:               int remove(std::shared_ptr<graph::Graph> g, int c)
	// rst:
	// rst:		Remove `c` tokens from the place `v`.
	// rst:		The graph version is equivalent to ``remove(getNet()->getDG()->findVertex(g), c)``.
	// rst:
	// rst:		:returns: the new total token count on `v`.
	// rst:		:throws: :class:`LogicError` if `!v`.
	// rst:		:throws: :class:`LogicError` if `v.getDG() != getNet()->getDG()`.
	// rst:		:throws: :class:`LogicError` if `c < 0`.
	// rst:		:throws: :class:`LogicError` if not enough tokens are left to remove all `c`.
	// rst:		:throws: :class:`LogicError` if `!g`.
	// rst:		:throws: :class:`LogicError` if `!getNet()->getDG()->findVertex(g)`.
	// rst:
	// rst:		Requires :func:`syncSize` to have been called since the last time the underlying derivation graph has changed size.
	int remove(dg::DG::Vertex v, int c);
	int remove(std::shared_ptr<graph::Graph> g, int c);
	// rst: .. function:: int operator[](dg::DG::Vertex v)
	// rst:               int operator[](std::shared_ptr<graph::Graph> g) const
	// rst:
	// rst:		:returns: the token count for `v`.
	// rst:			The graph version is equivalent to first calling ``getNet()->getDG()->findVertex(g)``
	// rst:			before using it as index to the marking.
	// rst:		:throws: :class:`LogicError` if `!v`.
	// rst:		:throws: :class:`LogicError` if `v.getDG() != getNet()->getDG()`.
	// rst:		:throws: :class:`LogicError` if `!g`.
	// rst:		:throws: :class:`LogicError` if `!getNet()->getDG()->findVertex(g)`.
	// rst:
	// rst:		Requires :func:`syncSize` to have been called since the last time the underlying derivation graph has changed size.
	int operator[](dg::DG::Vertex v) const;
	int operator[](std::shared_ptr<graph::Graph> g) const;
	// rst: .. function:: int getNumTokens() const
	// rst:
	// rst:		:returns: the total number of tokens in the marking.
	int getNumTokens() const;
	// rst: .. function:: std::vector<dg::DG::HyperEdge> getAllEnabled() const
	// rst:
	// rst:		:returns: a list of all hyperedges currently enabled for firing.
	std::vector<dg::DG::HyperEdge> getAllEnabled() const;
	// rst: .. function:: std::vector<dg::DG::Vertex> getNonZeroPlaces() const
	// rst:
	// rst:		:returns: a list of all vertices with tokens.
	std::vector<dg::DG::Vertex> getNonZeroPlaces() const;
	// rst: .. function:: std::vector<dg::DG::Vertex> getEmptyPostPlaces(dg::DG::HyperEdge e) const
	// rst:
	// rst:		:returns: a list of all target vertices of the given hyperedge that do not have any tokens.
	// rst:			The list represents a set, so if vertex is a target multiple times it will only be included once.
	// rst:		:throws: :class:`LogicError` if `!e`.
	// rst:		:throws: :class:`LogicError` if `e.getDG() != getNet()->getDG()`.
	std::vector<dg::DG::Vertex> getEmptyPostPlaces(dg::DG::HyperEdge e) const;
	// rst: .. function:: bool isEnabled(dg::DG::HyperEdge e) const
	// rst:
	// rst:		:returns: whether the given hyperedge is enabled for firing.
	// rst:		:throws: :class:`LogicError` if `!e`.
	// rst:		:throws: :class:`LogicError` if `e.getDG() != getNet()->getDG()`.
	bool isEnabled(dg::DG::HyperEdge e) const;
	// rst: .. function:: void fire(dg::DG::HyperEdge e)
	// rst:
	// rst:		Fire the given edge.
	// rst:
	// rst:		:throws: :class:`LogicError` if `!e`.
	// rst:		:throws: :class:`LogicError` if `e.getDG() != getNet()->getDG()`.
	// rst:		:throws: :class:`LogicError` if `!isEnabled(e)`.
	void fire(dg::DG::HyperEdge e);
private:
	struct Pimpl;
	std::unique_ptr<Pimpl> p;
};
// rst-class-end:

struct MOD_DECL MarkingSet {
	MarkingSet();
	bool addIfNotSubset(const Marking &m);
private:
	std::vector<std::vector<int>> sets;
};

} // namespace mod::causality

#endif // MOD_CAUSALITY_PETRI_HPP