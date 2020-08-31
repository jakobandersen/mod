include("1xx_execute_helpers.py")

g1 = smiles('C', "g1")
g2 = smiles('N', "g2")
g3 = smiles('O', "g3")
g4 = smiles('P', "g4")
g5 = smiles('S', "g5")

exeStrat(addSubset(g1) >> DGStrat.makeFilter(False, False), [], [g1])
exeStrat(addSubset(g1) >> DGStrat.makeFilter(True, False), [], [])
exeStrat(addSubset(g1) >> DGStrat.makeFilter(False, True), [g1], [g1])
exeStrat(addSubset(g1) >> DGStrat.makeFilter(True, True), [g1], [g1])
exeStrat(addSubset(g1) >> filterSubset(False), [], [g1])
exeStrat(addSubset(g1) >> filterUniverse(False), [], [])
exeStrat(addSubset(g1) >> filterSubset(True), [g1], [g1])
exeStrat(addSubset(g1) >> filterUniverse(True), [g1], [g1])

rules = [g1, g2, g3, g4, g5]
rs = []
def f(g, gs, first):
	assert first == (g == g1)
	assert list(gs.subset) == rules
	assert list(gs.universe) == rules
	rs.append(g)
	return True
exeStrat(addSubset(rules) >> filterUniverse(f), rules, rules)
assert rs == rules
