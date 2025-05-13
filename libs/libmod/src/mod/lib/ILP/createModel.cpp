#include "ILP.hpp"

#include <mod/Config.hpp>

#include <mod/lib/ILP/CPLEX.hpp>
#include <mod/lib/ILP/CBC.hpp>
#include <mod/lib/ILP/Gurobi.hpp>
#include <mod/lib/IO/IO.hpp>

#ifdef MOD_HAVE_CBC
#include <OsiClpSolverInterface.hpp>
#endif

namespace mod::lib::ILP {
namespace {

// The order of the solvers should be the same in all three functions.

std::unique_ptr<Model> createDefaultSolver() {
#if defined(MOD_HAVE_CPLEX)
	return std::make_unique<CPLEX>();
#elif defined(MOD_HAVE_GUROBI)
	return std::make_unique<Gurobi>();
#elif defined(MOD_HAVE_CBC)
	return std::make_unique<CBC>(std::make_unique<OsiClpSolverInterface>());
#else
	throw LogicError("No ILP solver bindings available.");
#endif
}

} // namespace

std::vector<std::string> Model::getAvailableSolvers() {
	return {
#ifdef MOD_HAVE_CPLEX
		"CPLEX",
#endif
#ifdef MOD_HAVE_GUROBI
		"Gurobi",
#endif
#ifdef MOD_HAVE_CBC
		"CBC-CLP",
#endif
	};
}

std::unique_ptr<Model> Model::createModel(std::string solver) {
	if(solver == "default") {
		solver = getConfig().ilp.solver;
		if(solver == "default")
			return createDefaultSolver();
	}
	if(solver == "Gurobi") {
#ifndef MOD_HAVE_GUROBI
		throw LogicError("Can not create Gurobi solver. Rebuild with Gurobi enabled.");
#else
		return std::make_unique<Gurobi>();
#endif
	} else if(solver == "CPLEX") {
#ifndef MOD_HAVE_CPLEX
		throw LogicError("Can not create CPLEX solver. Rebuild with CPLEX enabled.");
#else
		return std::make_unique<CPLEX>();
#endif
	} else if(solver == "CBC-CLP") {
#ifndef MOD_HAVE_CBC
		throw LogicError("Can not create CBC-CLP solver. Rebuild with CBC enabled.");
#else
		return std::make_unique<CBC>(std::make_unique<OsiClpSolverInterface>());
#endif
	} else {
		throw LogicError("No bindings for ILP solver '" + solver + "'.");
	}
}

} // namespace mod::lib::ILP