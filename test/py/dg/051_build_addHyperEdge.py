include("xx0_helpers.py")

g1 = smiles('O', name="g1")
g2 = smiles('C', name="g2")
r1 = ruleGMLString('rule [ ruleID "r1" context [ node [ id 0 label "O" ] ] ]')
r2 = ruleGMLString('rule [ ruleID "r2" context [ node [ id 0 label "C" ] ] ]')


dg = DG()
with dg.build() as b:
	d = Derivations()
	d.left = [g1]
	d.rules = [r1, r2]
	d.right = [g2]
	e = b.addDerivation(d)
assert dg.numEdges == 1


dg2 = DG()
with dg2.build() as b:
	fail(lambda: b.addHyperEdge(DG.HyperEdge()), "The hyperedge is null.")

	e2 = b.addHyperEdge(e)

assert set(v.graph for v in e.sources) == set(v.graph for v in e2.sources)
assert set(v.graph for v in e.targets) == set(v.graph for v in e2.targets)
assert set(e.rules) == set(e2.rules)
