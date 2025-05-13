#include "ModelImplementationView.hpp"

#include <mod/Error.hpp>
#include <mod/Post.hpp>
#include <mod/lib/CombiOpt/Solver.hpp>
#include <mod/lib/DG/IO/Expanded.hpp>
#include <mod/lib/DG/IO/Write.hpp>
#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/HyperFlow/Flow.hpp>
#include <mod/lib/ILP/ILP.hpp>

namespace mod::hyperflow {

ModelImplementationView::ModelImplementationView(std::shared_ptr<Model> model) : model(model) {
	if(!model) throw LogicError("Model is null.");
	if(!model->isSpecificationLocked())
		throw LogicError("Specification not yet locked.");
}

std::string ModelImplementationView::printExpandedVertex(dg::DG::Vertex v) const {
	if(!v)
		throw LogicError("Can not print expanded network for a null vertex.");
	if(v.getDG() != model->getDG())
		throw LogicError("Can not print expanded network for a vertex from a different derivation graph.");
	using boost::vertices;
	const auto &dg = model->getDG()->getHyper().getGraph();
	const auto vHyper = vertices(dg).first[v.getId()];
	assert(dg[vHyper].kind == lib::DG::HyperVertexKind::Vertex);
	auto res = lib::DG::Write::Expanded::pdfVertex(model->getFlow().getModel().getExpanded(), vHyper);
	std::string title = "ExpandedVertex, flow=" +
							  std::to_string(model->getId()) + ", dg=" +
							  std::to_string(model->getDG()->getId()) + ", vertex=" +
							  std::to_string(get(boost::vertex_index_t(), dg, vHyper)) + ", " +
							  dg[vHyper].graph->getName();
	post::command("summarySubsection \"" + title + "\"");
	post::command("summaryPDF \"" + std::string(res.begin(), res.end() - 4) + "\"");
	return res;
}

std::string ModelImplementationView::printExpandedVertex(std::shared_ptr<graph::Graph> g) const {
	return printExpandedVertex(model->getDG()->findVertex(g));
}

std::string ModelImplementationView::printExpanded() const {
	lib::DG::Write::Data data(model->getDG()->getHyper());
	lib::DG::Write::Printer printer;

	post::command("summarySubsection \"Expanded DG, flow=" +
					  std::to_string(model->getId()) + ", dg=" +
					  std::to_string(model->getDG()->getId()) + "\"");
	auto res = lib::DG::Write::Expanded::pdf(model->getFlow().getModel().getExpanded(), data, printer);
	post::command("summaryPDF \"" + std::string(res.begin(), res.end() - 4) + "\"");
	return res;
}

std::string ModelImplementationView::printEnumerationTree() const {
	post::FileHandle s(post::makeUniqueFilePrefix() + "enumerationTree.dot");
	model->getFlow().getModel().printEnumerationTree(s);
	return s;
}

std::string ModelImplementationView::writeMILP() const {
	return writeMILP("lp");
}

std::string ModelImplementationView::writeMILP(std::string format) const {
	for(char &c : format)
		c = std::tolower(c);
	std::string filename = post::makeUniqueFilePrefix() + "hyperflow_" + std::to_string(model->getId())
	                       + "_milp." + format;
	model->getFlow().getModel().getSolver().getILP().write(filename);
	return filename;
}

int ModelImplementationView::getNumMILPVariables() const {
	return model->getFlow().getModel().getSolver().getILP().getNumVariables();
}

int ModelImplementationView::getNumMILPConstraints() const {
	return model->getFlow().getModel().getSolver().getILP().getNumConstraints();
}

} // namespace mod::hyperflow