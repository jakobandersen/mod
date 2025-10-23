include("1xx_execute_helpers.py")

g1 = Graph.fromSMILES('O', "g1")
g2 = Graph.fromSMILES('C', "g2")
g3 = Graph.fromSMILES('N', "g3")

fail(lambda: DGStrat.makeSequence([]), "Can not create an empty-length sequence strategy.")
fail(lambda: DGStrat.makeSequence([None]), "One of the strategies is a null pointer.")
fail(lambda: addSubset(g1) >> None, "Can not convert type '{}' to DGStrat.".format(type(None)), err=TypeError)
exeStrat(DGStrat.makeSequence([addSubset(g1), addUniverse(g2)]), [g1], [g1, g2])
exeStrat(addSubset(g1) >> addUniverse(g2), [g1], [g1, g2])
exeStrat(addSubset(g1) >> addUniverse(g2) >> addSubset(g3), [g1, g3], [g1, g2, g3])
exeStrat((addSubset(g1) >> addUniverse(g2)) >> addSubset(g3), [g1, g3], [g1, g2, g3])
exeStrat(addSubset(g1) >> (addUniverse(g2) >> addSubset(g3)), [g1, g3], [g1, g2, g3])
