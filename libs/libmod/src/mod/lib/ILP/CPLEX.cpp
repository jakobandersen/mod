#include "CPLEX.hpp"

#ifdef MOD_HAVE_CPLEX

#define MOD_WITH_CPLEX_TRY_CATCH
#ifdef MOD_WITH_CPLEX_TRY_CATCH
#define MOD_cplexTry try
#define MOD_cplexCatch catch(const IloException &e) {   \
        std::string msg = "Exception from CPLEX: ";     \
        throw mod::FatalError(msg + e.getMessage(), 0); \
    }
#else
#define MOD_cplexTry
#define MOD_cplexCatch
#endif

#include <mod/Config.hpp>
#include <mod/Error.hpp>
#include <mod/lib/IO/IO.hpp>

#include <algorithm>
#include <unordered_map>

namespace mod::lib::ILP {

CPLEX::CPLEX() MOD_cplexTry: Model::Model(), model(env), cplex(model) {
	vars.push_back(nullptr);

	//IloCplex::Callback cb = MakeTestCallback(env, cplex);
	//cplex.use(cb);
	// OBS (TODO): danger, take a look at this when the callback is expanded
	//cplex.setParam(IloCplex::ParallelMode, IloCplex::Deterministic);
	//cplex.setParam(IloCplex::MIPSearch, IloCplex::Dynamic);
} MOD_cplexCatch

CPLEX::~CPLEX() {
	env.end(); // because it doesn't have a proper destructor
}

Var CPLEX::addVariableFloatImpl(const std::string &name) MOD_cplexTry {
	Var vExternal(vars.size());
	std::string fullName = initVariableName(name, vExternal);
	IloNumVar vInternal(env, 0, IloInfinity, IloNumVar::Float, fullName.c_str());
	if(debugOutput)
		std::cout << "ILP::CPLEX::addVariableFloat: " << vInternal << "\tid=" << vInternal.getId() << std::endl;
	vars.push_back(vInternal);
	return vExternal;
} MOD_cplexCatch

Var CPLEX::addVariableIntegerImpl(const std::string &name) MOD_cplexTry {
	Var vExternal(vars.size());
	std::string fullName = initVariableName(name, vExternal);
	IloNumVar vInternal(env, 0, IloInfinity, IloNumVar::Int, fullName.c_str());
	if(debugOutput)
		std::cout << "ILP::CPLEX::addVariableInteger: " << vInternal << "\tid=" << vInternal.getId() << std::endl;
	vars.push_back(vInternal);
	return vExternal;
} MOD_cplexCatch

Var CPLEX::addVariableBinaryImpl(const std::string &name) MOD_cplexTry {
	Var vExternal(vars.size());
	std::string fullName = initVariableName(name, vExternal);
	IloNumVar vInternal(env, 0, IloInfinity, IloNumVar::Bool, fullName.c_str());
	if(debugOutput)
		std::cout << "ILP::CPLEX::addVariableBinary: " << vInternal << "\tid=" << vInternal.getId() << std::endl;
	vars.push_back(vInternal);
	return vExternal;
} MOD_cplexCatch

void CPLEX::setLBImpl(Var v, double bound) MOD_cplexTry {
	vars[getData(v)].setLB(bound);
}
MOD_cplexCatch

void CPLEX::setUBImpl(Var v, double bound) MOD_cplexTry {
	vars[getData(v)].setUB(bound);
} MOD_cplexCatch

void CPLEX::setBranchPriorityImpl(Var var, unsigned int prio) MOD_cplexTry {
	assert(getData(var) >= 1);
	assert(getData(var) < vars.size());
	cplex.setPriority(vars[getData(var)], prio);
} MOD_cplexCatch

void CPLEX::setObjectiveFunctionImpl(const LinExp &exp) MOD_cplexTry {
	IloExpr expr(env);
	for(const LinExp::IntElement &e: exp.getIntElements()) {
		assert(getData(e.second) < vars.size());
		expr += e.first.getValue() * vars[getData(e.second)];
	}
	for(const LinExp::DoubleElement &e: exp.getDoubleElements()) {
		assert(getData(e.second) < vars.size());
		expr += e.first.getValue() * vars[getData(e.second)];
	}

	IloObjective::Sense sense = IloObjective::Minimize;
	IloObjective objective(env, expr, sense);
	if(debugOutput) std::cout << "ILP::CPLEX::setObjectiveFunction: " << objective << std::endl;
	model.add(objective);
} MOD_cplexCatch

void CPLEX::addConstraintImpl(const LinConstraint &constraint) MOD_cplexTry {
	if(debugOutput) {
		std::cout << "ILP::CPLEX::addConstraint(" << constraints.size() << "): ";
		bool first = true;
		const auto printTerm = [&](const auto &e) {
			if(first) {
				first = false;
				if(e.first.getValue() == -1) std::cout << "-";
				else if(e.first.getValue() != 1) std::cout << e.first << "*";
			} else {
				if(e.first.getValue() < 0) std::cout << " - ";
				else std::cout << " + ";
				if(std::abs(e.first.getValue()) != 1)
					std::cout << e.first << "*";
			}
			std::cout << this->vars[getData(e.second)].getName();
		};
		for(const auto &e: constraint.getExp().getIntElements())
			printTerm(e);
		for(const auto &e: constraint.getExp().getDoubleElements())
			printTerm(e);
		const auto bound = constraint.getBound().getValue();
		switch(constraint.getRelation()) {
		case LinConstraint::Relation::Leq:
			std::cout << " <= ";
			break;
		case LinConstraint::Relation::Eq:
			std::cout << " = ";
			break;
		case LinConstraint::Relation::Geq:
			std::cout << " >= ";
			break;
		}
		std::cout << bound << std::endl;
	}

	IloExpr expr(env);
	for(const LinExp::IntElement &e: constraint.getExp().getIntElements()) {
		assert(getData(e.second) >= 1);
		assert(getData(e.second) < vars.size());
		expr += e.first.getValue() * vars[getData(e.second)];
	}
	for(const LinExp::DoubleElement &e: constraint.getExp().getDoubleElements()) {
		assert(getData(e.second) >= 1);
		assert(getData(e.second) < vars.size());
		expr += e.first.getValue() * vars[getData(e.second)];
	}

	IloRange r(env, expr);
	switch(constraint.getRelation()) {
	case LinConstraint::Relation::Leq:
		r.setUB(constraint.getBound().getValue());
		break;
	case LinConstraint::Relation::Eq:
		r.setUB(constraint.getBound().getValue());
		r.setLB(constraint.getBound().getValue());
		break;
	case LinConstraint::Relation::Geq:
		r.setLB(constraint.getBound().getValue());
		break;
	}

	if(debugOutput) std::cout << r.getId() << "\t" << r << std::endl;
	constraints.push_back(r);
	model.add(r);
} MOD_cplexCatch

void CPLEX::popConstraintImpl() MOD_cplexTry {
	assert(constraints.size() > 0);
	IloRange r = constraints.back();
	r.end();
	constraints.pop_back();
} MOD_cplexCatch

Int CPLEX::getLBInt(Var v) const MOD_cplexTry {
	assert(getData(v) >= 1);
	assert(getData(v) < vars.size());
	IloNumVar cplexVar = vars[getData(v)];
	double value = cplexVar.getLB();
	switch(cplexVar.getType()) {
	case IloNumVar::Type::Int:
		return Int(std::lround(value));
	case IloNumVar::Type::Float:
		MOD_ABORT;
	case IloNumVar::Type::Bool:
		Int res(std::lround(value));
		if(res != 0 && res != 1) MOD_ABORT;
		return res;
	}
	MOD_ABORT;
} MOD_cplexCatch

Int CPLEX::getUBInt(Var v) const MOD_cplexTry {
	assert(getData(v) >= 1);
	assert(getData(v) < vars.size());
	IloNumVar cplexVar = vars[getData(v)];
	double value = cplexVar.getUB();
	switch(cplexVar.getType()) {
	case IloNumVar::Type::Int:
		return Int(std::lround(value));
	case IloNumVar::Type::Float:
		MOD_ABORT;
	case IloNumVar::Type::Bool:
		Int res(std::lround(value));
		if(res != 0 && res != 1) MOD_ABORT;
		return res;
	}
	MOD_ABORT;
} MOD_cplexCatch

bool CPLEX::isVariableReal(Var var) const MOD_cplexTry {
	return vars[getData(var)].getType() == IloNumVar::Type::Float;
} MOD_cplexCatch

std::string CPLEX::getVarName(Var var) const MOD_cplexTry {
	assert(getData(var) >= 1);
	assert(getData(var) < vars.size());
	const char *name = vars[getData(var)].getName();
	assert(name);
	return std::string(name);
} MOD_cplexCatch

Model::VarIter CPLEX::variablesBeginImpl() const {
	if(vars.size() == 1) return VarIter();
	else return VarIter(*this, Var(1));
}

Model::VarIter CPLEX::variablesIncrImpl(Var var) const {
	unsigned int newIndex = getData(var) + 1;
	if(newIndex >= vars.size()) return VarIter();
	else return VarIter(*this, Var(newIndex));
}

int CPLEX::getNumVariables() const {
	return vars.size();
}

int CPLEX::getNumConstraints() const {
	return constraints.size();
}

Response CPLEX::realSolve(std::ostream &log, int verbosity) MOD_cplexTry {
	setStuffBeforeSolving(log, verbosity);

	bool res = cplex.solve();
	if(!res) {
		//		std::cout << "The program --------------------------------------------------" << std::endl;
		//		std::cout << *model;
		//		std::cout << "--------------------------------------------------------------" << std::endl;
		return Response::Failed;
	}
	if(debugOutput) log << "ILP::CPLEX::realSolve: Intopt ok" << std::endl;
	IloCplex::Status stat = cplex.getCplexStatus();
	if(stat == IloCplex::AbortTimeLim) return Response::TimedOut;
	res = stat == IloCplex::Optimal;
	if(debugOutput) log << "ILP::CPLEX::realSolve: Is optimal = " << std::boolalpha << res << std::endl;
	if(res) return Response::Optimal;
	else {
		if(debugOutput) log << "ILP::CPLEX::realSolve: stat:\t" << stat << std::endl;
		return Response::Feasible;
	}
} MOD_cplexCatch

void CPLEX::pushBranchingConstraint(const LinConstraint &constraint) MOD_cplexTry {
	IloExpr expr(env);
	for(const LinExp::IntElement &e: constraint.getExp().getIntElements()) {
		assert(getData(e.second) >= 1);
		assert(getData(e.second) < vars.size());
		expr += e.first.getValue() * vars[getData(e.second)];
	}
	for(const LinExp::DoubleElement &e: constraint.getExp().getDoubleElements()) {
		MOD_ABORT;
		assert(getData(e.second) >= 1);
		assert(getData(e.second) < vars.size());
		expr += e.first.getValue() * vars[getData(e.second)];
	}

	IloRange r(env, expr);
	switch(constraint.getRelation()) {
	case LinConstraint::Relation::Leq:
		r.setUB(constraint.getBound().getValue());
		break;
	case LinConstraint::Relation::Eq:
		r.setUB(constraint.getBound().getValue());
		r.setLB(constraint.getBound().getValue());
		break;
	case LinConstraint::Relation::Geq:
		r.setLB(constraint.getBound().getValue());
		break;
	}

	if(debugOutput)
		std::cout << "ILP::CPLEX::BranchingConstraint: " << r.getId() << "\t" << r << std::endl;
	branchingConstraints.push_back(r);
	model.add(r);
} MOD_cplexCatch

void CPLEX::popBranchingConstraint() MOD_cplexTry {
	assert(!branchingConstraints.empty());
	branchingConstraints.back().end();
	branchingConstraints.pop_back();
} MOD_cplexCatch

void CPLEX::popAllBranchingConstraints() MOD_cplexTry {
	for(auto v: branchingConstraints) v.end();
	branchingConstraints.clear();
} MOD_cplexCatch

Int CPLEX::getVarValIntegral(Var var) const MOD_cplexTry {
	assert(getData(var) >= 1);
	assert(getData(var) < vars.size());
	IloNumVar cplexVar = vars[getData(var)];
	double value = cplex.getValue(cplexVar);
	switch(cplexVar.getType()) {
	case IloNumVar::Type::Int:
		return Int(std::lround(value));
	case IloNumVar::Type::Float:
		MOD_ABORT;
	case IloNumVar::Type::Bool:
		Int res(std::lround(value));
		if(res != 0 && res != 1) MOD_ABORT;
		return res;
	}
	MOD_ABORT;
} catch(const IloCplex::NotExtractedException &e) {
	std::cout << "WARNING: (M)ILP Variable " << getVarName(var) << " not constrained. Set to its lower bound." << std::endl;
	return getLBInt(var);
} MOD_cplexCatch

Float CPLEX::getVarValReal(Var var) const MOD_cplexTry {
	assert(getData(var) >= 1);
	assert(getData(var) < vars.size());
	IloNumVar cplexVar = vars[getData(var)];
	double value = cplex.getValue(cplexVar);
	switch(cplexVar.getType()) {
	case IloNumVar::Type::Int:
		MOD_ABORT;
	case IloNumVar::Type::Float:
		return value;
	case IloNumVar::Type::Bool:
		MOD_ABORT;
	}
	MOD_ABORT;
} catch(const IloCplex::NotExtractedException &e) {
	std::cout << "WARNING: (M)ILP Variable " << getVarName(var) << " not constrained. Set to its lower bound." << std::endl;
	return vars[getData(var)].getLB();
} MOD_cplexCatch

void CPLEX::setStuffBeforeSolving(std::ostream &log, int verbosity) MOD_cplexTry {
	if(verbosity > 0) {
		env.setError(log);
		env.setOut(log);
		env.setWarning(log);
		cplex.setError(log);
		cplex.setOut(log);
		cplex.setWarning(log);
	} else {
		env.setError(IO::nullStream());
		env.setOut(IO::nullStream());
		env.setWarning(IO::nullStream());
		cplex.setError(IO::nullStream());
		cplex.setOut(IO::nullStream());
		cplex.setWarning(IO::nullStream());
	}
	cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, 0);
	cplex.setParam(IloCplex::Param::TimeLimit, getConfig().ilp.timeLimit);
	cplex.setParam(IloCplex::Param::Threads, getConfig().common.numThreads);
	cplex.setParam(IloCplex::Param::Emphasis::MIP, IloCplex::MIPEmphasisOptimality);
	if(getConfig().ilp.useDFS)
		cplex.setParam(IloCplex::Param::MIP::Strategy::NodeSelect, IloCplex::NodeSelect::DFS);
	else
		cplex.setParam(IloCplex::Param::MIP::Strategy::NodeSelect, IloCplex::NodeSelect::BestBound);
} MOD_cplexCatch

Int CPLEX::getObjValIntegral() const MOD_cplexTry {
	if(!getIsObjectiveFunctionIntegral()) MOD_ABORT;
	return std::lround(cplex.getObjValue());
} MOD_cplexCatch

Float CPLEX::getObjValReal() const MOD_cplexTry {
	if(getIsObjectiveFunctionIntegral()) MOD_ABORT;
	return cplex.getObjValue();
} MOD_cplexCatch

void CPLEX::write(const std::string &filename) const {
	try {
		cplex.exportModel(filename.c_str());
	} catch(const IloException &e) {
		throw LogicError(std::string("Writing MILP model from CPLEX failed: ") + e.getMessage());
	}
}

} // namespace mod::lib::ILP

#endif // MOD_HAVE_CPLEX