#ifndef MOD_LIB_DG_NONHYPER_HPP
#define MOD_LIB_DG_NONHYPER_HPP

#include <mod/dg/ForwardDecl.hpp>
#include <mod/dg/DG.hpp>
#include <mod/lib/DG/GraphDecl.hpp>
#include <mod/lib/Graph/Collection.hpp>
#include <mod/lib/Graph/GraphDecl.hpp>

#include <boost/graph/adjacency_list.hpp>

#include <iosfwd>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace mod::lib::Graph {
struct PropString;
} // namespace mod::lib::Graph
namespace mod::lib::DG {
struct HyperCreator;

struct NonHyper {
	friend struct HyperCreator;
public:
	using GraphType = NonHyperGraphType;
	using Vertex = NonHyperVertex;
	using Edge = NonHyperEdge;
protected:
	// pre: no nullptrs in graphDatabase
	NonHyper(LabelSettings labelSettings,
	         const std::vector<std::shared_ptr<graph::Graph> > &graphDatabase, IsomorphismPolicy graphPolicy);
public: // general
	virtual ~NonHyper();
	std::size_t getId() const;
	std::shared_ptr<dg::DG> getAPIReference() const;
	void setAPIReference(std::shared_ptr<dg::DG> dg);
	LabelSettings getLabelSettings() const;
	virtual std::string getType() const = 0;
public: // calculation
	bool getHasStartedCalculation() const;
	bool getHasCalculated() const;
protected: // calculation
	void calculatePrologue(); // call this before modifying the DG
	void calculateEpilogue(); // call this when done with the modifications
	// Overall Idea
	// ------------
	// For a new generated graph:
	// 1. Check for a new graph, by isomorphism with checkIfNew.
	// 2. Then use trustAddGraph or trustAddGraphAsVertex to add without isomorphism check (but still pointer check).
	// For given graphs:
	// - Use tryAddGraph to add with isomorphism check.
	// - Use trustAddGraph or trustAddGraphAsVertex to trustfully add it.
	// ======================================================================================
	// Insert g in the database, unless an isomorphic graph is therein already.
	// If so, it throws LogicError.
	// If we are using terms it may throw TermParsingError as well.
	void tryAddGraph(std::shared_ptr<graph::Graph> g);
	// Adds the graph to the graph database without isomorphism check.
	// Returns true iff the graph object was not there already.
	bool trustAddGraph(std::shared_ptr<graph::Graph> g);
	// Uses trustAddGraph.
	// Makes a vertex in the DG with the graph.
	// Returns the value from trustAddGraph.
	bool trustAddGraphAsVertex(std::shared_ptr<graph::Graph> g);
	// Searches the database for the given graph by isomorphism.
	// If found, returns the found graph and the input,
	// If not found, returns the given wrapped given graph and nullptr.
	// Does NOT change the graphDatabse.
	std::pair<std::shared_ptr<graph::Graph>, std::unique_ptr<lib::Graph::Single>>
	checkIfNew(std::unique_ptr<lib::Graph::Single> g) const;
	// trustAddGraph and then rename if it was a new graph.
	// Returns the value from trustAddGraph.
	bool addProduct(std::shared_ptr<graph::Graph> g);
	// checks if this derivation already exists
	// if it does then the edge descriptor of that derivation is returned, otherwise the edge descriptor is bogus
	std::pair<Edge, bool> isDerivation(const GraphMultiset &gmsSrc,
	                                   const GraphMultiset &gmsTar,
	                                   const lib::Rules::Real *r) const;
	// adds a derivation if it does not exist already
	// the edge descriptor of the derivation is returned, along with the existence status before the call
	// the rule may be nullptr
	std::pair<Edge, bool> suggestDerivation(const GraphMultiset &gmsSrc,
	                                        const GraphMultiset &gmsTar,
	                                        const lib::Rules::Real *r);
	const GraphType &getGraphDuringCalculation() const;
private: // calculation
	// adds the graph as a vertex, if it's not there already, and returns the vertex
	Vertex getVertex(const GraphMultiset &gms);
	void findReversiblePairs();
public: // post calculation
	const GraphType &getGraph() const;
	const Hyper &getHyper() const;
	const Graph::Collection &getGraphDatabase() const;
	const std::vector<std::shared_ptr<graph::Graph> > &getProducts() const;
	void print() const;
	HyperVertex getHyperEdge(Edge e) const;
	HyperVertex findHyperEdge(const std::vector<HyperVertex> &sources, const std::vector<HyperVertex> &targets) const;
private: // general
	std::size_t id;
	std::weak_ptr<dg::DG> apiReference;
	const LabelSettings labelSettings;
	Graph::Collection graphDatabase;
	GraphType dg;
public: // TODO: make private again
	std::unordered_map<GraphMultiset, Vertex> multisetToVertex;
private:
	std::unique_ptr<Hyper> hyper;
	std::unique_ptr<HyperCreator> hyperCreator; // only valid during calculation
private: // calculation
	bool hasStartedCalculation = false;
	bool hasCalculated = false;
	unsigned int productNum = 0;
	std::vector<std::shared_ptr<graph::Graph> > products;
public:
	static void diff(const NonHyper &dg1, const NonHyper &dg2);
};

} // namespace mod::lib::DG

#endif // MOD_LIB_DG_NONHYPER_HPP