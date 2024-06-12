#include "NonHyperBuilder.hpp"

#include <mod/Error.hpp>
#include <mod/Function.hpp>
#include <mod/Misc.hpp>
#include <mod/rule/Rule.hpp>
#include <mod/lib/DG/RuleApplicationUtils.hpp>
#include <mod/lib/DG/Strategies/GraphState.hpp>
#include <mod/lib/DG/Strategies/Strategy.hpp>
#include <mod/lib/DG/IO/Read.hpp>
#include <mod/lib/Graph/IO/Read.hpp>
#include <mod/lib/Graph/Properties/Term.hpp>
#include <mod/lib/IO/Config.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/IO/Json.hpp>
#include <mod/lib/RC/ComposeRuleReal.hpp>
#include <mod/lib/RC/MatchMaker/Super.hpp>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/lexical_cast.hpp>

namespace mod::lib::DG {

ExecuteResult::ExecuteResult(NonHyperBuilder *owner, int execution)
		: owner(owner), execution(execution) {}

const Strategies::GraphState &ExecuteResult::getResult() const {
	return owner->executions[execution].strategy->getOutput();
}

void ExecuteResult::list(bool withUniverse) const {
	owner->executions[execution].strategy->printInfo(
			Strategies::PrintSettings(std::cout, withUniverse));
	std::cout << std::flush;
}

// -----------------------------------------------------------------------------

Builder::Builder(NonHyperBuilder *dg,
                 std::shared_ptr<Function<void(dg::DG::Vertex)>> onNewVertex,
                 std::shared_ptr<Function<void(dg::DG::HyperEdge)>> onNewHyperEdge) : dg(dg) {
	if(dg->getHasCalculated()) {
		this->dg = nullptr;
		throw LogicError(dg->getType() + ": has already been build.");
	}
	dg->calculatePrologue(onNewVertex, onNewHyperEdge);
}

Builder::Builder(Builder &&other) : dg(other.dg) {
	other.dg = nullptr;
}

Builder &Builder::operator=(Builder &&other) {
	if(&other == this) return *this;
	dg = other.dg;
	other.dg = nullptr;
	return *this;
}

Builder::~Builder() {
	if(dg) dg->calculateEpilogue();
}

std::pair<NonHyper::Edge, bool> Builder::addDerivation(const Derivations &d, IsomorphismPolicy graphPolicy) {
	assert(!d.left.empty());
	assert(!d.right.empty());
	// add graphs
	switch(graphPolicy) {
	case IsomorphismPolicy::Check: {
		for(const auto &g: d.left)
			dg->tryAddGraph(g);
		for(const auto &g: d.right)
			dg->tryAddGraph(g);
		break;
	}
	case IsomorphismPolicy::TrustMe:
		for(const auto &g: d.left)
			dg->trustAddGraph(g);
		for(const auto &g: d.right)
			dg->trustAddGraph(g);
		break;
	}
	// add hyperedges
	const auto makeSide = [](const mod::Derivation::GraphList &graphs) -> GraphMultiset {
		std::vector<const lib::Graph::Single *> gPtrs;
		gPtrs.reserve(graphs.size());
		for(const auto &g: graphs) gPtrs.push_back(&g->getGraph());
		return GraphMultiset(std::move(gPtrs));
	};
	auto gmsLeft = makeSide(d.left);
	auto gmsRight = makeSide(d.right);
	dg->rules.insert(d.rules.begin(), d.rules.end());
	if(d.rules.size() <= 1) {
		const lib::Rules::Real *rule = nullptr;
		if(!d.rules.empty()) rule = &d.rules.front()->getRule();
		return dg->suggestDerivation(std::move(gmsLeft), std::move(gmsRight), rule);
	} else {
		auto res = dg->suggestDerivation(gmsLeft, gmsRight, &d.rules.front()->getRule());
		for(const auto &r: asRange(d.rules.begin() + 1, d.rules.end()))
			dg->suggestDerivation(gmsLeft, gmsRight, &r->getRule());
		return res;
	}
}

struct NonHyperBuilder::ExecutionEnv final : public Strategies::ExecutionEnv {
	ExecutionEnv(NonHyperBuilder &owner, LabelSettings labelSettings, bool doRuleIsomorphism,
	             Rules::GraphAsRuleCache &graphAsRuleCache)
			: Strategies::ExecutionEnv(labelSettings, doRuleIsomorphism, graphAsRuleCache), owner(owner) {}

	void tryAddGraph(std::shared_ptr<graph::Graph> gCand) override {
		owner.tryAddGraph(gCand);
	}

	bool trustAddGraph(std::shared_ptr<graph::Graph> g) override {
		return owner.trustAddGraph(g);
	}

	bool trustAddGraphAsVertex(std::shared_ptr<graph::Graph> g) override {
		return owner.trustAddGraphAsVertex(g);
	}

	bool doExit() const override {
		return doExit_;
	}

	bool checkLeftPredicate(const mod::Derivation &d) const override {
		for(const auto &pred: asRange(leftPredicates.rbegin(), leftPredicates.rend())) {
			bool result = (*pred)(d);
			if(!result) return false;
		}
		return true;
	}

	bool checkRightPredicate(const mod::Derivation &d) const override {
		for(const auto &pred: asRange(rightPredicates.rbegin(), rightPredicates.rend())) {
			bool result = (*pred)(d);
			if(!result) return false;
		}
		return true;
	}

	virtual std::shared_ptr<graph::Graph> checkIfNew(std::unique_ptr<lib::Graph::Single> g) const override {
		return owner.checkIfNew(std::move(g)).first;
	}

	bool addProduct(std::shared_ptr<graph::Graph> g) override {
		return owner.addProduct(g);
	}

	bool isDerivation(const GraphMultiset &gmsSrc,
	                  const GraphMultiset &gmsTar,
	                  const lib::Rules::Real *r) const override {
		return owner.isDerivation(gmsSrc, gmsTar, r).second;
	}

	bool suggestDerivation(const GraphMultiset &gmsSrc,
	                       const GraphMultiset &gmsTar,
	                       const lib::Rules::Real *r) override {
		return owner.suggestDerivation(gmsSrc, gmsTar, r).second;
	}

	void pushLeftPredicate(std::shared_ptr<mod::Function<bool(const mod::Derivation &)> > pred) override {
		leftPredicates.push_back(pred);
	}

	void pushRightPredicate(std::shared_ptr<mod::Function<bool(const mod::Derivation &)> > pred) override {
		rightPredicates.push_back(pred);
	}

	void popLeftPredicate() override {
		leftPredicates.pop_back();
	}

	void popRightPredicate() override {
		rightPredicates.pop_back();
	}

public:
	NonHyperBuilder &owner;
private: // state for computation
	std::vector<std::shared_ptr<mod::Function<bool(const mod::Derivation &)> > > leftPredicates;
	std::vector<std::shared_ptr<mod::Function<bool(const mod::Derivation &)> > > rightPredicates;
	bool doExit_ = false;
};

ExecuteResult
Builder::execute(std::unique_ptr<Strategies::Strategy> strategy_, int verbosity, bool ignoreRuleLabelTypes) {
	const bool doRuleIsomorphism = getConfig().dg.doRuleIsomorphismDuringBinding.get();
	NonHyperBuilder::StrategyExecution exec{
			std::make_unique<NonHyperBuilder::ExecutionEnv>(*dg, dg->getLabelSettings(), doRuleIsomorphism,
			                                                dg->graphAsRuleCache),
			std::make_unique<Strategies::GraphState>(),
			std::move(strategy_)
	};
	exec.strategy->setExecutionEnv(*exec.env);

	exec.strategy->preAddGraphs(
			[this](std::shared_ptr<graph::Graph> gCand, IsomorphismPolicy graphPolicy) {
				if(dg->getLabelSettings().type == LabelType::Term) {
					const auto &term = get_term(gCand->getGraph().getLabelledGraph());
					if(!isValid(term)) {
						std::string msg =
								"Parsing failed for graph '" + gCand->getName() + "' in static add strategy. " +
								term.getParsingError();
						throw TermParsingError(std::move(msg));
					}
				}
				switch(graphPolicy) {
				case IsomorphismPolicy::Check:
					dg->tryAddGraph(gCand);
					break;
				case IsomorphismPolicy::TrustMe:
					dg->trustAddGraph(gCand);
					break;
				}
			}
	);
	if(!ignoreRuleLabelTypes) {
		exec.strategy->forEachRule([&](const lib::Rules::Real &r) {
			if(!r.getLabelType()) return;
			if(*r.getLabelType() != dg->getLabelSettings().type) {
				std::string msg = "Rule '" + r.getName() + "' has intended label type '" +
				                  boost::lexical_cast<std::string>(*r.getLabelType()) + "', but the DG is using '" +
				                  boost::lexical_cast<std::string>(dg->getLabelSettings().type) + "'.";
				throw LogicError(std::move(msg));
			}
		});
	}

	exec.strategy->execute(Strategies::PrintSettings(std::cout, false, verbosity), *exec.input);
	dg->executions.push_back(std::move(exec));
	return ExecuteResult(dg, dg->executions.size() - 1);
}

std::vector<std::pair<NonHyper::Edge, bool>>
Builder::apply(const std::vector<std::shared_ptr<graph::Graph>> &graphs,
               std::shared_ptr<rule::Rule> rOrig,
               int verbosity, IsomorphismPolicy graphPolicy) {
	const bool doRuleIsomorphism = getConfig().dg.doRuleIsomorphismDuringBinding.get();
	IO::Logger logger(std::cout);
	dg->rules.insert(rOrig);
	switch(graphPolicy) {
	case IsomorphismPolicy::Check:
		for(const auto &g: graphs)
			dg->tryAddGraph(g);
		break;
	case IsomorphismPolicy::TrustMe:
		for(const auto &g: graphs)
			dg->trustAddGraph(g);
		break;
	}

	if(verbosity >= V_RuleApplication) {
		logger.indent() << "Binding " << graphs.size() << " graphs to rule '" << rOrig->getName() << "' with "
		                << rOrig->getNumLeftComponents() << " left-hand components." << std::endl;
		++logger.indentLevel;
	}

	if(graphs.empty()) return {};
	std::vector<const lib::Graph::Single *> libGraphs;
	libGraphs.reserve(graphs.size());
	for(const auto &g: graphs)
		libGraphs.push_back(&g->getGraph());

	std::vector<BoundRule> resultRules;
	const auto ls = dg->getLabelSettings();
	{
		// we must bind each graph, so increase the span of graphs one at a time,
		// and only keep bound rules that still have left-hand components
		std::vector<BoundRule> inputRules{{&rOrig->getRule(), {}, 0}};
		const auto firstGraph = libGraphs.begin();
		for(int round = 0; round != libGraphs.size(); ++round) {
			const auto onOutput = [
					isLast = round + 1 == libGraphs.size(),
					assumeConfluence = getConfig().dg.applyAssumeConfluence.get(),
					&resultRules]
					(IO::Logger logger, BoundRule br) -> bool {
				if(isLast) {
					// save only the fully bound ones
					if(br.rule->isOnlyRightSide()) {
						resultRules.push_back(std::move(br));
						return !assumeConfluence; // returns "make more matches"
					} else return true;
				} else {
					// discard the fully bound ones
					if(br.rule->isOnlyRightSide()) {
						delete br.rule;
						return true;
					} else return !assumeConfluence; // returns "make more matches"
				}
			};
			std::vector<BoundRule> outputRules = bindGraphs(
					verbosity, logger,
					round,
					firstGraph, firstGraph + round + 1, inputRules,
					dg->graphAsRuleCache, ls, doRuleIsomorphism,
					onOutput);
			for(BoundRule &br: outputRules) {
				// always go to the next graph
				++br.nextGraphOffset;
			}
			if(round != 0) {
				// in round 0 the inputRules is the actual original input rule, so don't delete it
				for(auto &br: inputRules)
					delete br.rule;
			}
			std::swap(inputRules, outputRules);
			if(verbosity >= V_RuleApplication)
				logger.indent(1) << "Result after apply filtering: " << inputRules.size() << " rules" << std::endl;
		} // for each round
		// after the last round we may still have rules with connected components in L
		// which go unused, so delete them
		for(auto &br: inputRules)
			delete br.rule;
	} // end of binding

	std::vector<std::pair<NonHyper::Edge, bool>> res;
	for(const BoundRule &br: resultRules) {
		if(getConfig().dg.applyLimit.get() == res.size()) break;

		const auto &r = *br.rule;
		assert(r.isOnlyRightSide());
		auto products = splitRule(
				r.getDPORule(), ls.type, ls.withStereo,
				[this](std::unique_ptr<lib::Graph::Single> gCand) {
					return dg->checkIfNew(std::move(gCand)).first;
				},
				[verbosity, &logger](std::shared_ptr<graph::Graph> gWrapped, std::shared_ptr<graph::Graph> gPrev) {
					if(verbosity >= V_RuleApplication_Binding)
						logger.indent(1) << "Discarding product " << gWrapped->getName()
						                 << ", isomorphic to other product " << gPrev->getName()
						                 << "." << std::endl;
				}
		);
		if(products.empty()) {
			if(verbosity >= V_RuleApplication)
				logger.indent(1) << "Discarding derivation, empty result." << std::endl;
			continue;
		}
		for(const auto &p: products)
			dg->addProduct(p);
		std::vector<const lib::Graph::Single *> rightGraphs;
		rightGraphs.reserve(products.size());
		for(const auto &p: products)
			rightGraphs.push_back(&p->getGraph());
		lib::DG::GraphMultiset gmsLeft(br.boundGraphs), gmsRight(std::move(rightGraphs));
		const auto derivationRes = dg->suggestDerivation(gmsLeft, gmsRight, &rOrig->getRule());
		res.push_back(derivationRes);
	}

	for(const auto &br: resultRules)
		delete br.rule;

	return res;
}

std::vector<std::pair<NonHyper::Edge, bool>>
Builder::applyRelaxed(const std::vector<std::shared_ptr<graph::Graph>> &graphs,
                      std::shared_ptr<rule::Rule> rOrig,
                      int verbosity, IsomorphismPolicy graphPolicy) {
	const bool doRuleIsomorphism = getConfig().dg.doRuleIsomorphismDuringBinding.get();
	IO::Logger logger(std::cout);
	dg->rules.insert(rOrig);
	switch(graphPolicy) {
	case IsomorphismPolicy::Check:
		for(const auto &g: graphs)
			dg->tryAddGraph(g);
		break;
	case IsomorphismPolicy::TrustMe:
		for(const auto &g: graphs)
			dg->trustAddGraph(g);
		break;
	}

	if(verbosity >= V_RuleApplication) {
		logger.indent() << "Binding " << graphs.size() << " graphs to rule '" << rOrig->getName() << "' with "
		                << rOrig->getNumLeftComponents() << " left-hand components." << std::endl;
		++logger.indentLevel;
	}

	if(graphs.empty()) return {};
	std::vector<const lib::Graph::Single *> libGraphs;
	libGraphs.reserve(graphs.size());
	for(const auto &g: graphs)
		libGraphs.push_back(&g->getGraph());

	const auto ls = dg->getLabelSettings();
	// we must bind each graph, so increase the span of graphs one at a time,
	// and only keep bound rules that still have left-hand components
	std::vector<BoundRule> inputRules{{&rOrig->getRule(), {}, 0}};
	std::vector<std::pair<NonHyper::Edge, bool>> res;
	for(int round = 0; round != rOrig->getNumLeftComponents(); ++round) {
		const auto firstGraph = libGraphs.begin();
		const auto lastGraph = libGraphs.end();

		const auto onOutput = [this, verbosity, ls, &res, rOrig]
				(IO::Logger logger, BoundRule br) -> bool {
			if(!br.rule->isOnlyRightSide())
				return true;

			const auto &r = *br.rule;
			if(verbosity >= V_RuleApplication_Binding) {
				logger.indent() << "Splitting " << r.getName() << " into "
				                << get_num_connected_components(get_labelled_right(r.getDPORule()))
				                << " graphs" << std::endl;
				++logger.indentLevel;
			}

			assert(r.isOnlyRightSide());
			auto products = splitRule(
					r.getDPORule(), ls.type, ls.withStereo,
					[this](std::unique_ptr<lib::Graph::Single> gCand) {
						return dg->checkIfNew(std::move(gCand)).first;
					},
					[verbosity, &logger](std::shared_ptr<graph::Graph> gWrapped, std::shared_ptr<graph::Graph> gPrev) {
						if(verbosity >= V_RuleApplication_Binding)
							logger.indent(1) << "Discarding product " << gWrapped->getName()
							                 << ", isomorphic to other product " << gPrev->getName()
							                 << "." << std::endl;
					}
			);
			if(products.empty()) {
				if(verbosity >= V_RuleApplication)
					logger.indent(1) << "Discarding derivation, empty result." << std::endl;
				delete br.rule;
				return true;
			}
			for(const auto &p: products)
				dg->addProduct(p);
			std::vector<const lib::Graph::Single *> rightGraphs;
			rightGraphs.reserve(products.size());
			for(const auto &p: products)
				rightGraphs.push_back(&p->getGraph());
			lib::DG::GraphMultiset gmsLeft(br.boundGraphs), gmsRight(std::move(rightGraphs));
			const auto derivationRes = dg->suggestDerivation(gmsLeft, gmsRight, &rOrig->getRule());
			res.push_back(derivationRes);

			if(verbosity >= V_RuleApplication_Binding)
				--logger.indentLevel;

			delete br.rule;
			return true;
		};
		std::vector<BoundRule> outputRules = bindGraphs
				(verbosity, logger,
				 round,
				 firstGraph, lastGraph, inputRules,
				 dg->graphAsRuleCache, ls, doRuleIsomorphism,
				 onOutput);
		for(BoundRule &br: outputRules) {
			// always go to the next graph
			++br.nextGraphOffset;
		}
		if(round != 0) {
			// in round 0 the inputRules is the actual original input rule, so don't delete it
			for(auto &br: inputRules)
				delete br.rule;
		}
		std::swap(inputRules, outputRules);
	} // for each round based on numComponents
	// the last round should not produce any results with non-empty L,
	// as we do exactly |CC(L)| number of rounds.
	if(rOrig->getNumLeftComponents() != 0)
		assert(inputRules.empty());
	return res;
}

void Builder::addAbstract(const std::string &description) {
	std::ostringstream err;
	const auto res = lib::DG::Read::abstract(description, err);
	if(!res) throw InputError("Could not parse description of abstract derivations.\n" + err.str());
	const auto &derivations = *res;
	std::unordered_map<std::string, std::shared_ptr<graph::Graph>> graphFromStr;
	for(const auto &gOuter: dg->getGraphDatabase().asList()) {
		const auto &gInner = gOuter->getGraph().getGraph();
		if(num_vertices(gInner) != 1) continue;
		const auto v = *vertices(gInner).first;
		const auto &s = get_string(gOuter->getGraph().getLabelledGraph())[v];
		assert(graphFromStr.find(s) == graphFromStr.end());
		graphFromStr[s] = gOuter;
	}

	const auto handleSide = [this, &graphFromStr](const lib::DG::Read::AbstractDerivation::List &side) {
		for(const auto &[coef, name]: side) {
			const auto iter = graphFromStr.find(name);
			if(iter != graphFromStr.end()) continue;
			auto gBoost = std::make_unique<lib::Graph::GraphType>();
			auto pString = std::make_unique<lib::Graph::PropString>(*gBoost);
			const auto v = add_vertex(*gBoost);
			pString->addVertex(v, name);
			auto gLib = std::make_unique<lib::Graph::Single>(std::move(gBoost), std::move(pString), nullptr);
			auto g = graph::Graph::create(std::move(gLib));
			dg->addProduct(g); // this renames it
			g->setName(name);
			graphFromStr[name] = g;
		}
	};
	for(const auto &der: derivations) {
		handleSide(der.left);
		handleSide(der.right);
	}

	using Side = std::unordered_map<std::shared_ptr<graph::Graph>, unsigned int>;
	const auto makeSide = [&graphFromStr](const lib::DG::Read::AbstractDerivation::List &side) {
		Side result;
		for(const auto &e: side) {
			const auto g = graphFromStr[e.second];
			assert(g);
			auto iter = result.find(g);
			if(iter == end(result)) iter = result.insert(std::make_pair(g, 0)).first;
			iter->second += e.first;
		}
		return result;
	};
	for(const auto &der: derivations) {
		const Side left = makeSide(der.left);
		const Side right = makeSide(der.right);
		std::vector<const lib::Graph::Single *> leftGraphs, rightGraphs;
		for(const auto &e: left) {
			for(unsigned int i = 0; i < e.second; i++)
				leftGraphs.push_back(&e.first->getGraph());
		}
		for(const auto &e: right) {
			for(unsigned int i = 0; i < e.second; i++)
				rightGraphs.push_back(&e.first->getGraph());
		}
		lib::DG::GraphMultiset gmsLeft(std::move(leftGraphs)), gmsRight(std::move(rightGraphs));
		dg->suggestDerivation(gmsLeft, gmsRight, nullptr);
		if(der.reversible)
			dg->suggestDerivation(gmsRight, gmsLeft, nullptr);
	}
}

bool Builder::load(const std::vector<std::shared_ptr<rule::Rule>> &ruleDatabase,
                   const std::string &file, std::ostream &err, int verbosity) {
	boost::iostreams::mapped_file_source ifs;
	try {
		ifs.open(file);
	} catch(const BOOST_IOSTREAMS_FAILURE &e) {
		err << "Could not open file '" << file << "':\n" << e.what();
		return {};
	}
	if(ifs.size() > 8 && std::string(ifs.begin(), ifs.begin() + 8) == "version:") {
		ifs.close();
		err << "Dump version too old to load into an existing DG."
		    << " Load it as a locked DG (dg::DG::load()/DG.load()) and dump it again to convert it to a newer format.";
		return false;
	}
	ifs.close();
	auto jOpt = lib::DG::Read::loadDump(file, err);
	if(!jOpt) return {};
	auto &j = *jOpt;

	LabelSettings labelSettings = from_json(j["labelSettings"]);
	if(labelSettings != dg->getLabelSettings()) {
		err << "Mismatch of label settings. This DG has "
		    << dg->getLabelSettings()
		    << " but the dump to be loaded has "
		    << labelSettings << ".";
		return false;
	}
	auto res = trustLoadDump(std::move(j), ruleDatabase, err, verbosity);
	return res;
}

bool Builder::trustLoadDump(nlohmann::json &&j,
                            const std::vector<std::shared_ptr<rule::Rule>> &ruleDatabase,
                            std::ostream &err,
                            int verbosity) {
	constexpr int V_Link = 2;
	constexpr bool printStereoWarnings = true;

	assert(j["version"].get<int>() == 3);

	// first do graph wrapping against the underlying graph database
	// we assume that the incomming graphs are all pairwise non-isomorphic
	struct Vertex {
		int id;
		std::shared_ptr<graph::Graph> graph;
		bool wasNew;
	};
	auto &jVertices = j["vertices"];
	std::vector<Vertex> vertices;
	vertices.reserve(jVertices.size());
	for(auto &jv: jVertices) {
		Vertex v;
		v.id = jv[0].get<int>();
		const std::string &gml = jv[2].get<std::string>();
		lib::IO::Warnings warnings;
		auto gDatasRes = lib::Graph::Read::gml(warnings, gml, printStereoWarnings);
		err << warnings;
		if(!gDatasRes) {
			err << gDatasRes.extractError() << '\n';
			err << "Error when loading graph GML in DG dump, for graph '";
			err << jv[1].get<std::string>() << "', in vertex " << v.id << ".";
			return false;
		}
		auto gDatas = std::move(*gDatasRes);
		if(gDatas.size() != 1) {
			err << "Loaded graph has multiple connected components (" << gDatas.size() << "). ";
			err << "Error when loading graph GML in DG dump, for graph '";
			err << jv[1].get<std::string>() << "', in vertex " << v.id << ".";
			return false;
		}
		auto gCand = std::make_unique<lib::Graph::Single>(
				std::move(gDatas.front().g), std::move(gDatas.front().pString), std::move(gDatas.front().pStereo));
		gCand->setName(jv[1].get<std::string>());
		auto p = dg->checkIfNew(std::move(gCand));
		v.graph = p.first;
		v.wasNew = p.second == nullptr;
		vertices.push_back(std::move(v));
	}
	// now the vertices are ready to be added

	// prepare the rules, we assume those in the dump are unique
	const auto &jRules = j["rules"];
	std::vector<std::shared_ptr<rule::Rule>> rules;
	rules.reserve(jRules.size());
	const auto ls = dg->getLabelSettings();
	for(const auto &j: jRules) {
		const std::string &jr = j.get<std::string>();
		auto rCand = rule::Rule::fromGMLString(jr, false);
		const auto iter = std::find_if(ruleDatabase.begin(), ruleDatabase.end(), [rCand, ls](const auto &r) {
			return r->isomorphism(rCand, 1, ls) == 1;
		});
		if(iter == end(ruleDatabase)) {
			dg->rules.insert(rCand);
			rules.push_back(rCand);
		} else {
			rules.push_back(*iter);
			if(verbosity >= V_Link) {
				std::cout << "DG loading: loaded rule '" << rCand->getName()
				          << "' isomorphic to existing rule '" << (*iter)->getName() << "'." << std::endl;
			}
		}
	}

	// do merge of vertices and edges in order of increasing id
	std::unordered_map<int, const lib::Graph::Single *> graphFromId;
	const auto &jEdges = j["edges"];
	const int numVertices = vertices.size();
	const int numEdges = jEdges.size();
	int iVertices = 0;
	int iEdges = 0;
	for(int id = 0; id != numVertices + numEdges; ++id) {
		if(iVertices < numVertices && vertices[iVertices].id == id) {
			const auto &v = vertices[iVertices];
			const bool wasNewAsVertex = dg->trustAddGraphAsVertex(v.graph);
			graphFromId[v.id] = &v.graph->getGraph();
			if(verbosity >= V_Link && !v.wasNew) {
				std::cout << "DG loading: loaded graph '" << jVertices[iVertices][1].get<std::string>()
				          << "' isomorphic to existing graph '" << v.graph->getName() << "'." << std::endl;
			}
			//if(wasNewAsVertex) giveProductStatus(v.graph);
			(void) wasNewAsVertex;
			++iVertices;
		} else if(iEdges < numEdges && jEdges[iEdges][0].get<int>() == id) {
			const auto &e = jEdges[iEdges];
			std::vector<const lib::Graph::Single *> srcGraphs, tarGraphs;
			srcGraphs.reserve(e[1].size());
			tarGraphs.reserve(e[2].size());
			for(int src: e[1]) {
				auto gIter = graphFromId.find(src);
				if(gIter == end(graphFromId)) {
					err << "Corrupt data for edge " << e[0].get<int>() << ". Source " << src
					    << " is not a yet a vertex.";
					return false;
				}
				srcGraphs.push_back(gIter->second);
			}
			for(int tar: e[2]) {
				auto gIter = graphFromId.find(tar);
				if(gIter == end(graphFromId)) {
					err << "Corrupt data for edge " << e[0].get<int>() << ". Target " << tar
					    << " is not a yet a vertex.";
					return false;
				}
				tarGraphs.push_back(gIter->second);
			}
			GraphMultiset gmsSrc(std::move(srcGraphs)), gmsTar(std::move(tarGraphs));
			const auto &ruleIds = e[3];
			if(ruleIds.empty()) {
				dg->suggestDerivation(std::move(gmsSrc), std::move(gmsTar), nullptr);
			} else {
				for(const int rId: ruleIds) {
					if(rId < 0 || rId >= rules.size()) {
						err << "Corrupt data for edge " << e[0].get<int>() << ". Rule offset " << rId
						    << " is not in range.";
						return false;
					}
					const auto r = rules[rId];
					dg->suggestDerivation(std::move(gmsSrc), std::move(gmsTar), &r->getRule());
				}
			}
			++iEdges;
		} else {
			err << "Corrupt data for derivation graph during addition (";
			err << "ID: " << id;
			err << ", vertices: " << iVertices << " of " << numVertices;
			if(iVertices < numVertices) err << ", next vertex: " << vertices[iVertices].id;
			err << ", edges: " << iEdges << " of " << numEdges;
			if(iEdges < numEdges) err << ", next edge: " << jEdges[iEdges][0].get<int>();
			err << ").";
			return false;
		}
	}
	return true;
}

// -----------------------------------------------------------------------------

NonHyperBuilder::NonHyperBuilder(LabelSettings
                                 labelSettings,
                                 const std::vector<std::shared_ptr<graph::Graph> > &graphDatabase,
                                 IsomorphismPolicy graphPolicy)
		: NonHyper(labelSettings, graphDatabase, graphPolicy) {}

NonHyperBuilder::~NonHyperBuilder() =
default;

std::string NonHyperBuilder::getType() const {
	return "DG";
}

Builder NonHyperBuilder::build(std::shared_ptr<Function<void(dg::DG::Vertex)>> onNewVertex,
                               std::shared_ptr<Function<void(dg::DG::HyperEdge)>> onNewHyperEdge) {
	if(getHasCalculated()) throw LogicError(getType() + ": has already been build.");
	return Builder(this, onNewVertex, onNewHyperEdge);
}

} // namespace mod::lib::DG