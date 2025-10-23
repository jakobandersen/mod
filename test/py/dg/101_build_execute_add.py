include("1xx_execute_helpers.py")

g1a = Graph.fromSMILES('O', "g1a")
g1b = Graph.fromSMILES('O', "g1b")
g2a = Graph.fromSMILES('C', "g2a")

# static
# --------------------------------------------------------
exeStrat(DGStrat.makeAddStatic(True, [], IsomorphismPolicy.Check), [], [])
exeStrat(DGStrat.makeAddStatic(False, [], IsomorphismPolicy.Check), [], [])
fail(lambda: addSubset([None]), "One of the graphs is a null pointer.")
fail(lambda: addUniverse([None]), "One of the graphs is a null pointer.")
exeStrat(addSubset([]), [], [])
exeStrat(addUniverse([]), [], [])
exeStrat(addSubset([g1a]), [g1a], [g1a])
exeStrat(addUniverse([g1a]), [], [g1a])
exeStrat(addSubset(g1a), [g1a], [g1a])
exeStrat(addUniverse(g1a), [], [g1a])
exeStrat(addSubset([g1a], [g2a]), [g1a, g2a], [g1a, g2a])
exeStrat(addUniverse([g1a], [g2a]), [], [g1a, g2a])
exeStrat(addSubset(g1a, [g2a]), [g1a, g2a], [g1a, g2a])
exeStrat(addUniverse(g1a, [g2a]), [], [g1a, g2a])
exeStrat(addSubset([g1a], g2a), [g1a, g2a], [g1a, g2a])
exeStrat(addUniverse([g1a], g2a), [], [g1a, g2a])
exeStrat(addSubset(g1a, g2a), [g1a, g2a], [g1a, g2a])
exeStrat(addUniverse(g1a, g2a), [], [g1a, g2a])

# isomorphic
fail(lambda: exeStrat(addSubset(g1a, g1b)),
	"Isomorphic graphs. Candidate graph 'g1b' is isomorphic to 'g1a' in the graph database.")
fail(lambda: exeStrat(addUniverse(g1a, g1b)),
	"Isomorphic graphs. Candidate graph 'g1b' is isomorphic to 'g1a' in the graph database.")
exeStrat(addSubset(g1a, g1b, graphPolicy=IsomorphismPolicy.TrustMe))
exeStrat(addUniverse(g1a, g1b, graphPolicy=IsomorphismPolicy.TrustMe))


# dynamic
# --------------------------------------------------------
exeStrat(DGStrat.makeAddDynamic(True, lambda: [], IsomorphismPolicy.Check), [], [])
exeStrat(DGStrat.makeAddDynamic(False, lambda: [], IsomorphismPolicy.Check), [], [])
fail(lambda: exeStrat(addSubset(lambda: [None])), "A graph returned in a dynamic add strategy is a null pointer.")
fail(lambda: exeStrat(addUniverse(lambda: [None])), "A graph returned in a dynamic add strategy is a null pointer.")
exeStrat(addSubset(lambda: []), [], [])
exeStrat(addUniverse(lambda: []), [], [])
exeStrat(addSubset(lambda: [g1a]), [g1a], [g1a])
exeStrat(addUniverse(lambda: [g1a]), [], [g1a])
fail(lambda: exeStrat(addSubset(lambda: [g1a], 42)),
	"The dynamic version of addSubset/addUniverse takes exactly 1 argument (2 given).", err=TypeError)
fail(lambda: exeStrat(addUniverse(lambda: [g1a], 42)),
	"The dynamic version of addSubset/addUniverse takes exactly 1 argument (2 given).", err=TypeError)

# isomorphic
fail(lambda: exeStrat(addSubset(lambda: [g1a, g1b])),
	"Isomorphic graphs. Candidate graph 'g1b' is isomorphic to 'g1a' in the graph database.")
fail(lambda: exeStrat(addUniverse(lambda: [g1a, g1b])),
	"Isomorphic graphs. Candidate graph 'g1b' is isomorphic to 'g1a' in the graph database.")
exeStrat(addSubset(lambda: [g1a, g1b], graphPolicy=IsomorphismPolicy.TrustMe))
exeStrat(addUniverse(lambda: [g1a, g1b], graphPolicy=IsomorphismPolicy.TrustMe))


# test verbose print
inputGraphs[:] = []
for i in range(42):
	Graph.fromSMILES("C"*(i + 1))
exeStrat(addSubset(inputGraphs) >> addUniverse(inputGraphs) >> addSubset(lambda: inputGraphs) >> addUniverse(lambda: inputGraphs), verbosity=10)
