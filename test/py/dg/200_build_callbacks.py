
doAssert = False
vs = []
es = []

def onNewVertex(v):
	print("Vertex:", v, v.graph)
	if doAssert:
		assert False
	vs.append(v)

def onNewHyperEdge(e):
	print("HyperEdge:", e)
	if doAssert:
		assert False
	es.append(e)

g = Graph.fromSMILES('[O]', "O")
g2 = Graph.fromSMILES('[P]', "P")
r = Rule.fromDFS("[O]1>>[P]1")

dg = DG(graphDatabase=inputGraphs)
with dg.build(onNewVertex=onNewVertex, onNewHyperEdge=onNewHyperEdge) as b:
	b.execute(addSubset(g) >> r)
	print("try again")
	doAssert = True
	b.execute(addSubset(g) >> r)
	print("done")
assert [v.graph for v in vs] == [g, g2], [v.graph for v in vs]
assert len(es) == 1
assert [v.graph for v in es[0].sources] == [g]
assert [v.graph for v in es[0].targets] == [g2]
