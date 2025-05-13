dg = DG()
r = dg.build().addAbstract("""
	#1 A + 2 B -> X
	#2 B + 3 C -> Y + A
""")
A = r.getGraph("A")
B = r.getGraph("B")
C = r.getGraph("C")
X = r.getGraph("X")
Y = r.getGraph("Y")
e1 = r.getEdge("1")
e2 = r.getEdge("2")

flow = hyperflow.Model(dg)
flow.addSource(A)
flow.addSource(B)
flow.addSource(C)
flow.addSink(X)
flow.addSink(Y)
flow.addConstraint(inFlow <= 12)
flow.addConstraint(2*edgeFlow[e1] + edgeFlow[e2] <= 3)
flow.objectiveFunction = -2*outFlow[X] - 3*outFlow[Y]
flow.findSolutions(maxNumSolutions=9)
flow.solutions.print()
