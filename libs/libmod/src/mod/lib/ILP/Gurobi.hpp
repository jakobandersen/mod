#ifndef MOD_LIB_ILP_GUROBI_HPP
#define MOD_LIB_ILP_GUROBI_HPP

#include <mod/BuildConfig.hpp>

#ifdef MOD_HAVE_GUROBI
#include <mod/lib/ILP/ILP.hpp>

#include <gurobi_c++.h>

namespace mod::lib::ILP {

struct Gurobi final : public Model {
	Gurobi();
	virtual ~Gurobi();
private: // construction
	Var addVariable(const std::string &name, char type);
	// The returned variable must have bounds [0; infinity[
	virtual Var addVariableFloatImpl(const std::string &name) override;
	// The returned variable must have bounds [0; infinity[
	virtual Var addVariableIntegerImpl(const std::string &name) override;
	virtual Var addVariableBinaryImpl(const std::string &name) override;
	virtual void setLBImpl(Var v, double bound) override;
	virtual void setUBImpl(Var v, double bound) override;
	virtual void setBranchPriorityImpl(Var var, unsigned int prio) override;
	virtual void setObjectiveFunctionImpl(const LinExp &exp) override;
	virtual void addConstraintImpl(const LinConstraint &constraint) override;
	virtual void popConstraintImpl() override;
public: // model querying
	virtual Int getLBInt(Var v) const override;
	virtual Int getUBInt(Var v) const override;
	virtual bool isVariableReal(Var var) const override;
	virtual std::string getVarName(Var var) const override;
public:
	virtual int getNumVariables() const override;
	virtual int getNumConstraints() const override;
private:
	virtual VarIter variablesBeginImpl() const override;
	virtual VarIter variablesIncrImpl(Var var) const override;
public: // calculation
	virtual Response realSolve(std::ostream &log, int verbosity) override;
	// solutionReady must return (continueSearch, solutionId)
private: // calculation
	virtual void pushBranchingConstraint(const LinConstraint &constraint) override;
	virtual void popBranchingConstraint() override;
	virtual void popAllBranchingConstraints() override;
public: // calculated data querying
	virtual Int getVarValIntegral(Var var) const override;
	virtual Float getVarValReal(Var var) const override;
	virtual Int getObjValIntegral() const override;
	virtual Float getObjValReal() const override;
private:
	GRBEnv env;
	GRBModel model;
	std::vector<GRBVar> vars;
	int numConstraints = 0;
public:
	virtual void write(const std::string &filename) const override;
};

} // namespace mod::lib::ILP

#endif // MOD_HAVE_GUROBI

#endif // MOD_LIB_ILP_GUROBI_HPP