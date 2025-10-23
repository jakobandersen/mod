include("xx0_helpers.py")

def check(rs):
	dg = DG()
	with dg.build() as b:
		g1 = Graph.fromSMILES('O', name="g1")
		g2 = Graph.fromSMILES('C', name="g2")
		g3 = Graph.fromSMILES('OO', name="g3")
		g4 = Graph.fromSMILES('CC', name="g4")
		d = Derivations()
		d.left = [g1, g2]
		d.rules = rs
		d.right = [g3, g4]
		def asserts():
			assert e
			assert dg.numVertices == 4
			assert sorted((v.graph for v in dg.vertices)) == sorted(([g1, g2, g3, g4]))
			assert dg.numEdges == 1
			assert next(iter(dg.edges)) == e
			assert sorted(e.rules) == sorted(rs)
			assert sorted((v.graph for v in e.sources)) == sorted([g1, g2])
			assert sorted((v.graph for v in e.targets)) == sorted([g3, g4])
		e = b.addDerivation(d)
		asserts()
		e = b.addDerivation(d, IsomorphismPolicy.Check)
		asserts()
		e = b.addDerivation(d, IsomorphismPolicy.TrustMe)
		asserts()

r1 = Rule.fromGMLString('rule [ ruleID "r1" context [ node [ id 0 label "O" ] ] ]')
r2 = Rule.fromGMLString('rule [ ruleID "r2" context [ node [ id 0 label "C" ] ] ]')
check([])
check([r1])
check([r1, r2])

d = Derivations()
d.right = [Graph.fromSMILES('O')]
fail(lambda: DG().build().addDerivation(d), "Derivation has empty left side: %s" % d)
d = Derivations()
d.left = [Graph.fromSMILES('O')]
fail(lambda: DG().build().addDerivation(d), "Derivation has empty right side: %s" % d)

ga1 = Graph.fromSMILES('O', "ga1")
ga2 = Graph.fromSMILES('O', "ga2")
gb = Graph.fromSMILES('C', "gb")

d = Derivations()
d.left = [ga2]
d.right = [gb]
fail(lambda: DG(graphDatabase=[ga1]).build().addDerivation(d), "Isomorphic graphs. Candidate graph 'ga2' is isomorphic to 'ga1' in the graph database.")
d = Derivations()
d.left = [gb]
d.right = [ga2]
fail(lambda: DG(graphDatabase=[ga1]).build().addDerivation(d), "Isomorphic graphs. Candidate graph 'ga2' is isomorphic to 'ga1' in the graph database.")
d = Derivations()
d.left = [ga1, ga2]
d.right = [gb]
fail(lambda: DG().build().addDerivation(d), "Isomorphic graphs. Candidate graph 'ga2' is isomorphic to 'ga1' in the graph database.")
d = Derivations()
d.left = [ga1]
d.right = [ga2]
fail(lambda: DG().build().addDerivation(d), "Isomorphic graphs. Candidate graph 'ga2' is isomorphic to 'ga1' in the graph database.")
d = Derivations()
d.left = [gb]
d.right = [ga1, ga2]
fail(lambda: DG().build().addDerivation(d), "Isomorphic graphs. Candidate graph 'ga2' is isomorphic to 'ga1' in the graph database.")

d = Derivations()
d.left = [None]
d.right = [Graph.fromSMILES("O")]
fail(lambda: DG().build().addDerivation(d), "Derivation has a null pointer in the left side: " + str(d))
d.left = [Graph.fromSMILES("O")]
d.right = [None]
fail(lambda: DG().build().addDerivation(d), "Derivation has a null pointer in the right side: " + str(d))
d.left = [Graph.fromSMILES("O")]
d.right = [Graph.fromSMILES("O")]
d.rules = [None]
fail(lambda: DG().build().addDerivation(d), "Derivation has a null pointer in the rule list: " + str(d))


g = Graph.fromSMILES("O")
d = Derivation()
d.left = [g]
d.right = [g]
d.rule = Rule.fromGMLString("""rule [
	context [ node [ id 0 label "O" ] ]
]""", add=False)
dg = DG()
dg.build().addDerivation(d)
del d
assert dg.numEdges == 1
e = next(iter(dg.edges))
assert len(e.rules) == 1
r = next(iter(e.rules))
print(r.name)
