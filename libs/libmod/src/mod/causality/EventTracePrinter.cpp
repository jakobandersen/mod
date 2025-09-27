#include "EventTracePrinter.hpp"

#include <mod/Error.hpp>
#include <mod/lib/Causality/IO/Write.hpp>
#include <mod/lib/DG/Hyper.hpp>

namespace mod::causality {

EventTracePrinter::EventTracePrinter() : printer(new lib::Causality::Write::EventTracePrinter()) {}

EventTracePrinter::EventTracePrinter(const EventTracePrinter &other) : printer(
		new lib::Causality::Write::EventTracePrinter(*other.printer)) {}

EventTracePrinter &EventTracePrinter::operator=(const EventTracePrinter &other) {
	printer.reset(new lib::Causality::Write::EventTracePrinter(*other.printer));
	return *this;
}

EventTracePrinter::~EventTracePrinter() = default;

const lib::Causality::Write::EventTracePrinter &EventTracePrinter::getPrinter() const {
	return *printer;
}

void EventTracePrinter::setMaxPointsPerVertex(int value) {
	printer->maxPointsPerMolecule = value;
}

int EventTracePrinter::getMaxPointsPerVertex() const {
	return printer->maxPointsPerMolecule;
}

void EventTracePrinter::setLogTime(bool value) {
	printer->logTime = value;
}

bool EventTracePrinter::getLogTime() const {
	return printer->logTime;
}

void EventTracePrinter::setLogCount(bool value) {
	printer->logCount = value;
}

bool EventTracePrinter::getLogCount() const {
	return printer->logCount;
}

void EventTracePrinter::pushOptions(std::function<std::string(std::shared_ptr<dg::DG>)> f) {
	if(!f) throw LogicError("Can not push empty callback.");
	printer->options.push_back([f](const lib::DG::Hyper &dg) {
		return f(dg.getNonHyper().getAPIReference());
	});
}

void EventTracePrinter::popOptions() {
	if(printer->options.empty())
		throw LogicError("No options callback to pop.");
	printer->options.pop_back();
}

void EventTracePrinter::pushVertexVisible(std::function<bool(dg::DG::Vertex)> f) {
	if(!f) throw LogicError("Can not push empty callback.");
	printer->vertexVisibles.push_back([f](lib::DG::HyperVertex v, const lib::DG::Hyper &dg) {
		return f(dg.getInterfaceVertex(v));
	});
}

void EventTracePrinter::popVertexVisible() {
	if(printer->vertexVisibles.empty())
		throw LogicError("No vertex visible callback to pop.");
	printer->vertexVisibles.pop_back();
}

void EventTracePrinter::pushVertexOptions(std::function<std::string(dg::DG::Vertex)> f) {
	if(!f) throw LogicError("Can not push empty callback.");
	printer->vertexOptions.push_back([f](lib::DG::HyperVertex v, const lib::DG::Hyper &dg) {
		return f(dg.getInterfaceVertex(v));
	});
}

void EventTracePrinter::popVertexOptions() {
	if(printer->vertexOptions.empty())
		throw LogicError("No vertex options callback to pop.");
	printer->vertexOptions.pop_back();
}

void EventTracePrinter::setPreContent(std::function<std::string(std::shared_ptr<dg::DG>)> f) {
	if(!f) throw LogicError("Can not set empty callback.");
	printer->preContent = [f](const lib::DG::Hyper &dg) {
		return f(dg.getNonHyper().getAPIReference());
	};
}

void EventTracePrinter::setPostContent(std::function<std::string(std::shared_ptr<dg::DG>)> f) {
	if(!f) throw LogicError("Can not set empty callback.");
	printer->postContent = [f](const lib::DG::Hyper &dg) {
		return f(dg.getNonHyper().getAPIReference());
	};
}

} // namespace mod::causality