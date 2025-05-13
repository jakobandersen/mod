include("xx5_solutions.py")

makeDG("A -> B")

print("#" * 80)
print("Non-integral enum vars, integral objective function --- 1 solution")
print("#" * 80)
flow = hyperflow.Model(dg)
flow.addSource(A)
flow.addSink(B)
flow.relaxed = True
flow.addEnumerationVar(edgeFlow)
flow.objectiveFunction = hyperflow.LinExp()
flow.findSolutions(maxNumSolutions=1, verbosity=10, ilpVerbosity=0)
print("=" * 80)
print("Second solution")
try:
	flow.findSolutions(maxNumSolutions=1, verbosity=10, ilpVerbosity=0)
	assert False
except LogicError as e:
	msg = str(e)
	print(msg)
	assert msg == "Can not enumerate solutions over non-integral variable 'edge(2)' of type Float."
print("=" * 80)
print("Checked")
flow = hyperflow.Model(dg)
flow.addSource(A)
flow.addSink(B)
flow.relaxed = True
flow.addEnumerationVar(edgeFlow)
flow.objectiveFunction = hyperflow.LinExp()
checkSolutions(flow, [[
]])


print("#" * 80)
print("Non-integral enum vars, integral objective function --- multiple solutions")
print("#" * 80)
flow = hyperflow.Model(dg)
flow.addSource(A)
flow.addSink(B)
flow.relaxed = True
flow.addEnumerationVar(edgeFlow)
flow.objectiveFunction = hyperflow.LinExp()
try:
	flow.findSolutions(maxNumSolutions=2, verbosity=10, ilpVerbosity=0)
	assert False
except LogicError as e:
	msg = str(e)
	print(msg)
	assert msg == "Can not enumerate solutions over non-integral variable 'edge(2)' of type Float."


print("#" * 80)
print("Integral enum vars, non-integral objective function --- 1 solution")
print("#" * 80)
flow = hyperflow.Model(dg)
flow.addSource(A)
flow.addSink(B)
flow.objectiveFunction = 1.1 * edgeFlow
flow.findSolutions(maxNumSolutions=1, verbosity=10, ilpVerbosity=0)
print("=" * 80)
print("Second solution")
flow.findSolutions(maxNumSolutions=1, verbosity=10, ilpVerbosity=0)
print("=" * 80)
print("Checked")
flow = hyperflow.Model(dg)
flow.addSource(A)
flow.addSink(B)
flow.objectiveFunction = 1.1 * edgeFlow
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
flow = hyperflow.Model(dg)
flow.addSource(A)
flow.addSink(B)
flow.objectiveFunction = 1.1 * edgeFlow
flow.findSolutions(maxNumSolutions=2, verbosity=10, ilpVerbosity=0)
