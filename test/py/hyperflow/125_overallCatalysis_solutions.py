include("xx5_solutions.py")
include("121_overallCatalysis_var_spec.py")

# ForceExistence
# =================================================================
makeFlow("A -> B")
flow.overallCatalysis.enable()
checkSolutions(flow, [])

makeFlow("A -> B")
flow.overallCatalysis.enable()
flow.overallCatalysis.forceExistence = False
checkSolutions(flow, [[]])

# =================================================================
makeFlow("A <=> B   2 F + B <=> C   C <=> A + D")
flow.addSource(A)
flow.addSource(F)
flow.addSink(A)
flow.addSink(D)
flow.addConstraint(inFlow[F] == 2)
flow.overallCatalysis.enable()
checkSolutions(flow, [[
	(inFlow, A, 1),
	(inFlow, F, 2),
	(outFlow, A, 1),
	(outFlow, D, 1),
	(isOverallCata, A, 1),
	(edgeFlow, dg.findEdge([A], [B]), 1),
	(edgeFlow, dg.findEdge([F, F, B], [C]), 1),
	(edgeFlow, dg.findEdge([C], [A, D]), 1),
]], maxNumSolutions=2)

# StrictTransit
# =================================================================
makeFlow("A + F <=> B   B <=> A + W")
flow.addSource(A)
flow.addSource(F)
flow.addSink(A)
flow.addSink(W)
flow.addConstraint(inFlow[F] <= 2)
flow.overallCatalysis.enable()
flow.overallCatalysis.strictTransit = False
checkSolutions(flow, [[
	(inFlow, A, 1),
	(inFlow, F, 1),
	(outFlow, W, 1),
	(outFlow, A, 1),
	(isOverallCata, A, 1),
	(edgeFlow, dg.findEdge([A, F], [B]), 1),
	(edgeFlow, dg.findEdge([B], [A, W]), 1),
], [
	(inFlow, A, 1),
	(inFlow, F, 2),
	(outFlow, W, 2),
	(outFlow, A, 1),
	(isOverallCata, A, 1),
	(edgeFlow, dg.findEdge([A, F], [B]), 2),
	(edgeFlow, dg.findEdge([B], [A, W]), 2),
], [
	(inFlow, A, 2),
	(inFlow, F, 2),
	(outFlow, W, 2),
	(outFlow, A, 2),
	(isOverallCata, A, 1),
	(edgeFlow, dg.findEdge([A, F], [B]), 2),
	(edgeFlow, dg.findEdge([B], [A, W]), 2),
]], maxNumSolutions=4)

makeFlow("A + F <=> B   B <=> A + W")
flow.addSource(A)
flow.addSource(F)
flow.addSink(A)
flow.addSink(W)
flow.addConstraint(inFlow[F] <= 2)
flow.overallCatalysis.enable()
assert flow.overallCatalysis.strictTransit
checkSolutions(flow, [[
	(inFlow, A, 1),
	(inFlow, F, 1),
	(outFlow, W, 1),
	(outFlow, A, 1),
	(isOverallCata, A, 1),
	(edgeFlow, dg.findEdge([A, F], [B]), 1),
	(edgeFlow, dg.findEdge([B], [A, W]), 1),
], [
	(inFlow, A, 2),
	(inFlow, F, 2),
	(outFlow, W, 2),
	(outFlow, A, 2),
	(isOverallCata, A, 1),
	(edgeFlow, dg.findEdge([A, F], [B]), 2),
	(edgeFlow, dg.findEdge([B], [A, W]), 2),
]], maxNumSolutions=3)

# Interaction with allow*Reversal
# =================================================================
makeFlow("A <=> B   2 F + B <=> C   C <=> A + D")
flow.addSource(A)
flow.addSource(F)
flow.addSink(A)
flow.addSink(D)
flow.addConstraint(inFlow[F] == 2)
flow.overallCatalysis.enable()
flow.allowReversal = True
checkSolutions(flow, [[
	(inFlow, A, 1),
	(inFlow, F, 2),
	(outFlow, A, 1),
	(outFlow, D, 1),
	(isOverallCata, A, 1),
	(edgeFlow, dg.findEdge([A], [B]), 1),
	(edgeFlow, dg.findEdge([F, F, B], [C]), 1),
	(edgeFlow, dg.findEdge([C], [A, D]), 1),
]], maxNumSolutions=100, absGap=0)

makeFlow("A <=> B   2 F + B <=> C   C <=> A + D")
flow.addSource(A)
flow.addSource(F)
flow.addSink(A)
flow.addSink(D)
flow.addConstraint(inFlow[F] == 2)
flow.overallCatalysis.enable()
flow.allowIOReversal = True
checkSolutions(flow, [], maxNumSolutions=100, absGap=0)

makeFlow("A <=> B   2 F + B <=> C   C <=> A + D")
flow.addSource(A)
flow.addSource(F)
flow.addSink(A)
flow.addSink(D)
flow.addConstraint(inFlow[F] == 2)
flow.overallCatalysis.enable()
flow.allowReversal = True
flow.allowIOReversal = True
checkSolutions(flow, [], maxNumSolutions=100, absGap=0)
