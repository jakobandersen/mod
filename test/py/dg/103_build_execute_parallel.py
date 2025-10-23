include("1xx_execute_helpers.py")

g1 = Graph.fromSMILES('O', "g1")
g2 = Graph.fromSMILES('C', "g2")

addSub1 = addSubset(g1)
addUni2 = addUniverse(g2)
fail(lambda: DGStrat.makeParallel([]), "Can not create parallel strategy without any substrategies.")
fail(lambda: DGStrat.makeParallel([None]), "One of the strategies is a null pointer.")
exeStrat([addSub1], [g1], [g1])
exeStrat([addSub1, addUni2], [g1], [g1, g2])
