include("xx0_helpers.py")

def check(dg, res, exp):
	for name in exp:
		g = res.getGraph(name)
		assert g, name
		assert g.numVertices == 1
		vName = next(iter(g.vertices)).stringLabel
		assert vName == name, (vName, name)
	for v in dg.vertices:
		g = v.graph
		if g.numVertices != 1:
			continue
		name = next(iter(g.vertices)).stringLabel
		gr = res.getGraph(name)
		if not gr:
			assert name in exp, (name, g.name)
			continue
		assert g == gr, (g.name, gr.name)

def checkEdge(e, expSrc, expTar):
	assert e
	nsSrc = "".join(sorted(v.graph.name for v in e.sources))
	nsTar = "".join(sorted(v.graph.name for v in e.targets))
	assert nsSrc == expSrc, (nsSrc, expSrc)
	assert nsTar == expTar, (nsTar, expTar)

dg = DG()
r = dg.build().addAbstract("""
	#1 a -> b
	#2 b <=> c
""")
check(dg, r, "abc")
checkEdge(r.getEdge("1"), "a", "b")
checkEdge(r.getEdge("2"), "b", "c")
checkEdge(r.getEdge("2").inverse, "c", "b")

dg = DG()
with dg.build() as b:
	r = b.addAbstract("""
		#1 a -> b
		#2 b <=> c
	""")
	check(dg, r, "abc")
	checkEdge(r.getEdge("1"), "a", "b")
	checkEdge(r.getEdge("2"), "b", "c")
	checkEdge(r.getEdge("2").inverse, "c", "b")

dg = DG()
with dg.build() as b:
	r1 = b.addAbstract("#1 a -> b")
	check(dg, r1, "ab")
	r2 = b.addAbstract("#1 a -> b")
	check(dg, r2, "ab")
	assert r1.getEdge("1") == r2.getEdge("1")
