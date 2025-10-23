include("1xx_execute_helpers.py")

g1 = Graph.fromSMILES("[C]", "g1")
g2 = Graph.fromSMILES("[N]", "g2")
r = Rule.fromGMLString("""rule [
	left  [ node [ id 0 label "C" ] ]
	right [ node [ id 0 label "N" ] ]
]""")

fail(lambda: DGStrat.makeLeftPredicate (False, None), "The substrategy is a null pointer.")
fail(lambda: DGStrat.makeRightPredicate(False, None), "The substrategy is a null pointer.")

exeStrat(DGStrat.makeLeftPredicate (False, addSubset(g1)), [g1], [g1])
exeStrat(DGStrat.makeRightPredicate(False, addSubset(g1)), [g1], [g1])
exeStrat(leftPredicate [False](addSubset(g1)), [g1], [g1])
exeStrat(rightPredicate[False](addSubset(g1)), [g1], [g1])

def leftFalse(d):
	assert d.left == [g1]
	assert d.rule == r
	return False
leftTrue = lambda d: not leftFalse(d)
def rightFalse(d):
	assert d.right == [g2]
	return leftFalse(d)
rightTrue = lambda d: not rightFalse(d)
exeStrat(leftPredicate [leftFalse ](addSubset(g1) >> r), [],   [g1],     graphDatabase=[g1, g2])
exeStrat(leftPredicate [leftTrue  ](addSubset(g1) >> r), [g2], [g1, g2], graphDatabase=[g1, g2])
exeStrat(rightPredicate[rightFalse](addSubset(g1) >> r), [],   [g1],     graphDatabase=[g1, g2])
exeStrat(rightPredicate[rightTrue ](addSubset(g1) >> r), [g2], [g1, g2], graphDatabase=[g1, g2])
