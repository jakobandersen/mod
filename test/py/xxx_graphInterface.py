include("xxx_helpers.py")
post.disableCommands()

def checkGraph(g, *, string: str, vertexString: str, edgeString: str,
		graphNameInElements: str = None,
		vIdFull: bool = True):
	assert g == g
	assert not g != g
	assert not g < g

	Vertex = g.Vertex
	Edge = g.Edge
	VertexRange = g.VertexRange
	EdgeRange = g.EdgeRange
	IncidentEdgeRange = g.IncidentEdgeRange
	assert str(g) == string
	assert g.numVertices == len(list(g.vertices))
	assert g.numEdges == len(list(g.edges))
	assert type(g.vertices) == g.VertexRange
	assert type(g.edges) == g.EdgeRange

	vNull = Vertex()
	assert str(vNull) == vertexString + "(null)"
	assert type(vNull.__hash__()) == int
	assert not vNull
	assert vNull.isNull()
	fail(lambda: vNull.id, "Can not get ID on a null vertex.")
	fail(lambda: vNull.graph, "Can not get graph on a null vertex.")
	fail(lambda: vNull.degree, "Can not get degree on a null vertex.")
	fail(lambda: vNull.incidentEdges, "Can not get incident edges on a null vertex.")

	graphNameInElements = graphNameInElements if graphNameInElements else str(g)
	i = 0
	for v in g.vertices:
		assert type(v) == Vertex
		assert str(v) == "{}({}, {})".format(vertexString, graphNameInElements,
			v.id)
		assert not (vNull == v)
		assert vNull != v
		assert vNull < v
		assert v > vNull
		assert hash(v) == v.id
		assert v
		assert not v.isNull()
		assert type(v.id) == int
		if vIdFull:
			assert v.id == i
		assert v.graph == g
		assert v.degree == len(list(v.incidentEdges))
		assert type(v.incidentEdges) == g.IncidentEdgeRange
		for e in v.incidentEdges:
			assert type(e.source) == Vertex
			assert type(e.target) == Vertex
			assert e.source == v
			assert e.target != v
		i += 1

	for v1 in g.vertices:
		for v2 in g.vertices:
			assert (v1 == v2) == (v1.id == v2.id)
			assert (v1 < v2) == (v1.id < v2.id)
			assert (v1 > v2) == (v1.id > v2.id)

	eNull = Edge()
	assert str(eNull) == edgeString + "(null)"
	assert not eNull
	assert eNull.isNull()

	for e in g.edges:
		assert type(e) == Edge
		assert str(e) == "{}({}, {}, {})".format(edgeString,
			graphNameInElements, e.source.id, e.target.id)
		assert not (eNull == e)
		assert eNull != e
		assert eNull < e
		assert e > eNull
		assert e.graph == g
		assert type(e.source) == Vertex
		assert type(e.target) == Vertex

	for e1 in g.edges:
		for e2 in g.edges:
			assert (e1 == e2) == ((e1.source.id, e1.target.id) == (e2.source.id, e2.target.id))
			assert (e1 < e2) == ((e1.source.id, e1.target.id) < (e2.source.id, e2.target.id))
			assert (e1 > e2) == ((e1.source.id, e1.target.id) > (e2.source.id, e2.target.id))
		

def checkLabelledGraph(g, *, string: str, vertexString: str, edgeString: str,
		graphNameInElements: str = None,
		vIdFull: bool = True):
	checkGraph(g, string=string, vertexString=vertexString,
		edgeString=edgeString, graphNameInElements=graphNameInElements,
		vIdFull=vIdFull)

	vNull = g.Vertex()
	fail(lambda: vNull.stringLabel, "Can not get string label on a null vertex.")
	fail(lambda: vNull.atomId, "Can not get atom id on a null vertex.")
	fail(lambda: vNull.isotope, "Can not get isotope on a null vertex.")
	fail(lambda: vNull.charge, "Can not get charge on a null vertex.")
	fail(lambda: vNull.radical, "Can not get radical status on a null vertex.")
	fail(lambda: vNull.printStereo(), "Can not print stereo on a null vertex.")
	fail(lambda: vNull.printStereo(GraphPrinter()), "Can not print stereo on a null vertex.")

	for v in g.vertices:
		assert type(v.stringLabel) == str
		assert type(v.atomId) == AtomId
		assert type(v.isotope) == Isotope
		assert type(v.charge) == Charge
		assert type(v.radical) == bool
		v.printStereo()
		v.printStereo(GraphPrinter())

	eNull = g.Edge()
	fail(lambda: eNull.stringLabel, "Can not get string label on a null edge.")
	fail(lambda: eNull.bondType, "Can not get bond type on a null edge.")

	for e in g.edges:
		assert type(e.stringLabel) == str
		assert type(e.bondType) == BondType
