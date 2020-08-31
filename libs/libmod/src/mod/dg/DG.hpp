#ifndef MOD_DG_DG_H
#define MOD_DG_DG_H

#include <mod/BuildConfig.hpp>
#include <mod/Config.hpp>
#include <mod/dg/ForwardDecl.hpp>
#include <mod/graph/ForwardDecl.hpp>
#include <mod/rule/ForwardDecl.hpp>

#include <memory>
#include <unordered_set>
#include <vector>

namespace mod {
namespace dg {

// rst-class: dg::DG
// rst:
// rst:		The derivation graph class. A derivation graph is a directed multi-hypergraph
// rst:		:math:`\mathcal{H} = (V, E)`. Each hyperedge :math:`e\in E` is thus an ordered pair
// rst:		:math:`(e^+, e^-)` of multisets of vertices, the sources and the targets.
// rst:		Each vertex is annotated with a graph, and each hyperedge is annotated with list of transformation rules.
// rst:		A derivation graph is constructed incrementally using a :cpp:class:`Builder` obtained from the :cpp:func:`build()`
// rst:		function. When the obtained builder is destructed the derivation graph becomes locked and can no longer be modified.
// rst: 
// rst-class-start:
struct MOD_DECL DG {
	DG(const DG &) = delete;
	DG &operator=(const DG &) = delete;
public:
	class Vertex;
	class HyperEdge;
	class VertexIterator;
	class VertexRange;
	class EdgeIterator;
	class EdgeRange;
	class InEdgeIterator;
	class InEdgeRange;
	class OutEdgeIterator;
	class OutEdgeRange;
	class SourceIterator;
	class SourceRange;
	class TargetIterator;
	class TargetRange;
	class RuleIterator;
	class RuleRange;
private:
	DG(std::unique_ptr<lib::DG::NonHyper> dg);
public:
	~DG();
	// rst: .. function:: std::size_t getId() const
	// rst:
	// rst:		:returns: the instance identifier for the object.
	std::size_t getId() const;
	// rst: .. function:: friend std::ostream &operator<<(std::ostream &s, const DG &dg)
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const DG &dg);
	// rst: .. function:: const lib::DG::NonHyper &getNonHyper() const
	// rst:               const lib::DG::Hyper &getHyper() const
	// rst: 
	// rst:		:returns: the internal data structures of the derivation graph.
	const lib::DG::NonHyper &getNonHyper() const;
	const lib::DG::Hyper &getHyper() const;
	// rst: .. function:: LabelSettings getLabelSettings() const
	// rst:
	// rst:		:returns: the label settings for the derivation graph.
	LabelSettings getLabelSettings() const;
public: // object state
	// rst: .. function:: bool hasActiveBuilder() const
	// rst:
	// rst:		:returns: whether :cpp:func:`build` has been called and the returned :cpp:class:`Builder` is still active.
	bool hasActiveBuilder() const;
	// rst: .. function:: bool isLocked() const
	// rst:
	// rst:		:returns: whether the derivation graph is locked or not.
	bool isLocked() const;
public: // hypergraph interface
	// rst: .. function:: std::size_t numVertices() const
	// rst:
	// rst:		:returns: the number of vertices in the derivation graph.
	// rst:		:throws: :class:`LogicError` if not `hasActiveBuilder()` or `isLocked()`.
	std::size_t numVertices() const;
	// rst: .. function:: VertexRange vertices() const
	// rst:
	// rst:		:returns: a range of all vertices in the derivation graph.
	// rst:		:throws: :class:`LogicError` if not `hasActiveBuilder()` or `isLocked()`.
	VertexRange vertices() const;
	// rst: .. function:: std::size_t numEdges() const
	// rst:
	// rst:		:returns: the number of edges in the derivation graph.
	// rst:		:throws: :class:`LogicError` if not `hasActiveBuilder()` or `isLocked()`.
	std::size_t numEdges() const;
	// rst: .. function:: EdgeRange edges() const
	// rst:
	// rst:		:returns: a range of all edges in the derivation graph.
	// rst:		:throws: :class:`LogicError` if not `hasActiveBuilder()` or `isLocked()`.
	EdgeRange edges() const;
public: // searching for vertices and hyperedges
	// rst: .. function:: Vertex findVertex(std::shared_ptr<graph::Graph> g) const
	// rst:
	// rst:		:returns: a vertex descriptor for which the given graph is associated,
	// rst:			or a null descriptor if no such vertex exists.
	// rst:		:throws: :class:`LogicError` if not `hasActiveBuilder()` or `isLocked()`.
	// rst:		:throws: :class:`LogicError` if `g` is a `nullptr`.
	Vertex findVertex(std::shared_ptr<graph::Graph> g) const;
	// rst: .. function:: HyperEdge findEdge(const std::vector<Vertex> &sources, const std::vector<Vertex> &targets) const
	// rst:               HyperEdge findEdge(const std::vector<std::shared_ptr<graph::Graph> > &sources, const std::vector<std::shared_ptr<graph::Graph> > &targets) const
	// rst: 
	// rst:		:returns: a hyperedge with the given sources and targets.
	// rst:			If no such hyperedge exists in the derivation graph then a null edge is returned.
	// rst:			In the second version, the graphs are put through :func:`findVertex` first.
	// rst:		:throws: :class:`LogicError` if a vertex descriptor is null, or does not belong to the derivation graph.
	// rst:		:throws: :class:`LogicError` if not `hasActiveBuilder()` or `isLocked()`.
	HyperEdge findEdge(const std::vector<Vertex> &sources, const std::vector<Vertex> &targets) const;
	HyperEdge findEdge(const std::vector<std::shared_ptr<graph::Graph>> &sources,
	                   const std::vector<std::shared_ptr<graph::Graph>> &targets) const;
public:
	// rst: .. function:: Builder build()
	// rst:
	// rst:		:returns: an RAII-style move-only object which can be used to construct the derivation graph.
	// rst:			Only one of these objects can be active at the same time, and on destruction an active builder object
	// rst:			will lock the associated DG object for further modification.
	// rst:		:throws: :cpp:class:`LogicError` if `hasActiveBuilder()`.
	// rst:		:throws: :cpp:class:`LogicError` if `isLocked()`.
	Builder build();
	// rst: .. function:: const std::vector<std::shared_ptr<graph::Graph>> &getGraphDatabase() const
	// rst:
	// rst:		:returns: a list of all graphs created by the derivation graph,
	// rst:			and all graphs given when constructed.
	const std::vector<std::shared_ptr<graph::Graph>> &getGraphDatabase() const;
	// rst: .. function:: const std::vector<std::shared_ptr<graph::Graph> > &getProducts() const
	// rst:
	// rst:		:returns: the list of new graphs discovered by the derivation graph.
	const std::vector<std::shared_ptr<graph::Graph> > &getProducts() const;
	// rst: .. function:: std::pair<std::string, std::string> print(const PrintData &data, const Printer &printer) const
	// rst:
	// rst:		Print the derivation graph in style of a hypergraph.
	// rst:
	// rst:		:returns: the name of the PDF-file that will be compiled in post-processing and the name of the coordinate tex-file used.
	// rst:		:throws: :class:`LogicError` if the print data is not for this DG.
	std::pair<std::string, std::string> print(const PrintData &data, const Printer &printer) const;
	// rst: .. function:: std::string dump() const
	// rst:
	// rst:		Exports the derivation graph to a text file, which can be imported.
	// rst:
	// rst:		:returns: the name of the file with the exported data.
	// rst:		:throws: :class:`LogicError` if the DG has not been calculated.
	std::string dump() const;
	// rst: .. function:: void listStats() const
	// rst: 
	// rst:		Output various stats of the derivation graph.
	// rst:
	// rst:		:throws: :class:`LogicError` if the DG has not been calculated.
	void listStats() const;
private:
	struct Pimpl;
	std::unique_ptr<Pimpl> p;
public:
	// rst: .. function:: static std::shared_ptr<DG> make(LabelSettings labelSettings, \
	// rst:                                              const std::vector<std::shared_ptr<graph::Graph> > &graphDatabase, \
	// rst:                                              IsomorphismPolicy graphPolicy)
	// rst:
	// rst:		Create an empty unlocked derivation graph object.
	// rst:
	// rst:		The given :cpp:class:`LabelSettings` defines which category the derivation graph object works in.
	// rst:		All morphism calculations (monomorphism and isomorphism) are thus defined by the :cpp:enum:`LabelType`,
	// rst:		while the :cpp:enum:`LabelRelation` is used for for monomorphism enumeration.
	// rst:
	// rst:		The graphs is the given :cpp:var:`graphDatabase` are used as the initial graph database.
	// rst:		Any subsequently added or constructed graph for this object
	// rst:		will be checked for isomorphism against the graph database.
	// rst:
	// rst:		The given :cpp:var:`graphPolicy` refers to how the graphs of :cpp:var:`graphDatabase` are checked for
	// rst:		isomorphism against each other initially. Only use :cpp:enumerator:`IsomorphismPolicy::TrustMe` if you are
	// rst:		absolutely sure that the graphs are unique up to isomorphism.
	// rst:
	// rst:		:throws: :class:`LogicError` if `graphPolicy == IsomorphismPolicy::Check` and two graph objects
	// rst:			in :cpp:any:`graphDatabase` are different objects but represents isomorphic graphs.
	// rst:		:throws: :class:`LogicError` if there is a `nullptr` in `graphDatabase`.
	static std::shared_ptr<DG> make(LabelSettings labelSettings,
	                                const std::vector<std::shared_ptr<graph::Graph> > &graphDatabase,
	                                IsomorphismPolicy graphPolicy);
	// rst: .. function:: static std::shared_ptr<DG> load(const std::vector<std::shared_ptr<graph::Graph>> &graphDatabase, \
	// rst:                                              const std::vector<std::shared_ptr<rule::Rule>> &ruleDatabase, \
	// rst:                                              const std::string &file, \
	// rst:                                              IsomorphismPolicy graphPolicy)
	// rst:               static std::shared_ptr<DG> load(const std::vector<std::shared_ptr<graph::Graph>> &graphDatabase, \
	// rst:                                              const std::vector<std::shared_ptr<rule::Rule>> &ruleDatabase, \
	// rst:                                              const std::string &file, \
	// rst:                                              IsomorphismPolicy graphPolicy, int verbosity)
	// rst:
	// rst:		Load a derivation graph dump as a locked object.
	// rst:		Use :cpp:func:`Builder::load` to load a dump into a derivation graph under construction.
	// rst:
	// rst:		This is done roughly by calling :cpp:func:`make` with the given `graphDatabase` and `graphPolicy`.
	// rst:		The label settings are retrieved from the dump file.
	// rst:		Vertices with graphs and hyperedges with rules are then added from the dump.
	// rst:		Any graph in the dump which is isomorphic to a given graph is replaced by the given graph.
	// rst:		The same procedure is done for the rules.
	// rst:		If a graph/rule is not found in the given lists, a new object is instantiated and used.
	// rst:		In the end the derivation graph is locked.
	// rst:
	// rst:		.. note:: If the dump to be loaded was made by version 0.10 or earlier, it does not contain the full rules
	//	rst:			but only the rule name. It is then crucial that the names of the given rules match with those used to
	//	rst:			create the dump in the first place.
	// rst:
	// rst:		The :cpp:var:`verbosity` defaults to level 2.
	// rst:		The levels have the following meaning:
	// rst:
	// rst:		- 0: print nothing.
	// rst:		- 2: print a message whenever a loaded graph or rule is isomorphic to an existing graph or rule.
	// rst:
	// rst:		:throws: the same exceptions :func:`make` throws related to `graphDatabase` and `graphPolicy`.
	// rst:		:throws: :class:`LogicError` if there is a `nullptr` in `ruleDatabase`.
	// rst: 		:throws: :class:`InputError` if the file can not be opened or its content is bad.
	static std::shared_ptr<DG> load(const std::vector<std::shared_ptr<graph::Graph>> &graphDatabase,
	                                const std::vector<std::shared_ptr<rule::Rule>> &ruleDatabase,
	                                const std::string &file,
	                                IsomorphismPolicy graphPolicy);
	static std::shared_ptr<DG> load(const std::vector<std::shared_ptr<graph::Graph>> &graphDatabase,
	                                const std::vector<std::shared_ptr<rule::Rule>> &ruleDatabase,
	                                const std::string &file,
	                                IsomorphismPolicy graphPolicy, int verbosity);
	// rst: .. function:: static void diff(std::shared_ptr<DG> dg1, std::shared_ptr<DG> dg2)
	// rst:
	// rst: 		Compare two derivation graphs and lists the difference.
	// rst: 		This is not a general isomorphism check; two vertices are equal if they have
	// rst: 		the same graph attached. Edges are equal if the head and tail sets are equal
	// rst: 		and if the attached rule is the same.
	static void diff(std::shared_ptr<DG> dg1, std::shared_ptr<DG> dg2);
};
// rst-class-end:

} // namespace dg
} // namespace mod

#endif /* MOD_DG_DG_H */
