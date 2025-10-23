include("1xx_execute_helpers.py")

g1 = Graph.fromSMILES("[C]", "g1")
g2 = Graph.fromSMILES("[N]", "g2")
g3 = Graph.fromSMILES("[O]", "g3")
r = Rule.fromGMLString("""rule [
	left  [ node [ id 0 label "C" ] ]
	right [ node [ id 0 label "N" ] ]
]""")

fail(lambda: DGStrat.makeRevive(None), "The substrategy is a null pointer.")
exeStrat(DGStrat.makeRevive(addSubset(g1)), [g1], [g1])
exeStrat(revive(addSubset(g1)), [g1], [g1])

exeStrat(addSubset(g1, g3) >>        r , [g2    ], [g1, g2, g3], graphDatabase=[g1, g2, g3])
exeStrat(addSubset(g1, g3) >> revive(r), [g2, g3], [g1, g2, g3], graphDatabase=[g1, g2, g3])
