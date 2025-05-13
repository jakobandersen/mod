#include "Gurobi.hpp"

#ifdef MOD_HAVE_GUROBI

#define MOD_WITH_GUROBI_TRY_CATCH
#ifdef MOD_WITH_GUROBI_TRY_CATCH
#define MOD_gurobiTry try
#define MOD_gurobiCatch catch(const GRBException &e) {   \
        std::string msg = "Exception from Gurobi(error code=" + std::to_string(e.getErrorCode()) + ": ";     \
        throw mod::FatalError(msg + e.getMessage(), 0); \
    }
#else
#define MOD_gurobiTry
#define MOD_gurobiCatch
#endif

#include <mod/Config.hpp>
#include <mod/lib/IO/IO.hpp>

#include <gurobi_c++.h>

namespace mod::lib::ILP {
#define MISSING_IMPL() std::cout << "Gurobi::" << __func__ << ": missing implementation." << std::endl

Gurobi::Gurobi() MOD_gurobiTry: model(env) {} MOD_gurobiCatch

Gurobi::~Gurobi() = default;

Var Gurobi::addVariable(const std::string &name, char type) {
	Var vExternal(vars.size());
	std::string fullName = initVariableName(name, vExternal);
	const auto grbVar = model.addVar(0, GRB_MAXINT, 0, type, fullName);
	model.update();
	vars.push_back(grbVar);
	if(debugOutput)
		std::cout << "ILP::Gurobi::addVariable(" << name << ", " << type << "): " << getData(vExternal) << std::endl;
	return vExternal;
}


Var Gurobi::addVariableFloatImpl(const std::string &name) {
	return addVariable(name, GRB_CONTINUOUS);
}

Var Gurobi::addVariableIntegerImpl(const std::string &name) {
	return addVariable(name, GRB_INTEGER);
}

Var Gurobi::addVariableBinaryImpl(const std::string &name) {
	return addVariable(name, GRB_BINARY);
}

void Gurobi::setLBImpl(Var v, double bound) {
	assert(getData(v) < model.get(GRB_IntAttr_NumVars));
	auto &var = vars[getData(v)];
	var.set(GRB_DoubleAttr_LB, bound);
	model.update();
	if(debugOutput) std::cout << "ILP::Gurobi::setLB(" << getData(v) << ", " << bound << ")" << std::endl;
}

void Gurobi::setUBImpl(Var v, double bound) {
	assert(getData(v) < model.get(GRB_IntAttr_NumVars));
	auto &var = vars[getData(v)];
	var.set(GRB_DoubleAttr_UB, bound);
	model.update();
	if(debugOutput) std::cout << "ILP::Gurobi::setUB(" << getData(v) << ", " << bound << ")" << std::endl;
}

void Gurobi::setBranchPriorityImpl(Var var, unsigned int prio) {
	MISSING_IMPL();
}

void Gurobi::setObjectiveFunctionImpl(const LinExp &exp) {
	GRBLinExpr expr;
	for(const LinExp::IntElement &e: exp.getIntElements()) {
		assert(getData(e.second) < vars.size());
		expr += e.first.getValue() * vars[getData(e.second)];
	}
	for(const LinExp::DoubleElement &e: exp.getDoubleElements()) {
		assert(getData(e.second) < vars.size());
		expr += e.first.getValue() * vars[getData(e.second)];
	}
	if(debugOutput)
		std::cout << "ILP::Gurobi::setObjectiveFunctionImpl(" << expr << ")" << std::endl;
	model.setObjective(std::move(expr));
	model.update();
}

void Gurobi::addConstraintImpl(const LinConstraint &constraint) {
	GRBLinExpr expr;
	for(const LinExp::IntElement &e: constraint.getExp().getIntElements()) {
		assert(getData(e.second) < vars.size());
		expr += e.first.getValue() * vars[getData(e.second)];
	}
	for(const LinExp::DoubleElement &e: constraint.getExp().getDoubleElements()) {
		assert(getData(e.second) < vars.size());
		expr += e.first.getValue() * vars[getData(e.second)];
	}

	char sense = 0;
	switch(constraint.getRelation()) {
	case LinConstraint::Relation::Leq:
		sense = GRB_LESS_EQUAL;
		break;
	case LinConstraint::Relation::Eq:
		sense = GRB_EQUAL;
		break;
	case LinConstraint::Relation::Geq:
		sense = GRB_GREATER_EQUAL;
		break;
	}
	if(debugOutput)
		std::cout << "ILP::Gurobi::addConstraintImpl(" << expr << ", " << sense << ", "
		          << constraint.getBound().getValue() << ")" << std::endl;
	model.addConstr(std::move(expr), sense, constraint.getBound().getValue());
	model.update();
	++numConstraints;
}

void Gurobi::popConstraintImpl() {
	MISSING_IMPL();
}

Int Gurobi::getLBInt(Var v) const {
	assert(getData(v) < model.get(GRB_IntAttr_NumVars));
	const auto &var = vars[getData(v)];
	const double value = var.get(GRB_DoubleAttr_LB);
	switch(var.get(GRB_CharAttr_VType)) {
	case GRB_CONTINUOUS:
		MOD_ABORT;
	case GRB_BINARY: {
		Int res(std::lround(value));
		if(res != 0 && res != 1) MOD_ABORT;
		return res;
	}
	case GRB_INTEGER:
		return Int(std::lround(value));
	}
	MOD_ABORT;
}

Int Gurobi::getUBInt(Var v) const {
	assert(getData(v) < model.get(GRB_IntAttr_NumVars));
	const auto &var = vars[getData(v)];
	const double value = var.get(GRB_DoubleAttr_UB);
	switch(var.get(GRB_CharAttr_VType)) {
	case GRB_CONTINUOUS:
		MOD_ABORT;
	case GRB_BINARY: {
		Int res(std::lround(value));
		if(res != 0 && res != 1) MOD_ABORT;
		return res;
	}
	case GRB_INTEGER:
		return Int(std::lround(value));
	}
	MOD_ABORT;
}

bool Gurobi::isVariableReal(Var var) const {
	assert(getData(var) < model.get(GRB_IntAttr_NumVars));
	return vars[getData(var)].get(GRB_CharAttr_VType) == GRB_CONTINUOUS;
}

std::string Gurobi::getVarName(Var var) const {
	assert(getData(var) < model.get(GRB_IntAttr_NumVars));
	return vars[getData(var)].get(GRB_StringAttr_VarName);
}

int Gurobi::getNumVariables() const {
	return vars.size();
}

int Gurobi::getNumConstraints() const {
	return numConstraints;
}

Gurobi::VarIter Gurobi::variablesBeginImpl() const {
	return VarIter(*this, Var(0));
}

Gurobi::VarIter Gurobi::variablesIncrImpl(Var var) const {
	if(getData(var) + 1 == model.get(GRB_IntAttr_NumVars)) return VarIter();
	else return VarIter(*this, Var(getData(var) + 1));
}

Response Gurobi::realSolve(std::ostream &log, int verbosity) {
	if(verbosity > 0)
		model.set(GRB_IntParam_OutputFlag, 1);
	else
		model.set(GRB_IntParam_OutputFlag, 0);
	model.optimize();
	const auto status = model.get(GRB_IntAttr_Status);
	switch(status) {
	case GRB_OPTIMAL:
		return Response::Optimal;
	case GRB_INFEASIBLE:
		return Response::Failed;
	case GRB_TIME_LIMIT:
		return Response::TimedOut;
	default:
		throw FatalError("Gurobi optimization gave unknown status " + std::to_string(status) + ".");
	}
}

void Gurobi::pushBranchingConstraint(const LinConstraint &constraint) {
	MISSING_IMPL();
}

void Gurobi::popBranchingConstraint() {
	MISSING_IMPL();
}

void Gurobi::popAllBranchingConstraints() {
	MISSING_IMPL();
}

Int Gurobi::getVarValIntegral(Var v) const {
	assert(getData(v) < model.get(GRB_IntAttr_NumVars));
	const auto &var = vars[getData(v)];
	const double value = var.get(GRB_DoubleAttr_X);
	switch(var.get(GRB_CharAttr_VType)) {
	case GRB_CONTINUOUS:
		MOD_ABORT;
	case GRB_BINARY: {
		Int res(std::lround(value));
		if(res != 0 && res != 1) MOD_ABORT;
		return res;
	}
	case GRB_INTEGER:
		return Int(std::lround(value));
	}
	MOD_ABORT;
}

Float Gurobi::getVarValReal(Var var) const {
	assert(isVariableReal(var));
	assert(getData(var) < model.get(GRB_IntAttr_NumVars));
	return vars[getData(var)].get(GRB_DoubleAttr_X);
}

Int Gurobi::getObjValIntegral() const {
	if(!getIsObjectiveFunctionIntegral()) MOD_ABORT;
	return std::lround(model.get(GRB_DoubleAttr_ObjVal));
}

Float Gurobi::getObjValReal() const {
	return model.get(GRB_DoubleAttr_ObjVal);
}

void Gurobi::write(const std::string &filename) const {
	auto &m = const_cast<GRBModel &>(model);
	try {
		m.write(filename);
	} catch(const GRBException &e) {
		throw LogicError("Writing MILP model from Gurobi failed (error code="
			+ std::to_string(e.getErrorCode()) + "): " + e.getMessage());
	}
}

} // namespace mod::lib::ILP

#endif // MOD_HAVE_GUROBI