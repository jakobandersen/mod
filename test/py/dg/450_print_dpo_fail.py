include("../xxx_helpers.py")

e = DG.HyperEdge()
fail(lambda: e.print(), "Can not print a null hyperedge.")

dg = DG()
dg.build().addAbstract("A -> B")
e = next(iter(dg.edges))
fail(lambda: e.print(), "The hyperedge has no rules.")

r = ruleGMLString("""rule [
	left [ node [ id 1 label "B" ] ]
	context [
		node [ id 0 label "A" ]
		edge [ source 0 target 1 label "-" ]
	]
	right [ node [ id 1 label "Q" ] ]
]""")
g = graphDFS("[A][B][C]")
gFail = smiles("O")

dg = DG() 
with dg.build() as b:
	es = b.apply([g], r)
	assert len(es) == 1
	e = es[0]

	d = Derivation()
	d.left = [gFail]
	d.right = [gFail]
	d.rule = r
	eFail = b.addDerivation(d)
	
fail(lambda: eFail.print(), "No derivation exists for rule {}.".format(r.name))

def printOk(withGraphvizCoords: bool):
	post.enableInvokeMake()
	p = GraphPrinter()
	p.withGraphvizCoords = withGraphvizCoords
	e.print(p)
	e.print(p, nomatchColour="yellow")
	e.print(p, matchColour="red")
	e.print(p, nomatchColour="yellow", matchColour="red")
	p.withIndex = True
	e.print(p)
