include("xxx_common.py")

avail = ["Gurobi", "CPLEX", "CBC-CLP"]
res = getAvailableILPSolvers()
for s in res:
	assert s in avail
assert config.ilp.solver in res + ["default"]
