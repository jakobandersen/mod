#ifndef MOD_LIB_CAUSALITY_STOCHSIM_HPP
#define MOD_LIB_CAUSALITY_STOCHSIM_HPP

#include<mod/dg/Builder.hpp>
#include <mod/causality/EventTrace.hpp>
#include <mod/causality/Petri.hpp>
#include <mod/causality/Stochsim.hpp>
#include <mod/lib/DG/GraphDecl.hpp>
#include <mod/lib/Causality/EventTrace.hpp>

#include <functional>

namespace mod::lib::DG {
struct Hyper;
} // namespace mod::lib::DG
namespace mod::lib::Causality {

struct ChoiceAccess {
	static causality::Simulator::DrawFunction::Choice makeChoice(std::size_t type, std::size_t index, std::shared_ptr<dg::DG> dg) {
		return {type, index, dg};
	}

	static std::pair<std::size_t, std::size_t> getData(const causality::Simulator::DrawFunction::Choice &c) {
		return {c.type, c.index};
	}
};

struct DrawChoice {
	int type;
	std::size_t index;
};

struct DrawMassActionFunction {
	DrawMassActionFunction(const std::string &impl,
	                       const Marking &state,
	                       std::function<std::pair<double, bool>(const lib::DG::Hyper &, lib::DG::HyperVertex)>
	                       inputRate,
	                       std::function<std::pair<double, bool>(const lib::DG::Hyper &, lib::DG::HyperVertex)>
	                       reactionRate,
	                       std::function<std::pair<double, bool>(const lib::DG::Hyper &, lib::DG::HyperVertex)>
	                       outputRate);
	~DrawMassActionFunction();
	void syncSize();
	// .second is 0.0 when no actions are possible
	std::pair<DrawChoice, double> draw();
	void stateUpdated();
private:
	double reactionPropensity(lib::DG::HyperVertex e, const Marking &m);
	std::pair<Action, double> draw_v0(const Marking &m);
public:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

struct Simulator {
	using ExpandNetwork = std::function<bool(
		std::shared_ptr<dg::Builder>,
		const std::vector<std::shared_ptr<mod::graph::Graph>> &,
		const std::vector<std::shared_ptr<mod::graph::Graph>> &)>;
	Simulator(LabelSettings labelSettings,
	          const std::vector<std::shared_ptr<mod::graph::Graph>> &graphDatabase,
	          IsomorphismPolicy graphPolicy,
	          ExpandNetwork expandNetwork,
	          const std::vector<std::pair<std::shared_ptr<mod::graph::Graph>, int>> &initialState,
	          std::function<std::shared_ptr<causality::Simulator::DrawFunction>(const causality::Marking&)> draw,
	          std::function<double(double)> drawTime,
	          bool withSetCompare);
public:
	std::shared_ptr<dg::DG> getDG() const { return dg; }
	int getIteration() const { return iteration; }
	double getTime() const { return time; }
	const causality::Marking &getState() const { return state; }
	const causality::EventTrace &getTrace() const { return trace; }
	bool isNetworkOpen() const;
	void closeNetwork();
public:
	void setOnIterationBegin(std::function<void(Simulator&)> f, int interval) {
		assert(interval > 0);
		onIterationBegin = f;
		onIterationBeginInterval = interval;
	}
	void setOnIterationEnd(std::function<bool(Simulator&)> f) { onIterationEnd = f; }
	void setOnDeadlock(std::function<void(Simulator &)> f) { onDeadlock = f; }
	void setOnExpand(std::function<void(Simulator &)> f) { onExpand = f; }
	void setOnExpandAvoided(std::function<void(Simulator &)> f) { onExpandAvoided = f; }
public:
	void simulate(std::optional<double> time, bool advanceToEndTime, std::optional<int> iterations, bool keepNetworkOpen);
private:
	void expandNeighbourhood(const std::vector<dg::DG::Vertex> &subset);
private:
	std::shared_ptr<dg::DG> dg;
	std::shared_ptr<dg::Builder> builder;
	std::shared_ptr<causality::Net> net;
	causality::Marking state;
	causality::EventTrace trace;
	std::shared_ptr<causality::Simulator::DrawFunction> draw;
	std::function<double(double)> drawTime;
	int iteration = 0;
	double time = 0;
	bool doExpansion = true;
	const bool withSetCompare;
	struct MarkingSet {
		bool addIfNotSubset(const Marking &m);
	private:
		std::vector<std::vector<int>> sets;
	} markingSet;
private:
	ExpandNetwork expandNetwork;
	std::function<void(Simulator&)> onIterationBegin;
	std::function<bool(Simulator&)> onIterationEnd;
	std::function<void(Simulator&)> onDeadlock;
	std::function<void(Simulator&)> onExpand, onExpandAvoided;
	int onIterationBeginInterval = 1;
};

} // namespace mod::lib::Causality

#endif // MOD_LIB_CAUSALITY_STOCHSIM_HPP