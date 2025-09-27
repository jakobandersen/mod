#include "Stochsim.hpp"

#include <mod/Error.hpp>
#include <mod/causality/Petri.hpp>
#include <mod/causality/EventTrace.hpp>
#include <mod/lib/Causality/Stochsim.hpp>

namespace mod::causality {

struct DrawMassActionFunction::Pimpl {
	std::shared_ptr<dg::DG> dg_;
	lib::Causality::DrawMassActionFunction m;
};

DrawMassActionFunction::DrawMassActionFunction(std::shared_ptr<dg::DG> dg_,
                                               std::function<std::pair<double, bool>(dg::DG::Vertex)> inputRate,
                                               std::function<std::pair<double, bool>(dg::DG::HyperEdge)> reactionRate,
                                               std::function<std::pair<double, bool>(dg::DG::Vertex)> outputRate) {
	if(!dg_) throw LogicError("The derivation graph is a null pointer.");
	if(!dg_->hasActiveBuilder() && !dg_->isLocked())
		throw LogicError("The DG neither has an active builder nor is locked yet.");

	using F = std::function<std::pair<double, bool>(const lib::DG::Hyper &, lib::DG::HyperVertex)>;
	F inputRateInner, reactionRateInner, outputRateInner;
	if(inputRate) {
		inputRateInner = [inputRate](const lib::DG::Hyper &dgHyper,
		                             const lib::DG::HyperVertex v) -> std::pair<double, bool> {
			return inputRate(dgHyper.getInterfaceVertex(v));
		};
	}
	if(reactionRate) {
		reactionRateInner = [reactionRate](const lib::DG::Hyper &dgHyper,
		                                   const lib::DG::HyperVertex e) -> std::pair<double, bool> {
			return reactionRate(dgHyper.getInterfaceEdge(e));
		};
	}
	if(outputRate) {
		outputRateInner = [outputRate](const lib::DG::Hyper &dgHyper,
		                               const lib::DG::HyperVertex v) -> std::pair<double, bool> {
			return outputRate(dgHyper.getInterfaceVertex(v));
		};
	}
	p.reset(new Pimpl{
		dg_, lib::Causality::DrawMassActionFunction(
				dg_->getHyper(), inputRateInner, reactionRateInner, outputRateInner)
	});
}

DrawMassActionFunction::~DrawMassActionFunction() = default;
DrawMassActionFunction::DrawMassActionFunction(DrawMassActionFunction &&) = default;
DrawMassActionFunction &DrawMassActionFunction::operator=(DrawMassActionFunction &&) = default;

DrawMassActionFunction::DrawMassActionFunction(const DrawMassActionFunction &other) {
	p.reset(new Pimpl(*other.p));
}

DrawMassActionFunction &DrawMassActionFunction::operator=(const DrawMassActionFunction &other) {
	if(&other != this)
		p.reset(new Pimpl(*other.p));
	return *this;
}

void DrawMassActionFunction::syncSize() {
	p->m.syncSize();
}

std::pair<std::optional<Action>, double> DrawMassActionFunction::draw(const Marking &m) {
	if(m.getNet()->getDG() != p->dg_) throw LogicError("The marking is not on the underlying derivation graph.");
	const auto [actionInner, total] = p->m.draw(m.getMarking());
	if(total == 0) return {std::nullopt, 0};
	struct Convert {
		Action operator()(lib::Causality::EdgeAction a) const {
			return EdgeAction(dgHyper.getInterfaceEdge(a.e));
		}

		Action operator()(lib::Causality::InputAction a) const {
			return InputAction(dgHyper.getInterfaceVertex(a.v));
		}

		Action operator()(lib::Causality::OutputAction a) const {
			return OutputAction(dgHyper.getInterfaceVertex(a.v));
		}
	public:
		const lib::DG::Hyper &dgHyper;
	};
	return {std::visit(Convert{p->dg_->getHyper()}, actionInner), total};
}

// =============================================================================================================

struct SimulatorImpl::Pimpl {
	lib::Causality::Simulator sim;
};

SimulatorImpl::SimulatorImpl() : p(new Pimpl()) {}

SimulatorImpl::~SimulatorImpl() = default;

int SimulatorImpl::getIteration() const { return p->sim.getIteration(); }
double SimulatorImpl::getTime() const { return p->sim.getTime(); }
void SimulatorImpl::setTime_delete(double value) { p->sim.setTime_delete(value); }

void SimulatorImpl::doIteration() {
	p->sim.doIteration();
}

} // namespace mod::causality