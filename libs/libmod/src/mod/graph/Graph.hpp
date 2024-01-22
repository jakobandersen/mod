#ifndef MOD_GRAPH_GRAPH_HPP
#define MOD_GRAPH_GRAPH_HPP

#include <mod/BuildConfig.hpp>
#include <mod/Config.hpp>
#include <mod/graph/ForwardDecl.hpp>

#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace mod {

template<typename Sig>
struct Function;

template<typename Domain, typename Codomain>
struct VertexMap;

} // namespace mod
namespace mod::graph {

// rst-class: graph::Graph
// rst:
// rst:		This class models an undirected graph with labels on vertices and edges,
// rst:		without loops and without parallel edges.
// rst:		See :ref:`graph-model` for more details.
// rst:		See also :ref:`cpp-graph/GraphInterface` for the documentation for the
// rst:		graph interface for this class.
// rst:
// rst-class-start:
struct MOD_DECL Graph {
	using Handle = std::shared_ptr<Graph>;
	class Vertex;
	class Edge;
	class VertexIterator;
	class VertexRange;
	class EdgeIterator;
	class EdgeRange;
	class IncidentEdgeIterator;
	class IncidentEdgeRange;
public:
	struct Aut;
	struct AutGroup;
private: // The actual class interface
	Graph(std::unique_ptr<lib::Graph::Single> g);
	Graph(const Graph &) = delete;
	Graph &operator=(const Graph &) = delete;
public:
	~Graph();
	// rst: .. function:: std::size_t getId() const
	// rst:
	// rst:		:returns: the unique instance id among :class:`Graph` objects.
	std::size_t getId() const;
	MOD_DECL friend std::ostream &operator<<(std::ostream &s, const Graph &g);
	lib::Graph::Single &getGraph() const;
public: // graph interface
	// rst: .. function:: std::size_t numVertices() const
	// rst:
	// rst:		:returns: the number of vertices in the graph.
	std::size_t numVertices() const;
	// rst: .. function:: VertexRange vertices() const
	// rst:
	// rst:		:returns: a range of all vertices in the graph.
	VertexRange vertices() const;
	// rst: .. function:: std::size_t numEdges() const
	// rst:
	// rst:		:returns: the number of edges in the graph.
	std::size_t numEdges() const;
	// rst: .. function:: EdgeRange edges() const
	// rst:
	// rst:		:returns: a range of all edges in the graph.
	EdgeRange edges() const;
public:
	// rst: .. function:: AutGroup aut(LabelSettings labelSettings) const
	// rst:
	// rst:		:returns: an object representing the automorphism group of the graph.
	AutGroup aut(LabelSettings labelSettings) const;
public:
	// rst: .. function:: std::pair<std::string, std::string> print() const
	// rst:               std::pair<std::string, std::string> print(const Printer &first, const Printer &second) const
	// rst:
	// rst:		Print the graph, using either the default options or the options in `first` and `second`.
	// rst:		If `first` and `second` are the same, only one depiction will be made.
	// rst:
	// rst:		:returns: a pair of names for the PDF-files that will be compiled in post-processing.
	// rst:			If `first` and `second` are the same, the two file prefixes are equal.
	std::pair<std::string, std::string> print() const;
	std::pair<std::string, std::string> print(const Printer &first, const Printer &second) const;
	// rst: .. function:: void printTermState() const
	// rst:
	// rst:		Print the term state for the graph.
	void printTermState() const;
	// rst: .. function:: std::string getGMLString(bool withCoords = false) const
	// rst:
	// rst:		:returns: the :ref:`GML <graph-gml>` representation of the graph,
	// rst:		          optionally with generated 2D coordinates.
	// rst:		:throws: :any:`LogicError` when coordinates are requested, but
	// rst:		         none can be generated.
	std::string getGMLString(bool withCoords = false) const;
	// rst: .. function:: std::string printGML(bool withCoords = false) const
	// rst:
	// rst:		Print the :ref:`GML <graph-gml>` representation of the graph,
	// rst:		optionally with generated 2D coordinates.
	// rst:
	// rst:		:returns: the filename of the printed GML file.
	// rst:		:throws: :any:`LogicError` when coordinates are requested, but
	// rst:		         none can be generated.
	std::string printGML(bool withCoords = false) const;
	// rst: .. function:: const std::string &getName() const
	// rst:               void setName(std::string name) const
	// rst:
	// rst:		Access the name of the graph.
	const std::string &getName() const;
	void setName(std::string name) const;
	// rst: .. function:: const std::string &getSmiles() const
	// rst:
	// rst:		:returns: the canonical :ref:`SMILES string <graph-smiles>` of the graph, if it's a molecule.
	// rst:		:throws: :any:`LogicError` if the graph is not a molecule.
	const std::string &getSmiles() const;
	// rst: .. function:: const std::string &getSmilesWithIds() const
	// rst:
	// rst:		:returns: the canonical :ref:`SMILES string <graph-smiles>` of the graph,
	// rst:			that includes the internal vertex id as a class label on each atom, if it's a molecule.
	// rst:		:throws: :any:`LogicError` if the graph is not a molecule.
	const std::string &getSmilesWithIds() const;
	// rst: .. function:: const std::string &getGraphDFS() const
	// rst:
	// rst:		:returns: a :ref:`GraphDFS <format-graphDFS>` string of the graph.
	const std::string &getGraphDFS() const;
	// rst: .. function:: const std::string &getGraphDFSWithIds() const
	// rst:
	// rst:		:returns: a :ref:`GraphDFS <format-graphDFS>` string of the graph, where each vertex have an explicit id,
	// rst:			corresponding to its internal vertex id.
	const std::string &getGraphDFSWithIds() const;
	// rst: .. function:: const std::string &getLinearEncoding() const
	// rst:
	// rst:		:returns: the :ref:`SMILES <graph-smiles>` string if the graph is a molecule, otherwise the :ref:`GraphDFS <format-graphDFS>` string.
	const std::string &getLinearEncoding() const;
	// rst: .. function:: bool getIsMolecule() const
	// rst:
	// rst:		:returns: whether the graph models a molecule. See :ref:`mol-enc`.
	bool getIsMolecule() const;
	// rst: .. function:: double getEnergy() const
	// rst:
	// rst:		:returns: some energy value if the graph is a molecule.
	// rst:			The energy is calculated using Open Babel, unless already calculated or cached by :cpp:func:`Graph::cacheEnergy`.
	double getEnergy() const;
	// rst: .. function:: void cacheEnergy(double value) const
	// rst:
	// rst:		If the graph models a molecule, sets the energy to a given value.
	// rst:
	// rst:		:throws: :class:`LogicError` if the graph is not a molecule.
	void cacheEnergy(double value) const;
	// rst: .. function:: double getExactMass() const
	// rst:
	// rst:		:returns: the exact mass of the graph, if it is a molecule.
	// rst:			It is the sum of the exact mass of each atom, with the mass of electrons subtracted corresponding to the integer charge.
	// rst:			That is, the mass is :math:`\sum_a (mass(a) - mass(e)\cdot charge(a))`.
	// rst:			If an atom has no specified isotope, then the most abundant is used.
	// rst:		:throws: :class:`LogicError` if it is not a molecule, including if some isotope has not been tabulated.
	double getExactMass() const;
	// rst: .. function:: unsigned int vLabelCount(const std::string &label) const
	// rst:
	// rst:		:returns: the number of vertices in the graph with the given label.
	unsigned int vLabelCount(const std::string &label) const;
	// rst: .. function:: unsigned int eLabelCount(const std::string &label) const
	// rst:
	// rst:		:returns: the number of edges in the graph with the given label.
	unsigned int eLabelCount(const std::string &label) const;
public: // Morphisms
	// rst: .. function:: std::size_t isomorphism(std::shared_ptr<Graph> codomain, std::size_t maxNumMatches, LabelSettings labelSettings) const
	// rst:               std::size_t monomorphism(std::shared_ptr<Graph> codomain, std::size_t maxNumMatches, LabelSettings labelSettings) const
	// rst:
	// rst:		:returns: the number of isomorphisms/monomorphisms found from this graph to `codomain`, but at most `maxNumMatches`.
	// rst:		:throws LogicError: if `codomain` is null.
	std::size_t
	isomorphism(std::shared_ptr<Graph> codomain, std::size_t maxNumMatches, LabelSettings labelSettings) const;
	std::size_t
	monomorphism(std::shared_ptr<Graph> codomain, std::size_t maxNumMatches, LabelSettings labelSettings) const;
	// rst: .. function:: void enumerateIsomorphisms(std::shared_ptr<Graph> codomain, \
	// rst:                  std::shared_ptr<Function<bool(VertexMap<Graph, Graph>)>> callback, \
	// rst:                  LabelSettings labelSettings) const
	// rst:               void enumerateMonomorphisms(std::shared_ptr<Graph> codomain, \
	// rst:                  std::shared_ptr<Function<bool(VertexMap<Graph, Graph>)>> callback, \
	// rst:                  LabelSettings labelSettings) const
	// rst:
	// rst:		Perform (sub)structure search of this graph into the given codomain graph.
	// rst:		Whenever a match is found, the corresponding isomorphism/monomorphism is copied into a vertex map
	// rst:		and the given callback is invoked with it.
	// rst:		The return value from the callback determines whether to continue the search or not.
	// rst:
	// rst:		:throws LogicError: if `codomain` is null.
	// rst:		:throws LogicError: if `callback` is null.
	void enumerateIsomorphisms(std::shared_ptr<Graph> codomain,
	                           std::shared_ptr<Function<bool(VertexMap<Graph, Graph>)>> callback,
	                           LabelSettings labelSettings) const;
	void enumerateMonomorphisms(std::shared_ptr<Graph> codomain,
	                            std::shared_ptr<Function<bool(VertexMap<Graph, Graph>)>> callback,
	                            LabelSettings labelSettings) const;
public:
	// rst: .. function:: std::shared_ptr<Graph> makePermutation() const
	// rst:
	// rst:		:returns: a graph isomorphic to this, but with the vertex indices randomly permuted.
	// rst:
	// rst:		.. note: Currently, permuting stereo-information is not supported, and the resulting
	// rst:			graphs thus have any stereo-information stripped.
	std::shared_ptr<Graph> makePermutation() const;
	// rst: .. function:: void setImage(std::shared_ptr<Function<std::string()>> image)
	// rst:
	// rst:		Set a custom depiction for the graph. The depiction file used will be the string
	// rst:		returned by the given function, with ``.pdf`` appended.
	// rst:		The function will only be called once.
	// rst:		Give `nullptr` to use auto-generated depiction.
	void setImage(std::shared_ptr<Function<std::string()>> image);
	// rst: .. function:: std::shared_ptr<Function<std::string()> > getImage() const
	// rst:
	// rst:		:returns: the current custom depiction file function.
	std::shared_ptr<Function<std::string()> > getImage() const;
	// rst: .. function:: void setImageCommand(std::string cmd)
	// rst:
	// rst:		Set a command to be run in post-processing if a custom depiction is set.
	// rst:		The command is only run once.
	void setImageCommand(std::string cmd);
	// rst: .. function:: std::string getImageCommand() const
	// rst:
	// rst:		:returns: the current post-processing command.
	std::string getImageCommand() const;
	// rst: .. function:: void instantiateStereo() const
	// rst:
	// rst: 		Make sure that stereo data is instantiated.
	// rst:
	// rst: 		:throws: `StereoDeductionError` if the data was not instantiated and deduction failed.
	void instantiateStereo() const;
public: // external data
	// rst: .. function:: Vertex getVertexFromExternalId(int id) const
	// rst:
	// rst:		If the graph was not loaded from an external data format, then this function
	// rst:		always return a null descriptor.
	// rst:		If the graph was loaded from a SMILES string, but *any* class label was not unique,
	// rst:		then the function always return a null descriptor.
	// rst:
	// rst:		.. note:: In general there is no correlation between external and internal ids.
	// rst:
	// rst:		:returns: the vertex descriptor for the given external id.
	// rst:		          The descriptor is null if the external id was not used.
	Vertex getVertexFromExternalId(int id) const;
	// rst: .. function:: int getMinExternalId() const
	// rst:               int getMaxExternalId() const
	// rst:
	// rst:		If the graph was not loaded from an external data format, then these functions
	// rst:		always return 0. Otherwise, return the minimum/maximum external id from which
	// rst:		non-null vertices can be obtained from :cpp:func:`getVertexFromExternalId`.
	// rst:		If no such minimum and maximum exists, then 0 will be returned.
	int getMinExternalId() const;
	int getMaxExternalId() const;
	// rst: .. function:: std::vector<std::pair<std::string, bool>> getLoadingWarnings() const
	// rst:
	// rst:		:returns: the list of warnings stored when the graph was created from an external format.
	// rst:			Each entry is a message and then an indicator of whether
	// rst:			the warning was printed before construction (``true``), or was a silenced warning (``false``).
	// rst:		:throws: :class:`LogicError` if the graph does not have data from external loading
	std::vector<std::pair<std::string, bool>> getLoadingWarnings() const;
private:
	const std::unique_ptr<lib::Graph::Single> g;
	struct ExternalData;
	// this is only instantiated when the graph is loaded from an external format
	std::unique_ptr<ExternalData> externalData;
public:
	// rst: .. function:: static std::shared_ptr<Graph> fromGMLString(const std::string &data, bool printStereoWarnings = true)
	// rst:               static std::shared_ptr<Graph> fromGMLFile(const std::string &file, bool printStereoWarnings = true)
	// rst:
	// rst:		:param printStereoWarnings: whether to print warnings due to unhandled stereo information.
	// rst:		:returns: a graph created from the given :ref:`GML <graph-gml>` in a string or file.
	// rst:			The graph must be connected. Use :func:`fromGMLStringMulti` or :func:`fromGMLFile` if it is not.
	// rst:		:throws: :class:`InputError` on bad input.
	static std::shared_ptr<Graph> fromGMLString(const std::string &data, bool printStereoWarnings = true);
	static std::shared_ptr<Graph> fromGMLFile(const std::string &file, bool printStereoWarnings = true);
	// rst: .. function:: std::vector<std::shared_ptr<Graph>> fromGMLStringMulti(const std::string &data, bool printStereoWarnings = true)
	// rst:               std::vector<std::shared_ptr<Graph>> fromGMLFileMulti(const std::string &file, bool printStereoWarnings = true)
	// rst:
	// rst:		:param printStereoWarnings: whether to print warnings due to unhandled stereo information.
	// rst:		:returns: a list of graphs, loaded from the given :ref:`GML <graph-gml>` in a string or file.
	// rst:			The graphs are the connected components of the graph specified in the data.
	// rst:		:throws: :class:`InputError` on bad input.
	static std::vector<std::shared_ptr<Graph>>
	fromGMLStringMulti(const std::string &data, bool printStereoWarnings = true);
	static std::vector<std::shared_ptr<Graph>>
	fromGMLFileMulti(const std::string &file, bool printStereoWarnings = true);
	// ===========================================================================
	// rst: .. function:: static std::shared_ptr<Graph> fromDFS(const std::string &graphDFS)
	// rst:
	// rst:		:returns: a graph loaded from the given :ref:`GraphDFS <format-graphDFS>` string.
	// rst:			The graph must be connected. Use :func:`fromDFSMulti` if it is not.
	// rst:		:throws: :class:`InputError` on bad input.
	static std::shared_ptr<Graph> fromDFS(const std::string &graphDFS);
	// rst: .. function:: static std::vector<std::shared_ptr<Graph>> fromDFSMulti(const std::string &graphDFS)
	// rst:
	// rst:		:returns: a list of graphs loaded from the given :ref:`GraphDFS <format-graphDFS>` string.
	// rst:			The graphs are the connected components of the graph specified in the data.
	// rst:		:throws: :class:`InputError` on bad input.
	static std::vector<std::shared_ptr<Graph>> fromDFSMulti(const std::string &graphDFS);
// ===========================================================================
	// rst: .. function:: static std::shared_ptr<Graph> fromSMILES(const std::string &smiles)
	// rst:               static std::shared_ptr<Graph> fromSMILES(const std::string &smiles, bool allowAbstract, \
	// rst:                                                        SmilesClassPolicy classPolicy, bool printStereoWarnings)
	// rst:
	// rst:		:param printStereoWarnings: whether to print warnings due to unhandled stereo information. Defaults to `true`.
	// rst:		:param allowAbstract: whether abstract atoms, e.g., ``*``, are allowed. Defaults to `false`.
	// rst:		:param classPolicy: which policy to use for class labels. Defaults to `SmilesClassPolicy::NoneOnDuplicate`.
	// rst:		:returns: a graph representing a molecule, loaded from the given :ref:`SMILES <graph-smiles>` string.
	// rst:			The graph must be connected. Use :func:`fromSMILESMulti` if it is not.
	// rst:		:throws: :class:`InputError` on bad input.
	// rst:		:throws: :class:`InputError` if `classPolicy == SmilesClassPolicy::NoneOnDuplicate` and a class label is duplicated.
	static std::shared_ptr<Graph> fromSMILES(const std::string &smiles);
	static std::shared_ptr<Graph> fromSMILES(const std::string &smiles, bool allowAbstract,
	                                         SmilesClassPolicy classPolicy, bool printStereoWarnings);
	// rst: .. function:: static std::vector<std::shared_ptr<Graph>> fromSMILESMulti(const std::string &smiles)
	// rst:               static std::vector<std::shared_ptr<Graph>> fromSMILESMulti(const std::string &smiles, bool allowAbstract, \
	// rst:                                                                          SmilesClassPolicy classPolicy, bool printStereoWarnings)
	// rst:
	// rst:		See :func:`fromSMILES` for parameter and exception descriptions.
	// rst:
	// rst:		:returns: a list of graphs representing molecules, loaded from the given :ref:`SMILES <graph-smiles>` string.
	// rst:			The graphs are the connected components of the graph specified in the SMILES string.
	static std::vector<std::shared_ptr<Graph>> fromSMILESMulti(const std::string &smiles);
	static std::vector<std::shared_ptr<Graph>> fromSMILESMulti(const std::string &smiles, bool allowAbstract,
	                                                           SmilesClassPolicy classPolicy, bool printStereoWarnings);
	// ===========================================================================
	// rst: .. function:: static std::shared_ptr<Graph> fromMOLString(const std::string &data, const MDLOptions &options)
	// rst:               static std::shared_ptr<Graph> fromMOLFile(const std::string &file, const MDLOptions &options)
	// rst:
	// rst:		:returns: a graph created from the given :ref:`MOL <graph-mdl>` data.
	// rst:		:throws: :class:`InputError` on bad input.
	static std::shared_ptr<Graph> fromMOLString(const std::string &data, const MDLOptions &options);
	static std::shared_ptr<Graph> fromMOLFile(const std::string &file, const MDLOptions &options);
	// rst: .. function:: static std::vector<std::shared_ptr<Graph>> fromMOLStringMulti(const std::string &data, const MDLOptions &options)
	// rst:               static std::vector<std::shared_ptr<Graph>> fromMOLFileMulti(const std::string &file, const MDLOptions &options)
	// rst:
	// rst:		See :func:`fromMOLString` and :func:`fromMOLFile` for parameter and exception descriptions.
	// rst:
	// rst:		:returns: a list of graphs representing molecules, loaded from the given string or file with :ref:`MOL <graph-mdl>` data.
	// rst:			The graphs are the connected components of the graph specified in the data.
	static std::vector<std::shared_ptr<Graph>> fromMOLStringMulti(const std::string &data, const MDLOptions &options);
	static std::vector<std::shared_ptr<Graph>> fromMOLFileMulti(const std::string &file, const MDLOptions &options);
	// ===========================================================================
	// rst: .. function:: static std::vector<std::shared_ptr<Graph>> fromSDString(const std::string &data, const MDLOptions &options)
	// rst:               static std::vector<std::shared_ptr<Graph>> fromSDFile(const std::string &file, const MDLOptions &options)
	// rst:
	// rst:		:returns: a list of graphs created from the given :ref:`SD <graph-mdl>` data.
	// rst:		:throws: :class:`InputError` on bad input.
	static std::vector<std::shared_ptr<Graph>> fromSDString(const std::string &data, const MDLOptions &options);
	static std::vector<std::shared_ptr<Graph>> fromSDFile(const std::string &file, const MDLOptions &options);
	// rst: .. function:: static std::vector<std::vector<std::shared_ptr<Graph>>> fromSDStringMulti(const std::string &data, const MDLOptions &options)
	// rst:               static std::vector<std::vector<std::shared_ptr<Graph>>> fromSDFileMulti(const std::string &file, const MDLOptions &options)
	// rst:
	// rst:		:returns: a list of lists of graphs created from the given :ref:`SD <graph-mdl>` data.
	// rst:		:throws: :class:`InputError` on bad input.
	static std::vector<std::vector<std::shared_ptr<Graph>>>
	fromSDStringMulti(const std::string &data, const MDLOptions &options);
	static std::vector<std::vector<std::shared_ptr<Graph>>>
	fromSDFileMulti(const std::string &file, const MDLOptions &options);
	// ===========================================================================
	// rst: .. function:: static std::shared_ptr<Graph> create(std::unique_ptr<lib::Graph::Single> g)
	// rst:               static std::shared_ptr<Graph> create(std::unique_ptr<lib::Graph::Single> g, \
	// rst:                                                    std::map<int, std::size_t> externalToInternalIds, \
	// rst:                                                    std::vector<std::pair<std::string, bool>> warnings)
	// rst:
	// rst:		:param externalToInternalIds: map from user-defined integers to the vertex indices of ``g``.
	// rst:			See also :cpp:func:`getVertexFromExternalId`.
	// rst:		:param warnings: a list of warning messages to store. The second component is supposed to indicate whether
	// rst:			the warning was printed before construction (``true``), or was a silenced warning (``false``).
	// rst:			See also :cpp:func:`getLoadingWarnings`.
	// rst:		:returns: a graph wrapping the given internal graph object.
	// rst:			The second version instantiates a structure holding the given data created from an external data source.
	static std::shared_ptr<Graph> create(std::unique_ptr<lib::Graph::Single> g);
	static std::shared_ptr<Graph> create(std::unique_ptr<lib::Graph::Single> g,
	                                     std::map<int, std::size_t> externalToInternalIds,
	                                     std::vector<std::pair<std::string, bool>> warnings);
};
// rst-class-end:

struct GraphLess {
	bool operator()(std::shared_ptr<Graph> g1, std::shared_ptr<Graph> g2) const {
		return g1->getId() < g2->getId();
	}
};

} // namespace mod::graph

#endif // MOD_GRAPH_GRAPH_HPP