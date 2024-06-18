include("grammar.py")

strat = (addUniverse(water) >> addSubset(ribuloseP)
	>> aldoKetoF >> transKeto
	>> [
		transAldo >> transKeto,
		aldoKetoB >> aldolase >> phosphohydro
	])
dg = DG(graphDatabase=inputGraphs)
dg.build().execute(strat)
