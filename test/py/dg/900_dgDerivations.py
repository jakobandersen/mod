include("xx0_helpers.py")

def check(rs):
	g1 = smiles('O', name="g1")
	g2 = smiles('C', name="g2")
	g3 = smiles('OO', name="g3")
	g4 = smiles('CC', name="g4")
	ders = []
	if len(rs) == 0:
		d = Derivation()
		d.left = [g1, g2]
		d.right = [g3, g4]
		ders.append(d)
	else:
		for r in rs:
			d = Derivation()
			d.left = [g1, g2]
			d.rule = r
			d.right = [g3, g4]
			ders.append(d)
	dg = checkDeprecated(lambda: dgDerivations(ders))
	assert dg.numVertices == 4
	assert sorted((v.graph for v in dg.vertices)) == sorted(([g1, g2, g3, g4]))
	assert dg.numEdges == 1
	e = next(iter(dg.edges))
	assert sorted(e.rules) == sorted(rs)
	assert sorted((v.graph for v in e.sources)) == sorted([g1, g2])
	assert sorted((v.graph for v in e.targets)) == sorted([g3, g4])

r1 = ruleGMLString('rule [ ruleID "r1" context [ node [ id 0 label "O" ] ] ]')
r2 = ruleGMLString('rule [ ruleID "r2" context [ node [ id 0 label "C" ] ] ]')
check([])
check([r1])
check([r1, r2])
