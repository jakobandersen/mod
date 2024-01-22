#include "LabelledGraph.hpp"

#include <mod/Config.hpp>
#include <mod/lib/Graph/Properties/Molecule.hpp>
#include <mod/lib/Graph/Properties/Stereo.hpp>
#include <mod/lib/Graph/Properties/String.hpp>
#include <mod/lib/Graph/Properties/Term.hpp>
#include <mod/lib/GraphMorphism/Finder.hpp>
#include <mod/lib/Stereo/Inference.hpp>

#include <cassert>
#include <iostream>

namespace mod::lib::Graph {

LabelledGraph::LabelledGraph(std::unique_ptr<GraphType> g, std::unique_ptr<PropStringType> pString,
                             std::unique_ptr<PropStereoType> pStereo)
		: g(std::move(g)), pString(std::move(pString)), pStereo(std::move(pStereo)) {
	assert(this->g);
	assert(this->pString);
	this->pString->verify(this->g.get());
	if(this->pStereo) this->pStereo->verify(this->g.get());
}

LabelledGraph::LabelledGraph(const LabelledGraph &other) {
	g = std::make_unique<GraphType>(get_graph(other));
	pString = std::make_unique<PropStringType>(get_string(other), *g);
	if(other.pStereo) {
		std::cout << "WARNING: missing copying of stereo info in LabelledGraph copy ctor" << std::endl;
	}
}

LabelledGraph::~LabelledGraph() {}

GraphType &get_graph(LabelledGraph &g) {
	return *g.g;
}

const LabelledGraph::GraphType &get_graph(const LabelledGraph &g) {
	return *g.g;
}

LabelledGraph::PropStringType &get_string(LabelledGraph &g) {
	assert(g.pString || g.pTerm);
	return *g.pString;
}

const LabelledGraph::PropStringType &get_string(const LabelledGraph &g) {
	assert(g.pString || g.pTerm);
	return *g.pString;
}

LabelledGraph::PropTermType &get_term(LabelledGraph &g) {
	assert(g.pString || g.pTerm);
	if(!g.pTerm) g.pTerm.reset(new LabelledGraph::PropTermType(get_graph(g), get_string(g), lib::Term::getStrings()));
	return *g.pTerm;
}

const LabelledGraph::PropTermType &get_term(const LabelledGraph &g) {
	assert(g.pString || g.pTerm);
	if(!g.pTerm) g.pTerm.reset(new LabelledGraph::PropTermType(get_graph(g), get_string(g), lib::Term::getStrings()));
	return *g.pTerm;
}

bool has_stereo(const LabelledGraph &g) {
	return bool(g.pStereo);
}

const LabelledGraph::PropStereoType &get_stereo(const LabelledGraph &g) {
	if(!has_stereo(g)) {
		auto inference = lib::Stereo::Inference(get_graph(g), get_molecule(g), false);
		lib::IO::Warnings warnings;
		const bool printWarnings = true;
		auto result = inference.finalize(warnings, printWarnings, [&g](Vertex v) {
			return std::to_string(get(boost::vertex_index_t(), get_graph(g), v));
		});
		std::cout << warnings;
		result.throwIfError<StereoDeductionError>();
		g.pStereo.reset(new PropStereo(get_graph(g), std::move(inference)));
	}
	return *g.pStereo;
}

const LabelledGraph::PropMoleculeType &get_molecule(const LabelledGraph &g) {
	// this makes some GCC versions generate better code than having the unlikely branch in the if
	if(g.pMolecule)
		return *g.pMolecule;
	g.pMolecule.reset(new LabelledGraph::PropMoleculeType(get_graph(g), get_string(g)));
	return *g.pMolecule;
}

const std::vector<typename boost::graph_traits<GraphType>::vertex_descriptor> &
get_vertex_order(const LabelledGraph &g) {
	if(g.vertex_order.empty()) {
		g.vertex_order = get_vertex_order(mod::lib::GraphMorphism::DefaultFinderArgsProvider(), get_graph(g));
	}
	return g.vertex_order;
}

} // namespace mod::lib::Graph