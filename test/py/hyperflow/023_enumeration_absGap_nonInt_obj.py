include("xx5_solutions.py")

makeDG("A -> B")

print("#" * 80)
print("Integral enum vars, non-integral objective function --- 1 solution")
print("#" * 80)
def makeFlow():
	flow = hyperflow.Model(dg)
	flow.addSource(A)
	flow.addSink(B)
	flow.objectiveFunction = 1.1 * edgeFlow
	flow.absGap = 2**30
	return flow
flow = makeFlow()
flow.findSolutions(maxNumSolutions=1, verbosity=10, ilpVerbosity=0)
print("=" * 80)
print("Second solution")
flow.findSolutions(maxNumSolutions=1, verbosity=10, ilpVerbosity=0)
print("=" * 80)
print("Checked")
flow = makeFlow()
checkSolutions(flow, [
	[],
	[
		(inFlow, A, 1),
		(outFlow, B, 1),
		(edgeFlow, dg.findEdge([A], [B]), 1),
	]
], maxNumSolutions=2, old=False, objFunc=1.1 * edgeFlow)


print("#" * 80)
print("Integral enum vars, non-integral objective function --- multiple solutions")
print("#" * 80)
flow = makeFlow()
flow.findSolutions(maxNumSolutions=2, verbosity=10, ilpVerbosity=0)
