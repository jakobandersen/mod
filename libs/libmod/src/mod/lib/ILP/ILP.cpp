#include "ILP.hpp"

#include <mod/Config.hpp>
#include <mod/Error.hpp>
#include <mod/Post.hpp>
#include <mod/lib/IO/IO.hpp>

#include <boost/lexical_cast.hpp>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <list>
#include <queue>

namespace mod::lib::ILP {

// LinExp
//------------------------------------------------------------------------------

LinExp::LinExp(Var var) {
	intElements.emplace_back(1, var);
}

const std::vector<LinExp::IntElement> &LinExp::getIntElements() const {
	return intElements;
}

const std::vector<LinExp::DoubleElement> &LinExp::getDoubleElements() const {
	return doubleElements;
}

LinExp &LinExp::operator+=(const LinExp &e) {
	const auto &is = e.getIntElements();
	intElements.insert(end(intElements), begin(is), end(is));
	const auto &ds = e.getDoubleElements();
	doubleElements.insert(end(doubleElements), begin(ds), end(ds));
	return *this;
}

LinExp &LinExp::operator-=(const LinExp &e) {
	for(const auto &el : e.getIntElements())
		intElements.emplace_back(-el.first, el.second);
	for(const auto &el : e.getDoubleElements())
		doubleElements.emplace_back(-el.first, el.second);
	return *this;
}

LinExp &LinExp::operator*=(Int c) {
	for(auto &el : intElements) el.first *= c;
	for(auto &el : doubleElements) el.first *= Float(c);
	return *this;
}

LinExp &LinExp::operator*=(Float c) {
	for(auto el : intElements) doubleElements.emplace_back(Float(el.first), el.second);
	intElements.clear();
	for(auto &el : doubleElements) el.first *= c;
	return *this;
}

LinExp &LinExp::operator/=(Int c) {
	for(auto el : intElements) doubleElements.emplace_back(Float(el.first), el.second);
	intElements.clear();
	for(auto &el : doubleElements) el.first /= Float(c);
	return *this;
}

LinExp &LinExp::operator/=(Float c) {
	for(auto el : intElements) doubleElements.emplace_back(ILP::Float(el.first), el.second);
	intElements.clear();
	for(auto &el : doubleElements) el.first /= c;
	return *this;
}

LinConstraint::LinConstraint(LinExp exp, Relation relation, Float bound)
		: exp(exp), relation(relation), bound(bound) {}

const LinExp &LinConstraint::getExp() const {
	return exp;
}

const LinConstraint::Relation &LinConstraint::getRelation() const {
	return relation;
}

Float LinConstraint::getBound() const {
	return bound;
}

// Model
//------------------------------------------------------------------------------

struct Model::CalculationProxy {
	void setLB(Model &model, Var v, double bound) {
		model.setLBImpl(v, bound);
	}

	void setUB(Model &model, Var v, double bound) {
		model.setUBImpl(v, bound);
	}

	bool isExternallyModified(Model &model) {
		return model.externallyModified;
	}

	void setExternallyModified(Model &model, bool value) {
		model.externallyModified = value;
	}
};

Model::Model() : externallyModified(false), isObjectiveFunctionIntegral(true),
                 debugOutput(getConfig().ilp.debugOutput) {}

Model::~Model() = default;

Var Model::addVariableFloat(const std::string &name) {
	externallyModified = true;
	return addVariableFloatImpl(name);
}

Var Model::addVariableInteger(const std::string &name) {
	externallyModified = true;
	return addVariableIntegerImpl(name);
}

Var Model::addVariableBinary(const std::string &name) {
	externallyModified = true;
	return addVariableBinaryImpl(name);
}

void Model::setLB(Var v, double bound) {
	externallyModified = true;
	setLBImpl(v, bound);
}

void Model::setUB(Var v, double bound) {
	externallyModified = true;
	setUBImpl(v, bound);
}

void Model::setBranchPriority(Var var, unsigned int prio) {
	externallyModified = true;
	setBranchPriorityImpl(var, prio);
}

void Model::setObjectiveFunction(const LinExp &exp) {
	externallyModified = true;
	isObjectiveFunctionIntegral = true;

	for(const auto &e : exp.getIntElements()) {
		if(isVariableReal(e.second)) {
			isObjectiveFunctionIntegral = false;
			break;
		}
	}
	if(!exp.getDoubleElements().empty()) isObjectiveFunctionIntegral = false;
	setObjectiveFunctionImpl(exp);
}

bool Model::getIsObjectiveFunctionIntegral() const {
	return isObjectiveFunctionIntegral;
}

void Model::addConstraint(const LinConstraint &constraint) {
	externallyModified = true;
	addConstraintImpl(constraint);
}

void Model::popConstraint() {
	externallyModified = true;
	popConstraintImpl();
}

Model::VariablesProxy::VariablesProxy(const Model &model) : model(model) {}

Model::VarIter::VarIter() : model(nullptr), currentVar(0) {}

Model::VarIter::VarIter(const Model &model, Var currentVar) : model(&model), currentVar(currentVar) {}

bool Model::VarIter::operator!=(const VarIter &other) const {
	if(!model || !other.model) return model != other.model;
	assert(model == other.model);
	return currentVar != other.currentVar;
}

Model::VarIter Model::VarIter::operator++() {
	if(!model) return *this;
	VarIter old(*this);
	*this = model->variablesIncrImpl(currentVar);
	return old;
}

Var Model::VarIter::operator*() const {
	assert(model);
	return currentVar;
}

Model::VarIter Model::VariablesProxy::begin() const {
	return model.variablesBeginImpl();
}

Model::VarIter Model::VariablesProxy::end() const {
	return VarIter();
}

Model::VariablesProxy Model::Variables() const {
	return VariablesProxy(*this);
}

std::string Model::initVariableName(const std::string &name, Var v) {
	return "x_" + boost::lexical_cast<std::string>(getData(v)) + "_" + std::string(name);
}

std::ostream &operator<<(std::ostream &s, const lib::ILP::Response &res) {
	switch(res) {
	case lib::ILP::Response::Failed:
		s << "Failed";
		break;
	case lib::ILP::Response::Optimal:
		s << "Optimal";
		break;
	case lib::ILP::Response::Feasible:
		s << "Not optimal, but feasable";
		break;
	case lib::ILP::Response::TimedOut:
		s << "Time limit reached";
		break;
	}
	return s;
}

namespace {

const std::string logPrefix = "AbstractSolver: ";

enum class ParentDir {
	Lt, Eq, Gt, Resolve
};

enum class State {
	Created, Infeasible, InfeasibleUserLazy, InfeasibleUserObjBound, Solved, Duplicate, NonOptimal, ImplicitlySolved
};

struct Bound {

	Bound() : lower(Int::INF_NEG), upper(Int::INF_POS) {}

	Bound(Int lower, Int upper) : lower(lower), upper(upper) {}

	Int lower;
	Int upper;
};

struct Bounds;

template<typename ObjHandler>
struct SearchNode {
	// a node with 'var' represents that this node is trying to fix var
	// the current bounds on var is 'varBounds'
	// 'solution' is a feasible solution for which all var' < var should be fixed

	SearchNode(unsigned int id, SearchNode *parent, unsigned int var, Bound varBounds, std::vector<Int> solution,
	           ParentDir parentDir)
			: id(id), parent(parent), var(var), solution(solution), varBounds(varBounds),
			  parentDir(parentDir), lt(nullptr), eq(nullptr), gt(nullptr), solveAgain(nullptr),
			  state(State::Created), solNum(-1) {
		assert(varBounds.lower <= varBounds.upper);
	}

	~SearchNode() {
		delete lt;
		delete eq;
		delete gt;
		delete solveAgain;
	}

	void recordSolution(const Model &model, const std::vector<Var> &enumerationVars) {
		for(unsigned int i = 0; i < var; i++)
			assert(solution[i] == model.getVarValIntegral(enumerationVars[i]));
		for(unsigned int i = var; i < enumerationVars.size(); i++)
			solution[i] = model.getVarValIntegral(enumerationVars[i]);
	}

	void
	createResolveChild(unsigned int &prevId, const std::vector<Var> &enumerationVars, std::list<SearchNode *> &todo,
	                   const Bounds &origBounds, bool verboseDetail);
	void createChildren(unsigned int &prevId, const std::vector<Var> &enumerationVars, std::list<SearchNode *> &todo,
	                    const Bounds &origBounds, bool verboseDetail);
public:
	unsigned int id;
	SearchNode *const parent;
	const unsigned int var;
	std::vector<Int> solution;
	const Bound varBounds;

	const ParentDir parentDir;

	SearchNode *lt;
	SearchNode *eq;
	SearchNode *gt;
	SearchNode *solveAgain;

	State state;
	typename ObjHandler::Type objVal;
	int solNum;
};

struct SearchNodePtrLess {

	template<typename ObjHandler>
	bool operator()(const SearchNode<ObjHandler> *a, const SearchNode<ObjHandler> *b) const {
		assert(a);
		assert(b);
		return a->objVal > b->objVal;
	}
};

struct Bounds {

	Bounds(Model &model, const std::vector<Var> &enumerationVars)
			: model(model), enumerationVars(enumerationVars), data(enumerationVars.size()),
			  oldExternallyModified(Model::CalculationProxy().isExternallyModified(model)) {
		for(unsigned int i = 0; i < enumerationVars.size(); i++) {
			data[i].lower = Int(model.getLBInt(enumerationVars[i]));
			data[i].upper = Int(model.getUBInt(enumerationVars[i]));
			if(data[i].lower > data[i].upper) {
				std::cerr << "WTF: " << model.getLBInt(enumerationVars[i]) << " ? "
				          << model.getUBInt(enumerationVars[i])
				          << "\n" << data[i].lower << " > " << data[i].upper << std::endl;
			}
			assert(data[i].lower <= data[i].upper);
		}
		Model::CalculationProxy().setExternallyModified(model, false);
	}

	~Bounds() {
		for(unsigned int i = 0; i < enumerationVars.size(); i++) reset(i);
		Model::CalculationProxy().setExternallyModified(model,
		                                                oldExternallyModified ||
		                                                Model::CalculationProxy().isExternallyModified(model)
		);
	}

	void reset(unsigned int varNum) {
		Model::CalculationProxy().setLB(model, enumerationVars[varNum], data[varNum].lower.getValue());
		Model::CalculationProxy().setUB(model, enumerationVars[varNum], data[varNum].upper.getValue());
	}

	template<typename ObjHandler>
	void reset(const SearchNode<ObjHandler> *n) {
		reset(n->var);
	}

	const Bound &operator[](std::size_t varNum) const {
		return data[varNum];
	}

	//	const Bound &operator[](const SearchNode *n) const {
	//		return (*this)[n->var];
	//	}

	Model &model;
	const std::vector<Var> &enumerationVars;
	std::vector<Bound> data;
	bool oldExternallyModified;
};

template<typename ObjHandler>
void SearchNode<ObjHandler>::createResolveChild(unsigned int &prevId,
                                                const std::vector<Var> &enumerationVars,
                                                std::list<SearchNode *> &todo,
                                                const Bounds &origBounds,
                                                bool verboseDetail) {
	solveAgain = new SearchNode(++prevId, this, var, varBounds, solution, ParentDir::Resolve);
	solveAgain->objVal = objVal;
	todo.push_front(solveAgain);
}

template<typename ObjHandler>
void SearchNode<ObjHandler>::createChildren(unsigned int &prevId,
                                            const std::vector<Var> &enumerationVars,
                                            std::list<SearchNode *> &todo,
                                            const Bounds &origBounds,
                                            bool verboseDetail) {
	if(var + 1 < enumerationVars.size()) {
		eq = new SearchNode(++prevId, this, var + 1, origBounds[var + 1], solution, ParentDir::Eq);
		eq->objVal = objVal;
		todo.push_front(eq);
	}
	if(solution[var] - 1 >= varBounds.lower) {
		lt = new SearchNode(++prevId, this, var, {varBounds.lower, solution[var] - 1}, solution, ParentDir::Lt);
		todo.push_front(lt);
	}
	if(solution[var] + 1 <= varBounds.upper) {
		gt = new SearchNode(++prevId, this, var, {solution[var] + 1, varBounds.upper}, solution, ParentDir::Gt);
		todo.push_back(gt);
	}
}

template<typename ObjHandler>
struct TreePrinter {

	TreePrinter(const Model &model,
	            const SearchNode<ObjHandler> &root,
	            const std::vector<Var> &enumerationVars,
	            SearchNode<ObjHandler> **lastBranchingConstraint,
	            const Bounds &origBounds)
			: model(model), root(root), enumerationVars(enumerationVars),
			  lastBranchingConstraint(lastBranchingConstraint), origBounds(origBounds) {}

	~TreePrinter() {
		post::FileHandle s(IO::makeUniqueFilePrefix() + "branchingTree.dot");
		s << "digraph g {" << std::endl;
		s << "legend_varNames [ label=\"varNum: varName, bounds\" ]" << std::endl;
		for(unsigned int i = 0; i < enumerationVars.size(); i++)
			s << "legend_varNames_" << i << " [ label=\"" << i << ": " << model.getVarName(enumerationVars[i])
			  << ", [" << origBounds[i].lower << ", " << origBounds[i].upper << "]" << "\" ]" << std::endl;
		s << "legend_varNames -> legend_varNames_0" << std::endl;
		for(unsigned int i = 1; i < enumerationVars.size(); i++)
			s << "legend_varNames_" << (i - 1) << " -> legend_varNames_" << i << std::endl;
		s << "legend_colours_0 [ label=\"Meaning of colours and line style\" ]" << std::endl;
		std::vector<std::pair<std::string, std::string> > colours = {
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
		for(unsigned int i = 0; i < colours.size(); i++) {
			s << "legend_colours_" << (i + 1) << " [ label=\"" << colours[i].first << "\" " << colours[i].second << " ]"
			  << std::endl;
			s << "legend_colours_" << i << " -> legend_colours_" << (i + 1) << std::endl;
		}
		s << "legend_text [ label=\"Text: varNum, objVal, solutionId\" ]" << std::endl;
		print(&root, s);
		s << "}" << std::endl;
	}

private:

	void print(const SearchNode<ObjHandler> *n, std::ostream &s) {
		if(!n) return;
		s << n->id << " [ label=\"" << n->id << ", " << n->var; // << ":" << getVarName(enumerationVars[n->var]);
		switch(n->state) {
		case State::Created:
		case State::Infeasible:
			break;
		case State::InfeasibleUserObjBound:
		case State::InfeasibleUserLazy:
		case State::NonOptimal:
			s << ", " << n->objVal;
			break;
		case State::Solved:
		case State::Duplicate:
		case State::ImplicitlySolved:
			s << ", " << n->objVal;
			s << ", " << n->solNum;
			break;
		}
		s << "\"";
		switch(n->state) {
		case State::Created:
			break;
		case State::Infeasible:
			s << " color=gray";
			break;
		case State::InfeasibleUserObjBound:
			s << " color=purple";
			break;
		case State::InfeasibleUserLazy:
			s << " color=purple style=dashed";
			break;
		case State::NonOptimal:
			s << " style=dashed";
			break;
		case State::Solved:
			s << " color=green";
			break;
		case State::ImplicitlySolved:
			s << " color=green";
			s << " style=dashed";
			break;
		case State::Duplicate:
			s << " color=blue";
			break;
		}
		if(n == *lastBranchingConstraint) s << " fontcolor=red";
		s << " ] " << std::endl;
		if(n->parent) {
			s << n->parent->id << " -> " << n->id << " [ label=\"";
			if(n == n->parent->lt) s << "< ";
			else if(n == n->parent->eq) s << " == ";
			else if(n == n->parent->gt) s << " > ";
			else if(n == n->parent->solveAgain) s << "resolve, == ";
			else
				MOD_ABORT;
			s << n->parent->solution[n->parent->var] << "\" ]" << std::endl;
		}
		print(n->lt, s);
		print(n->eq, s);
		print(n->gt, s);
		print(n->solveAgain, s);
	}

private:
	const Model &model;
	const SearchNode<ObjHandler> &root;
	const std::vector<Var> &enumerationVars;
	SearchNode<ObjHandler> **lastBranchingConstraint;
	const Bounds &origBounds;
};

} // namespace

template<typename ObjHandler>
void Model::solveImpl(const std::unordered_set<Var> &enumerationVarsAsSet,
                      std::function<void(CallbackResponse &, CallbackResponse::ObjectiveMode)> solutionReady,
                      int verbosity, int ilpVerbosity) {
	std::vector<Var> enumerationVars(begin(enumerationVarsAsSet), end(enumerationVarsAsSet));
	const bool verboseDetail = verbosity >= 2;
	const bool verbose = verbosity >= 1;
	if(verbose) {
		std::cout << "Enumeration by variable set (" << enumerationVars.size() << " variables)" << std::endl
		          << "============================================================" << std::endl;
	}
	if(enumerationVars.empty()) {
		std::cout << "No enumeration values given." << std::endl;
		MOD_ABORT;
	}
	for(Var v : enumerationVars) {
		if(isVariableReal(v)) {
			std::cout << logPrefix << "Can not enumerate on non-integer variable." << std::endl;
			MOD_ABORT;
		}
	}

	// tight variables should be first in the tree
	auto iter = std::partition(begin(enumerationVars), end(enumerationVars), [this](Var var) {
		return getLBInt(var) == getUBInt(var);
	});
	if(verbose)
		std::cout << logPrefix << " non-fixed enumeration vars: " << std::distance(iter, end(enumerationVars))
		          << std::endl;
	// boolean variables should be before other, TODO

	std::size_t statSolveCalls = 0;
	const auto doRealSolve = [this, &statSolveCalls, verboseDetail, &ilpVerbosity](SearchNode<ObjHandler> *n) -> bool {
		statSolveCalls++;
		if(verboseDetail)
			std::cout << logPrefix << "Calling real solver (call no. " << statSolveCalls << ")" << std::endl;
		Response res = realSolve(std::cout, ilpVerbosity);
		if(res == Response::Failed || res == Response::TimedOut) {
			if(verboseDetail) std::cout << logPrefix << "Solving failed. Response = " << res << std::endl;
			n->state = State::Infeasible;
			return false;
		} else {
			n->state = State::Solved;
			n->objVal = ObjHandler::getObjVal(*this);
			if(verboseDetail) std::cout << logPrefix << "Response = " << res << "\tobjVal = " << n->objVal << std::endl;
			return true;
		}
	};

	const auto statTimeStart = std::chrono::system_clock::now();

	Bounds origBounds(*this, enumerationVars);

	unsigned int prevNodeId = 0;
	unsigned int lastModificationId = 0; // id of prevNodeId when modified

	SearchNode<ObjHandler> root(++prevNodeId, nullptr, 0, origBounds[0], std::vector<Int>(enumerationVars.size()),
	                            ParentDir::Lt); // don't give it eq status
	SearchNode<ObjHandler> *lastBranchingConstraint = &root;
	TreePrinter<ObjHandler> treePrinter(*this, root, enumerationVars, &lastBranchingConstraint, origBounds);
	{ // initial solving
		if(verbose) std::cout << "Solving root node." << std::endl;
		bool feasible = doRealSolve(&root);
		if(!feasible) return;
		ilpVerbosity = 0;
		root.state = State::NonOptimal;
	}
	std::priority_queue<SearchNode<ObjHandler> *, std::vector<SearchNode<ObjHandler> *>, SearchNodePtrLess> nonOptimalNodes(
			SearchNodePtrLess{});
	nonOptimalNodes.push(&root);

	auto printVar = [this, &enumerationVars](std::ostream &s, SearchNode<ObjHandler> *n) -> std::ostream & {
		return s << n->var << ", " << getVarName(enumerationVars[n->var]);
	};

	const auto createBranchingConstraints = [this, &origBounds, &enumerationVars, &lastBranchingConstraint, &verboseDetail]
			(SearchNode<ObjHandler> *const n) {
		if(verboseDetail) std::cout << logPrefix << "Creating branching constraints" << std::endl;
		// reset bounds if we were further down before
		while(lastBranchingConstraint->var > n->var) {
			origBounds.reset(lastBranchingConstraint->parent);
			lastBranchingConstraint = lastBranchingConstraint->parent;
		}
		// fix previous variables
		for(unsigned int i = 0; i < n->var; i++) {
			Var var = enumerationVars[i];
			Int val = n->solution[i];
			if(verboseDetail) std::cout << logPrefix << getVarName(var) << " == " << val << std::endl;
			setLBImpl(var, val.getValue());
			setUBImpl(var, val.getValue());
		}
		// set bounds for our variable
		assert(n->varBounds.lower <= n->varBounds.upper);
		setLBImpl(enumerationVars[n->var], n->varBounds.lower.getValue());
		setUBImpl(enumerationVars[n->var], n->varBounds.upper.getValue());
		lastBranchingConstraint = n;
	};

	int highestSolNum = -1;
	std::size_t statNumSolutions = 0;
	auto statTimeLast = std::chrono::system_clock::now();
	typename ObjHandler::Type objValBound = ObjHandler::Type::INF_POS;

	auto printStatus = [verbose, &prevNodeId, &statNumSolutions, &highestSolNum,
			&statSolveCalls, &nonOptimalNodes, &statTimeStart, &statTimeLast]
			(const std::list<SearchNode<ObjHandler> *> &todo, typename ObjHandler::Type currentObjValue) {
		if(!verbose) return;
		auto timeNow = std::chrono::system_clock::now();
		if(timeNow - statTimeLast < std::chrono::seconds(2)) return;
		statTimeLast = timeNow;
		static char numPrints = 0;
		if(numPrints == 0) {
			std::cout << "      Nodes       " "  " "     Solutions     " << std::endl;
			std::cout <<
			          "#Total    "
			          "#Open     "
			          "#Total    "
			          "#Accepted  "
			          "#Solves   "
			          "Obj. value  "
			          "Time (s)"
			          << std::endl;
		}
		numPrints = (numPrints + 1) % 20;
		std::size_t numOpenNodes = todo.size() + nonOptimalNodes.size();
		std::cout << std::right
		          << std::setw(8) << prevNodeId
		          << std::setw(10) << numOpenNodes
		          << std::setw(10) << statNumSolutions
		          << std::setw(11) << (highestSolNum + 1)
		          << std::setw(10) << statSolveCalls;
		if(currentObjValue != typename ObjHandler::Type(ObjHandler::Type::MAX)) {
			std::cout << std::right << std::setw(12) << currentObjValue;
		} else {
			std::cout << std::right << std::setw(12) << "max";
		}
		std::cout << std::right
		          << std::setw(10) << std::chrono::duration_cast<std::chrono::seconds>(timeNow - statTimeStart).count()
		          << std::endl;
	};
	do { // while !nonOptimalNodes.empty()
		std::list<SearchNode<ObjHandler> *> todo;

		const auto handleSolvedNode = [this, &lastModificationId, &prevNodeId, &solutionReady,
				&verboseDetail, &printVar, &highestSolNum, &statNumSolutions, &objValBound]
				(SearchNode<ObjHandler> *n) -> bool {
			statNumSolutions++;
			if(verboseDetail)
				printVar(std::cout << logPrefix << "Var(", n) << ") = " << n->solution[n->var] << std::endl;
			CallbackResponse response;
			solutionReady(response, ObjHandler::mode);
			if(externallyModified) lastModificationId = prevNodeId;
			externallyModified = false;
			{ // debugging
				switch(response.solType) {
				case CallbackResponse::SolutionType::New:
					n->solNum = response.solutionId;
					highestSolNum = n->solNum;
					break;
				case CallbackResponse::SolutionType::Duplicate:
					n->state = State::Duplicate;
					n->solNum = response.solutionId;
					break;
				case CallbackResponse::SolutionType::Rejected:
					n->state = State::InfeasibleUserLazy;
					break;
				}
			}
			if(ObjHandler::hasNewBound(response) && ObjHandler::getNewBound(response) < objValBound) {
				if(verboseDetail)
					std::cout << logPrefix << "Adjusting objective value bound to "
					          << ObjHandler::getNewBound(response) << " from " << objValBound << " (by user request)"
					          << std::endl;
				objValBound = ObjHandler::getNewBound(response);
			}
			if(!response.continueSearch) {
				if(verboseDetail) std::cout << logPrefix << "Aborting by user request." << std::endl;
				return false;
			}
			return true;
		};
		SearchNode<ObjHandler> *firstNewObjValNode = nonOptimalNodes.top();
		typename ObjHandler::Type currentObjVal = firstNewObjValNode->objVal;

		// skip if infeasible
		if(objValBound < currentObjVal) {
			if(verboseDetail)
				std::cout << logPrefix << "Infeasible due to objective value bound." << std::endl
				          << "\tobjVal = " << currentObjVal << std::endl
				          << "\tbound  = " << objValBound << std::endl;
			firstNewObjValNode->state = State::InfeasibleUserObjBound;
			nonOptimalNodes.pop();
			continue;
		}

		if(verbose) {
			statTimeLast -= std::chrono::seconds(60);
			printStatus(todo, currentObjVal);
			std::cout << "Enumeration of solutions with value " << currentObjVal;
			if(firstNewObjValNode != &root) std::cout << " = " << root.objVal << " + " << (currentObjVal - root.objVal);
			std::cout << std::endl;
		}
		do { // take all with same objVal onto the todo list
			nonOptimalNodes.pop();
			todo.push_back(firstNewObjValNode);
			firstNewObjValNode = nonOptimalNodes.top();
		} while(firstNewObjValNode->objVal == currentObjVal && !nonOptimalNodes.empty());

		while(!todo.empty()) {
			printStatus(todo, currentObjVal);
			SearchNode<ObjHandler> *currentNode = todo.front();
			todo.pop_front();
			createBranchingConstraints(currentNode);
			if(verboseDetail) printVar(std::cout << logPrefix << "Solving node, var = ", currentNode) << std::endl;
			bool hasCachedSolution =
					currentNode->parentDir == ParentDir::Eq || currentNode->parentDir == ParentDir::Resolve;
			if(currentNode->parent && currentNode->parent->id <= lastModificationId) {
				if(verboseDetail)
					std::cout << logPrefix << "Problem modified, releasing cache; id = "
					          << currentNode->id << ", lastModificationId = " << lastModificationId << std::endl;
				hasCachedSolution = false;
			}
			bool newSolution = false;
			if(hasCachedSolution) {
				assert(currentNode->parent);
				assert(currentNode->parentDir == ParentDir::Eq);
				assert(currentNode->objVal == currentObjVal);
				if(verboseDetail)
					printVar(std::cout << logPrefix << "Var(", currentNode)
							<< ") = " << currentNode->solution[currentNode->var] << " (implicitly solved)" << std::endl;
				currentNode->state = State::ImplicitlySolved;
				currentNode->solNum = currentNode->parent->solNum;
			} else { // solution not cached
				currentNode->state = State::Infeasible;
				// try easy bounds check
				if(currentNode->parent) {
					Var parentVar = enumerationVars[currentNode->parent->var];
					if(getLBInt(parentVar) > getUBInt(parentVar)) {
						if(verboseDetail)
							std::cout << logPrefix << "Infeasible bounds for parent var: "
							          << getLBInt(parentVar) << " > " << getUBInt(parentVar) << std::endl;
						continue;
					} else { // debugging
						if(getLBInt(parentVar) < origBounds[currentNode->parent->var].lower
						   || getUBInt(parentVar) > origBounds[currentNode->parent->var].upper) {
							printVar(std::cout << logPrefix << "LB for ", currentNode->parent)
									<< ": current = "
									<< getLBInt(parentVar)
									<< ", orig = "
									<< origBounds[currentNode->parent->var].lower
									<< std::endl;
							printVar(std::cout << logPrefix << "UB for ", currentNode->parent)
									<< ": current = "
									<< getUBInt(parentVar)
									<< ", orig = "
									<< origBounds[currentNode->parent->var].upper
									<< std::endl;
						}
						assert(getLBInt(parentVar) >= origBounds[currentNode->parent->var].lower);
						assert(getUBInt(parentVar) <= origBounds[currentNode->parent->var].upper);
					}
				}
				// must do the real solving
				bool feasible = doRealSolve(currentNode);
				if(!feasible) continue;
				currentNode->state = State::Solved;
				newSolution = true;
			}
			// assert: currentNode must have objVal, varVal, solution and state set
			if(currentNode->objVal != currentObjVal) {
				if(verboseDetail)
					std::cout << logPrefix << "Non-optimal objective value: " << currentNode->objVal << " (optimal is "
					          << currentObjVal << ")" << std::endl;
				if(currentObjVal > currentNode->objVal) {
					std::cout
							<< "===================================================================================="
							<< std::endl;
					std::cout
							<< "===================================================================================="
							<< std::endl;
					std::cout
							<< "===================================================================================="
							<< std::endl;
					std::cout << std::boolalpha << "State=" << static_cast<int> (currentNode->state) << ", newSolution="
					          << newSolution << std::endl;
					std::cout << "ERROR: id=" << currentNode->id << ", var=" << currentNode->var << ", bounds="
					          << currentNode->varBounds.lower << "," << currentNode->varBounds.upper << std::endl;
					std::cout
							<< "===================================================================================="
							<< std::endl;
					std::cout
							<< "===================================================================================="
							<< std::endl;
					std::cout
							<< "===================================================================================="
							<< std::endl;
					MOD_ABORT;
				}
				nonOptimalNodes.push(currentNode);
				currentNode->state = State::NonOptimal;
			} else {
				if(newSolution) {
					currentNode->recordSolution(*this, enumerationVars);
					bool carryOn = handleSolvedNode(currentNode);
					if(!carryOn) {
						if(verbose) std::cout << "Ending search (by user request)" << std::endl;
						statTimeLast -= std::chrono::seconds(60);
						printStatus(todo, currentObjVal);
						return;
					}
				}
				// if the user rejected the solution but didn't modify the problem, then resolving won't help
				if(currentNode->state == State::InfeasibleUserLazy
				   && lastModificationId != currentNode->id && currentNode->var == enumerationVars.size() - 1) {
					std::cout
							<< "WARNING(ILP tree search): solution rejected by user, but problem not modified and it's the last enumeration variable"
							<< std::endl;
				}
				if(currentNode->state == State::InfeasibleUserLazy && lastModificationId == currentNode->id) {
					currentNode->createResolveChild(prevNodeId, enumerationVars, todo, origBounds, verboseDetail);
				} else {
					currentNode->createChildren(prevNodeId, enumerationVars, todo, origBounds, verboseDetail);
				}
			} // if(non-optimal objVal)
		} // while(!todo.empty())
	} while(!nonOptimalNodes.empty());

	if(verbose) std::cout << "Ending search (no more solutions)" << std::endl;
	statTimeLast -= std::chrono::seconds(60);
	printStatus(std::list<SearchNode<ObjHandler> *>(), ObjHandler::Type::MAX);
}

namespace {

struct ObjHandlerIntegral {
	using Type = Int;
	static constexpr CallbackResponse::ObjectiveMode mode = CallbackResponse::ObjectiveMode::Integral;

	static Int getObjVal(Model &model) {
		return model.getObjValIntegral();
	}

	static bool hasNewBound(const CallbackResponse &response) {
		return response.newIntegralObjValBound.has_value();
	}

	static Int getNewBound(const CallbackResponse &response) {
		if(!response.newIntegralObjValBound) MOD_ABORT;
		return *response.newIntegralObjValBound;
	}
};

struct ObjHandlerReal {
	using Type = Float;
	static constexpr CallbackResponse::ObjectiveMode mode = CallbackResponse::ObjectiveMode::Real;

	static bool hasNewBound(const CallbackResponse &response) {
		return response.newRealObjValBound.has_value();
	}

	static Float getNewBound(const CallbackResponse &response) {
		if(!response.newRealObjValBound) MOD_ABORT;
		return *response.newRealObjValBound;
	}
};

} // namespace

void Model::solve(const std::unordered_set<Var> &enumerationVarsAsSet,
                  std::function<void(CallbackResponse &, CallbackResponse::ObjectiveMode)> solutionReady,
                  int verbosity, int ilpVerbosity) {
	if(getIsObjectiveFunctionIntegral()) {
		solveImpl<ObjHandlerIntegral>(enumerationVarsAsSet, solutionReady, verbosity, ilpVerbosity);
	} else {
		throw mod::FatalError("Solution enumeration for real valued objective functions is not supported.");
	}
}

unsigned int Model::getData(Var v) {
	return v.data;
}

} // namespace mod::lib::ILP