#include "Solver.hpp"

#include <mod/lib/CombiOpt/ILPImplementation.hpp>
#include <mod/lib/CombiOpt/LoadedSolution.hpp>

#include <algorithm>
#include <chrono>
#include <list>
#include <iomanip>
#include <queue>

namespace mod::lib::CombiOpt {
namespace {
const std::string logPrefix = "Solver: ";

std::string varTypeToString(VarType t) {
	switch(t) {
	case VarType::Int:
		return "Int";
	case VarType::Bool:
		return "Bool";
	case VarType::Float:
		return "Float";
	}
	return "<unknown>";
}

enum class State {
	Created,
	/**/ Infeasible,
	/**/ SolvedNonOptimal, SolvedOptimal, SolvedImplicitly
};

struct Bound {
	Int lower;
	Int upper;
};

struct TreeNode {
	// Represents the attempt to fix the variable enumerationVars[varOffset].
	// The current bounds for it is bound.
	TreeNode(int id, TreeNode *parent, int varOffset, Bound bound)
			: id(id), parent(parent), varOffset(varOffset), bound(bound) {
		assert(bound.lower <= bound.upper);
	}

	Int getVarVal(IntegralVar v) {
		assert(state == State::SolvedNonOptimal || state == State::SolvedOptimal || state == State::SolvedImplicitly);
		assert(ancestorWithSolution);
		const auto iter = ancestorWithSolution->solutionIntegral.find(v.getData());
		if(iter == end(ancestorWithSolution->solutionIntegral)) return 0;
		else return iter->second;
	}

	Float getVarVal(FloatVar v) {
		assert(state == State::SolvedNonOptimal || state == State::SolvedOptimal || state == State::SolvedImplicitly);
		assert(ancestorWithSolution);
		const auto iter = ancestorWithSolution->solutionNonIntegral.find(v.getData());
		if(iter == end(ancestorWithSolution->solutionNonIntegral)) return 0.0;
		else return iter->second;
	}
public:
	const int id;
	TreeNode *const parent;
	const int varOffset;
	const Bound bound;
public:
	State state = State::Created;
	// if state == Solved*
	Value objVal;
	// CombiOpt::Var::id -> val, the union of both maps contain all non-zero values
	std::unordered_map<std::size_t, Int> solutionIntegral;
	std::unordered_map<std::size_t, Float> solutionNonIntegral;
	TreeNode *ancestorWithSolution = nullptr; // it is self if state != SolvedImplicitly
	// endif

	std::unique_ptr<TreeNode> lt, eq, gt;

	std::list<TreeNode *>::iterator nodesToExplorePos;
};

struct TreeNodeLess {
	bool operator()(const TreeNode *a, const TreeNode *b) const {
		assert(a->state == State::SolvedNonOptimal);
		assert(b->state == State::SolvedNonOptimal);
		assert(a->state == b->state);
		return a->objVal > b->objVal;
	}
};

int makeILPVerbosity(int verbosity, int ilpVerbosity, bool isRoot) {
	if(verbosity == 0 || ilpVerbosity == 0) return 0;
	if(isRoot) return ilpVerbosity;
	return ilpVerbosity <= 1 ? 0 : ilpVerbosity;
}

struct SharedData {
	const Model model;
	std::vector<IntegralVar> enumerationVars;
	std::vector<Bound> origBounds;
public: // non-integral
	std::optional<Var> nonIntegralEnumVar;
public:
	SharedData(Model model_, const std::unordered_set<Var> &enumerationVarsSet, int verbosity, std::ostream &log)
			: model(std::move(model_)) {
		enumerationVars.reserve(enumerationVars.size());
		for(Var v: enumerationVarsSet) {
			switch(v.getType()) {
			case VarType::Bool:
			case VarType::Int:
				enumerationVars.emplace_back(v.getType(), v.getData());
				break;
			default:
				nonIntegralEnumVar = v;
				break;
			}
		}

		// tight variables should be first in the tree
		const auto iter = std::partition(begin(enumerationVars), end(enumerationVars),
										 [this](const auto var) {
											 return model.getLB(var) == model.getUB(var);
										 });
		if(verbosity >= 1) {
			const auto nonFixed = std::distance(iter, end(enumerationVars));
			log << logPrefix << enumerationVars.size() << " enumeration variables, "
				<< "thereof " << nonFixed << " non-fixed variables\n";
		}

		origBounds.resize(enumerationVars.size());
		for(int i = 0; i != enumerationVars.size(); ++i) {
			const auto var = enumerationVars[i];
			origBounds[i] = Bound{model.getLB(var), model.getUB(var)};
		}
	}

	std::ostream &printNode(std::ostream &s, const TreeNode *node) const {
		s << "nodeId = " << node->id;
		s << ", var " << node->varOffset;
		s << " in [" << node->bound.lower << ", " << node->bound.upper << "] (";
		s << model.getVarName(enumerationVars[node->varOffset]) << ")";
		return s;
	}
};

struct WorkerData {
	const SharedData *data;
	ILPImplementation ilp;
	const TreeNode *nodeForLastBranchingConstraint;
public:
	WorkerData(const SharedData *data) : data(data) { assert(data); }

	// This is separate from the ctor as the master data does not need an ILP model
	// when just loading solutions.
	void initializeILP(const std::string &solver) {
		if(ilp.model) return;
		ilp = makeILPModel(data->model, 100, solver);
	}

	void createBranchingConstraints(const TreeNode *node, int verbosity, int ilpVerbosity, std::ostream &log) {
		assert(ilp.model);
		assert(node->state == State::Created);
		if(verbosity >= 2) {
			data->printNode(log << logPrefix << "    Creating branching constraints: ", node) << std::endl;
			data->printNode(log << logPrefix << "      lastBranchingConstraintNode: ", nodeForLastBranchingConstraint)
					<< std::endl;
		}
		// reset bounds if we were further down before
		for(; nodeForLastBranchingConstraint->varOffset > node->varOffset;
			  nodeForLastBranchingConstraint = nodeForLastBranchingConstraint->parent) {
			const TreeNode *last = nodeForLastBranchingConstraint;
			const Var var = data->enumerationVars[last->varOffset];
			const Bound bound = data->origBounds[last->varOffset];
			if(verbosity >= 2)
				log << logPrefix << "      reset var " << last->varOffset
					<< " in [" << bound.lower << ", " << bound.upper << "]"
					<< " (" << data->model.getVarName(var) << ")" << std::endl;
			const ILP::Var ilpVar = ilp.getVar(data->enumerationVars[last->varOffset]);
			ilp.model->setLB(ilpVar, bound.lower.getValue());
			ilp.model->setUB(ilpVar, bound.upper.getValue());
		}
		// fix previous variables
		for(int i = 0; i != node->varOffset; ++i) {
			const IntegralVar var = data->enumerationVars[i];
			const Int val = node->parent->getVarVal(var);
			const ILP::Var ilpVar = ilp.getVar(var);
			if(verbosity >= 2)
				if(val != 0)
					log << logPrefix << "      var " << i << " == " << val << " (" << data->model.getVarName(var) << ")"
						<< std::endl;
			ilp.model->setLB(ilpVar, val.getValue());
			ilp.model->setUB(ilpVar, val.getValue());
		}
		// set bounds for our variable
		assert(node->bound.lower <= node->bound.upper);
		if(verbosity >= 2)
			log << logPrefix << "      var " << node->varOffset
				<< " in [" << node->bound.lower << ", " << node->bound.upper << "]"
				<< " (" << data->model.getVarName(data->enumerationVars[node->varOffset]) << ")" << std::endl;
		const ILP::Var ilpVar = ilp.getVar(data->enumerationVars[node->varOffset]);
		ilp.model->setLB(ilpVar, node->bound.lower.getValue());
		ilp.model->setUB(ilpVar, node->bound.upper.getValue());
		nodeForLastBranchingConstraint = node;
	}

	bool solveNode(TreeNode *n, int verbosity, int ilpVerbosity, int callNumber, std::ostream &log) {
		assert(ilp.model);
		if(verbosity >= 2)
			data->printNode(log << logPrefix << "    solveNode(): ", n) << std::endl;
		const int effectiveIlpVerbosity = makeILPVerbosity(verbosity, ilpVerbosity, !n->parent);
		if(effectiveIlpVerbosity > 0)
			log << logPrefix << "      Calling ILP solver (call #" << callNumber << ")" << std::endl;
		ILP::Response res = ilp.model->realSolve(log, effectiveIlpVerbosity);
		if(effectiveIlpVerbosity > 0)
			log << logPrefix << "      ILP solver done (call #" << callNumber << ")" << std::endl;
		if(res == ILP::Response::Failed || res == ILP::Response::TimedOut) {
			if(verbosity >= 2)
				log << logPrefix << "      Solving failed. Response = " << res << "\n";
			n->state = State::Infeasible;
			return false;
		} else {
			n->state = State::SolvedNonOptimal;
			if(ilp.model->getIsObjectiveFunctionIntegral())
				n->objVal = ilp.model->getObjValIntegral();
			else
				n->objVal = ilp.model->getObjValReal();
			n->ancestorWithSolution = n;
			if(verbosity >= 2)
				log << logPrefix << "     Response = " << res << ", objVal = " << n->objVal << std::endl;

			// record solution
			for(const auto ilpImplVar: ilp.vars) {
				const auto varId = ilpImplVar.first;
				const auto ilpVar = ilpImplVar.second;
				if(ilp.model->isVariableReal(ilpVar)) {
					const auto val = ilp.model->getVarValReal(ilpVar);
					if(val != 0.0)
						n->solutionNonIntegral.emplace(varId, val);
				} else {
					const auto val = ilp.model->getVarValIntegral(ilpVar);
					if(val != 0)
						n->solutionIntegral.emplace(varId, val);
				}
			}
			return true;
		}
	}
};

} // namespace

struct Result::Pimpl {
	Pimpl(Solver::Pimpl *solver) : solver(solver), node(nullptr) {}

	Pimpl(Solver::Pimpl *solver, TreeNode *node) : solver(solver), node(node) {
		assert(node->state == State::SolvedOptimal);
	}

	void lock() { this->locked = true; }

	bool isEnumResult() const {
		return node != nullptr;
	}
public:
	bool locked = false;
	Solver::Pimpl *solver;
	TreeNode *node;
};

struct Solver::Pimpl {
	Pimpl(Model model_, std::unordered_set<Var> enumerationVarsSet, const std::string &solver,
		  int verbosity, std::ostream &log)
			: solver(solver), log(log),
			  data(std::move(model_), std::move(enumerationVarsSet), verbosity, log),
			  masterData(&data) {

		nonIntegralObjFunction = !data.model.getObjectiveFunction().isIntegral();
		statTimeLast = std::chrono::system_clock::now();
	}

	Result nextNonIntegral(int verbosity, int ilpVerbosity) {
		masterData.initializeILP(solver);
		if(nonIntegralState != State::Created) {
			if(data.nonIntegralEnumVar)
				throw LogicError("Can not enumerate solutions over non-integral variable '"
								 + data.model.getVarName(*data.nonIntegralEnumVar) + "' of type "
								 + varTypeToString(data.nonIntegralEnumVar->getType()) + ".");
			MOD_ABORT;
		}
		if(verbosity >= 2) {
			log << logPrefix << "Solving model.\n";
		}
		ILP::Response res = masterData.ilp.model->realSolve(log, makeILPVerbosity(verbosity, ilpVerbosity, true));
		if(res == ILP::Response::Failed || res == ILP::Response::TimedOut) {
			if(verbosity >= 2)
				log << logPrefix << "Solving failed. Response = " << res << "\n";
			nonIntegralState = State::Infeasible;
			return Result();
		} else {
			nonIntegralState = State::SolvedNonOptimal;
			if(masterData.ilp.model->getIsObjectiveFunctionIntegral())
				nonIntegralObjVal = masterData.ilp.model->getObjValIntegral();
			else
				nonIntegralObjVal = masterData.ilp.model->getObjValReal();
			if(verbosity >= 2)
				log << logPrefix << "Response = " << res << "\tobjVal = " << nonIntegralObjVal << "\n";
			// record solution
			for(const auto ilpImplVar: masterData.ilp.vars) {
				const auto varId = ilpImplVar.first;
				const auto ilpVar = ilpImplVar.second;
				if(masterData.ilp.model->isVariableReal(ilpVar)) {
					const auto val = masterData.ilp.model->getVarValReal(ilpVar);
					if(val != 0.0)
						nonIntegralSolutionNonIntegral.emplace(varId, val);
				} else {
					const auto val = masterData.ilp.model->getVarValIntegral(ilpVar);
					if(val != 0)
						nonIntegralSolutionIntegral.emplace(varId, val);
				}
			}
			return Result(std::make_unique<Result::Pimpl>(this));
		}
	}

	Result next(int verbosity, int ilpVerbosity) {
		if(verbosity >= 2) log << logPrefix << "next()" << std::endl;
		masterData.initializeILP(solver);
		if(currentResult) {
			currentResult->lock();
			currentResult = nullptr;
		}
		if(data.nonIntegralEnumVar)
			return nextNonIntegral(verbosity, ilpVerbosity);

		if(!root) {
			if(verbosity >= 2) log << logPrefix << "  Creating root" << std::endl;
			const int varOffset = 0;
			root.reset(new TreeNode(nextNodeId++, nullptr, varOffset, data.origBounds[varOffset]));
			masterData.nodeForLastBranchingConstraint = root.get();
			const bool feasible = solveNode(root.get(), verbosity, ilpVerbosity);
			if(!feasible) {
				if(verbosity >= 2) log << logPrefix << "  root infeasible" << std::endl;
				return Result();
			}
			solvedNodes.push(root.get());
		}

		auto res = serialEnumeration(verbosity, ilpVerbosity);
		if(res) return res;

		if(verbosity >= 1) log << "Ending search (no more solutions)" << std::endl;
		printStatus(true, verbosity);
		assert(solvedNodes.empty());
		assert(nodesToExplore.empty());
		return Result();
	}

	Result loadSolution(LoadedSolution &&s, int verbosity) {
		if(verbosity >= 2) log << logPrefix << "loadSolution(s)" << std::endl;
		if(currentResult) {
			currentResult->lock();
			currentResult = nullptr;
		}
		if(data.nonIntegralEnumVar) {
			//			if(nonIntegralState != State::Created) {
			//				if(nonIntegralEnumVar)
			//					throw LogicError("Can not enumerate solutions over non-integral variable '"
			//									 + model.getVarName(*nonIntegralEnumVar) + "' of type "
			//									 + varTypeToString(nonIntegralEnumVar->getType()) + ".");
			//				if(nonIntegralObjFunction)
			//					throw LogicError("Can not calculate more than one solution with non-integral objective function.");
			//				MOD_ABORT;
			//			}
			//			if(verbosity >= 2) {
			//				log << logPrefix << "Solving model.\n";
			//			}
			//			const bool oldSolverVerbose = getConfig().ilp.solverVerbose;
			//			getConfig().ilp.solverVerbose.set(verbosity >= 1);
			//			ILP::Response res = ilp.model->realSolve();
			//			getConfig().ilp.solverVerbose.set(oldSolverVerbose);
			// TODO: validate solution
			//			if(res == ILP::Response::Failed || res == ILP::Response::TimedOut) {
			//				if(verbosity >= 2)
			//					log << logPrefix << "Solving failed. Response = " << res << "\n";
			//				nonIntegralState = State::Infeasible;
			//				return Result();
			//			} else {
			nonIntegralState = State::SolvedNonOptimal;
			nonIntegralObjVal = s.objVal;
			if(verbosity >= 2)
				log << logPrefix << "Response = " << "?" << "\tobjVal = " << nonIntegralObjVal << "\n";
			// record solution
			for(const auto &p: s.integralValues) {
				assert(p.second != 0);
				nonIntegralSolutionIntegral.emplace(p.first.getData(), p.second);
			}
			for(const auto &p: s.floatValues) {
				assert(p.second != 0.0);
				nonIntegralSolutionNonIntegral.emplace(p.first.getData(), p.second);
			}
			return Result(std::make_unique<Result::Pimpl>(this));
		}

		const auto assignSolution = [this, &s, verbosity](TreeNode *n) {
			masterData.nodeForLastBranchingConstraint = n;
			// TODO: validate solution
//			if(res == ILP::Response::Failed || res == ILP::Response::TimedOut) {
//				if(verbosity >= 2)
//					log << logPrefix << "      Solving failed. Response = " << res << "\n";
//				n->state = State::Infeasible;
//				return false;
//				if(!feasible) {
//					if(verbosity >= 2) log << logPrefix << "  node infeasible" << std::endl;
//					return Result();
//				}
//			} else
			{
				n->state = State::SolvedNonOptimal;
				n->objVal = s.objVal;
				n->ancestorWithSolution = n;
				currentObjVal = s.objVal;
				if(verbosity >= 2)
					log << logPrefix << "     Valid, objVal = " << n->objVal << std::endl;

				// record solution
				n->solutionIntegral.reserve(s.integralValues.size());
				n->solutionNonIntegral.reserve(s.floatValues.size());
				for(const auto &p: s.integralValues) {
					assert(p.second != 0);
					n->solutionIntegral.emplace(p.first.getData(), p.second);
				}
				for(const auto &p: s.floatValues) {
					assert(p.second != 0.0);
					n->solutionNonIntegral.emplace(p.first.getData(), p.second);
				}
				createChildren(n, verbosity);
				return handleAndReportSolution(n, verbosity);
			}
		};

		if(!root) {
			if(verbosity >= 2) log << logPrefix << "  Creating root" << std::endl;
			const int varOffset = 0;
			root.reset(new TreeNode(nextNodeId++, nullptr, varOffset, data.origBounds[varOffset]));
			return assignSolution(root.get());
		}

		TreeNode *nextNode = root.get();
		while(true) {
			const auto gotoBranch = [this, nextNode, &s]() {
				const auto var = data.enumerationVars[nextNode->varOffset];
				const auto val = s.getVal(var);
				const auto thisVal = nextNode->getVarVal(var);
				if(val < thisVal) return nextNode->lt.get();
				else if(val == thisVal) return nextNode->eq.get();
				else return nextNode->gt.get();
			};
			printStatus(false, verbosity);
			switch(nextNode->state) {
			case State::Created:
				nodesToExplore.remove(nextNode);
				assert(nextNode->state == State::Created);
				assert(nextNode->parent);
				if(verbosity >= 2)
					data.printNode(log << logPrefix << "  Handling created node: ", nextNode) << std::endl;
				assert(nextNode->parent->state == State::SolvedOptimal ||
					   nextNode->parent->state == State::SolvedImplicitly);
				if(nextNode->parent->eq.get() == nextNode) {
					if(verbosity >= 2) log << logPrefix << "    Eq node, implicitly solved" << std::endl;
					nextNode->state = State::SolvedImplicitly;
					nextNode->objVal = nextNode->parent->objVal;
					nextNode->ancestorWithSolution = nextNode->parent->ancestorWithSolution;
					createChildren(nextNode, verbosity);
					nextNode = gotoBranch();
					continue;
				} else {
					return assignSolution(nextNode);
				}
			case State::Infeasible:
				MOD_ABORT;
				break;
			case State::SolvedNonOptimal:
				MOD_ABORT;
				break;
			case State::SolvedOptimal:
			case State::SolvedImplicitly:
				nextNode = gotoBranch();
				continue;
			}
			MOD_ABORT;
		}
	}
private:
	Result serialEnumeration(int verbosity, int ilpVerbosity) {
		do { // while !solvedNodes.empty()
			if(!solvedNodes.empty()) {
				if(verbosity >= 2) log << logPrefix << "  Solved nodes pending" << std::endl;
				TreeNode *nextNode = solvedNodes.top();
				const auto nextObjVal = nextNode->objVal;
				assert(nextObjVal >= currentObjVal);
				if(currentObjVal == nextObjVal) {
					solvedNodes.pop();
					createChildren(nextNode, verbosity);
					return handleAndReportSolution(nextNode, verbosity);
				} else if(nodesToExplore.empty()) {
					if(verbosity >= 2) log << logPrefix << "  No created nodes pending" << std::endl;
					currentObjVal = nextObjVal;
					if(verbosity >= 1) {
						log << "Starting enumeration of solutions with value " << nextObjVal;
						if(nextNode != root.get())
							log << " = " << root->objVal << " + " << (nextObjVal - root->objVal);
						log << std::endl;
					}
					solvedNodes.pop();
					createChildren(nextNode, verbosity);
					return handleAndReportSolution(nextNode, verbosity);
				}
			}
			while(!nodesToExplore.empty()) {
				printStatus(false, verbosity);
				TreeNode *nextNode = nodesToExplore.front();
				nodesToExplore.pop_front();
				assert(nextNode->state == State::Created);
				assert(nextNode->parent);
				if(verbosity >= 2)
					data.printNode(log << logPrefix << "  Handling created node: ", nextNode) << std::endl;
				assert(nextNode->parent->state == State::SolvedOptimal ||
					   nextNode->parent->state == State::SolvedImplicitly);
				if(nextNode->parent->eq.get() == nextNode) {
					if(verbosity >= 2) log << logPrefix << "    Eq node, implicitly solved" << std::endl;
					nextNode->state = State::SolvedImplicitly;
					nextNode->objVal = nextNode->parent->objVal;
					nextNode->ancestorWithSolution = nextNode->parent->ancestorWithSolution;
					createChildren(nextNode, verbosity);
				} else {
					masterData.createBranchingConstraints(nextNode, verbosity, ilpVerbosity, log);
					// try easy bounds check
					const Var var = data.enumerationVars[nextNode->varOffset];
					const ILP::Var ilpVar = masterData.ilp.getVar(var);
					const Int lb = masterData.ilp.model->getLBInt(ilpVar);
					const Int ub = masterData.ilp.model->getUBInt(ilpVar);
					if(verbosity >= 2) log << logPrefix << "    Bounds: [" << lb << ", " << ub << "]" << std::endl;
					assert(lb >= nextNode->bound.lower);
					assert(ub <= nextNode->bound.upper);
					if(lb > ub) {
						if(verbosity >= 2) log << logPrefix << "    Quick infeasible by bounds" << std::endl;
						assert(false);
						nextNode->state = State::Infeasible;
						continue;
					}
					const bool feasible = solveNode(nextNode, verbosity, ilpVerbosity);
					if(!feasible) {
						if(verbosity >= 2) log << logPrefix << "    Node infeasible" << std::endl;
						continue;
					}
					assert(nextNode->objVal >= currentObjVal);
					if(nextNode->objVal == currentObjVal) {
						createChildren(nextNode, verbosity);
						return handleAndReportSolution(nextNode, verbosity);
					} else {
						assert(nextNode->state == State::SolvedNonOptimal);
						if(verbosity >= 2) {
							data.printNode(log << logPrefix << "  non-optimal solution, ", nextNode)
									<< " = " << nextNode->getVarVal(data.enumerationVars[nextNode->varOffset])
									<< std::endl;
							log << logPrefix << "  current objVal = " << currentObjVal << std::endl;
						}
						solvedNodes.push(nextNode);
					}
				}
			} // while !nodesToExplore.empty()
		} while(!solvedNodes.empty());
		return Result();
	}

	bool solveNode(TreeNode *n, int verbosity, int ilpVerbosity) {
		++statSolveCalls;
		const auto feasible = masterData.solveNode(n, verbosity, ilpVerbosity, statSolveCalls, log);
		return feasible;
	}

	void createChildren(TreeNode *node, int verbosity) {
		if(verbosity >= 2)
			data.printNode(log << logPrefix << "  createChildren(", node) << ")" << std::endl;
		assert(!node->lt);
		assert(!node->eq);
		assert(!node->gt);
		// create potentially three new search nodes:
		// upper       -+
		// ...          | gt node, same var
		// varVal + 1  -+
		// varVal      -  eq node, done with this var, make it for the next var
		// varVal - 1  -+
		// ...          | lt node, same var
		// lower       -+

		if(node->varOffset + 1 < data.enumerationVars.size()) {
			if(verbosity >= 2)
				log << logPrefix << "    eq: " << nextNodeId << std::endl;
			node->eq.reset(new TreeNode(nextNodeId, node, node->varOffset + 1,
										data.origBounds[node->varOffset + 1]));
			++nextNodeId;
		}
		const IntegralVar var = data.enumerationVars[node->varOffset];
		const auto varVal = node->getVarVal(var);
		const Bound bound = node->bound;
		if(varVal - 1 >= bound.lower) {
			if(verbosity >= 2) {
				log << logPrefix << "    lt: " << nextNodeId << " [" << bound.lower << ", " << (varVal - 1) << "]"
					<< std::endl;
			}
			node->lt.reset(new TreeNode(nextNodeId, node, node->varOffset,
										Bound{bound.lower, varVal - 1}));
			++nextNodeId;
		}
		if(varVal + 1 <= bound.upper) {
			if(verbosity >= 2) {
				log << logPrefix << "    gt: " << nextNodeId << " [" << (varVal + 1) << ", " << bound.upper << "]"
					<< std::endl;
			}
			node->gt.reset(new TreeNode(nextNodeId, node, node->varOffset,
										Bound{varVal + 1, bound.upper}));
			++nextNodeId;
		}

		if(node->eq) nodesToExplore.push_front(node->eq.get());
		if(node->lt) nodesToExplore.push_front(node->lt.get());
		if(node->gt) nodesToExplore.push_back(node->gt.get());
	}

	Result handleAndReportSolution(TreeNode *node, int verbosity) {
		assert(node->state == State::SolvedNonOptimal);
		if(verbosity >= 2) {
			data.printNode(log << logPrefix << "  reporting solution, ", node)
					<< " = " << node->getVarVal(data.enumerationVars[node->varOffset]) << std::endl;
		}
		node->state = State::SolvedOptimal;
		reportedSolutionNodes.push_back(node);

		printStatus(true, verbosity);
		return Result(std::make_unique<Result::Pimpl>(this, node));
	}
private:
	void printStatus(bool disregardTime, int verbosity) {
		if(verbosity < 1) return;
		auto timeNow = std::chrono::system_clock::now();
		if(!disregardTime && timeNow - statTimeLast < std::chrono::seconds(2)) return;
		statTimeLast = timeNow;
		if(statStatusPrints == 0) {
			log << "            Nodes                  Solutions     \n";
			log << "----------------------------- -------------------\n";
			log << "   #Total     #Open  #Pending    #Total #Accepted   #Solves Obj. value  Time (s)\n";
		}
		statStatusPrints = (statStatusPrints + 1) % 30;
		log << std::right
			<< std::setw(9) << nextNodeId
			<< std::setw(10) << nodesToExplore.size()
			<< std::setw(10) << solvedNodes.size()
			<< std::setw(10) << reportedSolutionNodes.size()
			<< std::setw(10) << reportedSolutionNodes.size()
			<< std::setw(10) << statSolveCalls;
		if(currentObjVal == Int(Int::MIN))
			log << std::right << std::setw(11) << "min";
		else
			log << std::right << std::setw(11) << currentObjVal;
		log << std::right << std::setw(10)
			<< std::chrono::duration_cast<std::chrono::seconds>(timeNow - statTimeStart).count()
			<< std::endl;
	}
public:
	void printTree(std::ostream &s) const {
		s << "digraph g {\n";
		s << "legend_varNames [ label=\"varNum: varName\" ]\n";
		for(int i = 0; i < data.enumerationVars.size(); i++)
			s << "legend_varNames_" << i << " [ label=\"" << i << ": "
			  << data.model.getVarName(data.enumerationVars[i])
			  << ", [" << data.origBounds[i].lower << ", " << data.origBounds[i].upper << "]" << "\" ]\n";
		s << "legend_varNames -> legend_varNames_0\n";
		for(int i = 1; i < data.enumerationVars.size(); i++)
			s << "legend_varNames_" << (i - 1) << " -> legend_varNames_" << i << "\n";
		s << "legend_colours_0 [ label=\"Meaning of colours and line style\" ]" << "\n";
		std::vector<std::pair<std::string, std::string>> colours = {
				{"Not yet processed at all",    ""},
				{"Infeasible",                  "color=gray"},
				{"Infeasible by user request",  "color=purple style=dashed"},
				{"Infeasible due to absGap",    "color=purple"},
				{"Currently not optimal",       "style=dashed"},
				{"Feasible",                    "color=green"},
				{"Feasible, implicitly solved", "color=green style=dashed"},
				{"Duplicate solution",          "color=blue"},
				{"Current node at exit",        "fontcolor=red"}
		};
		for(int i = 0; i < colours.size(); i++) {
			s << "legend_colours_" << (i + 1) << " [ label=\"" << colours[i].first << "\" "
			  << colours[i].second << " ]\n";
			s << "legend_colours_" << i << " -> legend_colours_" << (i + 1) << "\n";
		}
		s << "legend_text [ label=\"Text: nodeId, varNum, objVal, solNodeId\" ]\n";
		printTree(s, root.get());
		s << "}" << std::endl;
	}
private:
	void printTree(std::ostream &s, const TreeNode *n) const {
		if(!n) return;
		if(n->parent) {
			s << n->parent->id << " -> " << n->id << " [ label=\"";
			if(n == n->parent->lt.get()) s << "< ";
			else if(n == n->parent->eq.get()) s << " == ";
			else if(n == n->parent->gt.get()) s << " > ";
			else
				MOD_ABORT;
			s << n->parent->getVarVal(data.enumerationVars[n->parent->varOffset]) << "\" ]\n";
		}
		s << n->id << " [ label=\"" << n->id << ", " << n->varOffset; // << ":" << getVarName(enumerationVars[n->var]);
		switch(n->state) {
		case State::Created:
		case State::Infeasible:
			break;
		case State::SolvedNonOptimal:
			s << ", " << n->objVal;
			break;
		case State::SolvedOptimal:
		case State::SolvedImplicitly:
			s << ", " << n->objVal;
			break;
		}
		s << ", ";
		if(n->ancestorWithSolution) s << n->ancestorWithSolution->id;
		else s << "-";
		s << "\"";
		switch(n->state) {
		case State::Created:
			break;
		case State::Infeasible:
			s << " color=gray";
			break;
		case State::SolvedNonOptimal:
			s << " style=dashed";
			break;
		case State::SolvedOptimal:
			s << " color=green";
			break;
		case State::SolvedImplicitly:
			s << " color=green";
			s << " style=dashed";
			break;
		}
		s << " ]\n";
		printTree(s, n->lt.get());
		printTree(s, n->eq.get());
		printTree(s, n->gt.get());
	}
public:
	const std::string solver;
	std::ostream &log;
public:
	const SharedData data;
	WorkerData masterData;
	std::unique_ptr<TreeNode> root;
	int nextNodeId = 0;
	Value currentObjVal = Int(Int::MIN);
public:
	struct NodeList {
		std::size_t size() const { return nodes.size(); }
		bool empty() const { return nodes.empty(); }
		TreeNode *front() const { return nodes.front(); }
	public:
		void push_front(TreeNode *node) {
			nodes.push_front(node);
			assert(*nodes.begin() == node);
			node->nodesToExplorePos = nodes.begin();
		}

		void push_back(TreeNode *node) {
			nodes.push_back(node);
			assert(*--nodes.end() == node);
			node->nodesToExplorePos = --nodes.end();
		}

		void pop_front() { nodes.pop_front(); }

		void remove(TreeNode *node) {
			nodes.erase(node->nodesToExplorePos);
			node->nodesToExplorePos = {};
		}
	private:
		std::list<TreeNode *> nodes;
	};

	std::priority_queue<TreeNode *, std::vector<TreeNode *>, TreeNodeLess> solvedNodes;
	NodeList nodesToExplore;
	std::list<TreeNode *> reportedSolutionNodes;
	Result::Pimpl *currentResult = nullptr;
public: // non-integral
	bool nonIntegralObjFunction = false;
	State nonIntegralState = State::Created;
	Value nonIntegralObjVal;
	std::unordered_map<std::size_t, Int> nonIntegralSolutionIntegral;
	std::unordered_map<std::size_t, Float> nonIntegralSolutionNonIntegral;
public: // stats
	int statSolveCalls = 0;
	const std::chrono::system_clock::time_point statTimeStart = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point statTimeLast;
	int statStatusPrints = 0;
};

// ====================================================================================================

Solver::Solver(Model model_, std::unordered_set<Var> enumerationVarsSet, const std::string &solver,
			   int verbosity, std::ostream &log)
		: p(new Pimpl(std::move(model_), std::move(enumerationVarsSet), solver, verbosity, log)) {}

Solver::~Solver() = default;

const Model &Solver::getModel() const {
	return p->data.model;
}

bool Solver::isEnumVarsIntegral() const {
	return !p->data.nonIntegralEnumVar;
}

bool Solver::isObjValueIntegral() const {
	return !p->nonIntegralObjFunction;
}

void Solver::printEnumerationTree(std::ostream &s) const {
	p->printTree(s);
}

const ILP::Model &Solver::getILP() const {
	p->masterData.initializeILP(p->solver);
	return *p->masterData.ilp.model;
}

Result Solver::next(int verbosity, int ilpVerbosity) {
	return p->next(verbosity, ilpVerbosity);
}

Result Solver::loadSolution(LoadedSolution &&s, int verbosity) {
	return p->loadSolution(std::move(s), verbosity);
}

// ====================================================================================================================

Result::Result(std::unique_ptr<Pimpl> p) : p(std::move(p)) {}

Result::Result(Result &&other) : p(std::move(other.p)) {}

Result &Result::operator=(Result &&other) {
	p = std::move(other.p);
	return *this;
}

Result::~Result() = default;

Int Result::getObjValInt() const {
	auto opt = p->isEnumResult()
			   ? p->node->objVal.getAsInt()
			   : p->solver->nonIntegralObjVal.getAsInt();
	assert(opt);
	return *opt;
}

Float Result::getObjValFloat() const {
	auto opt = p->isEnumResult()
			   ? p->node->objVal.getAsFloat()
			   : p->solver->nonIntegralObjVal.getAsFloat();
	assert(opt);
	return *opt;
}

Value Result::getValue(Var v) const {
	if(v.getType() == VarType::Bool || v.getType() == VarType::Int) {
		return getValueIntegral(IntegralVar(v.getType(), v.getData()));
	} else if(v.getType() == VarType::Float) {
		return getValueFloat(FloatVar(v.getData()));
	} else {
		MOD_ABORT;
	}
}

Int Result::getValue(IntVar v) const {
	return getValueIntegral(v);
}

Int Result::getValue(BoolVar v) const {
	return getValueIntegral(v);
}

Float Result::getValue(FloatVar v) const {
	return getValueFloat(v);
}

Float Result::getValueCastFloat(Var v) const {
	if(v.getType() == VarType::Bool || v.getType() == VarType::Int) {
		return Float(getValueIntegral(IntegralVar(v.getType(), v.getData())));
	} else if(v.getType() == VarType::Float) {
		return getValueFloat(FloatVar(v.getData()));
	} else {
		MOD_ABORT;
	}
}

Value Result::evaluate(const CombiOpt::LinExpAny &expr) const {
	Int resInt;
	expr.forEachIntegral([this, &resInt](const auto coef, const auto var) {
		resInt += coef * getValueIntegral(var);
	});
	Float resFloat;
	bool hasFloat = false;
	expr.forEachFloat([this, &resFloat, &hasFloat](const auto coef, const auto var) {
		resFloat += coef * getValueCastFloat(var);
		hasFloat = true;
	});
	if(hasFloat) return resFloat + Float(resInt);
	else return resInt;
}

Int Result::getValueIntegral(IntegralVar v) const {
	if(p->isEnumResult()) {
		return p->node->getVarVal(IntegralVar(v.getType(), v.getData()));
	} else {
		const auto iter = p->solver->nonIntegralSolutionIntegral.find(v.getData());
		if(iter == p->solver->nonIntegralSolutionIntegral.end()) return Int();
		else return iter->second;
	}
}

Float Result::getValueFloat(FloatVar v) const {
	if(p->isEnumResult()) {
		return p->node->getVarVal(v);
	} else {
		const auto iter = p->solver->nonIntegralSolutionNonIntegral.find(v.getData());
		if(iter == p->solver->nonIntegralSolutionNonIntegral.end()) return Float();
		else return iter->second;
	}
}

} // namespace mod::lib::CombiOpt