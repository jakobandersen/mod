include("xx5_solutions.py")
include("101_overallAutocatalysis_var_spec.py")

# ForceExistence
# =================================================================
makeFlow("A -> B")
flow.overallAutocatalysis.enable()
checkSolutions(flow, [])

makeFlow("A -> B")
flow.overallAutocatalysis.enable()
flow.overallAutocatalysis.forceExistence = False
checkSolutions(flow, [[]])

# =================================================================
makeFlow("A <=> B   2 F + B <=> C   C <=> A + B")
flow.addSource(A)
flow.addSource(F)
flow.addSink(A)
flow.addConstraint(inFlow[F] == 2)
flow.overallAutocatalysis.enable()
checkSolutions(flow, [[
	(inFlow, A, 1),
	(inFlow, F, 2),
	(outFlow, A, 2),
	(isOverallAutocata, A, 1),
	(edgeFlow, dg.findEdge([A], [B]), 1),
	(edgeFlow, dg.findEdge([F, F, B], [C]), 1),
	(edgeFlow, dg.findEdge([C], [A, B]), 1),
	(edgeFlow, dg.findEdge([B], [A]), 1),
]], maxNumSolutions=2)

# StrictTransit
# =================================================================
makeFlow("A + 2 F <=> B   B <=> 2 A")
flow.addSource(A)
flow.addSource(F)
flow.addSink(A)
flow.addConstraint(inFlow[F] <= 4)
flow.overallAutocatalysis.enable()
flow.overallAutocatalysis.strictTransit = False
checkSolutions(flow, [[
	(inFlow, A, 1),
	(inFlow, F, 2),
	(outFlow, A, 2),
	(isOverallAutocata, A, 1),
	(edgeFlow, dg.findEdge([A, F, F], [B]), 1),
	(edgeFlow, dg.findEdge([B], [A, A]), 1),
], [
	(inFlow, A, 1),
	(inFlow, F, 4),
	(outFlow, A, 3),
	(isOverallAutocata, A, 1),
	(edgeFlow, dg.findEdge([A, F, F], [B]), 2),
	(edgeFlow, dg.findEdge([B], [A, A]), 2),
], [
	(inFlow, A, 2),
	(inFlow, F, 4),
	(outFlow, A, 4),
	(isOverallAutocata, A, 1),
	(edgeFlow, dg.findEdge([A, F, F], [B]), 2),
	(edgeFlow, dg.findEdge([B], [A, A]), 2),
]], maxNumSolutions=4)

makeFlow("A + 2 F <=> B   B <=> 2 A")
flow.addSource(A)
flow.addSource(F)
flow.addSink(A)
flow.addConstraint(inFlow[F] <= 4)
flow.overallAutocatalysis.enable()
assert flow.overallAutocatalysis.strictTransit
checkSolutions(flow, [[
	(inFlow, A, 1),
	(inFlow, F, 2),
	(outFlow, A, 2),
	(isOverallAutocata, A, 1),
	(edgeFlow, dg.findEdge([A, F, F], [B]), 1),
	(edgeFlow, dg.findEdge([B], [A, A]), 1),
], [
	(inFlow, A, 2),
	(inFlow, F, 4),
	(outFlow, A, 4),
	(isOverallAutocata, A, 1),
	(edgeFlow, dg.findEdge([A, F, F], [B]), 2),
	(edgeFlow, dg.findEdge([B], [A, A]), 2),
]], maxNumSolutions=3)

# BFSExclusive
# =================================================================
makeFlow("A <=> C  C <=> B")
flow.addSource(A)
flow.addSink(B)
flow.addSource(C)
flow.addSink(C)
flow.addConstraint(inFlow[A] == 2)
flow.addConstraint(outFlow[B] == 1)
flow.overallAutocatalysis.enable()
flow.overallAutocatalysis.bfsExclusive = False
checkSolutions(flow, [[
	(inFlow, A, 2),
	(inFlow, C, 1),
	(outFlow, C, 2),
	(outFlow, B, 1),
	(isOverallAutocata, C, 1),
	(edgeFlow, dg.findEdge([A], [C]), 2),
	(edgeFlow, dg.findEdge([C], [B]), 1),
]], maxNumSolutions=2)

makeFlow("A <=> C  C <=> B")
flow.addSource(A)
flow.addSink(B)
flow.addSource(C)
flow.addSink(C)
flow.addConstraint(inFlow[A] == 2)
flow.addConstraint(outFlow[B] == 1)
flow.overallAutocatalysis.enable()
assert flow.overallAutocatalysis.bfsExclusive
checkSolutions(flow, [])

# Interaction with allow*Reversal
# =================================================================
makeFlow("A <=> B   2 F + B <=> C   C <=> A + B")
flow.addSource(A)
flow.addSource(F)
flow.addSink(A)
flow.addConstraint(inFlow[F] == 2)
flow.overallAutocatalysis.enable()
flow.allowReversal = True
checkSolutions(flow, [[
	(inFlow, A, 1),
	(inFlow, F, 2),
	(outFlow, A, 2),
	(isOverallAutocata, A, 1),
	(edgeFlow, dg.findEdge([A], [B]), 1),
	(edgeFlow, dg.findEdge([F, F, B], [C]), 1),
	(edgeFlow, dg.findEdge([C], [A, B]), 1),
	(edgeFlow, dg.findEdge([B], [A]), 1),
], [
	(inFlow, A, 1),
	(inFlow, F, 2),
	(outFlow, A, 2),
	(isOverallAutocata, A, 1),
	(edgeFlow, dg.findEdge([F, F, B], [C]), 1),
	(edgeFlow, dg.findEdge([C], [A, B]), 2),
	(edgeFlow, dg.findEdge([A, B], [C]), 1),
]], maxNumSolutions=100, absGap=0)

makeFlow("A <=> B   2 F + B <=> C   C <=> A + B")
flow.addSource(A)
flow.addSource(F)
flow.addSink(A)
flow.addConstraint(inFlow[F] == 2)
flow.overallAutocatalysis.enable()
flow.allowIOReversal = True
checkSolutions(flow, [], maxNumSolutions=100, absGap=0)

makeFlow("A <=> B   2 F + B <=> C   C <=> A + B")
flow.addSource(A)
flow.addSource(F)
flow.addSink(A)
flow.addConstraint(inFlow[F] == 2)
flow.overallAutocatalysis.enable()
flow.allowReversal = True
flow.allowIOReversal = True
checkSolutions(flow, [], maxNumSolutions=100, absGap=0)
