include("xx5_solutions.py")

# Multiedges
# =================================================================
makeFlow("2 A -> B B -> 2 C")
flow.addSource(A)
flow.addSink(C)
flow.addConstraint(inFlow[A] == 2)
checkSolutions(flow, [[
	(inFlow, A, 2),
	(outFlow, C, 2),
	(edgeFlow, dg.findEdge([A, A], [B]), 1),
	(edgeFlow, dg.findEdge([B], [C, C]), 1),
]])
