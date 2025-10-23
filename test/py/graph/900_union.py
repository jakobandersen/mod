include("../xxx_helpers.py")
include("../xxx_graphInterface.py")

fail(lambda: UnionGraph([None]), "A graph is null.")


O = Graph.fromSMILES("O")

def check(ug, gs):
	s = "UnionGraph{%s}" % ', '.join(a.name for a in gs)
	checkLabelledGraph(ug, string=s, vertexString="UnionGraphVertex",
		edgeString="UnionGraphEdge")

	assert str(ug) == s
	assert len(ug) == len(gs)
	assert list(ug) == gs
	for i in range(len(ug)):
		assert ug[i] == gs[i]
	assert ug == UnionGraph(gs)
	assert ug != UnionGraph(gs + [O])


	def compVertex(vCand, vReal):
		assert len(list(vCand.incidentEdges)) == vReal.degree
		assert vCand.degree == vReal.degree
		assert vCand.stringLabel == vReal.stringLabel
		assert vCand.atomId == vReal.atomId
		assert vCand.isotope == vReal.isotope
		assert vCand.charge == vReal.charge
		assert vCand.radical == vReal.radical

		assert vCand.vertex == vReal
		assert vCand.graph[vCand.graphIndex] == vReal.graph

	def compEdge(eCand, eReal):
		compVertex(eCand.source, eReal.source)
		compVertex(eCand.target, eReal.target)
		assert eCand.stringLabel == eReal.stringLabel
		assert eCand.bondType == eReal.bondType

		assert eCand.edge == eReal

	# vertices
	n = sum(a.numVertices for a in ug)
	assert ug.numVertices == n
	vs = []
	for g in gs:
		vs.extend(g.vertices)
	assert len(vs) == n
	vsIdx = list(ug.vertices[i] for i in range(ug.numVertices))
	assert list(ug.vertices) == vsIdx
	for i in range(n):
		assert ug.vertices[i].id == i
	assert len(list(ug.vertices)) == n
	i = 0
	for vCand, vReal in zip(ug.vertices, vs):
		assert vCand.id == i
		assert vCand
		assert not vCand.isNull()
		assert vCand.graph == ug
		i += 1
		compVertex(vCand, vReal)
		for eOutCand, eOutReal in zip(vCand.incidentEdges, vReal.incidentEdges):
			compEdge(eOutCand, eOutReal)

	# edges
	m = sum(a.numEdges for a in ug)
	assert ug.numEdges == m
	es = []
	for g in gs:
		es.extend(g.edges)
	assert len(es) == m
	assert len(list(ug.edges)) == m
	for eCand, eReal in zip(ug.edges, es):
		assert eCand
		assert not eCand.isNull()
		compEdge(eCand, eReal)


check(UnionGraph(), [])
check(UnionGraph([O]), [O])
check(UnionGraph([O, O]), [O, O])
