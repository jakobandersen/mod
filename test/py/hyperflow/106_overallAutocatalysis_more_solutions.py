include("xx5_solutions.py")
include("101_overallAutocatalysis_var_spec.py")

# BFSExclusive not enough
# =================================================================
makeFlow("A + C1 <=> C + C2  C <=> B  C1 <=> C2")
flow.addSource(A)
flow.addSink(B)
flow.addSource(C)
flow.addSink(C)
flow.addConstraint(inFlow[A] == 2)
flow.addConstraint(outFlow[B] == 1)
flow.overallAutocatalysis.enable()
assert flow.overallAutocatalysis.bfsExclusive
checkSolutions(flow, [[
	(inFlow, A, 2),
	(inFlow, C, 1),
	(outFlow, C, 2),
	(outFlow, B, 1),
	(isOverallAutocata, C, 1),
	(edgeFlow, dg.findEdge([A, C1], [C, C2]), 2),
	(edgeFlow, dg.findEdge([C], [B]), 1),
	(edgeFlow, dg.findEdge([C2], [C1]), 2),
]], maxNumSolutions=2)
