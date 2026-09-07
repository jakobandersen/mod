#include "Stochsim.hpp"

#include <mod/Error.hpp>
#include <mod/Misc.hpp>
#include <mod/causality/Petri.hpp>
#include <mod/causality/EventTrace.hpp>
#include <mod/lib/Causality/Stochsim.hpp>
#include <mod/lib/Causality/EventTrace.hpp>
#include <mod/lib/DG/Hyper.hpp>

namespace mod::causality {

struct Simulator::Pimpl {
	Pimpl(LabelSettings labelSettings,
	      const std::vector<std::shared_ptr<graph::Graph>> &graphDatabase,
	      IsomorphismPolicy graphPolicy,
	      lib::Causality::Simulator::ExpandNetwork expandNetwork,
	      const std::vector<std::pair<std::shared_ptr<graph::Graph>, int>> &initialState,
	      std::function<std::shared_ptr<DrawFunction>(const Marking &)> draw,
	      std::function<double(double)> drawTime,
	      bool withSetCompare)
		: sim(labelSettings, graphDatabase, graphPolicy, expandNetwork, initialState, draw, drawTime, withSetCompare) {}
public:
	lib::Causality::Simulator sim;
};

Simulator::Simulator(LabelSettings labelSettings,
                     const std::vector<std::shared_ptr<graph::Graph>> &graphDatabase,
                     IsomorphismPolicy graphPolicy,
                     ExpandNetwork expandNetwork,
                     const std::vector<std::pair<std::shared_ptr<graph::Graph>, int>> &initialState,
                     std::function<std::shared_ptr<DrawFunction>(const Marking &)> draw,
                     std::function<double(double)> drawTime,
                     bool withSetCompare) {
	if(!draw) throw LogicError("Can not create simulator with null draw function.");
	if(!drawTime) drawTime = DrawTimeExponential();
	p.reset(new Pimpl(labelSettings, graphDatabase, graphPolicy,
	                  expandNetwork, initialState, draw, drawTime, withSetCompare));
}

Simulator::~Simulator() = default;

std::shared_ptr<dg::DG> Simulator::getDG() const { return p->sim.getDG(); }
int Simulator::getIteration() const { return p->sim.getIteration(); }
double Simulator::getTime() const { return p->sim.getTime(); }

int Simulator::state(dg::DG::Vertex v) const {
	return p->sim.getState()[v];
}

int Simulator::state(std::shared_ptr<graph::Graph> g) const {
	return p->sim.getState()[g];
}

const EventTrace &Simulator::getTrace() const {
	return p->sim.getTrace();
}

bool Simulator::isNetworkOpen() const {
	return p->sim.isNetworkOpen();
}

void Simulator::closeNetwork() {
	p->sim.closeNetwork();
}

void Simulator::setOnIterationBegin(std::function<void(Simulator &)> f, int interval) {
	if(interval <= 0)
		throw LogicError("Can not set callback with non-positive interval.");
	if(f) {
		p->sim.setOnIterationBegin([this, f](lib::Causality::Simulator &sim) {
			return f(*this);
		}, interval);
	} else {
		p->sim.setOnIterationBegin({}, interval);
	}
}

void Simulator::setOnIterationEnd(std::function<bool(Simulator &)> f) {
	if(f) {
		p->sim.setOnIterationEnd([this, f](lib::Causality::Simulator &sim) {
			return f(*this);
		});
	} else {
		p->sim.setOnIterationEnd({});
	}
}

void Simulator::setOnDeadlock(std::function<void(Simulator &)> f) {
	if(f) {
		p->sim.setOnDeadlock([this, f](lib::Causality::Simulator &sim) {
			return f(*this);
		});
	} else {
		p->sim.setOnDeadlock({});
	}
}

void Simulator::setOnExpand(std::function<void(Simulator &)> f) {
	if(f) {
		p->sim.setOnExpand([this, f](lib::Causality::Simulator &sim) {
			return f(*this);
		});
	} else {
		p->sim.setOnExpand({});
	}
}

void Simulator::setOnExpandAvoided(std::function<void(Simulator &)> f) {
	if(f) {
		p->sim.setOnExpandAvoided([this, f](lib::Causality::Simulator &sim) {
			return f(*this);
		});
	} else {
		p->sim.setOnExpandAvoided({});
	}
}

const EventTrace &Simulator::simulate(std::optional<double> time, bool advanceToEndTime, std::optional<int> iterations,
                                      bool keepNetworkOpen) {
	p->sim.simulate(time, advanceToEndTime, iterations, keepNetworkOpen);
	return p->sim.getTrace();
}

// =============================================================================================================

double Simulator::DrawTimeExponential::operator()(double activitySum) const {
	return -std::log(rngUniformReal()) / activitySum;
}

// =============================================================================================================

namespace {

struct ChoiceFromAction {
	std::tuple<std::size_t, std::size_t, std::shared_ptr<dg::DG>> operator()(EdgeAction a) {
		return {0, a.edge.getId(), a.edge.getDG()};
	}

	std::tuple<std::size_t, std::size_t, std::shared_ptr<dg::DG>> operator()(InputAction a) {
		return {1, a.vertex.getId(), a.vertex.getDG()};
	}

	std::tuple<std::size_t, std::size_t, std::shared_ptr<dg::DG>> operator()(OutputAction a) {
		return {2, a.vertex.getId(), a.vertex.getDG()};
	}
};

} // namespace

Simulator::DrawFunction::Choice::Choice(Action a) {
	std::tie(type, index, dg) = std::visit(ChoiceFromAction(), a);
}

Simulator::DrawFunction::Choice::Choice(InputAction a) {
	std::tie(type, index, dg) = ChoiceFromAction()(a);
}

Simulator::DrawFunction::Choice::Choice(OutputAction a) {
	std::tie(type, index, dg) = ChoiceFromAction()(a);
}

Simulator::DrawFunction::Choice::Choice(EdgeAction a) {
	std::tie(type, index, dg) = ChoiceFromAction()(a);
}

std::optional<Action> Simulator::DrawFunction::Choice::asAction() const {
	if(type == -1) return {};
	const auto &dgHyper = dg->getHyper();
	switch(type) {
	case 0:
		return EdgeAction(dgHyper.getInterfaceEdge(index));
	case 1:
		return InputAction(dgHyper.getInterfaceVertex(index));
	case 2:
		return OutputAction(dgHyper.getInterfaceVertex(index));
	}
	__builtin_unreachable();
}

Simulator::DrawFunction::~DrawFunction() = default;

// =============================================================================================================

Simulator::DrawMassAction::DrawMassAction(std::function<std::pair<double, bool>(dg::DG::Vertex)> inputRate,
                                          std::function<std::pair<double, bool>(dg::DG::HyperEdge)> reactionRate,
                                          std::function<std::pair<double, bool>(dg::DG::Vertex)> outputRate,
                                          const std::string &implementationName)
	: inputRate(inputRate), reactionRate(reactionRate), outputRate(outputRate), implementationName(implementationName) {}

Simulator::DrawMassAction::~DrawMassAction() = default;

std::shared_ptr<Simulator::DrawMassAction::Function> Simulator::DrawMassAction::operator()(const Marking &state) {
	return std::make_shared<Function>(state, inputRate, reactionRate, outputRate, implementationName);
}

// =============================================================================================================

struct Simulator::DrawMassAction::Function::Pimpl {
	std::shared_ptr<dg::DG> dg_;
	lib::Causality::DrawMassActionFunction m;
};

Simulator::DrawMassAction::Function::Function(const Marking &state,
                                              std::function<std::pair<double, bool>(dg::DG::Vertex)> inputRate,
                                              std::function<std::pair<double, bool>(dg::DG::HyperEdge)> reactionRate,
                                              std::function<std::pair<double, bool>(dg::DG::Vertex)> outputRate,
                                              const std::string &implementation) {
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
		state.getNet()->getDG(), lib::Causality::DrawMassActionFunction(implementation,
		                                            state.getMarking(), inputRateInner, reactionRateInner,
		                                            outputRateInner)
	});
}

Simulator::DrawMassAction::Function::~Function() = default;

void Simulator::DrawMassAction::Function::syncSize() {
	p->m.syncSize();
}

std::pair<Simulator::DrawMassAction::Function::Choice, double> Simulator::DrawMassAction::Function::draw() {
	const auto [choiceInner, total] = p->m.draw();
	if(total == 0) return {{}, 0};
	return {lib::Causality::ChoiceAccess::makeChoice(choiceInner.type, choiceInner.index, p->dg_), total};
}

void Simulator::DrawMassAction::Function::stateUpdated() {
	return p->m.stateUpdated();
}

} // namespace mod::causality