#include "Stochsim.hpp"

#include <mod/Error.hpp>
#include <mod/causality/Petri.hpp>
#include <mod/lib/Random.hpp>
#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/DG/Hyper.hpp>
#include <mod/lib/DG/Strategies/Strategy.hpp>
#include <mod/lib/Causality/EventTrace.hpp>
#include <mod/lib/Causality/Petri.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <boost/math/special_functions/binomial.hpp>

#include <algorithm>
#include <iostream>

namespace mod::lib::Causality {

struct DrawMassActionFunction::Impl {
	Impl(const Marking &state,
	     std::function<std::pair<double, bool>(const lib::DG::Hyper &, lib::DG::HyperVertex)> inputRate,
	     std::function<std::pair<double, bool>(const lib::DG::Hyper &, lib::DG::HyperVertex)> reactionRate,
	     std::function<std::pair<double, bool>(const lib::DG::Hyper &, lib::DG::HyperVertex)> outputRate)
		: state(state), inputRate(inputRate), reactionRate(reactionRate), outputRate(outputRate) {}

	virtual ~Impl() = default;
	virtual void syncSize() = 0;
	virtual std::pair<DrawChoice, double> draw() = 0;
	virtual void stateUpdated() = 0;
public:
	const Marking &state;
	const std::function<std::pair<double, bool>(const lib::DG::Hyper &, lib::DG::HyperVertex)>
			inputRate, reactionRate, outputRate;
};

namespace {

double reactionCombinatorialFactor(lib::DG::HyperVertex e, const Marking &m) {
	const auto &outerNet = m.getNet();
	const petri::Transition t = outerNet.getTransition(e);
	const auto &marking = m.getMarking();
	assert(marking.isEnabled(t));
	const auto &net = outerNet.getNet();
	const auto &g = net.getGraph();
	const auto vt = net.vertexFromTransition(t);
	double res = 1.0;
	for(const auto eIn: asRange(in_edges(vt, g))) {
		const auto vIn = source(eIn, g);
		const int c = marking[net.placeFromVertex(vIn)];
		const int w = g[eIn];
		switch(w) {
		case 1:
			res *= c;
			break;
		case 2:
			res *= c * (c - 1) / 2;
			break;
		default:
			res *= boost::math::binomial_coefficient<double>(c, w);
			break;
		}
	}
	return res;
}


struct V0_base : DrawMassActionFunction::Impl {
	using DrawMassActionFunction::Impl::Impl;
public:
	virtual void syncSize() override {
		const auto &g = state.getNet().getDG().getGraph();
		const auto n = num_vertices(g);
		cachedInputRates.resize(n, -1.0);
		cachedRates.resize(n, -1.0);
	}

	virtual std::pair<DrawChoice, double> draw() override {
		constexpr bool VERBOSE = false;

		if(VERBOSE) std::cout << __func__ << ":" << __LINE__ << ":" << std::endl;

		const auto &dg = state.getNet().getDG();
		const auto &dgGraph = dg.getGraph();

		std::vector<std::pair<int, double>> propensities; // .first: non-negative==reaction/output, negative: -input - 1
		propensities.reserve(num_vertices(dgGraph));

		for(const auto e: state.getAllEnabled()) {
			const auto idx = get(boost::vertex_index_t(), dgGraph, e);
			assert(idx < cachedRates.size());
			double r = cachedRates[idx];
			if(r < 0) {
				if(reactionRate) {
					bool cache;
					std::tie(r, cache) = reactionRate(dg, e);
					assert(r >= 0);
					if(cache) cachedRates[idx] = r;
				} else {
					cachedRates[idx] = r = 1.0;
				}
			}
			if(r != 0) propensities.emplace_back(idx, r * reactionCombinatorialFactor(e, state));
		}
		for(const auto v: state.getNonZeroPlaces()) {
			const auto idx = get(boost::vertex_index_t(), dgGraph, v);
			assert(idx < cachedRates.size());
			double r = cachedRates[idx];
			if(r < 0) {
				if(outputRate) {
					bool cache;
					std::tie(r, cache) = outputRate(dg, v);
					assert(r >= 0);
					if(cache) cachedRates[idx] = r;
				} else {
					cachedRates[idx] = r = 0.0;
				}
			}
			if(r != 0) propensities.emplace_back(idx, r * state.getMarking()[state.getNet().getPlace(v)]);
		}
		for(const auto v: asRange(vertices(dgGraph))) {
			if(dgGraph[v].kind != lib::DG::HyperVertexKind::Vertex) continue;
			const auto idx = get(boost::vertex_index_t(), dgGraph, v);
			assert(idx < cachedInputRates.size());
			double r = cachedInputRates[idx];
			if(r < 0) {
				if(inputRate) {
					bool cache;
					std::tie(r, cache) = inputRate(dg, v);
					assert(r >= 0);
					if(cache) cachedInputRates[idx] = r;
				} else {
					cachedInputRates[idx] = r = 0.0;
				}
			}
			if(r != 0) propensities.emplace_back(-idx - 1, r);
		}

		if(propensities.empty()) {
			if(VERBOSE) std::cout << __func__ << ":" << __LINE__ << ": no actions" << std::endl;
			return {{}, 0.0};
		}

		std::vector<double> accPropensities(propensities.size());
		{
			// TODO: when GCC 8 can be dropped, change to the commented code
			//		std::inclusive_scan(propensities.begin(), propensities.end(), accPropensities.begin(),
			//		                    [](double a, std::pair<int, double> b) {
			//			                    return a + b.second;
			//		                    }, 0.0);
			double sum = 0;
			auto out = accPropensities.begin();
			for(const auto &p: propensities) {
				sum += p.second;
				*out = sum;
				++out;
			}
		}

		if(VERBOSE) {
			std::cout << __func__ << ":" << __LINE__ << ": propensities and accPropensities:" << std::endl;
			for(int i = 0; i != propensities.size(); ++i) {
				std::cout << __func__ << ":" << __LINE__
						<< ": " << propensities[i].first
						<< " " << propensities[i].second
						<< " " << accPropensities[i] << std::endl;
			}
		}

		std::uniform_real_distribution<> dist(0, accPropensities.back());
		auto &rng = mod::lib::getRng();
		const double rnd = dist(rng);
		const auto pos = std::lower_bound(accPropensities.begin(), accPropensities.end(), rnd);
		const auto i = pos - accPropensities.begin();
		if(VERBOSE) std::cout << __func__ << ":" << __LINE__ << ": rnd=" << rnd << " i=" << i << std::endl;
		const auto actId = propensities[i].first;
		if(actId >= 0) {
			std::size_t idx = actId;
			const auto v = vertices(dgGraph).first[actId];
			assert(get(boost::vertex_index_t(), dgGraph, v) == actId);
			if(dgGraph[v].kind == lib::DG::HyperVertexKind::Edge) {
				return {DrawChoice{0, idx}, accPropensities.back()};
			} else {
				return {DrawChoice{2, idx}, accPropensities.back()};
			}
		} else {
			std::size_t idx = -actId - 1;
#ifdef  _NDEBUG
			const auto v = vertices(dgGraph).first[idx];
			assert(get(boost::vertex_index_t(), dgGraph, v) == idx);
			assert(dgGraph[v].kind == lib::DG::HyperVertexKind::Vertex);
#endif
			return {DrawChoice{1, idx}, accPropensities.back()};
		}
	}

	virtual void stateUpdated() override {}
public:
	std::vector<double> cachedInputRates, cachedRates /* reaction and output */;
};

struct V1_base : DrawMassActionFunction::Impl {
	static constexpr bool VERBOSE = false;
	using DrawMassActionFunction::Impl::Impl;
public:
	// Which vertices and hyperedges to actually query for rate data again.
	// So, if a rate callback returns that the rate should be cached, then it should be removed from here.
	std::vector<lib::DG::HyperVertex> nonStaticInputRates, nonStaticReactionRates, nonStaticOutputRates;

	struct ActionData {
		std::size_t propensityIdx = -1; // index into propensityCache, -1 means not present, i.e., rate 0 or unevaluated
		double rate = -1;
	};

	// dg idx -> ActionData
	// their length is updated in syncSize, except [0] is always empty if !inputRate
	// [0]: input
	// [1]: output, reaction
	std::array<std::vector<ActionData>, 2> actionData;

	struct ActionId {
		int type; // 0, 1: index into actionData
		std::size_t idx; // index into actionData[type]
	};

	struct Propensity {
		double value;
		ActionId action;
	};

	std::vector<Propensity> propensityCache;
	std::vector<double> accPropensities;
	std::size_t lastActionIdx = -1;
public:
	void print(std::ostream &s) const {
		s << "actionData:\n";
		const auto pActionData = [&](int id) {
			s << "[";
			for(int i = 0; i != actionData[id].size(); ++i) {
				if(i != 0) s << ", ";
				s << "(";
				if(actionData[id][i].propensityIdx == -1)
					s << "_";
				else
					s << actionData[id][i].propensityIdx;
				s << ", ";
				if(actionData[id][i].rate)
					s << "_";
				else
					s << actionData[id][i].rate;
				s << ")";
			}
			s << "]\n";
		};
		s << "  [0] input:       ";
		pActionData(0);
		s << "  [1] output/edge: ";
		pActionData(1);
	}

	virtual void syncSize() override {
		const auto &dg = state.getNet().getDG();
		const auto &g = dg.getGraph();
		const auto nOld = actionData[1].size();
		const auto n = num_vertices(g);
		if(VERBOSE) {
			std::cout << __func__ << ":" << __LINE__ << ": initial, nOld=" << nOld << ", n=" << n << std::endl;
			print(std::cout);
			std::cout << std::flush;
		}

		const auto vs = vertices(g);
		if(inputRate)
			actionData[0].resize(n);
		actionData[1].resize(n);
		for(const auto v: asRange(vs.first + nOld, vs.second)) {
			const auto idx = get(boost::vertex_index_t(), g, v);
			if(g[v].kind == lib::DG::HyperVertexKind::Vertex) {
				if(inputRate) {
					nonStaticInputRates.push_back(v);
					actionData[0][idx].propensityIdx = propensityCache.size();
					propensityCache.push_back(Propensity{0, ActionId{0, idx}});
				}
				if(outputRate) {
					nonStaticOutputRates.push_back(v);
					actionData[1][idx].propensityIdx = propensityCache.size();
					propensityCache.push_back(Propensity{0, ActionId{1, idx}});
				}
			} else {
				actionData[1][idx].propensityIdx = propensityCache.size();
				if(reactionRate) {
					nonStaticReactionRates.push_back(v);
					propensityCache.push_back(Propensity{0, ActionId{1, idx}});
				} else {
					actionData[1][idx].rate = 1;
					const auto p = this->state.isEnabled(v) ? reactionCombinatorialFactor(v, this->state) : 0;
					propensityCache.push_back(Propensity{p, ActionId{1, idx}});
				}
			}
		}
		if(VERBOSE) {
			std::cout << __func__ << ":" << __LINE__ << ": end" << std::endl;
			print(std::cout);
			std::cout << std::flush;
		}
		accPropensities.resize(propensityCache.size());
	}

	virtual std::pair<DrawChoice, double> draw() override {
		if(VERBOSE) std::cout << __func__ << ":" << __LINE__ << ":" << std::endl;

		const auto &dg = state.getNet().getDG();
		const auto &g = dg.getGraph();

		// Query for updated rates
		{
			if(VERBOSE) {
				std::cout << __func__ << ":" << __LINE__ << ": query updated input rates ("
						<< nonStaticInputRates.size() << ")" << std::endl;
			}
			assert(inputRate || nonStaticInputRates.empty());
			const auto iter = std::stable_partition(nonStaticInputRates.begin(), nonStaticInputRates.end(),
				[this, &dg, &g](const auto v) {
					const auto idx = get(boost::vertex_index_t(), g, v);
					if(VERBOSE) std::cout << "  " << idx << std::endl;
					const auto [rate, cache] = inputRate(dg, v);
					if(VERBOSE)
						std::cout << "    rate=" << rate << ", cache=" << std::boolalpha << cache << std::endl;
					auto &d = actionData[0][idx];
					if(d.rate != rate) {
						d.rate = rate;
						if(VERBOSE) std::cout << "    new rate/propensity: " << rate << std::endl;
						propensityCache[d.propensityIdx].value = rate;
					}
					return !cache;
				});
			const auto newSize = iter - nonStaticInputRates.begin();
			if(VERBOSE) {
				std::cout << "  shrink nonStaticInputRates to "
					<< newSize << " from " << nonStaticInputRates.size() << std::endl;
			}
			nonStaticInputRates.resize(newSize);
		}
		{
			if(VERBOSE) {
				std::cout << __func__ << ":" << __LINE__ << ": query updated output rates ("
						<< nonStaticOutputRates.size() << ")" << std::endl;
			}
			assert(outputRate || nonStaticOutputRates.empty());
			const auto iter = std::stable_partition(nonStaticOutputRates.begin(), nonStaticOutputRates.end(),
				[this, &dg, &g](const auto v) {
					const auto idx = get(boost::vertex_index_t(), g, v);
					if(VERBOSE) std::cout << "  " << idx << std::endl;
					const auto [rate, cache] = outputRate(dg, v);
					if(VERBOSE)
						std::cout << "    rate=" << rate << ", cache=" << std::boolalpha << cache << std::endl;
					auto &d = actionData[1][idx];
					if(d.rate != rate) {
						d.rate = rate;
						const auto p = rate * this->state[v];
						if(VERBOSE) std::cout << "    new propensity: " << p << std::endl;
						propensityCache[d.propensityIdx].value = p;
					}
					return !cache;
				});
			const auto newSize = iter - nonStaticOutputRates.begin();
			if(VERBOSE) {
				std::cout << "  shrink nonStaticInputRates to "
					<< newSize << " from " << nonStaticOutputRates.size() << std::endl;
			}
			nonStaticOutputRates.resize(newSize);
		}
		{
			if(VERBOSE) {
				std::cout << __func__ << ":" << __LINE__ << ": query updated reaction rates ("
						<< nonStaticReactionRates.size() << ")" << std::endl;
			}
			assert(reactionRate || nonStaticReactionRates.empty());
			const auto iter = std::stable_partition(nonStaticReactionRates.begin(), nonStaticReactionRates.end(),
				[this, &dg, &g](const auto e) {
					if(VERBOSE) {
						const auto idx = get(boost::vertex_index_t(), g, e);
						std::cout << "  " << idx << ": isEnabled=" << std::boolalpha << state.isEnabled(e) << std::endl;
					}
					if(!state.isEnabled(e)) return true; // let's check with the callback another iteration
					const auto [rate, cache] = reactionRate(dg, e);
					const auto idx = get(boost::vertex_index_t(), g, e);
					if(VERBOSE)
						std::cout << "    rate=" << rate << ", cache=" << std::boolalpha << cache << std::endl;
					auto &d = actionData[1][idx];
					if(d.rate != rate) {
						d.rate = rate;
						const auto p = rate * reactionCombinatorialFactor(e, this->state);
						if(VERBOSE) std::cout << "    new propensity: " << p << std::endl;
						propensityCache[d.propensityIdx].value = p;
					}
					return !cache;
				});
			const auto newSize = iter - nonStaticReactionRates.begin();
			if(VERBOSE) {
				std::cout << "  shrink nonStaticReactionRates to "
					<< newSize << " from " << nonStaticReactionRates.size() << std::endl;
			}
			nonStaticReactionRates.resize(newSize);
		}

		// now update the prefix sum of the propensities
		{
			double sum = 0;
			auto out = accPropensities.begin();
			for(const auto &p: propensityCache) {
				sum += p.value;
				*out = sum;
				++out;
			}
			if(VERBOSE) {
				std::cout << __func__ << ":" << __LINE__ << ": propensities and accPropensities:" << std::endl;
				for(int i = 0; i != propensityCache.size(); ++i) {
					std::cout << __func__ << ":" << __LINE__
							<< ": " << i << " (" << propensityCache[i].action.type << ", " << propensityCache[i].action.idx << ")"
							<< " " << propensityCache[i].value
							<< " " << accPropensities[i] << std::endl;
				}
				std::cout << __func__ << ":" << __LINE__ << ": sum=" << sum << std::endl;
			}
			if(sum == 0) // deadlock
				return {{}, 0};
		}

		// now we can draw an event
		std::uniform_real_distribution<> dist(0, accPropensities.back());
		auto &rng = mod::lib::getRng();
		const double rnd = dist(rng);
		const auto pos = std::lower_bound(accPropensities.begin(), accPropensities.end(), rnd);
		const auto pIdx = pos - accPropensities.begin();
		if(VERBOSE) std::cout << __func__ << ":" << __LINE__ << ": rnd=" << rnd << " i=" << pIdx << std::endl;
		const auto act = propensityCache[pIdx].action;
		const auto v = vertices(g).first[act.idx];
		assert(get(boost::vertex_index_t(), g, v) == act.idx);
		lastActionIdx = pIdx;
		if(g[v].kind == lib::DG::HyperVertexKind::Edge) {
			assert(act.type == 1);
			return {DrawChoice{0, act.idx}, accPropensities.back()};
		} else {
			assert(g[v].kind == lib::DG::HyperVertexKind::Vertex);
			if(act.type == 0) {
				return {DrawChoice{1, act.idx}, accPropensities.back()};
			} else {
				assert(act.type == 1);
				return {DrawChoice{2, act.idx}, accPropensities.back()};
			}
		}
	}

	virtual void stateUpdated() override {
		assert(lastActionIdx != -1);
		const auto &dg = state.getNet().getDG();
		const auto &g = dg.getGraph();

		const auto act = propensityCache[lastActionIdx].action;
		const auto v = vertices(g).first[act.idx];
		assert(get(boost::vertex_index_t(), g, v) == act.idx);

		const auto handleEdge = [this, &g](const auto e) {
			const auto eIdx = get(boost::vertex_index_t(), g, e);
			auto &dEdge = actionData[1][eIdx];
			assert(dEdge.propensityIdx != -1);
			if(state.isEnabled(e)) {
				if(dEdge.rate != -1)
					// if the rate has not been fetched, then the next iteration will fetch it and update anyway
					propensityCache[dEdge.propensityIdx].value = dEdge.rate * reactionCombinatorialFactor(e, state);
			} else {
				propensityCache[dEdge.propensityIdx].value = 0;
			}
		};
		const auto handleVertex = [this, &g](const auto v) {
			const auto vIdx = get(boost::vertex_index_t(), g, v);
			if(outputRate) {
				if(const auto &dOutput = actionData[1][vIdx]; dOutput.propensityIdx != -1)
					propensityCache[dOutput.propensityIdx].value = dOutput.rate * state[v];
			}
		};
		if(g[v].kind == lib::DG::HyperVertexKind::Edge) {
			assert(act.type == 1);
			for(const auto vIn: asRange(inv_adjacent_vertices(v, g))) {
				for(const auto e: asRange(adjacent_vertices(vIn, g)))
					handleEdge(e);
				handleVertex(vIn);
			}
			for(const auto vOut: asRange(adjacent_vertices(v, g))) {
				for(const auto e: asRange(adjacent_vertices(vOut, g)))
					handleEdge(e);
				handleVertex(vOut);
			}
		} else {
			assert(g[v].kind == lib::DG::HyperVertexKind::Vertex);
			handleVertex(v);
			for(const auto e: asRange(adjacent_vertices(v, g)))
				handleEdge(e);
		}
	}
};


std::unique_ptr<DrawMassActionFunction::Impl> makeImpl(
		const std::string &impl,
		const Marking &state,
		std::function<std::pair<double, bool>(const lib::DG::Hyper &, lib::DG::HyperVertex)> inputRate,
		std::function<std::pair<double, bool>(const lib::DG::Hyper &, lib::DG::HyperVertex)> reactionRate,
		std::function<std::pair<double, bool>(const lib::DG::Hyper &, lib::DG::HyperVertex)> outputRate) {
	if(impl == "v0") {
		return std::make_unique<V0_base>(state, inputRate, reactionRate, outputRate);
	} else if(impl == "v1") {
		return std::make_unique<V1_base>(state, inputRate, reactionRate, outputRate);
	} else {
		throw LogicError("Can not create DrawMassAction function. Implementation type '" + impl + "' not known.");
	}
}

} // namespace

DrawMassActionFunction::DrawMassActionFunction(
		const std::string &impl,
		const Marking &state,
		std::function<std::pair<double, bool>(const lib::DG::Hyper &, lib::DG::HyperVertex)> inputRate,
		std::function<std::pair<double, bool>(const lib::DG::Hyper &, lib::DG::HyperVertex)> reactionRate,
		std::function<std::pair<double, bool>(const lib::DG::Hyper &, lib::DG::HyperVertex)> outputRate)
	: impl(makeImpl(impl, state, inputRate, reactionRate, outputRate)) {
	syncSize();
}

DrawMassActionFunction::~DrawMassActionFunction() = default;

void DrawMassActionFunction::syncSize() {
	impl->syncSize();
}

std::pair<DrawChoice, double> DrawMassActionFunction::draw() {
	return impl->draw();
}

void DrawMassActionFunction::stateUpdated() {
	return impl->stateUpdated();
}

// =============================================================================================0

Simulator::Simulator(LabelSettings labelSettings,
                     const std::vector<std::shared_ptr<mod::graph::Graph>> &graphDatabase,
                     IsomorphismPolicy graphPolicy,
                     ExpandNetwork expandNetwork,
                     const std::vector<std::pair<std::shared_ptr<mod::graph::Graph>, int>> &initialState,
                     std::function<std::shared_ptr<causality::Simulator::DrawFunction>(const causality::Marking &)>
                     draw,
                     std::function<double(double)> drawTime,
                     bool withSetCompare)
	: dg(dg::DG::make(labelSettings, graphDatabase, graphPolicy)), builder(std::make_shared<dg::Builder>(dg->build())),
	  net(causality::Net::make(dg)), state(net), trace(state), draw(draw(state)), drawTime(drawTime),
	  withSetCompare(withSetCompare), expandNetwork(expandNetwork) {
	if(!this->draw)
		throw LogicError(
				"Creation of Simulator failed. Draw function initialization resulted in null function.");
	// make sure the initial state is represented in the DG
	std::vector<std::shared_ptr<mod::graph::Graph>> graphs;
	graphs.reserve(initialState.size());
	for(const auto &[g, c]: initialState)
		graphs.push_back(g);
	builder->execute(dg::Strategy::makeAdd(true, graphs, graphPolicy));
	state.syncSize();

	// set initial state
	for(const auto &[g, c]: initialState)
		state.add(g, c);

	// reinitialize the trace now the initial state is set
	trace = causality::EventTrace(state);
}

bool Simulator::isNetworkOpen() const {
	return builder != nullptr;
}

void Simulator::closeNetwork() {
	if(isNetworkOpen() && builder.use_count() == 1)
		builder = nullptr;
}

void Simulator::simulate(std::optional<double> time, bool advanceToEndTime, std::optional<int> iterations,
                         bool keepNetworkOpen) {
	if(time) *time += this->time;
	if(iterations) *iterations += this->iteration;

	std::vector<dg::DG::Vertex> subset = this->state.getNonZeroPlaces();
	if(doExpansion) {
		// Do an initial network expansion, e.g., to support a dynamically added
		// network, where no further expansions are doing anything.
		expandNeighbourhood(subset);
	}

	const auto &dgHyper = dg->getHyper();
	auto &marking = state.getMarking();
	auto &innerTrace = trace.getEventTrace();
	while(!iterations || this->iteration != *iterations) {
		++iteration;
		if(onIterationBegin && iteration % onIterationBeginInterval == 0)
			onIterationBegin(*this);

		// do we need to expand the neighbourhood?
		if(doExpansion && !subset.empty()) {
			if(!withSetCompare || markingSet.addIfNotSubset(state.getMarking())) {
				expandNeighbourhood(subset);
			} else {
				if(onExpandAvoided) onExpandAvoided(*this);
			}
		}

		// Pick reaction and time
		const auto [choice, totalActivity] = draw->draw();
		if(totalActivity == 0) {
			if(onDeadlock)
				onDeadlock(*this);
			break;
		}
		const auto timeInc = drawTime(totalActivity);
		if(time && this->time + timeInc > *time) {
			if(advanceToEndTime)
				this->time = *time;
			break;
		}
		this->time += timeInc;

		const auto[choiceType, choiceIndex] = ChoiceAccess::getData(choice);
		// Update state
		switch(choiceType) {
		case 0: {
			subset.clear();
			const auto &net = marking.getNet();
			const auto &pNet = net.getNet();
			const auto &gPetri = pNet.getGraph();
			const petri::Transition t = net.getTransition(choiceIndex);
			const auto vt = pNet.vertexFromTransition(t);
			for(auto pvOut : asRange(boost::adjacent_vertices(vt, gPetri))) {
				petri::Place pOut = pNet.placeFromVertex(pvOut);
				if(marking.getMarking()[pOut] == 0) {
					const auto v = net.getVertex(pOut);
					subset.push_back(dgHyper.getInterfaceVertex(v));
				}
			}
			break;
		}
		case 1:
			subset.resize(1);
			subset[0] = dgHyper.getInterfaceVertex(choiceIndex);
			break;
		case 2:
			subset.clear();
			break;
		}

		switch(choiceType) {
		case 0:
			marking.fire(choiceIndex);
			break;
		case 1:
			marking.add(choiceIndex, 1);
			break;
		case 2:
			marking.remove(choiceIndex, 1);
			break;
		}

		switch(choiceType) {
		case 0:
			innerTrace.add(EventTrace::Event{this->time, EdgeAction{choiceIndex}});
			break;
		case 1:
			innerTrace.add(EventTrace::Event{this->time, InputAction{choiceIndex}});
			break;
		case 2:
			innerTrace.add(EventTrace::Event{this->time, OutputAction{choiceIndex}});
			break;
		}
		draw->stateUpdated();
		if(onIterationEnd) {
			if(!onIterationEnd(*this))
				break;
		}
	}

	if(!keepNetworkOpen && isNetworkOpen())
		closeNetwork();
}

void Simulator::expandNeighbourhood(const std::vector<dg::DG::Vertex> &subset) {
	if(!isNetworkOpen())
		throw LogicError("Can not expand neighbourhood, the network is closed."
				" An earlier call to simulate() had keepNetworkOpen=False (the default).");
	if(onExpand) onExpand(*this);

	std::vector<std::shared_ptr<mod::graph::Graph>> subsetGraphs, universeGraphs;
	subsetGraphs.reserve(subset.size());
	for(const auto &v: subset)
		subsetGraphs.push_back(v.getGraph());
	for(const auto &v: state.getNonZeroPlaces())
		universeGraphs.push_back(v.getGraph());
	doExpansion = expandNetwork(builder, subsetGraphs, universeGraphs);
	state.syncSize();
	draw->syncSize();
}

// ==============================================================================================

bool Simulator::MarkingSet::addIfNotSubset(const Marking &m) {
	const auto &dgHyper = m.getNet().getDG();
	const auto idx = get(boost::vertex_index_t(), dgHyper.getGraph());
	const auto places = m.getNonZeroPlaces();
	std::vector<int> idxs(places.size());
	for(int i = 0; i != places.size(); ++i)
		idxs[i] = get(idx, places[i]);
	std::sort(idxs.begin(), idxs.end());
	for(const auto &b: sets)
		if(std::includes(b.begin(), b.end(), idxs.begin(), idxs.end()))
			return false;
	sets.emplace_back(std::move(idxs));
	return true;
}

} // namespace mod::lib::Causality