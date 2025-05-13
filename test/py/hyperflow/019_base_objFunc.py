include("xx5_solutions.py")

makeFlow("A -> B")
obj = edgeFlow + edgeFlow
flow.addSource(A)
flow.addSink(B)
flow.addConstraint(edgeFlow >= 1)
flow.objectiveFunction = obj
checkSolutions(flow, [[
	(inFlow, A, 1),
	(outFlow, B, 1),
	(edgeFlow, next(iter(dg.edges)), 1),
]], objFunc=obj)
