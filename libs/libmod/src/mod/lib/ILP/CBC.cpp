#include "CBC.hpp"

#ifdef MOD_HAVE_CBC

#include <mod/Config.hpp>
#include <mod/lib/IO/IO.hpp>

#include <CoinPackedVector.hpp>

namespace mod::lib::ILP {
#define MISSING_IMPL() std::cout << "CBC::" << __func__ << ": missing implementation." << std::endl

CBC::CBC(std::unique_ptr<OsiSolverInterface> p) : orig(std::move(p)) {
	orig->setIntParam(OsiNameDiscipline, 1);
}

CBC::~CBC() = default;

Var CBC::addVariable(const std::string &name) {
	// Apparently OSI only reallocates the variable type info once(!)
	// So we only set the variable type info after all additions are done.
	//	std::cout << __func__ << ": " << name << ", type=" << int(type) << std::endl;
	const auto colNum = orig->getNumCols();
	CoinPackedVector v;
	std::string clpName;
	clpName.reserve(name.size());
	for(const char c: name) {
		if(c == ' ') clpName.push_back('_');
		else if(c == '<') clpName.push_back('l');
		else if(c == '>') clpName.push_back('g');
		else clpName.push_back(c);
	}
	orig->addCol(v, 0, COIN_INT_MAX_AS_DOUBLE, 0,
	             initVariableName(clpName, Var(colNum)));
	return Var(colNum);
}

Var CBC::addVariableFloatImpl(const std::string &name) {
	variableType.push_back(0);
	return addVariable(name);
}

Var CBC::addVariableIntegerImpl(const std::string &name) {
	variableType.push_back(2);
	return addVariable(name);
}

Var CBC::addVariableBinaryImpl(const std::string &name) {
	variableType.push_back(1);
	const auto var = addVariable(name);
	setLB(var, 0);
	setUB(var, 1);
	return var;
}

void CBC::setLBImpl(Var v, double bound) {
	orig->setColLower(getData(v), bound);
}

void CBC::setUBImpl(Var v, double bound) {
	orig->setColUpper(getData(v), bound);
}

void CBC::setBranchPriorityImpl(Var var, unsigned int prio) {
	MISSING_IMPL();
}

void CBC::setObjectiveFunctionImpl(const LinExp &exp) {
	std::vector<double> values(variableType.size(), 0.0);
	for(const auto &e: exp.getDoubleElements())
		values[getData(e.second)] += e.first.getValue();
	for(const auto &e: exp.getIntElements())
		values[getData(e.second)] += e.first.getValue();
	for(int i = 0; i != values.size(); ++i)
		if(values[i] != 0)
			orig->setObjCoeff(i, values[i]);
}

void CBC::addConstraintImpl(const LinConstraint &constraint) {
	std::vector<int> idxs;
	std::vector<double> coefs;
	{
		const auto num = constraint.getExp().getDoubleElements().size()
		                 + constraint.getExp().getIntElements().size();
		idxs.reserve(num);
		coefs.reserve(num);
	}
	std::vector<int> offset(orig->getNumCols(), -1);
	for(const auto &e: constraint.getExp().getIntElements()) {
		const auto colId = getData(e.second);
		assert(colId < orig->getNumCols());
		if(offset[colId] == -1) {
			offset[colId] = idxs.size();
			idxs.push_back(colId);
			coefs.push_back(e.first.getValue());
		} else {
			coefs[offset[colId]] += e.first.getValue();
		}
	}
	for(const auto &e: constraint.getExp().getDoubleElements()) {
		const auto colId = getData(e.second);
		assert(colId < orig->getNumCols());
		if(offset[colId] == -1) {
			offset[colId] = idxs.size();
			idxs.push_back(colId);
			coefs.push_back(e.first.getValue());
		} else {
			coefs[offset[colId]] += e.first.getValue();
		}
	}
	const auto bound = constraint.getBound().getValue();
	if(debugOutput) {
		std::cout << "ILP::CBC::addConstraint(" << orig->getNumRows() << "): ";
		for(int i = 0; i != idxs.size(); ++i) {
			if(i == 0) {
				if(coefs[i] == -1) std::cout << "-";
				else if(coefs[i] != 1) std::cout << coefs[i] << "*";
			} else {
				if(coefs[i] < 0) std::cout << " - ";
				else std::cout << " + ";
				if(std::abs(coefs[i]) != 1)
					std::cout << coefs[i] << "*";
			}
			std::cout << orig->getColName(idxs[i]);
		}
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
	switch(constraint.getRelation()) {
	case LinConstraint::Relation::Leq:
		orig->addRow(idxs.size(), idxs.data(), coefs.data(), -orig->getInfinity(), bound);
		break;
	case LinConstraint::Relation::Eq:
		orig->addRow(idxs.size(), idxs.data(), coefs.data(), bound, bound);
		break;
	case LinConstraint::Relation::Geq:
		orig->addRow(idxs.size(), idxs.data(), coefs.data(), bound, orig->getInfinity());
		break;
	}
}

void CBC::popConstraintImpl() {
	MISSING_IMPL();
}

Int CBC::getLBInt(Var v) const {
	assert(getData(v) < orig->getNumCols());
	const double value = orig->getColLower()[getData(v)];
	switch(variableType[getData(v)]) {
	case 0: // real
		MOD_ABORT;
	case 1: // bool
	{
		Int res(std::lround(value));
		if(res != 0 && res != 1) MOD_ABORT;
		return res;
	}
	case 2: // int
		return Int(std::lround(value));
	}
	MOD_ABORT;
}

Int CBC::getUBInt(Var v) const {
	assert(getData(v) < orig->getNumCols());
	const double value = orig->getColUpper()[getData(v)];
	switch(variableType[getData(v)]) {
	case 0: // real
		MOD_ABORT;
	case 1: // bool
	{
		Int res(std::lround(value));
		if(res != 0 && res != 1) MOD_ABORT;
		return res;
	}
	case 2: // int
		return Int(std::lround(value));
	}
	MOD_ABORT;
}

bool CBC::isVariableReal(Var var) const {
	assert(getData(var) < orig->getNumCols());
	return variableType[getData(var)] == 0;
}

std::string CBC::getVarName(Var var) const {
	assert(getData(var) < orig->getNumCols());
	return orig->getColName(getData(var));
}

int CBC::getNumVariables() const {
	return variableType.size();
}

int CBC::getNumConstraints() const {
	return orig->getNumRows();
}

CBC::VarIter CBC::variablesBeginImpl() const {
	return VarIter(*this, Var(0));
}

CBC::VarIter CBC::variablesIncrImpl(Var var) const {
	if(getData(var) + 1 == orig->getNumCols()) return VarIter();
	else return VarIter(*this, Var(getData(var) + 1));
}

Response CBC::realSolve(std::ostream &log, int verbosity) {
	struct MessageHandler : CoinMessageHandler {
		MessageHandler(std::ostream &log) : log(log) {}
		virtual MessageHandler *clone() const { MOD_ABORT; }
		virtual int print() {
			log << messageBuffer_ << std::endl;
			return 0;
		}
	private:
		std::ostream &log;
	} messageHandler(log);
	for(int i = 0; i != variableType.size(); ++i) {
		if(variableType[i] != 0)
			orig->setInteger(i); // this seems needed
		// but is this?
		//orig->setColumnType(i, variableType[i]);
	}
	model.reset(new CbcModel(*orig));
	model->passInMessageHandler(&messageHandler); // before setLogLevel
	model->setLogLevel(verbosity == 0 ? 0 : 1);
	model->branchAndBound(verbosity == 0 ? 0 : 1);
	switch(model->status()) {
	case -1:
		MOD_ABORT;
	case 0:
		if(model->isProvenOptimal()) {
			return Response::Optimal;
		} else if(model->isProvenInfeasible()) {
			return Response::Failed;
		} else {
			log << "WARNING: CBC status is 0. Not proven optimal, not proven infeasible. Assuming infeasible."
			    << std::endl;
			return Response::Failed;
		}
	case 1:
		return Response::TimedOut;
	case 2: // actually: "difficulties so run was abandoned"
	case 5: // actually: "event user programmed event occurred"
	default: // can others happen?
		log << "WARNING: CBC status is " << model->status() << ". Assuming 'TIMEOUT'." << std::endl;
		return Response::TimedOut;
	}
}

void CBC::pushBranchingConstraint(const LinConstraint &constraint) {
	MISSING_IMPL();
}

void CBC::popBranchingConstraint() {
	MISSING_IMPL();
}

void CBC::popAllBranchingConstraints() {
	MISSING_IMPL();
}

Int CBC::getVarValIntegral(Var var) const {
	assert(getData(var) < model->getNumCols());
	const double value = model->getColSolution()[getData(var)];
	// the CbcModel may change the column types so query the original model
	switch(variableType[getData(var)]) {
	case 0: // real
		MOD_ABORT;
	case 1: // bool
	{
		Int res(std::lround(value));
		if(res != 0 && res != 1) MOD_ABORT;
		return res;
	}
	case 2: // int
		return Int(std::lround(value));
	}
	MOD_ABORT;
}

Float CBC::getVarValReal(Var var) const {
	assert(isVariableReal(var));
	assert(getData(var) < model->getNumCols());
	return model->getColSolution()[getData(var)];
}

Int CBC::getObjValIntegral() const {
	if(!getIsObjectiveFunctionIntegral()) MOD_ABORT;
	return std::lround(model->getObjValue());
}

Float CBC::getObjValReal() const {
	return model->getObjValue();
}

void CBC::write(const std::string &filename) const {
	const auto pos = filename.find_last_of('.');
	if(pos == std::string::npos)
		throw LogicError("Writing MILP model from CBC failed: could not find file extension, missing '.'.");
	std::string extension = filename.substr(pos + 1);
	if(extension == "lp") {
		orig->writeLp(filename.c_str());
	} else if(extension == "mps") {
		orig->writeMps(filename.c_str());
	} else {
		throw LogicError("Writing MILP model from CBC failed: file format '"
			+ extension + "' not know. Must be 'lp' or 'mps'.");
	}
}

} // namespace mod::lib::ILP

#endif // MOD_HAVE_CBC