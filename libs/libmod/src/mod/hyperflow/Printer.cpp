#include "Printer.hpp"

#include <mod/Error.hpp>
#include <mod/dg/Printer.hpp>
#include <mod/lib/HyperFlow/IO/Write.hpp>

namespace mod::hyperflow {

Printer::Printer() : dgPrinter(std::make_unique<dg::Printer>()),
                     printer(std::make_unique<lib::HyperFlow::Write::Printer>()) {}

Printer::~Printer() = default;

dg::Printer &Printer::getDGPrinter() {
	return *dgPrinter;
}

const dg::Printer &Printer::getDGPrinter() const {
	return *dgPrinter;
}

lib::HyperFlow::Write::Printer &Printer::getPrinter() const {
	return *printer;
}

void Printer::setWithFlowLabels(bool value) {
	printer->withFlowLabel = value;
}

bool Printer::getWithFlowLabels() const {
	return printer->withFlowLabel;
}

void Printer::pushInEdgeLabel(std::function<std::string(dg::DG::Vertex)> f) {
	printer->pushInEdgeLabel(f);
}

void Printer::popInEdgeLabel() {
	if(printer->inEdgeLabels.empty())
		throw LogicError("No in-edge label callback to pop.");
	printer->popInEdgeLabel();
}

void Printer::pushOutEdgeLabel(std::function<std::string(dg::DG::Vertex)> f) {
	printer->pushOutEdgeLabel(f);
}

void Printer::popOutEdgeLabel() {
	if(printer->outEdgeLabels.empty())
		throw LogicError("No out-edge label callback to pop.");
	printer->popOutEdgeLabel();
}

void Printer::setPrintUnfiltered(bool value) {
	printer->printUnfiltered = value;
}

bool Printer::getPrintUnfiltered() const {
	return printer->printUnfiltered;
}

void Printer::setPrintFiltered(bool value) {
	printer->printFiltered = value;
}

bool Printer::getPrintFiltered() const {
	return printer->printFiltered;
}

void Printer::setUnfilteredFlowColour(std::string value) {
	printer->unfilteredFlowColour = value;
}

std::string Printer::getUnfilteredFlowColour() const {
	return printer->unfilteredFlowColour;
}

} // namespace mod::hyperflow