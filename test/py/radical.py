def test(a):
	a.print()
	p = GraphPrinter()
	p.setMolDefault()
	p.withIndex = True
	a.print(p)
	print(a.smiles)
	b = Graph.fromSMILES(a.smiles)
	assert a.isomorphism(b) > 0
test(Graph.fromDFS("[C.]"))
test(Graph.fromDFS("[C.]" + "(C)"))
test(Graph.fromDFS("[C.]" + "(C)"*2))
test(Graph.fromDFS("[C.]" + "(C)"*3))
test(Graph.fromDFS("[C.]" + "(C)"*4))
test(Graph.fromDFS("[C.]" + "(C)"*5))
test(Graph.fromDFS("[C.]" + "(C)"*6))
test(Graph.fromDFS("[C.]" + "(C)"*7))
test(Graph.fromSMILES("[CH2+.:42]"))
test(Graph.fromSMILES("[CH+.:42]"))
