#ifndef MOD_LIB_GRAPH_LABELLED_GRAPH_HPP
#define MOD_LIB_GRAPH_LABELLED_GRAPH_HPP

#include <mod/lib/Graph/GraphDecl.hpp>

namespace mod::lib::graph {
struct PropMolecule;
struct PropStereo;
struct PropString;
struct PropTerm;

struct LabelledGraph { // models a mod::lib::LabelledGraphConcept
	using GraphType = lib::graph::GraphType;
	using PropStringType = PropString;
	using PropTermType = PropTerm;
	using PropStereoType = PropStereo;
public:
	using PropMoleculeType = PropMolecule;
public:
	LabelledGraph(std::unique_ptr<GraphType> g, std::unique_ptr<PropStringType> pString,
	              std::unique_ptr<PropStereoType> pStereo);
	LabelledGraph(const LabelledGraph &other);
	~LabelledGraph();
public: // LabelledGraphConcept
	friend GraphType &get_graph(LabelledGraph &g);
	friend const GraphType &get_graph(const LabelledGraph &g);
	friend PropStringType &get_string(LabelledGraph &g);
	friend const PropStringType &get_string(const LabelledGraph &g);
	friend PropTermType &get_term(LabelledGraph &g);
	friend const PropTermType &get_term(const LabelledGraph &g);
	friend bool has_stereo(const LabelledGraph &g);
	friend const PropStereoType &get_stereo(const LabelledGraph &g);
	friend const PropMoleculeType &get_molecule(const LabelledGraph &g);
public:
	friend const std::vector<typename boost::graph_traits<GraphType>::vertex_descriptor> &
	get_vertex_order(const LabelledGraph &g);
private: // intrinsic data
	std::unique_ptr<GraphType> g;
	mutable std::unique_ptr<PropStringType> pString;
	mutable std::unique_ptr<PropTermType> pTerm;
	mutable std::unique_ptr<PropStereo> pStereo;
private: // views
	mutable std::unique_ptr<PropMoleculeType> pMolecule;
private: // optimisation
	mutable std::vector<typename boost::graph_traits<GraphType>::vertex_descriptor> vertex_order;
};

} // namespace mod::lib::graph

#endif // MOD_LIB_GRAPH_LABELLED_GRAPH_HPP