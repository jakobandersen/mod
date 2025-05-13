include("xx5_solutions.py")

makeFlow("A -> B")
checkSolutions(flow, [[]])

makeFlow("A -> B")
flow.addConstraint(edgeFlow >= 1)
checkSolutions(flow, [])

# Sources/Sinks
# =================================================================
makeFlow("A -> B")
flow.addSource(A)
flow.addConstraint(edgeFlow >= 1)
checkSolutions(flow, [])

makeFlow("A -> B")
flow.addSink(B)
flow.addConstraint(edgeFlow >= 1)
checkSolutions(flow, [])

makeFlow("A -> B")
flow.addSource(A)
flow.addSink(B)
flow.addConstraint(edgeFlow >= 1)
checkSolutions(flow, [[
	(inFlow, A, 1),
	(outFlow, B, 1),
	(edgeFlow, next(iter(dg.edges)), 1),
]])

# SeparateIOInternalTransit
# =================================================================
if False:
	post.enableInvokeMake()
	makeFlow("A -> B B -> C")
	flow.addSource(A)
	flow.addSink(C)
	flow.calc()
	flow.implementationView.printExpandedVertex(B)
	makeFlow("A -> B B -> C")
	flow.addSource(A)
	flow.addSink(C)
	flow.separateIOInternalTransit(B)
	flow.calc()
	flow.implementationView.printExpandedVertex(B)
makeFlow("A -> B B -> C")
flow.addSource(A)
flow.addSink(C)
flow.addConstraint(inFlow[A] == 1)
checkSolutions(flow, [[
	(inFlow, A, 1),
	(outFlow, C, 1),
	(edgeFlow, dg.findEdge([A], [B]), 1),
	(edgeFlow, dg.findEdge([B], [C]), 1),
]])

makeFlow("A -> B B -> C")
flow.addSource(A)
flow.addSink(C)
flow.addConstraint(inFlow[A] == 1)
flow.separateIOInternalTransit(B)
checkSolutions(flow, [[
	(inFlow, A, 1),
	(outFlow, C, 1),
	(edgeFlow, dg.findEdge([A], [B]), 1),
	(edgeFlow, dg.findEdge([B], [C]), 1),
]])

makeFlow("A -> B B -> C")
flow.addSource(A)
flow.addSink(C)
flow.addConstraint(inFlow[A] == 1)
flow.separateIOInternalTransit(B)
flow.addConstraint(transitInternalFlow[B] == 0)
checkSolutions(flow, [])



# AllowHyperLoops
# =================================================================
makeFlow("A -> A")
assert not flow.allowHyperLoops
flow.addSource(A)
flow.addSink(A)
flow.addConstraint(edgeFlow >= 1)
checkSolutions(flow, [])

makeFlow("A -> A")
flow.allowHyperLoops = True
flow.addSource(A)
flow.addSink(A)
flow.addConstraint(edgeFlow >= 1)
checkSolutions(flow, [[
	(inFlow, A, 1),
	(outFlow, A, 1),
	(edgeFlow, next(iter(dg.edges)), 1),
]])

# AllowReversal
# =================================================================
makeFlow("A <=> B")
assert not flow.allowReversal
flow.addConstraint(edgeFlow >= 1)
checkSolutions(flow, [])

makeFlow("A <=> B")
flow.allowReversal = True
flow.addConstraint(edgeFlow >= 1)
checkSolutions(flow, [[
	(edgeFlow, dg.findEdge([A], [B]), 1),
	(edgeFlow, dg.findEdge([B], [A]), 1),
]])

# AllowIOReversal
# =================================================================
makeFlow("A -> B")
flow.allowIOReversal = False
flow.addSource(A)
flow.addSink(A)
flow.addConstraint(inFlow >= 1)
checkSolutions(flow, [])

makeFlow("A -> B")
assert flow.allowIOReversal
flow.addSource(A)
flow.addSink(A)
flow.addConstraint(inFlow >= 1)
checkSolutions(flow, [[
	(inFlow, A, 1),
	(outFlow, A, 1),
]])

# Relaxed
# =================================================================
makeFlow("""
	A + B -> G
	B + C -> G
	C + A -> G
""")
assert not flow.relaxed
for a in [A, B, C]:
	flow.addSource(a)
	flow.addConstraint(inFlow[a] <= 1)
flow.addSink(G)
flow.objectiveFunction = -outFlow[G]
checkSolutions(flow, [
	[
		(outFlow, G, 1),
		(inFlow, B, 1),
		(inFlow, C, 1),
		(edgeFlow, dg.findEdge([B, C], [G]), 1),
	],
	[
		(outFlow, G, 1),
		(inFlow, A, 1),
		(inFlow, B, 1),
		(edgeFlow, dg.findEdge([A, B], [G]), 1),
	],
	[
		(outFlow, G, 1),
		(inFlow, A, 1),
		(inFlow, C, 1),
		(edgeFlow, dg.findEdge([A, C], [G]), 1),
	],
	[ ],
], maxNumSolutions=5, objFunc=-outFlow[G])

makeFlow("""
	A + B -> G
	B + C -> G
	C + A -> G
""")
flow.relaxed = True
for a in [A, B, C]:
	flow.addSource(a)
	flow.addConstraint(inFlow[a] <= 1)
flow.addSink(G)
flow.objectiveFunction = -outFlow[G]
checkSolutions(flow, [
	[
		(outFlow, G, 1.5),
		(inFlow, A, 1.),
		(inFlow, B, 1.),
		(inFlow, C, 1.),
		(edgeFlow, dg.findEdge([A, B], [G]), 0.5),
		(edgeFlow, dg.findEdge([B, C], [G]), 0.5),
		(edgeFlow, dg.findEdge([C, A], [G]), 0.5),
	],
], objFunc=-outFlow[G])

# VertexFilter
# =================================================================
makeFlow("A -> B B -> C")
flow.addSource(A)
flow.addSink(C)
flow.addConstraint(edgeFlow >= 1)
checkSolutions(flow, [[
	(inFlow, A, 1),
	(outFlow, C, 1),
	(edgeFlow, dg.findEdge([A], [B]), 1),
	(edgeFlow, dg.findEdge([B], [C]), 1),
]])

makeFlow("A -> B B -> C")
flow.addSource(A)
flow.addSink(C)
flow.addConstraint(edgeFlow >= 1)
flow.exclude(B)
checkSolutions(flow, [])

# ObjectiveFunction
# =================================================================

# TODO: hmm
