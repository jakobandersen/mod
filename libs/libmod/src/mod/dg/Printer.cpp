#include "Printer.hpp"

#include <mod/Error.hpp>
#include <mod/dg/DG.hpp>
#include <mod/dg/GraphInterface.hpp>
#include <mod/graph/Printer.hpp>
#include <mod/lib/DG/Hyper.hpp>
#include <mod/lib/DG/IO/Write.hpp>
#include <mod/lib/Graph/Single.hpp>

#include <boost/lexical_cast.hpp>

namespace mod::dg {

//------------------------------------------------------------------------------
// PrintData
//------------------------------------------------------------------------------

namespace {

void printDataCheckHyperEdge(std::shared_ptr<DG> dg, DG::HyperEdge e) {
	if(!e) throw LogicError("The hyperedge is null.");
	if(e.getDG() != dg) throw LogicError("The hyperedge does not belong to the derivation graph this data is for.");
}

void reconnectCommon(std::shared_ptr<DG> dg, DG::HyperEdge e, unsigned int eDup, DG::Vertex v,
                     int vDupTar, bool isTail) {
	printDataCheckHyperEdge(dg, e);
	if(!v) throw LogicError("The vertex is null.");
	if(v.getDG() != dg) throw LogicError("The vertex does not belong to the derivation graph this data is for.");
	const bool found = [&]() {
		if(isTail) {
			const auto src = e.sources();
			return src.end() != std::find(src.begin(), src.end(), v);
		} else {
			const auto tar = e.targets();
			return tar.end() != std::find(tar.begin(), tar.end(), v);
		}
	}();
	if(!found) {
		std::string msg = "The vertex is not a ";
		msg += isTail ? "source" : "target";
		msg += " of the given hyperedge.";
		throw LogicError(std::move(msg));
	}
}

} // namespace

PrintData::PrintData(std::shared_ptr<DG> dg) : dg(dg), data(nullptr) {
	if(!dg->isLocked())
		throw LogicError("Can not create print data. The DG is not locked yet.");
	data.reset(new lib::DG::Write::Data(dg->getHyper()));
}

PrintData::PrintData(const PrintData &other) : dg(other.dg), data(new lib::DG::Write::Data(*other.data)) {}

PrintData &PrintData::operator=(const PrintData &other) {
	dg = other.dg;
	data = std::make_unique<lib::DG::Write::Data>(*other.data);
	return *this;
}

PrintData::~PrintData() = default;

lib::DG::Write::Data &PrintData::getData() {
	return *data;
}

const lib::DG::Write::Data &PrintData::getData() const {
	return *data;
}

std::shared_ptr<DG> PrintData::getDG() const {
	return dg;
}

void PrintData::makeDuplicate(DG::HyperEdge e, int eDup) {
	printDataCheckHyperEdge(dg, e);
	const auto &dg = this->dg->getHyper();
	const auto eInner = dg.getInternalVertex(e);
	const bool res = data->makeDuplicate(eInner, eDup);
	if(!res) throw LogicError("Duplicate already exists.");
}

void PrintData::removeDuplicate(DG::HyperEdge e, int eDup) {
	printDataCheckHyperEdge(dg, e);
	const auto &dg = this->dg->getHyper();
	const auto eInner = dg.getInternalVertex(e);
	const bool res = data->removeDuplicate(eInner, eDup);
	if(!res) throw LogicError("Duplicate does not exist.");
}

void PrintData::reconnectSource(DG::HyperEdge e, int eDup, DG::Vertex v, int vDupTar) {
	reconnectCommon(dg, e, eDup, v, vDupTar, true);
	const auto &dg = this->dg->getHyper();
	const auto eInner = dg.getInternalVertex(e);
	const auto vTail = dg.getInternalVertex(v);
	data->reconnectSource(eInner, eDup, vTail, vDupTar, std::numeric_limits<int>::max());
}

void PrintData::reconnectTarget(DG::HyperEdge e, int eDup, DG::Vertex v, int vDupTar) {
	reconnectCommon(dg, e, eDup, v, vDupTar, false);
	const auto &dg = this->dg->getHyper();
	const auto eInner = dg.getInternalVertex(e);
	const auto vHead = dg.getInternalVertex(v);
	data->reconnectTarget(eInner, eDup, vHead, vDupTar, std::numeric_limits<int>::max());
}

//------------------------------------------------------------------------------
// Printer
//------------------------------------------------------------------------------

Printer::Printer() : graphPrinter(std::make_unique<graph::Printer>()),
                     printer(std::make_unique<lib::DG::Write::Printer>()) {
	graphPrinter->enableAll();
	graphPrinter->setWithIndex(false);
}

Printer::~Printer() = default;

Printer::Printer(const Printer &other) : graphPrinter(std::make_unique<graph::Printer>(*other.graphPrinter)),
                                         printer(std::make_unique<lib::DG::Write::Printer>(*other.printer)) {}

Printer &Printer::operator=(const Printer &other) {
	*graphPrinter = *other.graphPrinter;
	*printer = *other.printer;
	return *this;
}

lib::DG::Write::Printer &Printer::getPrinter() const {
	return *printer;
}

graph::Printer &Printer::getGraphPrinter() {
	return *graphPrinter;
}

const graph::Printer &Printer::getGraphPrinter() const {
	return *graphPrinter;
}

void Printer::setWithShortcutEdges(bool value) {
	printer->baseOptions.WithShortcutEdges(value);
}

bool Printer::getWithShortcutEdges() const {
	return printer->baseOptions.withShortcutEdges;
}

void Printer::setWithGraphImages(bool value) {
	printer->baseOptions.WithGraphImages(value);
}

bool Printer::getWithGraphImages() const {
	return printer->baseOptions.withGraphImages;
}

void Printer::setLabelsAsLatexMath(bool value) {
	printer->baseOptions.LabelsAsLatexMath(value);
}

bool Printer::getLabelsAsLatexMath() const {
	return printer->baseOptions.labelsAsLatexMath;
}

void Printer::pushVertexVisible(std::function<bool(DG::Vertex)> f) {
	if(!f) throw LogicError("Can not push empty callback.");
	printer->pushVertexVisible([f](lib::DG::HyperVertex v, const lib::DG::Hyper &dg) {
		return f(dg.getInterfaceVertex(v));
	});
}

void Printer::popVertexVisible() {
	if(!printer->hasVertexVisible())
		throw LogicError("No vertex visible callback to pop.");
	printer->popVertexVisible();
}

void Printer::pushEdgeVisible(std::function<bool(DG::HyperEdge)> f) {
	if(!f) throw LogicError("Can not push empty callback.");
	printer->pushEdgeVisible([f](lib::DG::HyperVertex v, const lib::DG::Hyper &dg) {
		assert(dg.getGraph()[v].kind == lib::DG::HyperVertexKind::Edge);
		return f(dg.getInterfaceEdge(v));
	});
}

void Printer::popEdgeVisible() {
	if(!printer->hasEdgeVisible())
		throw LogicError("No edge visible callback to pop.");
	printer->popEdgeVisible();
}

void Printer::setWithShortcutEdgesAfterVisibility(bool value) {
	printer->baseOptions.WithShortcutEdgesAfterVisibility(value);
}

bool Printer::getWithShortcutEdgesAfterVisibility() const {
	return printer->baseOptions.withShortcutEdgesAfterVisibility;
}

void Printer::setVertexLabelSep(std::string sep) {
	printer->vertexLabelSep = sep;
}

const std::string &Printer::getVertexLabelSep() {
	return printer->vertexLabelSep;
}

void Printer::setEdgeLabelSep(std::string sep) {
	printer->edgeLabelSep = sep;
}

const std::string &Printer::getEdgeLabelSep() {
	return printer->edgeLabelSep;
}

void Printer::pushVertexLabel(std::function<std::string(DG::Vertex)> f) {
	if(!f) throw LogicError("Can not push empty callback.");
	printer->pushVertexLabel([f](lib::DG::HyperVertex v, const lib::DG::Hyper &dg) {
		return f(dg.getInterfaceVertex(v));
	});
}

void Printer::popVertexLabel() {
	if(!printer->hasVertexLabel())
		throw LogicError("No vertex label callback to pop.");
	printer->popVertexLabel();
}

void Printer::pushEdgeLabel(std::function<std::string(DG::HyperEdge)> f) {
	if(!f) throw LogicError("Can not push empty callback.");
	printer->pushEdgeLabel([f](lib::DG::HyperVertex v, const lib::DG::Hyper &dg) {
		assert(dg.getGraph()[v].kind == lib::DG::HyperVertexKind::Edge);
		return f(dg.getInterfaceEdge(v));
	});
}

void Printer::popEdgeLabel() {
	if(!printer->hasEdgeLabel())
		throw LogicError("No edge label callback to pop.");
	printer->popEdgeLabel();
}

void Printer::setWithGraphName(bool value) {
	printer->withGraphName = value;
}

bool Printer::getWithGraphName() const {
	return printer->withGraphName;
}

void Printer::setWithRuleName(bool value) {
	printer->withRuleName = value;
}

bool Printer::getWithRuleName() const {
	return printer->withRuleName;
}

void Printer::setWithRuleId(bool value) {
	printer->withRuleId = value;
}

bool Printer::getWithRuleId() const {
	return printer->withRuleId;
}

void Printer::setWithInlineGraphs(bool value) {
	printer->baseOptions.withInlineGraphs = value;
}

bool Printer::getWithInlineGraphs() const {
	return printer->baseOptions.withInlineGraphs;
}

void Printer::pushVertexColour(std::function<std::string(DG::Vertex)> f, bool extendToEdges) {
	if(!f) throw LogicError("Can not push empty callback.");
	printer->pushVertexColour([f](lib::DG::HyperVertex v, const lib::DG::Hyper &dg) {
		return f(dg.getInterfaceVertex(v));
	}, extendToEdges);
}

void Printer::popVertexColour() {
	if(!printer->hasVertexColour())
		throw LogicError("No vertex colour callback to pop.");
	printer->popVertexColour();
}

void Printer::pushEdgeColour(std::function<std::string(DG::HyperEdge)> f) {
	if(!f) throw LogicError("Can not push empty callback.");
	printer->pushEdgeColour([f](lib::DG::HyperVertex v, const lib::DG::Hyper &dg) {
		assert(dg.getGraph()[v].kind == lib::DG::HyperVertexKind::Edge);
		return f(dg.getInterfaceEdge(v));
	});
}

void Printer::popEdgeColour() {
	if(!printer->hasEdgeColour())
		throw LogicError("No edge colour callback to pop.");
	printer->popEdgeColour();
}

void Printer::setRotationOverwrite(std::function<int(std::shared_ptr<graph::Graph>)> f) {
	if(!f) throw LogicError("Can not set empty callback.");
	printer->setRotationOverwrite(f);
}

void Printer::setMirrorOverwrite(std::function<bool(std::shared_ptr<graph::Graph>)> f) {
	if(!f) throw LogicError("Can not set empty callback.");
	printer->setMirrorOverwrite(f);
}

void Printer::setImageOverwrite(std::function<std::pair<std::string, std::string>(DG::Vertex v, int dupNum)> f) {
	if(!f) {
		printer->setImageOverwrite(nullptr);
	} else {
		printer->setImageOverwrite([f](lib::DG::HyperVertex v, int dupNum, const lib::DG::Hyper &dg) {
			assert(dg.getGraph()[v].kind == lib::DG::HyperVertexKind::Vertex);
			return f(dg.getInterfaceVertex(v), dupNum);
		});
	}
}

void Printer::setGraphvizPrefix(const std::string &prefix) {
	printer->setGraphvizPrefix(prefix);
}

const std::string &Printer::getGraphvizPrefix() const {
	return printer->getGraphvizPrefix();
}

void Printer::setTikzpictureOption(const std::string &option) {
	printer->setTikzpictureOption(option);
}

const std::string &Printer::getTikzpictureOption() const {
	return printer->getTikzpictureOption();
}

} // namespace mod::dg